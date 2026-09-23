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
| S3 | Focused proof, unit 312/312, integration 20/20, documentation governance and stripped Release `nxvm_0_5_0506.exe` all pass; the D1--D10 closure audit retains the source/test delta and artifact SHA-256. |

The task closes with no unclassified selected DMA relation. The retained
proposal records the bounded source-first admission; later HDC, Model-40 D4
and CPU work remain separate queue candidates.
