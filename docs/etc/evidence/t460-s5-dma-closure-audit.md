# T460 S5 DMA Closure Audit

| Rows | Closure disposition |
| --- | --- |
| R1--R4 | One `dma.c` register/reset/request/status owner; focused DMA smoke covers programming, TC and reset. |
| F1--F2 | One logical acknowledge/service/release route through existing T449 hold/transaction ownership. Physical signal waves remain L4. |
| F3--F5 | Demand/single/block/cascade, terminal/EOP and auto-init are covered by the DMA channel smoke; cascade no longer invents TC. |
| F6 | M2M is one source-read then destination-write temporary-register sequence; failure/release clears the phase. |
| F7 | Compressed pin/latch pulse timing is L4; no Core tick is invented. |
| T1--T2 | Existing paired AT controller/page owners remain one Core path. |
| T3--T4 | PIT1 now reaches the sole DMA1 refresh binding; FDC cannot claim channel 1. The uncalibrated 3 MHz/five-clock conversion remains explicit L2. |
| T5 | T449 remains sole transaction/arbitration/time owner; DMA never publishes time or writes CPU state. |

The serial current gate completed its selected 282 of 294 registered tests with
no failure record (197.38 seconds current-gate time). The stripped Release
artifact is `build/output/nxvm_0_5_0460.exe`, SHA-256
`CED00F5946FB8CC9DA8DA8273486F073C19B611B10B9EF0D109A783D874F12EC`.
The actual-diff review finds one state owner (`dma.c`), one binding adapter
(`machine_board.c`) and one transaction/scheduler route (T449); no wrapper,
parallel parser or duplicate page/time store was introduced.

`M5:T460:S5:DMA-CLOSURE:OK`
