# IBM 5150/XT 8088 Baseline-Machine L3 Closure

## Purpose

Establish one documented IBM 5150 or IBM XT 8088 baseline machine after the
IBM PC/AT 5170 and DeskPro 386 L3 audits and the separate 8088 CPU-profile
closure, and before Windows 3.x work. The admitting task chooses
one exact board revision, CPU clock, memory/ROM population, display adapter,
storage path, and project-owned probe corpus; it must not merge PC and XT
variants into an invented generic 8088 PC. This is an 8088 physical-machine
contract, not an 8086 hardware claim.

## Required scope

Consume the closed 8088 CPU profile, then map the selected 8088 CPU,
memory/ROM, ISA, 8237 DMA, 8259 PIC, PIT, keyboard,
display, storage and NMI/relevant board paths to one source-labelled
availability, reset, publication, and trace contract. Reuse an existing core
mechanism only when its validation, ownership and timing semantics match the
selected board; otherwise admit a bounded owner repair. Use IBM/Intel primary
documentation and project-owned probes; where it gives a range or no exact
value, use named 86Box/MAME/PCjs comparisons only as non-imported,
profile-local cross-checks under the source policy.

## Non-goals and stop conditions

No 8086 substitution, unclosed 8088 CPU profile, XT/PC hybrid, universal ISA
timing, copied emulator code, protected-mode claim, Windows claim, host-time
coupling or firmware/media import. Stop if the exact board/manual/corpus cannot identify an owner or a
timing value; retain the receiver rather than borrowing PC/AT behavior.

## Evidence standard

Require board/profile provenance, a CPU/device/bus/NMI route ledger,
source-to-model or reference-exhausted timing table, contention and reset
replay, project-owned firmware/DOS probes, full current-gate evidence, and a
closure matrix that distinguishes this baseline from PC/AT and DeskPro 386.
