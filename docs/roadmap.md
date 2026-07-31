# Roadmap

Each milestone requires a runnable demonstration, focused automated tests,
recorded evidence, and preservation of established baselines.

Before M7, DOS program testing uses a project-owned temporary `load` command in
the NXVM Console. M5 introduces it after the M1 baseline as an empty-session
debugger command: it accepts an explicit host binary path only when debug mode
is enabled and no program is selected or loaded, then hands the image to the
approved DOS loader/test harness and remains paused. It is not asserted to exist
in NXVM today, is not the product CLI, creates no host drive mapping, and does
not alter the future `ntvdm64 run` contract.

## M1: Validate The NXVM Machine Foundation

**Goal:** establish a GCC-runnable, behavior-recorded full NXVM machine
baseline before pruning or redesigning it.

**Scope:** first approve a bounded GCC migration design: compiler compatibility
surface, compatibility-header rules, build graph, warning policy, expected
patch classes and per-class budget, test/rollback plan, and explicit stop
conditions. Only then record the then-current sibling NXVM `HEAD` and import
that exact complete baseline: CPU, memory, IVT, BIOS/POST and current boot path,
PIC/PIT, connected devices, keyboard, display, execution loop, and debugger
primitives. Use local `fdd.img`, `hdd.img`, `stop.com`, and `reset.com` fixture
identities to record FDD and HDD boot/stop/reset observations. After the
unmodified baseline is recorded, remove only proven irrelevant bridge code such
as a Bochs bridge and repository-only documentation or license metadata;
preserve required attribution, provenance, and independent third-party notices.
Preserve Linux platform source, but use the Windows GCC run as M1 acceptance.
These debugger primitives are not the future product debugger experience.

M1 may use the optional differential trace interface to correlate CPU,
interrupt, I/O-port, IRQ, disk, display, and stop events across baseline runs.
NTVDMx64 is not a reference for POST or disk boot; any external adapter is a
local research tool, not an M1 dependency.

**Non-goals:** a project-owned DOS backend, host-drive mapping, a product CLI,
new graphics or device features, or a claim about the contents or license of a
local disk image. The existing 80386 CPU is retained; M1 does not replace it
with a new 8086 implementation. A Linux release or Linux compatibility claim
is also outside M1.

**Demo and exit:** the approved GCC migration design gates all baseline changes.
GCC then builds the imported full baseline and runs repeatable
FDD/HDD text-mode boot/execution scenarios using the recorded fixtures. Evidence
captures source and fixture identities, exact configured devices, expected
checkpoints or stop reasons, instruction/time/no-progress budgets, terminal or
window observation, and reached state. A missing local fixture makes that
scenario unavailable; it is never silently replaced. The baseline then supplies
focused tests for later reduction. Risk: inherited global state, device coupling,
and legacy Win32 assumptions. Dependency: a provenance-reviewed NXVM import.

## M2: Design The Machine And DOS Architecture

**Goal:** define the smallest whole-machine structure that can host the owned
DOS runtime without inheriting NXVM global coupling.

**Scope:** research and specify the minimum virtual-device profile; reset,
boot, halt, fault, timing, and interrupt contracts; machine-runner ownership;
CPU/memory/I/O/device interfaces; Machine-to-DOS services; DOS interrupt and
loader ownership; and the abstract host-service trust boundary. This includes
the cross-layer filesystem-containment model, but not CLI grammar or Win32
implementation details.

**Demo and exit:** a versioned Machine/DOS architecture specification,
interface contracts, state/ownership diagrams, focused test plan, and the
approved bounded M3 Task/subtask breakdown are complete. **Non-goals:**
refactoring or implementing the DOS runtime.

## M3: Refactor The Machine To The Approved Architecture

**Goal:** move the verified NXVM-derived machine into the M2 structure and
expose the tested abstract Machine/DOS contract.

**Scope:** execute the M2 migration plan, isolate `machine`, `platform`, and
`adapters` ownership, remove obsolete baseline-only coupling, and preserve M1
behavior with focused tests. No owned DOS backend or Windows product CLI enters
this milestone.

**Demo and exit:** a Windows GCC machine target, stable documented Machine/DOS
contract, M1 regression evidence, and the required immutable snapshot.

## M4: Design The Owned DOS Module

**Goal:** specify the bounded DOS module before implementation.

**Scope:** finalize COM load state, PSP/environment/DTA layout, initial CPU
state, interrupt dispatch, register preservation, handle and fixture-filesystem
semantics, deterministic input-blocked protocol, error table, and M5 probes.

**Demo and exit:** versioned DOS specification and ABI test vectors, plus the
approved bounded M5 Task/subtask breakdown, are complete against the M3
contract. **Non-goal:** DOS implementation.

## M5: Add The Owned DOS Backend

**Goal:** run a simple COM program using DOS interrupts.

**Scope:** the bounded COM loader, PSP, environment, `INT 20h`, the approved
`INT 21h` subset, deterministic text/keyboard I/O, exit, an in-memory fixture
filesystem, fixed loader memory, defined DOS errors, and a temporary NXVM
Console `load <host-binary-path>` developer/test debugger command. It is
available only to an empty paused debug session and delegates to the COM loader.
The exact M5 profile is [M5 DOS Backend Requirements](requirements/m5-dos-backend.md).

M5 may correlate its DOS-service events with the same machine trace stream and
compare bounded COM probes against an external NTVDMx64 adapter. This adapter
is evidence only; it does not define the DOS profile or enter the product.

**Non-goals:** complete DOS API, dynamic MCB memory, MZ/EXEC, directories,
wildcards, FCB, device names, LFN, XMS/EMS/DPMI, Microsoft guest components,
host-drive mapping, and shell integration.

**Demo and exit:** the approved COM probes print and exit, perform fixture-file
open/read/seek/close, and verify defined failure results. Generated probes cover
each approved service. Risk: machine/DOS ABI leakage and unbounded API growth.
Dependency: M3 machine contract and M4 DOS specification.

## M6: Design The Windows Platform And CLI

**Goal:** specify non-invasive host integration before product implementation.

**Scope:** complete the `run` grammar, program-path mapping, exit-status table,
filesystem containment algorithm and Windows-version matrix, Console/window
state machine, graphics capability table, debugger command grammar, Ctrl+C and
Ctrl+Break ownership, and cleanup/error behavior.

**Demo and exit:** an approved Platform/CLI specification, security test matrix,
and bounded M7 Task/subtask breakdown. **Non-goal:** production platform
implementation.

## M7: Non-Invasive Windows Platform Integration

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
work without system changes. Tests cover the approved M6 path, argument, host
I/O, containment, cleanup, and exit-status cases. Risk: platform adapters
becoming DOS internals. Dependency: M6.

## M8: Real DOS Program Compatibility

**Goal:** grow from probes to representative DOS utilities and real programs.

**Scope:** add MZ loading, EXEC, MCB, FCB, wildcard and 8.3 semantics,
attributes, date/time, device names, selected XMS/EMS, graphics, and mouse only
when a corpus requirement justifies it.

**Non-goals:** theoretical API completeness or speculative Microsoft ABI.

**Demo and exit:** each declared corpus program has a reproducible result. Each
entry records legal basis, identity, required feature, and verdict before it can
expand scope. Separate redistributable project probes from local owner-provided
programs; prioritize the smallest missing feature and require evidence,
owner-approved scope, a bounded corpus-increment Task/subtask breakdown, and a
focused regression before expanding M8. Risk:
unbounded compatibility scope. Dependency: M7.

## M9: Optional Integration Research

**Goal:** investigate optional Windows integration, historical Microsoft NTVDM
components, and Win16 routing without blocking the owned DOS product.

### T1: Invasive Windows Integration

Research only: threat/risk analysis, reversible prototypes, installer/uninstaller
and rollback design, and Windows-version matrix. No default build, automatic
system change, release dependency, or security-policy change.

### T2: Microsoft NTVDM Components

After T1 establishes host boundaries, inventory owner-provided local BYOB
components, startup dependencies, private ABI, and feasibility with NXVM and
Platform. No loader, BOP dispatcher, or implementation promise.

### T3: Win16 Route

Compare WineVDM, Microsoft WOW, NE loading, thunking, launcher, ARM64,
licensing, and distribution options. No complete Win16-support promise.

**Exit:** each task records evidence and a recommendation; M9 closes only on an
owner-approved consolidated Go/No-go report. Research never becomes a default
runtime or release dependency.
