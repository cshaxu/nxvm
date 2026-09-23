# M5 T449: Core CPU-To-Board Transaction And Arbitration Contracts

T449 closes the five frozen Core capability batches without adding a bus
framework or profile callback. The accepted result is one copied, validated
transaction contract; one transaction lifecycle; one scheduler arbitrator; one
memory classifier; and one retirement/time/observation publication route.

## Accepted Subtasks

- S1 `e21d80c6`: froze the capability and owner ledger.
- S2 `32f190f2`: consolidated six raw selections into one copied contract.
- S3 `390517a2`: exhausted CPU/DMA/refresh classification and lifecycle.
- S4 `bccea141`: reconciled availability, BUSRDY and prefetch ownership.
- S5 `2887796d`: reconciled arbitration and memory classification.
- S6: reconciles retirement, trace, observation and the complete closure audit
  in [its evidence](../etc/evidence/t449-s6-retirement-closure.md).

The retained physical timing, controller/device phase and x87 boundaries are
explicit L2 transfers to the next queued controller/device contract; they are
not duplicate or unfinished T449 production paths.
