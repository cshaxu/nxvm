"""Reproduce the frozen S2 ownership audit; not a build or test selector.

Run from the repository root with Python 3 and Universal Ctags 6.2.1:
  python docs/nxvm/etc/evidence/t561-s2/inventory.py --check
Without --check, regenerate the adjacent TSV evidence from unchanged inputs.
No production, test, build graph or artifact is written.
"""

import argparse
import csv
import hashlib
import io
import json
from pathlib import Path
import re
import subprocess


BASELINE = "8db742b6d"
OUTPUT = Path(__file__).resolve().parent
ROOT = OUTPUT.parents[4]


def run(*arguments):
    return subprocess.check_output(arguments, cwd=ROOT, text=True, encoding="utf-8")


def tracked(*roots):
    return sorted(run("git", "ls-files", "--", *roots).splitlines())


def read(path):
    return (ROOT / path).read_text(encoding="utf-8-sig")


def destination(path):
    tree, _, relative = path.split("/", 2)
    prefix = tree + "/"
    if relative.startswith("chips/cpu/"):
        return prefix + "x86-core/cpu/" + relative[len("chips/cpu/"):], "C1"
    if relative.startswith("chips/fpu/"):
        return prefix + "x86-core/fpu/" + relative[len("chips/fpu/"):], "C2"
    if relative.startswith("x86/"):
        if path.endswith(("machine_80286_timing_manifest_runner.c",
                          "machine_80386_timing_manifest_runner.c")):
            return prefix + "x86-nxvm/board-base/qualification/" + relative[4:], "T2"
        return prefix + "x86-core/machine/" + relative[4:], "N1"
    if relative.startswith("support/"):
        if Path(relative).name.startswith("video_topology"):
            return prefix + "x86-nxvm/board-base/" + relative, "T3"
        return prefix + "x86-core/machine/" + relative, "T1"
    if "/" in relative:
        return prefix + "x86-nxvm/" + relative, "P1"
    if relative in ("CMakeLists.txt", "MANIFEST.sha256", "verify_corpus.cmake",
                    "verify_manifest.cmake", "verify_negative.cmake"):
        return prefix + "{x86-core,x86-nxvm}/" + relative, "B1"
    if relative == "x86_tests.cmake":
        return "test/{x86-core,x86-nxvm}/CMakeLists.txt", "B2"
    if relative == "board_tests.cmake":
        return "test/x86-nxvm/CMakeLists.txt", "B2"
    if relative.startswith(("verify_address_span", "verify_decode_admission",
                            "verify_host_arithmetic", "verify_stack_frame")):
        return "test/x86-core/" + relative, "G1"
    if relative.startswith(("cpu_bus_boundary", "fdc_boundary", "dependency_dag",
                            "verify_app_test_support", "verify_test_route_partition")):
        return "test/x86-nxvm/" + relative, "G2"
    raise ValueError("Unassigned path: " + path)


def symbol_rule(path, name, scope):
    if name.startswith("CORE_MACHINE_TIMING_") or name.startswith((
            "core_machine_timing_", "core_machine_get_timing_",
            "core_machine_validate_timing_", "core_machine_install_timing_")):
        return "P2: board-base/timing qualification; retain original IDs/grades"
    if name in ("timing_declarations", "timing_declarations_copied"):
        return "P2: board-base owned table/publication"
    if name.startswith("CORE_MACHINE_TRACE_") and any(
            word in name for word in ("DMA_ADVANCE", "PIT_ADVANCE", "PIC_REFRESH",
                                      "RTC_ADVANCE", "KBC_ADVANCE", "VADP_ADVANCE",
                                      "FDC_ADVANCE", "HDC_ADVANCE")):
        return "P3: board-base trace tags; keep numeric event identity"
    if path.startswith("src/core/x86/") and (name in (
            "pit_pic", "pit_ticks", "rtc", "media", "dma_ticks", "dma_advance",
            "dma_request", "refresh_request", "refresh_complete") or
            name.startswith("core_machine_attachment_pit") or
            name.startswith(("core_machine_arbitration_advance",
                             "core_machine_dma_grant_advance",
                             "core_machine_readiness_advance"))):
        return "P4: split neutral bus/time mechanics from board progression"
    if path.startswith("src/core/x86/") and name in (
            "core_machine_resolve_cpu_profile", "core_machine_resolve_memory_bytes",
            "CORE_MACHINE_DEFAULT_MEMORY_BYTES", "CORE_MACHINE_MINIMUM_MEMORY_BYTES",
            "CORE_MACHINE_MAXIMUM_MEMORY_BYTES"):
        return "P5: explicit board defaults/admission; preserve backing limits separately"
    if path.startswith("src/core/x86/") and name in (
            "pic_pending", "pic_acknowledge"):
        return "P6: neutral interrupt request/ack; PIC wiring remains board"
    if path.startswith("src/core/x86/") and name.startswith((
            "core_machine_a20_", "CORE_MACHINE_A20_")):
        return "P7: board address policy; retain atomic route/observation mechanism"
    return destination(path)[1] + ": destination owns declaration; namespace table in review.md"


def commands(text):
    """Keep complete CMake statements, including conditionals/loop expressions.

    This is a lexical index, not a replacement CMake interpreter. Bracket and
    quoted arguments are skipped when counting parentheses.
    """
    index = 0
    while index < len(text):
        match = re.search(r"(?m)^\s*([A-Za-z_][A-Za-z_0-9]*)\s*\(", text[index:])
        if not match:
            return
        start = index + match.start()
        position = index + match.end()
        depth = 1
        quote = False
        while position < len(text) and depth:
            char = text[position]
            if char == "\\":
                position += 2
                continue
            if char == '"':
                quote = not quote
            if not quote:
                bracket = re.match(r"\[(=*)\[", text[position:])
                if bracket:
                    ending = "]" + bracket.group(1) + "]"
                    close = text.find(ending, position + len(bracket.group()))
                    if close == -1:
                        raise ValueError("Unclosed CMake bracket")
                    position = close + len(ending)
                    continue
                if char == "#":
                    newline = text.find("\n", position)
                    position = len(text) if newline == -1 else newline + 1
                    continue
                if char == "(":
                    depth += 1
                if char == ")":
                    depth -= 1
            position += 1
        if depth:
            raise ValueError("Unclosed CMake command")
        yield text[:start].count("\n") + 1, match.group(1).lower(), text[start:position]
        index = position


def table(columns, rows):
    output = io.StringIO(newline="")
    writer = csv.writer(output, delimiter="\t", lineterminator="\n")
    writer.writerow(columns)
    writer.writerows(rows)
    return output.getvalue()


def configured_inventory():
    targets = []
    tests = []
    dependencies = []
    for width in ("x64", "x86"):
        graph = "build/mingw-gcc-" + width + "-release-ccache"
        ninja = read(graph + "/build.ninja")
        objects = {}
        order_dependencies = {}
        for line in ninja.splitlines():
            order = re.match(r"build cmake_object_order_depends_target_([^:]+): phony(.*)", line)
            if order:
                order_dependencies[order[1]] = sorted(set(re.findall(
                    r"cmake_object_order_depends_target_([^\s]+)", order[2])))
            match = re.match(r"build .*?CMakeFiles/([^/]+)\.dir/.*?: C_COMPILER\S* (\S+)", line)
            if not match:
                continue
            source = match[2].replace("$:", ":").replace("$ ", " ")
            root = ROOT.as_posix() + "/"
            if source.lower().startswith(root.lower()):
                source = source[len(root):]
            if source.startswith(("src/core/", "test/core/", "src/app-", "test/app-")):
                objects.setdefault(match[1], set()).add(source)
        for target, sources in sorted(objects.items()):
            core_sources = sorted(source for source in sources if source.startswith(("src/core/", "test/core/")))
            if not core_sources:
                continue
            linked = order_dependencies.get(target, [])
            app_links = [name for name in linked if name.startswith((
                "my5160-", "my5170-", "mydeskpro386-", "nxvm-machine"))]
            dependencies.append((width, target, ";".join(linked), ";".join(app_links),
                                 "review outer App link" if app_links else
                                 "retain inward mechanism at new owner"))
            owners = sorted({destination(source)[0].split("/")[1] for source in core_sources})
            disposition = ";".join(owners)
            if any(source.startswith(("src/app-", "test/app-")) for source in sources):
                disposition = "retain App target; inward receiver edits only"
            if target == "core-chip-cpu":
                renamed = "x86-core-cpu"
            elif target == "core-chip-fpu":
                renamed = "x86-core-fpu"
            elif target.startswith("core-chip-"):
                renamed = target.replace("core-chip-", "x86-nxvm-chip-", 1)
            elif target in ("core-x86", "core-x86-observable"):
                renamed = target.replace("core-x86", "x86-core-machine", 1)
            elif target in ("core-machine", "core-machine-media", "core-machine-conversion", "core-product") or target.startswith("core-board-"):
                renamed = target.replace("core-", "x86-nxvm-", 1)
            else:
                renamed = "retain behavior identity; owner prefix follows file destination"
            targets.append((width, target, ";".join(sorted(sources)), disposition, renamed))
        pending = [ROOT / graph / "CTestTestfile.cmake"]
        visited = set()
        registered = {}
        labels = {}
        while pending:
            path = pending.pop().resolve()
            if path in visited:
                continue
            visited.add(path)
            for line in path.read_text(encoding="utf-8").splitlines():
                child = re.match(r'subdirs\("([^"]+)"\)', line)
                if child:
                    pending.append(path.parent / child[1] / "CTestTestfile.cmake")
                added = re.match(r'add_test\(\[=\[([^]]+)\]=\]', line)
                if added:
                    if added[1] in registered:
                        raise ValueError("Duplicate configured test: " + added[1])
                    registered[added[1]] = True
                property_line = re.match(r'set_tests_properties\(\[=\[([^]]+)\]=\].*? LABELS "([^"]*)"', line)
                if property_line:
                    labels[property_line[1]] = property_line[2]
        for name in sorted(registered):
            owner_labels = labels.get(name, "")
            tests.append((width, name, owner_labels,
                          "partition by source/assertion owner" if "core" in owner_labels.split(";") else
                          "retain existing owner; update inward references if required"))
    return {
        "configured-targets.tsv": table(("width", "target", "sources", "disposition", "destination_target"), targets),
        "configured-dependencies.tsv": table(("width", "target", "ninja_order_dependencies", "app_dependencies", "disposition"), dependencies),
        "configured-tests.tsv": table(("width", "test", "labels", "disposition"), tests),
    }


def inventory():
    paths = tracked("src/core", "test/core")
    if run("git", "diff", BASELINE, "--", "src/core", "test/core").strip():
        raise ValueError("Core differs from frozen S2 baseline")
    files = []
    includes = []
    for path in paths:
        content = read(path)
        target, decision = destination(path)
        files.append((path, hashlib.sha256((ROOT / path).read_bytes()).hexdigest(),
                      len(content.splitlines()), target, decision))
        for line, value in enumerate(content.splitlines(), 1):
            match = re.match(r'\s*#\s*include\s*"([^"]+)"', value)
            if match:
                includes.append((path, line, match[1], target,
                                 "resolve at receiving owner; no cross-package private fixture"))
    tags = run("ctags", "--output-format=json", "--fields=+nKsf",
               "--kinds-C=+px", "--if0=yes", "--sort=no", "-f", "-",
               *[path for path in paths if Path(path).suffix in (".c", ".h")])
    symbols = []
    for value in tags.splitlines():
        tag = json.loads(value)
        if tag.get("_type") != "tag" or tag.get("kind") == "header":
            continue
        path = tag["path"].replace("\\", "/")
        scope = tag.get("scope", "")
        symbols.append((path, tag["line"], tag["kind"], scope, tag["name"],
                        "public-header" if path.endswith("_interface.h") else "internal/test",
                        symbol_rule(path, tag["name"], scope)))
    symbols.sort(key=lambda item: (item[0], item[1], item[2], item[4]))
    registrations = []
    consumers = []
    for path in tracked("src", "test", "cmake", "tools", "CMakeLists.txt", "CMakePresets.json"):
        if Path(path).suffix not in (".c", ".h", ".cmake", ".ps1", ".json") and not path.endswith("CMakeLists.txt"):
            continue
        content = read(path)
        if path.endswith((".cmake", "CMakeLists.txt")):
            for line, command, body in commands(content):
                if "core/" in body or "core-" in body or path.startswith(("src/core/", "test/core/")):
                    registrations.append((path, line, command, " ".join(body.split()),
                                          "B2: partition by actual source/assertion owner; retain conditions"))
        if not path.startswith(("src/core/", "test/core/")):
            for line, value in enumerate(content.splitlines(), 1):
                if re.search(r"(?:src/|test/)?core/|\bcore-(?:chip|x86|board|machine|product)", value):
                    consumers.append((path, line, value.strip(),
                                      "receiver-only update; retain original owner and assertions"))
    result = {
        "files.tsv": table(("path", "sha256", "lines", "destination", "decision"), files),
        "symbols.tsv": table(("path", "line", "kind", "scope", "symbol", "visibility", "disposition"), symbols),
        "includes.tsv": table(("path", "line", "include", "destination", "disposition"), includes),
        "registrations.tsv": table(("path", "line", "command", "statement", "disposition"), registrations),
        "receivers.tsv": table(("path", "line", "reference", "disposition"), consumers),
    }
    if len(files) != len(set(row[0] for row in files)):
        raise ValueError("Duplicate inventory path")
    result.update(configured_inventory())
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true")
    arguments = parser.parse_args()
    for name, content in inventory().items():
        path = OUTPUT / name
        if arguments.check:
            if not path.exists() or path.read_bytes() != content.encode("utf-8"):
                raise ValueError("Evidence differs: " + name)
        else:
            path.write_bytes(content.encode("utf-8"))
        print(name + ": " + str(len(content.splitlines()) - 1) + " rows")
    print("T561:S2:INVENTORY:OK")


if __name__ == "__main__":
    main()
