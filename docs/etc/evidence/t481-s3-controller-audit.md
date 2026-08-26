# T481 S3 Controller Function, Phase And Deadline Audit

`M5:T481:S3:CONTROLLER-OWNER:OK`

S1's controller rows remain the only denominator.  This reconciliation checks
the owner of each guest-visible function separately from eligibility for Core's
one deadline observation; a functional controller is not thereby a physical
time source.

| Controller | Sole current Core owner and source result | IRQ/DRQ, snapshot or media path | Deadline and L0--L3 disposition |
| --- | --- | --- | --- |
| 8259A PIC | `pic.c`; T456 closes the selected 18-row Intel/AT command, cascade, priority and logical acknowledgement model. | One CPU `scan`/`get` INTA consumer; devices bind IRQ sources to PIC. | No future deadline: pending IRQ is same-tick arbitration. Function/order is Manual-L3; elapsed acknowledgement latency is L1/L2 and electrical INTA is excluded. |
| 8237A DMA | `dma.c`; T460 closes register/reset/request/service, M2M, demand/single/block/cascade and compressed-timing phase ownership. | One transaction/HOLD route; FDC and refresh consume immutable bindings. | Blocks deadline advancement while active. Manual `TM` phase transition is L3; board conversion such as refresh clocks is L2 unless the copied board input is source-qualified. |
| 8254 PIT | `pit.c`; T461 retains one counter/gate/OUT owner and channel-0 IRQ0 composition. | PIT0 OUT -> PIC IRQ0 once; channel 1/2 bindings remain distinct board consumers. | Eligible only for a source-qualified rational clock. Counter/output behavior is Manual-L3; reset phase, board conversion and electrical/PPI topology are explicit L2/excluded boundaries. |
| MC146818A RTC/CMOS | `rtc.c` with one board index/data adapter; T450 identifies a partial but unique calendar, alarm, periodic and IRQ owner. | RTC alone publishes IRQ8; board owns port-70 NMI-mask effect. | Eligible only when configured L3 source timing and rational clock qualify it. Divider/UIP/SQW, reset/power and generic AT phase remain L2 receivers; no second RTC clock/store is permitted. |
| 8042 KBC/NMI boundary | `kbc.c`; one FIFO, selected command/status fields, keyboard command/translation and A20/reset owner. | One IRQ1 source; AUX only when profile-gated. NMI remains a board/RTC owner. | Blocks while response/serial/typematic work is pending. Selected host-interface logic is Manual-L3; controller/serial cadence and IRQ/A20 observation phase are L2 unless supplied by a qualified board input. |
| Intel 8272A FDC | `fdc.c`; one 8272A command/phase/reset/drive/media owner, not a uPD765 alias. | One DMA2 provider, TC path and IRQ6 source; media registry is the unique byte provider. | Blocks during seek, byte gate or non-command phase. Command/phase relations are Manual-L3; tick conversion, rotation/CRC/stream/mechanics and board readiness are L2 receivers. |
| VADP CGA/EGA/VGA | `vadp.c`; one guest port/memory/state owner produces copied snapshots, with CGA/EGA and unselected VGA capability retained. | guest writes -> VADP state -> copied frame -> VM presentation; no renderer state feeds back. | No guest-wakeup deadline. Selected digital mode/state behavior is Manual/Other-L3 as its ledger states; raster dot-clock, contention and presentation cadence remain L2/excluded. |
| HDC | `hdc.c`; explicit ATA PIO, Compaq WD and IBM WD1003 personalities share one Core controller/media boundary. | One IRQ14 source and one logical-media provider; no profile owns HDC register state. | Blocks while non-idle. Supported logical personality behavior is source-backed L3; IBM 5160/Xebec and ESDI lack profile receivers (L0 support), while mechanical/MFM/ESDI service deadlines remain unimplemented L1/L2 rather than fabricated. |

## Deadline And Board-Input Reconciliation

`core_machine_capture_time_observation_private` remains the only deadline
composer. It publishes the earliest source-qualified PIT/RTC boundary through
a copied observation and makes DMA, KBC, FDC and HDC active work blockers;
PIC and VADP do not expose a guest-wakeup deadline. Profile construction may
copy a proven board value into the existing Core plan, but cannot directly set
live controller time or generate a VM-to-Core tick.

Every selected controller therefore has one function owner and one explicit
timing disposition. The retained receivers are controller-local refinements or
immutable board/profile contracts, not a request for a new scheduler, port
adapter, state mirror, media cache or renderer path.

`M5:T481:S3:DEADLINE-CLASSIFICATION:OK`

`M5:T481:S3:CONTROLLER-AUDIT:OK`
