# Roadmap

Each milestone requires a runnable demonstration, focused automated tests,
recorded evidence, and preservation of established baselines.

## M1: Validate The NXVM Machine Foundation

**Goal:** establish a GCC-runnable, behavior-recorded full NXVM machine
baseline before reducing it into ntvdm64's final module boundaries.

**Scope:** import and build the complete current NXVM machine baseline: CPU,
memory, IVT, BIOS/POST and current boot path, PIC/PIT, connected devices,
keyboard, existing text display, execution loop, and machine-level debugger
primitives. Run it against the owner-provided local disk-image fixtures and
record the observed boot or stop state. Only after this baseline works, begin
subtractive isolation of units into the final `machine` and `platform`
boundaries. Preserve the existing Linux platform source as part of the baseline,
but use the Windows GCC run as M1 acceptance. These debugger primitives are not
the M3 `ntvdm64 run --debug` product experience.

**Non-goals:** a project-owned DOS backend, host-drive mapping, a product CLI,
new graphics or device features, or a claim about the contents or license of a
local disk image. The existing 80386 CPU is retained; M1 does not replace it
with a new 8086 implementation. A Linux release or Linux compatibility claim
is also outside M1.

**Demo and exit:** GCC builds the imported full baseline and runs a repeatable
text-mode boot/execution scenario using the recorded local fixtures. Evidence
captures source and fixture identities, configured device path, terminal or
window observation, and the reached boot or stop state. The resulting baseline
then supplies focused tests for later reduction. Risk: inherited global state,
device coupling, and legacy Win32 assumptions. Dependency: a
provenance-reviewed NXVM import.

## M2: Add The Owned DOS Backend

**Goal:** run a simple COM program using DOS interrupts.

**Scope:** the bounded COM loader, PSP, environment, `INT 20h`, the approved
`INT 21h` subset, deterministic text/keyboard I/O, exit, an in-memory fixture
filesystem, fixed loader memory, and defined DOS errors. The exact M2 profile
is [M2 DOS Backend Requirements](requirements/m2-dos-backend.md).

**Non-goals:** complete DOS API, dynamic MCB memory, MZ/EXEC, directories,
wildcards, FCB, device names, LFN, XMS/EMS/DPMI, Microsoft guest components,
host-drive mapping, and shell integration.

**Demo and exit:** the approved COM probes print and exit, perform fixture-file
open/read/seek/close, and verify defined failure results. Generated probes cover
each approved service. Risk: machine/DOS ABI leakage and unbounded API growth.
Dependency: M1 machine contract.

## M3: Non-Invasive Windows Platform Integration

**Goal:** make the default backend usable through `ntvdm64 run` on a clean
64-bit Windows system.

**Scope:** CLI, arguments, current directory, environment, host filesystem,
keyboard/mouse, text and basic graphics paths, exit code, logging, debug mode,
configuration, Ctrl+C, and adapted NXVM input/display components. Implement the
display and host-drive visibility contract in
[Runtime CLI Requirements](requirements/runtime-cli.md).

**Non-goals:** global file association, loader replacement, injection, drivers,
or registry-dependent operation.

**Demo and exit:** `ntvdm64 run hello.com` and an argument-bearing DOS program
work without system changes. Tests cover paths, arguments, host I/O, and exit
status. Risk: platform adapters becoming DOS internals. Dependency: M2.

## M4: Real DOS Program Compatibility

**Goal:** grow from probes to representative DOS utilities and real programs.

**Scope:** add MZ loading, EXEC, MCB, FCB, wildcard and 8.3 semantics,
attributes, date/time, device names, selected XMS/EMS, graphics, and mouse only
when a corpus requirement justifies it.

**Non-goals:** theoretical API completeness or speculative Microsoft ABI.

**Demo and exit:** each corpus program has a reproducible result. Risk:
unbounded compatibility scope. Dependency: M3.

## M5: Invasive Windows Integration Research

**Goal:** evaluate optional system integration outside the CLI path.

**Scope:** documents, risk analysis, reversible prototypes, installer and
uninstaller design, and Windows-version compatibility evaluation.

**Non-goals:** default build inclusion, automatic system changes, release
dependency, or security-policy changes.

**Demo and exit:** reviewed design and rollback prototype only. Tests validate
reversibility where a prototype is approved. Risk: security and distribution.
Dependency: explicit owner decision. Its host-ABI findings are an M6 entry
condition.

## M6: Microsoft NTVDM Component Research

**Goal:** after M5, evaluate the complete Microsoft component system and decide
whether any bounded, non-invasive, high-value integration is feasible.

**Scope:** inventory NTVDM host files, guest DOS files, ROMs, redirectors,
debuggers, configuration, WOW, and host facilities; map startup dependencies,
private ABI, BYOB usability, and interface feasibility with NXVM and Platform.

**Non-goals:** a component loader, BOP dispatcher, or implementation promise.

**Demo and exit:** component matrix, dependency graph, ABI findings, feasibility
report, and a formal Go/No-go decision. Risk: coupled private host architecture.
Dependency: M5.

## M7: Win16 Research

**Goal:** select a viable Win16 route without blocking DOS delivery.

**Scope:** WineVDM, Microsoft WOW, NE loading, KERNEL/USER/GDI, thunking,
unified launcher options, ARM64, licensing, distribution, and relation to M6.

**Non-goals:** a promise of complete Win16 support.

**Demo and exit:** architecture options, dependency map, proof of concept where
approved, and recommended path. Risk: large API surface. Dependency: M3 and
owner approval.
