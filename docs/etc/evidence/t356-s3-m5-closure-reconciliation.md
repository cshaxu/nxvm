# T356 S3: M5 closure reconciliation

## Decision

**M5 remains open.**  T356 closes the audit, not the milestone.  The current
source and 234-test current-gate establish the selected device/L3 contract,
but they do not establish the full physical-L3 mechanism required by the owner
request or by the M5 roadmap.  A planned proposal is not implementation
evidence.

## Closure-eligibility matrix

| Requirement | Current evidence | Decision |
| --- | --- | --- |
| Four CPU profiles and named state closure | T343 accepted form/state program. | Accepted baseline; no CPU-family re-opening in T356. |
| Selected PC/AT devices and deterministic L3 | T346--T354 histories, S1 ledger, and 234 discovered current-gate tests. | Accepted only for selected event/transaction order. |
| CPU instruction time | `clock.c` consumes executor elapsed ticks but no profile/form cost ledger exists. | Open: [instruction-timed execution](../../proposals/m5-instruction-timed-execution.md) is first receiver. |
| PC/AT bus availability and wait states | T354 transactions are synchronous and traceable, not reservations/durations. | T369 closes the logical availability stage; remaining physical values transfer to selected-profile phase refinement. [Retained proposal](../../history/M5-T369-bus-timed-pcat-operation-proposal.md). |
| Cycle exactness | No phase, prefetch, bus ownership or device microstate representation exists. | Closed T371 [cycle-exact selected profile](../../history/M5-T371-selected-profile-phase-refinement-proposal.md) depended on both predecessors. |
| Unselected PC/AT peripherals | NMI producer, PPI/speaker, serial/parallel/game and broader AUX/video remain corpus-gated TODO items. | Deliberately deferred; none is misrepresented as selected or implemented. |
| Storage/media breadth | Selected FDC/ATA readiness is accepted; sparse/WASM backing, LBA48, extended IDE and broad errors remain TODO. | Deliberately deferred to its named high-priority backing admission. |
| Product and compatibility | NXVM is the only current product; T355 has one host-observation HDD/INT13 checkpoint. | Mantle/DOS/NXVDM and Windows installation/boot remain outside M5 closure evidence. |
| x87 execution | Explicit owner exclusion and existing TODO. | Not part of this device/L3 closure; no claim made. |

## Final sweep

The audit reran current-gate discovery (234 tests) and searched all
core-machine/VM device owners, `PROJECT_CURRENT_*` CMake lists, T346--T355
records, Queue, capability baseline and all TODO priorities.  The S1 ledger
has no unclassified selected owner; the S2 plan gives each timing layer a
single ordered Queue receiver; every other residual has a stated TODO/product
boundary.  Documentation governance and diff checks pass.

## Queue and milestone handoff

When T356 closes, the audit candidate/proposal moves to history and the Queue
advances to instruction-timed execution.  Bus timing and selected-profile
cycle exactness must remain behind that dependency.  M6 is not admitted by
this audit: it cannot treat selected-L3 ordering, a passing HDD checkpoint, or
the M5 closure audit itself as proof that M5's device/timing program is done.
