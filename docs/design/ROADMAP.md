# Roadmap

The roadmap defines milestone outcomes and their dependency order. It does not
allocate tasks or prescribe implementation steps; those belong to
[QUEUE.md](../states/QUEUE.md), [TODO.md](../states/TODO.md), and approved task packets.
Detailed historical and M5 planning material is retained in
[etc/history/m5/roadmap-detail.md](../etc/history/m5/roadmap-detail.md).

## M0: Governance Reset

Establish the repository's ownership, documentation, source, build, artifact,
and execution foundations.

## M1: Preserve The NXVM Baseline

Produce a GCC-built, behavior-recorded bootable NXVM baseline before structural
change.

## M2: Design The Shared Foundation

Define the component boundaries, composition model, profile direction, host
boundary, and immediate refactoring plan.

## M3: Refactor The Shared Foundation

Move the verified NXVM machine into the approved shared-core source shape while
retaining full-PC regression behavior and a non-runnable VDM scaffold.

## M4: Design Firmware And Product Surfaces

Define VM firmware/profile behavior, retained NXVM interaction, and the bounded
implementation plan for core convergence.

## M5: Converge Core And NXVM

Converge the shared Core/NXVM boundary to Project A's L3 standard: Core uses
documentation-driven instruction and transaction timing where a constant,
formula, deterministic range selection or labelled reference-derived contract
exists; every remaining capability retains an explicit L2 proportional fallback
or unsupported disposition. L4 hardware-real timing is prohibited.

First reconcile the retained Core CPU, transaction, device and capability
ledgers against current code, then establish the copied validated Core timing
plan, CPU timing programs, CPU-to-board transactions, controller/device phases
and integrated Core contract catalog. Core stays machine-neutral throughout.

The existing 5170 baseline is the AT parent. DeskPro 386 first becomes a
runnable 80386DX/Compaq delta that inherits its retained 5170 detail; it does
not reopen a physical-board reconstruction. VM then normalizes built-in profile
composition: independent 5160 XT and 5170 AT roots, `generic-386dx-at` as a
5170 child, and DeskPro as its Compaq delta. VM owns profile inheritance,
provenance and session policy; Core owns execution, transactions and device
state.

The retained 5160-268 profile/device/phase/audit sequence follows that
foundation, including its 8088 attachment boundary. A pre-Windows audit then
checks the frozen Core contracts and selected 5170, DeskPro and 5160 profiles
before Windows 3.1 BYOB, Standard Mode and Enhanced Mode are permitted as final
compatibility consumers. Windows remains evidence of compatibility, never a
substitute for Core or profile timing proof; Windows media is never committed.
## M6: Build Mantle

Only after M5's L3 and Windows closure decision, implement the neutral VDM
session foundation over core, including the bounded
adapter capability required for the owned runtime and separately admitted
external runtime implementations.

## M7: Implement VM Profiles

Deliver reproducible machine profiles, beginning with PC/AT and then Compaq
DeskPro 386 and IBM PC 110 when their individual corpus and legal asset
boundaries are satisfied.

## M8: Build Owned DOS And NXVDM

Deliver the owned DOS implementation and non-invasive NXVDM product, including
its approved command, display, containment, debugging, cancellation, and exit
experience.

## M9 And Later: Compatibility And Web Delivery

Advance Windows 95 readiness only after its required CPU, profile, device,
timing, and corpus gates. Admit a WASM core/mantle target and TypeScript web product
layer through the same host-boundary, asset, and reproducibility rules. Future
external runtime support remains separately admitted and never silently becomes
the default backend or release dependency.
