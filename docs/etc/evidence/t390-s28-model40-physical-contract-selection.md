# T390 S28: Model-40 Physical-Contract Selection

`M5:T390:S28:MODEL40-PHYSICAL-CONTRACT:OK`

## Selection boundary

S28 consumes the accepted S27 82-child C0 direct-row batch. Both private
Model-40 session construction paths now select the existing
`CORE_MACHINE_RETIREMENT_TIME_PHYSICAL` contract in their `core_machine_config`.
No Core classifier, source row, public interface, clock ratio, device clock, or
board path changes. Non-Model-40 profiles retain the zero-value deterministic
contract through their existing owners.

## Positive and rejection proof

The retained Model-40 private-composition smoke creates the actual composition
with a project-owned synthetic external-ROM pair. It proves the constructed
Core machine selects the physical contract. Its reset vector supplies a
segment-prefixed NOP: a successfully decoded but source-unallocated form. The
physical contract returns `TYPE_STATUS_FAULT` before it publishes executed
instructions, result ticks, elapsed ticks, or an execution-provider advance.
This directly proves that later C1 work cannot silently use the C0 selection to
publish an uncited success.

A fresh owner-authorized local C0 replay under the selected contract reaches
`protected-return-c0` with 18,255 classified successes, zero source-unallocated
successes and 82 children over the unchanged 75 parents. It retains no asset
identity, path, digest, bytes, PC, trace, or provenance.

## Boundary and transfer

This admits only successful C0 CPU retirement to the existing physical contract.
It does not supply a CPU-to-board ratio, make elapsed Core ticks a DeskPro
board-clock claim, advance C1, or qualify DMA, PIT, device, firmware, or L3
time. The next T390 receiver is the first post-C0 unqualified successful form
or a separately admitted C1 batch; it remains nonphysical unless its complete
context, Intel-primary row and direct regression are established.
## Coordinator Acceptance

After actual-change review of P1 `da5dfd3a`, the coordinator accepts S28. Both
Model-40 composition paths select the existing physical-retirement contract;
the project-owned unallocated-success regression proves prepublication fault
behavior, and the scalar C0 replay proves the complete accepted C0 batch
continues under that selection. Full current gates, documentation governance
and diff hygiene pass. T390 remains open: the next receiver must identify and
disposition the first post-C0 successful form or admit C1 separately. No
DeskPro board timing or L3 claim is accepted.
