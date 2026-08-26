# T489 S4 XT DMA Page-Port Closure

`M5:T489:S4:XT-DMA-PAGE-PORT:ACCEPTED`

`core_machine_dma_initialize` now registers `81h`--`83h` for every selected
primary 8237A and registers `87h`, `89h`--`8Bh` and `8Fh` only with the second
controller. This removes the AT-only page-port visibility from the IBM XT
single-controller construction without changing the sole DMA state or port
owner.

`core_machine_plan_smoke` now freezes execution providers before calling
`core_machine_reset`, as the Core lifecycle requires. The same test proves
the selected primary ports and absence of all excluded page, secondary DMA,
secondary PIC and RTC ports before and after reset.

Focused rebuilt regressions pass: `core-machine-plan-smoke`,
`core-machine-dma-channel-smoke` and `vm-xt-5160-268-profile-smoke`.
The latter retains all XT profile/topology markers. IBM's elapsed DMA/refresh
formula remains a documented L2 boundary pending a qualified Core physical
time axis; no scheduler, timing setter or second route was added.
