# Roadmap

Each milestone requires recorded evidence, focused tests or review vectors, and
preservation of established baselines. The repository is a dual-product
successor to NXVM: `nxvm.exe` remains a bootable whole-machine VM and
`ntvdm64.exe` becomes the non-bootable DOS application runner. Both products
share the same core.

Before M9, DOS program testing uses developer/debugger entry points only. The
final user-facing `ntvdm64 run` CLI is designed in M8 and implemented in M9.

## M0: Governance Reset

**Goal:** define the repository as the canonical dual-product successor to
NXVM.

**Scope:** project boundaries, MIT source policy, artifact/version policy,
documentation ownership, milestone gates, and the shared-core direction for
`nxvm.exe` and `ntvdm64.exe`.

**Exit:** governance documents are concise, non-conflicting, and identify M1
through M2 as completed baselines before M3 begins.

## M1: Preserve The Current NXVM Baseline

**Goal:** establish a GCC-runnable, behavior-recorded full NXVM baseline before
structural changes.

**Scope:** approve the GCC migration design, import the current NXVM source
snapshot under the root MIT authorization, build with MinGW-w64 GCC/CMake/Ninja,
and record FDD/HDD fixture behavior using local `fdd.img`, `hdd.img`,
`stop.com`, and `reset.com` identities. Preserve Linux platform source as a
future portability asset, but use the Windows GCC run as M1 acceptance.

**Non-goals:** DOS backend work, product CLI, host-drive mapping, graphics
expansion, or a claim about protected local guest media.

**Exit:** the full baseline builds and reaches the recorded FDD/HDD checkpoints
under bounded watchdogs. The immutable M1 snapshot and local developer artifact
are recorded.

## M2: Design The Shared Core Architecture

**Goal:** define the architecture that lets `nxvm.exe` and `ntvdm64.exe` share
one machine core while keeping product policy out of that core.

**Scope:** specify `core/{machine,platform,product}` and
`{vm,vdm}/{machine,platform,product,profile}` ownership; profile/composition,
device, port/memory, interrupt,
firmware-service, DOS-service, host-capability, and debug/command registries;
machine lifecycle, threading, trace boundaries, host-service trust boundaries,
and the M3 task breakdown.

**Non-goals:** runtime refactoring, DOS implementation, or final product CLI
behavior.

**Exit:** the architecture specification and bounded M3 breakdown are approved.
M0 through M2 are closed and M3 remains inactive until explicitly started.

## M3: Refactor The Shared Core

**Goal:** move the verified NXVM-derived machine into the M2 shared-core shape.

**Scope:** create the documented Machine contract; instance CPU/RAM/bus,
port, memory, interrupt, device, trace, and debug state in `core/machine`;
isolate platform presentation; and preserve the M1 full-PC boot
regression through the `nxvm.full_pc` profile. No owned DOS backend or product
CLI enters M3.

**Exit:** the shared core builds with GCC, `nxvm.full_pc` reproduces M1
checkpoints, `ntvdm64.dos_minimal` has deterministic no-media profile tests,
forbidden dependencies are absent, and M3 produces the M4 breakdown.

## M4: Design Firmware And The nxvm Product Surface

**Goal:** specify the bootable VM product before implementation.

**Scope:** BIOS/POST/ROM and BIOS interrupt-service ownership under VM/VDM
profiles; machine-profile and DOS-profile composition;
disk-image and removable-media policy; retained interactive NXVM Console grammar
and behavior; debugger entry points; display/input expectations; artifact
identity; external-ROM manifest boundary; and regression rules that keep
whole-machine boot ability from becoming accidental. `nxvm.exe` has no new
process CLI. Its source entry is `vm/main.c`; the future VDM entry is
`vdm/main.c`.

**Exit:** versioned firmware and `nxvm.exe` product specifications plus the
bounded M5 breakdown. **Non-goal:** implementation.

## M5: Implement Firmware And nxvm.exe

**Goal:** make the bootable VM product a first-class output whose actual
full-PC execution path runs on the shared core.

**Scope:** establish CPU capability claims and optional Bochx/Bochs differential
verification; implement only `vm/profile/default_profile`; migrate the actual
CPU execution, machine lifecycle, BIOS/POST/ROM, boot devices, and presentation
path into `core/*` and `vm/*`; preserve the original NXVM Console and debugger
behavior;
retain FDD/HDD boot fixtures; and produce runnable artifacts. External-ROM
loading and additional machine profiles remain future design work. `nxvm.exe`
has no new process CLI.

**Exit:** `nxvm.exe` boots the recorded full-PC fixtures through the shared core
with focused regression evidence. No baseline or adapter source root remains in
the formal build graph. The source and CMake target graphs obey the directed
core/VM/VDM dependency model, and the `vm/` and `vdm/` roots are the only
composition roots. The legacy `device.h` aggregate is deleted; retained CPU,
RAM, and port execution has one core-machine state/API authority; and reusable
Win32/Linux host providers live in `core/platform` rather than `vm/platform`.
No ntvdm64 DOS runner behavior is required. The reopened closure plans are
`planning/m5-legacy-convergence.md` and
`planning/m5-naming-convergence.md`.

**Compatibility gate:** M5 must not alter NXVM functionality or user experience.
Any unavoidable user-visible change requires explicit owner approval before
implementation and a recorded before/after acceptance plan.

## M6: Design The Owned DOS Module

**Goal:** specify the bounded ntvdm64 DOS backend before implementation.

**Scope:** define `vdm/machine` COM load state, PSP/environment/DTA layout, initial CPU state, DOS
interrupt dispatch, register preservation, handle and fixture-filesystem
semantics, deterministic input-blocked protocol, error table, and M7 probes.

**Exit:** DOS ABI specification and test vectors plus the bounded M7 breakdown.
**Non-goal:** DOS implementation.

## M7: Implement The Owned DOS Backend

**Goal:** run simple DOS programs without booting a guest DOS image.

**Scope:** implement bounded `vdm/machine` loader, PSP, environment, `INT 20h`, approved `INT 21h`
subset, deterministic text/keyboard I/O, guest exit, in-memory fixture
filesystem, and developer/debugger loading through the owned DOS loader.

**Non-goals:** complete DOS API, MZ/EXEC, dynamic MCB memory, host-drive
mapping, shell integration, or final product CLI behavior.

**Exit:** approved COM probes print, read fixture files, handle defined errors,
and exit through the shared core.

## M8: Design Platform And Product CLI

**Goal:** specify non-invasive host integration before product implementation.

**Scope:** `ntvdm64 run` grammar, separation from the retained NXVM Console,
program-path mapping, exit-status table, filesystem containment, Windows 7
through Windows 11 matrix, Console/window state machine, graphics capability
table, debugger grammar, Ctrl+C/Ctrl+Break ownership, cleanup, and error
behavior. A windowed ntvdm64 session retains a product control Console for
online debugging; its exact lifetime and ownership are M8 decisions.

**Exit:** approved Platform/CLI specification, security matrix, and bounded M9
breakdown. **Non-goal:** production platform implementation.

## M9: Implement ntvdm64.exe

**Goal:** make the DOS app runner usable through `ntvdm64 run` on a clean
64-bit Windows system.

**Scope:** CLI, arguments, current directory, environment, host filesystem,
keyboard/mouse, text and basic graphics paths, exit codes, logging, debug mode,
configuration, Ctrl+C, cancellation, and adapted shared-core input/display
components.

**Non-goals:** global file association, loader replacement, injection, drivers,
or registry-dependent operation.

**Exit:** `ntvdm64 run hello.com` and an argument-bearing DOS program work
without system changes, and the approved M8 path, I/O, containment, cleanup,
and exit-status tests pass.

## M10: Compatibility And Extended Devices

**Goal:** grow from probes to representative DOS utilities and real programs.

**Scope:** admit features only through a declared corpus requirement: MZ
loading, EXEC, MCB, FCB, wildcard and 8.3 semantics, attributes, date/time,
device names, selected XMS/EMS, graphics, mouse, sound, serial, parallel, and
printing.

**Exit:** each corpus entry records legal basis, identity, required feature,
verdict, and regression evidence. Scope expands only by owner-approved corpus
increments.

## M11: Optional Integration Research

**Goal:** investigate optional Windows integration, historical Microsoft NTVDM
components, and Win16 routing without blocking the two default products.

- T1: invasive Windows integration research.
- T2: Microsoft NTVDM, NTVDMx64, and OpenNT component research.
- T3: Win16 route comparison, including WineVDM feasibility.

**Exit:** each task records evidence and a recommendation; M11 closes only on
an owner-approved consolidated Go/No-go report. Research never becomes a
default runtime or release dependency.
