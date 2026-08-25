# T460 S5 DMA Closure Audit

| Rows | Closure disposition |
| --- | --- |
| R1--R4 | One `dma.c` register/reset/request/status owner; focused DMA smoke covers programming, TC and reset. |
| F1--F2 | One logical acknowledge/service/release route through existing T449 hold/transaction ownership. `dma.c` advances the Intel service states one Core DMA tick at a time. |
| F3--F5 | Demand/single/block/cascade, terminal/EOP and auto-init are covered by the DMA channel smoke; cascade no longer invents TC. |
| F6 | M2M advances `S11` through `S24`, committing one source read at `S14` and one destination write at `S24`; failure/release clears the phase, and each channel applies its own programmed address direction. |
| F7 | `TM=0` is `S1 -> S2 -> S3 -> S4`; `TM=1` is `S1 -> S2 -> S4`. `S1` recurs only after the A8--A15 portion changes. The channel smoke proves that no transfer commits before S4 and that compressed timing removes exactly S3. |
| T1--T2 | Existing paired AT controller/page owners remain one Core path. |
| T3--T4 | PIT1 now reaches the sole DMA1 refresh binding; FDC cannot claim channel 1. The uncalibrated 3 MHz/five-clock conversion remains explicit L2. |
| T5 | T449 remains sole transaction/arbitration/time owner; DMA never publishes time or writes CPU state. |

The final M2M direction regression proves that channel 0 may decrement while
channel 1 increments: the two source bytes arrive at consecutive increasing
destination addresses. This corrects the prior shared-direction bug in the
sole `dma.c` owner; no extra phase owner or transaction route is introduced.

Intel 231466-005 defines compressed timing by removing `S3`; MAME's admitted
AM9517A state machine independently takes `S2` directly to `S4` when the same
command bit is set. NXVM implements that source-backed L3 transition in its
sole DMA owner. It neither imports external code nor claims an L4 waveform.

The final serial current gate completed all 294 selected tests with zero failed
or not-run entries (111.21 seconds current-gate time). The stripped Release
artifact rebuilt after the final M2M and service-phase corrections is
`build/output/nxvm_0_5_0460.exe`, SHA-256
`29FB7AC3D715B45D60A82F4D32F3B4D17C8B4A8601C60FBE482DBB332CE0AF62`.
The actual-diff review finds one state owner (`dma.c`), one binding adapter
(`machine_board.c`) and one transaction/scheduler route (T449); no wrapper,
parallel parser or duplicate page/time store was introduced.

`M5:T460:S5:DMA-CLOSURE:OK`
