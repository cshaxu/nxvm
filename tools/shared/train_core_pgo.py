"""Train the two Core Release objects from a configured Windows Ninja tree."""

import argparse
from ctypes import POINTER, WinDLL, c_int, c_void_p, c_wchar_p, cast
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory


ROOT = Path(__file__).resolve().parents[2]
BUILD = ROOT / "build"
OBJECTS = (
    "src/x86-core/CMakeFiles/x86-core-cpu.dir/cpu/cpu_instructions.c.obj",
    "src/x86-core/CMakeFiles/x86-core-machine.dir/machine/memory.c.obj",
)


def run(arguments, directory=ROOT, timeout=180):
    result = subprocess.run(
        arguments, cwd=directory, timeout=timeout, text=True, stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        creationflags=subprocess.CREATE_NO_WINDOW,
    )
    if result.returncode:
        raise RuntimeError(f"{Path(arguments[0]).name} failed: {result.stdout.strip()}")
    return result.stdout


def windows_arguments(command):
    shell = WinDLL("shell32", use_last_error=True)
    shell.CommandLineToArgvW.argtypes = (c_wchar_p, POINTER(c_int))
    shell.CommandLineToArgvW.restype = POINTER(c_wchar_p)
    kernel = WinDLL("kernel32", use_last_error=True)
    kernel.LocalFree.argtypes = (c_void_p,)
    kernel.LocalFree.restype = c_void_p
    count = c_int()
    values = shell.CommandLineToArgvW(command, count)
    if not values:
        raise OSError("Could not parse the Ninja compiler command")
    try:
        return [values[index] for index in range(count.value)]
    finally:
        kernel.LocalFree(cast(values, c_void_p))


def train(tree, destination):
    cache = {}
    for line in (tree / "CMakeCache.txt").read_text(encoding="utf-8").splitlines():
        if line and not line.startswith(("#", "//")) and "=" in line:
            key, value = line.split("=", 1)
            cache[key.split(":", 1)[0]] = value
    if cache["CMAKE_GENERATOR"] != "Ninja" or cache["CMAKE_BUILD_TYPE"] != "Release":
        raise ValueError("Configure a Windows GCC Ninja Release tree before training")
    if cache.get("REPOSITORY_CORE_PGO_DIRECTORY"):
        raise ValueError("Configure this tree without profile counters before training")
    if destination.exists():
        raise ValueError(f"Choose a new destination for training: {destination}")

    compiler = Path(cache["CMAKE_C_COMPILER"]).resolve(strict=True)
    ninja = cache["CMAKE_MAKE_PROGRAM"]
    run(["cmake", "--build", tree, "--target", "x86-core-machine", "--parallel", "8"], timeout=1200)
    with TemporaryDirectory(prefix="core-pgo-", dir=BUILD) as temporary:
        work = Path(temporary)
        counters = work / "counters"
        generate = f"-fprofile-generate={counters.as_posix()}"
        prefix = f"-fprofile-prefix-path={work}"
        for target in OBJECTS:
            command = run([ninja, "-C", tree, "-t", "commands", target]).splitlines()[-1]
            arguments = windows_arguments(command)
            index = next(i for i, item in enumerate(arguments)
                         if Path(item).name.lower() == compiler.name.lower())
            compilation = arguments[index:]
            if (Path(compilation[0]).resolve() != compiler or
                    compilation[compilation.index("-o") + 1] != target or
                    "-fno-fat-lto-objects" not in compilation):
                raise ValueError(f"Unexpected GCC Release IPO command for {target}")
            (work / target).parent.mkdir(parents=True, exist_ok=True)
            run([*compilation, generate, prefix], directory=work)

        harness = work / "workload.obj"
        run([compiler, "-O3", "-std=c11", "-flto=auto", "-Wall", "-Wextra", "-Wpedantic",
             "-Werror", "-DNDEBUG", "-I" + str(ROOT / "src"),
             "-c", ROOT / "tools/shared/core_pgo_workload.c", "-o", harness])
        libraries = [tree / f"src/x86-core/libx86-core-{part}.a"
                     for part in ("machine", "cpu", "fpu")]
        executable = work / "core-pgo.exe"
        run([compiler, "-O3", "-flto=auto", generate, harness,
             *(work / target for target in OBJECTS), *libraries, "-o", executable])
        run([executable], directory=work, timeout=90)
        expected = {name.removesuffix(".obj").replace("/", "#") + ".gcda" for name in OBJECTS}
        if {path.name for path in counters.glob("*.gcda") if path.stat().st_size} != expected:
            raise ValueError("Training did not write exactly the two expected Core profile counters")
        destination.parent.mkdir(parents=True, exist_ok=True)
        counters.rename(destination)
    print(destination)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("tree", type=Path, help="Existing Release Ninja tree below build/.ninja")
    parser.add_argument("destination", type=Path, help="New counter directory inside the tree")
    arguments = parser.parse_args()
    tree = arguments.tree.resolve(strict=True)
    destination = arguments.destination.resolve()
    if ((BUILD / ".ninja") not in tree.parents or tree not in destination.parents or
            BUILD.resolve() != BUILD):
        parser.error("Select a repository Release tree and a new counter directory beneath it")
    train(tree, destination)


if __name__ == "__main__":
    main()
