# T390 S22: Shared Core Classified Timing-Origin Attribution

## Delivered boundary

`core_machine_instruction_cost` remains the sole successful-retirement classifier orchestrator. It resets the per-attempt timing origin to `UNATTRIBUTED`, then records exactly one origin only after the first classifier that returns a cost succeeds. The copied observer publishes this origin with the existing timing disposition and opaque ledger form ID before physical-contract rejection or elapsed/timeline publication.

The origin identifies the shared classifier family--string/I-O, 80386 dynamic multiply, legacy dynamic arithmetic, 80386 secondary, 80386 privileged, primary, control/stack, profile fallback, or compatibility--rather than a guessed Intel row. It neither changes a timing value or disposition nor exposes a CPU pointer, raw instruction/PC, ROM/media data, or mutable machine state. The original explicit call ordering is retained so the T359 classifier-order verification remains meaningful.

## Proof

- The retirement-observation smoke proves a lookup-backed 80386 NOP and a physically rejected prefixed NOP both carry the 80386 fallback origin, preserving reset and prepublication behavior.
- The synthetic C0 capture gives otherwise identical NOP observations distinct fallback and primary origins; it requires four aggregate rows, proving origin participates in the non-raw aggregate key.
- A fresh owner-managed, deleted C0 replay reaches `protected-return-c0`: 18,255 classified successful retirements, zero source-unallocated successes, 75 aggregate rows, and no fault/status terminal. All 75 rows and all 18,255 successes have an origin; observed C0 origins are String/I-O, 80386 privileged, primary, control/stack and 80386 fallback. No asset identity, path, hash, byte, PC, raw trace or provenance is retained.
- The full current gate completes with 284/284 passing tests; documentation governance and diff checks pass.

## Transfer

The next finite C0 matrix slice must split each origin family into exact Intel-primary row/context dispositions using the existing normalized fields plus any narrowly admitted missing semantic facts. It must add project-owned regression or transfer every underdetermined class whole. This slice does not qualify physical retirement or claim C1 boot, DeskPro board/device timing or L3.