# T363 S7: complete instruction-timing corpus closure audit

## Decision

T363 closes the complete four-profile instruction-retirement timing corpus.
Its 256 primary and 256 secondary dispatch slots are mechanically inventoried;
every implemented successful form/profile/context reaches one source-backed
private classifier or a named, truthful receiver. This is not a claim of
physical or cycle-exact processor timing, PC/AT L3 completion, or x87 timing.

## Owner and receiver reconciliation

| Partition | T363 evidence | Closure disposition |
| --- | --- | --- |
| Primary arithmetic/data/FLAGS/ModR-M/EA | S2 reconciliation | One source owner plus dynamic/range owner where selected. |
| Control, stack and real-control | S3 reconciliation | Successful source rows; protected/task/delivery paths remain explicit receivers. |
| Strings, REP and ordinary I/O | S4 reconciliation | One restart-safe source publisher; 80286 `REP LODS`, provider and physical paths remain receivers. |
| 80386 non-privileged `0F`, prefixes and widths | S5 reconciliation | Exact successful rows and 106-clock preflight boundary. |
| Selected 80386 privileged/system forms | S6 reconciliation | Exact successful rows; incomplete 80286 contextual and delivery forms remain receivers. |
| Invalid, profile-rejected, faulting or illegal-prefix/LOCK forms | S1 inventory and owners | No successful-retirement tick. |

Remaining source-range/table/context rows retain the T360 receiver. Task/gate,
cross-privilege, VM86 and exception/IRQ/NMI delivery retain the selected-profile
cycle-exact receiver. Bus waits, prefetch/cache, HOLD/DMA and pin phases retain
the bus-timed PC/AT receiver; device service retains its dedicated Queue
receiver; x87 and VME/PVI retain their documented TODO receivers.

## Verification

On 2026-08-14, `verify-t359-instruction-timing-inventory` passed against the
current 256+256 dispatch inventory and sole successful-retirement publisher.
The complete `current-gate` label run passed 246/246 tests, and documentation
governance and `git diff --check` passed. S7 reviewed the S1--S6 evidence,
the publisher order, fallback/receiver boundaries, history, indexed evidence,
Queue and TODO links. No runnable path changed and no developer artifact is
created.
