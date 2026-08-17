# T390 S19: Shared Core Source-Form Attribution

## Delivered boundary

`core_machine_instruction_cost` resets the per-attempt opaque source-form ID to `CORE_MACHINE_RETIREMENT_SOURCE_FORM_UNATTRIBUTED`. The sole successful owner is `core_machine_source_timing_lookup`: when a ledger entry matches, it stores that Core-private form identifier before returning the authoritative tick value. The existing lifecycle-owned copied retirement observer then copies that scalar before physical-contract rejection or elapsed-time publication.

The interface exposes neither the private enum nor CPU pointers, instruction bytes, PC, ROM/media data, or mutable machine state. Its only new contract is the opaque `source_timing_form_id`; the named sentinel means a classified path did not select a timing ledger row. This is intentional: direct, dynamic, or special classifier paths are not retroactively inferred to have a source form.

## Proof

- `core-machine-retirement-observation-s3-smoke` proves a lookup-backed no-prefix 80386 NOP reports form ID `0`, while an unallocated prefixed NOP reports the explicit unattributed sentinel; both remain nonpublished with zero elapsed and timeline ticks.
- `vm-model40-byob-retirement-capture --synthetic-c0` still reaches the C0 protected-to-real checkpoint and emits `M5:T390:S17:M40-C0-CAPTURE:OK` using the extended aggregate key.
- A deleted owner-managed M40-C0 replay remains finite at `protected-return-c0`: 18,255 successful retirements, 75 aggregate rows, zero source-unallocated successes, and no fault/status terminal. It records no asset identity, path, hash, byte, PC, or raw trace.
- The full current gate built successfully and CTest reports 284/284 passing tests. Documentation governance and diff checks are recorded with this delivery.

## Transfer

The next T390 continuation must rerun the finite C0 corpus and build one batch owner/Intel-row/context disposition using this attribution. It must separately account for lookup-backed IDs and classified-but-unattributed paths, cite exact Intel-primary rows, and transfer every underdetermined class as a whole. This slice neither adds timing rows nor enables physical retirement, C1 boot, DeskPro board/device timing, or L3.
## Coordinator Acceptance

Coordinator actual-change review accepts P1 8c3003f8. The reset occurs at the per-attempt timing boundary, successful lookup is the single attribution owner, and the observer copies a scalar before rejection/publication without publishing or retaining forbidden raw state. The focused lifecycle proof, synthetic C0 proof, full 284/284 current gate, documentation gate, artifact baseline and actual diff satisfy S19. Classified-but-unattributed forms remain an explicit batch-matrix obligation; physical retirement, C1, board/device timing and L3 remain unaccepted.
