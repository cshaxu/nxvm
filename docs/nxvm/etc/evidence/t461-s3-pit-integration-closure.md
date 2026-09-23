# T461 S3: PIT Integration Closure

The P15 route has one production owner chain:

`shared_pit` channel 0 OUT -> `shared_pit_irq0_source` ->
`core_machine_pic_timer_output` -> shared PIC IRQ0.

`machine.c` binds that route once during construction. `machine_scheduler.c`
advances PIT before it refreshes PIC; `machine_timeline_s2_smoke` proves the
trace order `DMA -> PIT -> PIC`. `core_machine_pit_irq0_s2_smoke` proves OUT
edges, PIC assertion/deassertion and reset release. PIT reset/finalize releases
an asserted output before clearing local state.

The similar-route sweep found only channel 1/2 board bindings for selected
refresh/speaker functions and optional auxiliary PIT routing. They are distinct
counter/topology owners, not alternate IRQ0 routes. No runtime code change was
needed for S3.

P16 reset phase, P17 board input-clock conversion and P18 electrical/PPI/
speaker topology remain the exact explicit L2 boundaries from List 1. No
physical timing, host pacing, new scheduler or parallel PIT state was added.

The focused PIT group passes, full current-gate passes 294/294, and the
stripped Release artifact is `build/output/nxvm_0_5_0461.exe`, SHA-256
`D875AB15C1E63A3FFBBC1A064315AE9516773A65F70FC4783957FE21DE45829A`.

`M5:T461:S3:PIT-INTEGRATION-CLOSURE:OK`
