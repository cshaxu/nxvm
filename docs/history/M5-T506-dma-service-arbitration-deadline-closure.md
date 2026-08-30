# M5 T506: DMA Service And Arbitration Deadline Closure

T506 is the bounded selected-8237A follow-up to T504's DMA L1 receiver.  It
covers only PIT1 refresh, FDC DMA2, XT Xebec DMA3 and the selected primary/
secondary cascade.  Core DMA remains the sole request, service and completion
owner; producer devices retain their command timing and profiles supply only
copied construction values.

## Accepted Subtasks

| Subtask | Accepted result |
| --- | --- |
| S1 | The finite D1--D11 Intel/IBM ledger maps every selected request, arbitration, service, route and reset relation to its current owner. |
| S2 | The existing qualified Model-339 `3/8` DMA clock now publishes the next logical service-phase deadline through the sole Core scheduler; unqualified profiles retain their lower-tier boundary. |

S3 records the task-level regression, artifact and closure audit before the
task may close.
