# Roadmap

Each milestone requires recorded evidence, focused tests or review vectors, and
preservation of established baselines. The repository is a dual-product
successor to NXVM: `nxvm.exe` remains a bootable whole-machine VM; `nxvdm.exe`
becomes the non-invasive DOS application runner. `core`, `mantle`, and `dos`
are the planned shared-component targets.

M5 is the current NXVM/shared-core convergence milestone. M6 through M8 build
the future VDM stack in dependency order: mantle, VM profiles, then the owned
DOS and NXVDM product. M9 onward is a research queue; it does not schedule an
external VDM or external DOS backend in this repository.

## M0: Governance Reset

**Goal:** define the repository as the canonical dual-product successor to
NXVM.

**Scope:** project boundaries, MIT source policy, artifact/version policy,
documentation ownership, milestone gates, and the shared-core direction for
`nxvm.exe`, `nxvdm.exe`, and the core/mantle/dos component boundaries.

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

**Goal:** define the architecture that keeps core product-neutral, separates an
independent DOS component, and gives VM and shared VDM composition explicit
ownership.

**Scope:** specify the neutral `core/utils` layer, `core/{machine,platform,product}`, and
`vm/{machine,platform,product,profile}`, `mantle/{machine,platform,product}`,
`dos/{machine,platform,product,profile}`, and
`vdm/{machine,platform,product,profile}` ownership; profile/composition,
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
checkpoints, the planned DOS profile has deterministic no-media tests,
forbidden dependencies are absent, and M3 produces the M4 breakdown.

## M4: Design Firmware And The nxvm Product Surface

**Goal:** specify the bootable VM product before implementation.

**Scope:** BIOS/POST/ROM and BIOS interrupt-service ownership under VM and DOS
profiles; machine-profile and DOS-profile composition;
disk-image and removable-media policy; retained interactive NXVM Console grammar
and behavior; debugger entry points; display/input expectations; artifact
identity; external-ROM manifest boundary; and regression rules that keep
whole-machine boot ability from becoming accidental. `nxvm.exe` has no new
process CLI. Its source entry is `vm/main.c`; the future VDM entry is
`vdm/main.c`.

**Exit:** versioned firmware and `nxvm.exe` product specifications plus the
bounded M5 breakdown. **Non-goal:** implementation.

## M5: Converge Core And NXVM

**Goal:** finish a clean core/VM boundary and make `nxvm.exe` a credible PC/AT
foundation for Windows 3.x startup research and the later profile program.

**Scope:** complete the admitted `core` versus `vm` boundary migration and its
single-owner/single-path audit, including profile-neutral media contracts and
optional RTC/FDC/ATA controller mechanisms with VM-owned PC/AT wiring and
media policy; retain `vm/profile/default_profile`; strengthen
the hardware and CPU corpus needed by real DOS and Windows 3.x startup
experiments; preserve the retained NXVM Console/debugger, FDD/HDD boot, and
developer artifacts. Core may admit only policy-free contracts with a concrete
NXVM need and a stable, trusted research requirement; no external ABI or
runtime dependency enters core. `nxvm.exe` has no new process CLI. The detailed
queue remains [M5 NXVM PC/AT Hardware Convergence](m5-pcat-hardware-convergence.md).
T255 freezes machine-profile admission as an immutable VM declaration and
composition-owned construction boundary; it does not implement a new profile,
selector, or external-ROM loader.

**Exit:** `nxvm.exe` boots the recorded full-PC fixtures through the shared core
with focused regression evidence; the core/VM boundary has no duplicate state
or execution route; the admitted hardware corpus covers the selected Windows
3.x startup prerequisites or records each bounded deferral; and the source and
CMake graphs obey the directed component model. No NXVDM, mantle, or DOS
runtime is required. M5 remains open until the
[closure checklist](m5-closure-checklist.md) has current evidence for every
applicable item.

**Goal map:** M5 owns the common machine and PC/AT fidelity work: the Level 1
to Level 3 timing path, display/input/storage controller evidence, and the
CPU/FPU subsets specifically required by a reproducible Windows 3.x checkpoint.
M5 may use a core-only mantle-shape fixture to prove neutral configuration, but
does not implement mantle or promote host filesystem/path policy into core.
It does not promise Windows 3.x or Windows 95 compatibility by itself; broad
80386/486, profile-specific, and operating-system work remains evidence-led.

**Compatibility gate:** M5 must not alter NXVM functionality or user experience.
Any unavoidable user-visible change requires explicit owner approval before
implementation and a recorded before/after acceptance plan.

## M6: Build Mantle

**Goal:** implement the minimum policy-free VDM session envelope over core.

**Scope:** create mantle only around an approved core contract: session
lifecycle, bounded execution pump, neutral runtime bindings, transition
gateway, and structured stop/fault diagnostics. Trusted external research may
validate capability requirements, but mantle never contains an external ABI,
runtime file set, loader, selector, service table, or host policy.

**Exit:** one minimal owned-runtime-facing mantle session is exercised through
core contracts with explicit lifetime and failure probes; M7 profile work has a
bounded integration plan.

## M7: Implement VM Profiles

**Goal:** make NXVM machine models declarative and reproducibly composable.

**Scope:** formalize profile topology, ROM/provider manifests, firmware hooks,
media and device configuration, and profile-specific corpus. Implement the
default PC/AT profile as the reference, then admit Compaq DeskPro 386 and IBM
PC 110 profiles only through individually reproducible evidence and legal
asset boundaries. M5 T255 is the governing admission contract; each profile
implementation must instantiate its template rather than extend core or copy
the default-PC/AT assembly path.

**Non-goals:** bundled third-party ROMs, profile-specific hacks in core, and
VDM/DOS product policy.

**Exit:** NXVM can select and boot each admitted profile without duplicating
machine state, boot policy, or firmware ownership.

Compaq DeskPro 386 and IBM Palm Top PC 110 are explicit M7 targets. Their
respective PCjs and PC110-EMU work may inform bounded requirements, but their
code, ROMs, and media do not enter default builds or the repository.

## M8: Build Owned DOS And NXVDM

**Goal:** deliver the owned DOS runtime and non-invasive `nxvdm.exe` product
over mantle.

**Scope:** build the bounded owned DOS loader/services and the NXVDM product
shell together with the already approved CLI, containment, display, debugger,
cancellation, and exit-policy requirements. `vdm` owns product policy;
`dos` owns DOS semantics; mantle owns only neutral composition.

**Exit:** `nxvdm run` executes the admitted owned-DOS corpus without system
changes and proves its path, input/output, cleanup, containment, and exit
semantics on the approved Windows matrix.

External VDM/DOS binaries may be studied or used in isolated, user-supplied
research environments, but they are never the owned product backend, a shared
ABI, a default dependency, or release input.

## M9 And Later: Research Queue

**Goal:** conduct bounded research that proposes future corpus or architecture
admissions without silently changing a shipped product.

**Scope:** research tasks may cover compatibility corpora, Win16/Windows 3.x
and Windows 9x requirements, 486 and later CPU/timing fidelity, external-ROM legality and
profile evidence, AI-assisted validation, and optional Windows integration.
External VDM/DOS work is tracked outside this roadmap; its trustworthy reports
may be requirements references but are never this repository's backend,
default build, or release dependency.

**Exit:** each research task records evidence, non-goals, and an owner decision
about whether it creates a later implementation task. No research conclusion
changes a core contract or product dependency without that separate task.
