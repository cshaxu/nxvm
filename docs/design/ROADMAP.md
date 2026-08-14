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

Complete the shared-core/NXVM boundary through three source-backed L3 baseline
machines: IBM PC/AT 5170 80286 first, then one exact Compaq DeskPro 386 80386
configuration, and then a distinct 8088 CPU profile plus an IBM 5150/XT 8088
(8086-class ISA) baseline. For each baseline, every selected device,
chip, port, bus route, reset, IRQ/DRQ lifecycle, availability/wait state, and
cross-device event order has one owner and a declared timing contract. Carry
the Intel 8086, 8088, 80186, 80286, and 80386DX form/state program through its
evidence-led Queue sequence before using a guest operating system as
validation. The 8088 profile retains 8086 instruction semantics but separately
owns its external bus and prefetch/timing boundary. The 8086 and 80186 retain
CPU boundaries but have no physical-machine baseline until separately selected
systems are admitted. That program has
complementary instruction-form and architecture-state closure views; it
includes real/protected/VM86 transitions, privilege/exception delivery, paging,
and remaining system state. A completed opcode slice never closes a CPU family,
device, or stage by itself, and M5 makes no 80387 claim.

After the 5170, DeskPro 386, and 5150/XT 8088 baseline-machine L3 closure
audits accept those prerequisites, M5 ends with an approved BYOB Windows 3.1
corpus on the frozen DeskPro 386:
Standard Mode and 386 Enhanced Mode must each install, start normally, reach a
named ready checkpoint, and shut down normally. These are final compatibility
consumers, not substitutes for device/timing evidence. Windows media is never
committed or made a default dependency.

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
