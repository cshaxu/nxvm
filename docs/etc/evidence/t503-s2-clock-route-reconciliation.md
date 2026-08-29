# T503 S2 Clock, Interrupt And Refresh Route Reconciliation

`T503-S2-CLOCK-ROUTE`

S2 consumes R1 and R8 from the accepted T503 S1 ledger.  Source inspection
finds no connection defect and therefore makes no production-code change.
Adding a second clock, a VM wake-up path, or a profile callback would duplicate
the existing Core route and violate the task boundary.

| Form | One current Core route and observation | Result |
| --- | --- | --- |
| PIT0 normal | `machine.c` binds shared PIT counter 0 through `shared_pit_irq0_source`; `machine_scheduler.c` advances PIT before `core_machine_pic_refresh`; CPU owns the normal PIC scan/get delivery. | Present and focused proof passes. |
| PIT0 mask/ack/reset | PIC owns masking/acknowledgement; PIT and PIC own their respective reset release. No profile or VM filter is in the route. | Present; no duplicate route. |
| Selected PIT1 refresh | `core_machine_configure_dma()` binds only shared PIT counter 1 to the immutable Core refresh request; `core_machine_dma_refresh_pit_output()` asserts/deasserts it, and DMA remains the service owner. The selected boards use this edge only when their frozen topology provides the shared DMA construction. | Present; no direct CPU/VM refresh path. |
| Deadline/HLT | `core_machine_capture_time_observation_private()` derives a PIT output boundary only through the copied Core timing plan. An active DMA/FDC/HDC/D4 causal owner blocks fast advance rather than fabricating a deadline; `core_machine_advance_to_next_deadline()` is the only public advancement operation. | Present; no host-generated tick. |
| Four-profile construction | default-at, 5160, 5170 and Model-40 tests construct their selected PIT/DMA/PIC topology through the same Core construction direction. | Present; no profile-side controller state. |

Focused unit route set: `core-machine-pit-irq0-s2-smoke`,
`core-machine-d4-refresh-hold-smoke`, `core-machine-scheduler-smoke`,
`core-machine-timeline-s2-smoke`, `core-machine-time-smoke`,
`vm-default-pc-at-profile-smoke`, `vm-xt-5160-268-profile-smoke`,
`vm-ibm-5170-model-339-firmware-fdc-topology-smoke`, and
`vm-model40-byob-boot-media-s5-smoke`: 9/9 pass.

The source basis is the retained Intel 8253/8254 and IBM board records in
T490/T461, plus T469's deadline-eligibility reconciliation.  S3 receives the
separate R2--R4 DMA/controller service batch; no PIT/PIC/refresh issue is
transferred or concealed by that batch.
