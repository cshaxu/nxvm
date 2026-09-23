# T503 S1 Controller Signal-Chain Ledger

`T503-S1-SIGNAL-CHAIN-LEDGER`

## Scope and method

This is the finite connection ledger for T503.  It consumes the already
complete chip List 1/List 2 records named below; it neither substitutes an
emulator for a primary source nor reopens chip-internal command work.  A route
is accepted only when its Core owner, frozen board edge, consumer and normal
acknowledgement/removal path are all identifiable.  An active device is not a
guest-wakeup deadline merely because its per-tick maintenance callback runs.

The primary-source bases are the listed Intel, Motorola, NEC, ATA and IBM
records, reconciled by the retained controller audits.  Their independent
86Box/MAME/PCjs/Bochs/QEMU observations are corroboration only where a board
edge is ambiguous.  In particular, the Model-40 two-1.2-MB-mechanism,
one-media construction is the retained PCjs External-L2 observation; it is
not represented as a Compaq manual fact.

## Route/state matrix

| ID | State owner and complete production route | Normal / mask / reset / absence-or-error / acknowledgement | CPU-wait/deadline disposition | S1 result and source basis |
| --- | --- | --- | --- | --- |
| R1 PIT/PIC | `pit.c` counter 0 OUT -> `shared_pit_irq0_source` in `machine.c` -> `pic.c` -> CPU scan/get. Counter 1 is separately bound through `machine_board.c` to DMA refresh only when selected. | PIC IMR masks delivery; PIT reset releases its output; absent selected PIT binding is construction-invalid; CPU INTA/EOI consumes PIC state. | PIT can publish the next qualified OUT boundary; PIC is same-tick arbitration, not an independent deadline. | Existing one path. Intel 8253/8254 plus IBM board ledger: T490 List 1/2 and T461; no connection repair in S1. |
| R2 DMA | `dma.c` request/mask/priority -> one `machine_scheduler.c` service/transaction route -> bound provider terminal/EOP consumer. | Request mask suppresses service; reset clears requests/state; absent binding is rejected by construction; provider terminal callback owns completion. | Active DMA blocks blind fast-forward; maintenance cadence is not a public deadline. | Existing one path. Intel 8237A/IBM and 86Box/MAME/PCjs/Bochs/QEMU reconciliation: T489 List 1/2 and T469. |
| R3 FDC | `fdc.c` command phase -> immutable DMA2 binding -> `dma.c` provider -> result FIFO/`pic.c` IRQ6 -> firmware reads result. Media registry remains byte owner. | DOR reset clears/deasserts; ready/media/write-protect errors form FDC result; result consumption deasserts the normal IRQ lifecycle; no medium never manufactures readiness. | Seek and byte-gate fields are Core-owned; source-qualified conversions can schedule them, while mechanics remain an external boundary. | Existing one path, including T496's normal-result IRQ6 release. NEC uPD765A/B + IBM: T492 List 1/2; Model-40 observation: T501 S2/S5. |
| R4 HDC | `hdc.c` personality command phase -> its immutable DMA binding where selected -> logical-media provider -> `hdc.c` DRQ/IRQ -> PIC/firmware. | Device/personality reset clears DRQ/IRQ; no media/error is HDC-owned completion; status/data acknowledgement clears its pending IRQ. | Active non-idle HDC blocks blind advancement; source-qualified service deadline is absent for mechanical/MFM/ESDI paths. | Existing owner route; S3 must test every selected ATA/Compaq/WD1003/Xebec personality rather than infer from ATA PIO smoke. ATA/Xebec ledgers: T450, T479 and T494. |
| R5 KBC/PPI | Selected XT keyboard device -> XT PPI IRQ1 receiver, or `kbc.c` FIFO/translation/output port -> PIC IRQ1/A20/reset -> Core CPU/memory. | PPI/KBC line and output-buffer consumption deassert IRQ; reset and BAT are device-owned; absent AUX is topology-gated; NMI remains its board owner. | Pending serial/response work blocks blind advancement; not a host-generated time path. | Existing one-way Core route. Intel 8042/IBM AT and IBM XT keyboard/PPI records: T450, T491 and T496. |
| R6 RTC/CMOS/NMI | `rtc.c` calendar/periodic/alarm state -> its PIC IRQ8 source; port-70 adapter owns NMI-mask observation and selected board NMI request. | Register C acknowledgement drops RTC IRQ; reset clears the source; unsupported RTC/NMI topology is construction-gated. | `core_machine_rtc_ticks_until_irq` is a candidate only under the copied qualified timing plan. | Existing one path. MC146818A/IBM records: T450 and T469; S4 will exercise IRQ8/NMI masking and acknowledgement as a batch. |
| R7 VADP | Guest ports and mapped memory -> sole `vadp.c` state -> `core_machine_vadp_capture_snapshot` -> copied VM presentation snapshot. | Output-disable/text fallback and absent personality remain VADP state; reset remains VADP-owned; no renderer writes guest state. | No CPU-wakeup route is claimed; presentation/raster cadence is not a scheduler substitute. | Existing one-way snapshot route. IBM CGA/EGA/VGA records: T450, T480 and T493. |
| R8 Core progression | Each eligible owner supplies its actual next observable condition to `core_machine_capture_time_observation_private`; VM only observes through the copied public observation and asks Core to advance. | Reset/cancellation clears owner state before observation; an active noneligible controller blocks rather than yields a fabricated deadline. | No profile or VM may inject or skip guest ticks. | One Core boundary exists. T469 and T481 identify PIT/RTC candidates and DMA/KBC/FDC/HDC blockers. The earlier XT no-deadline symptom and T501's now-removed D4 duplicate ROM owner are regression evidence, not alternate routes. |

## Complete disposition

All matrix rows have a current single owner and a traceable consumer route.
S1 identifies no new code repair that may be slipped into the audit.  The
remaining work is intentionally split by shared connection class:

- S2 verifies and, if needed, repairs R1/R8 clock, IRQ and refresh edges.
- S3 verifies and, if needed, repairs the complete R2--R4 DMA/service/terminal
  and IRQ acknowledgement class across every selected HDC personality.
- S4 verifies and, if needed, repairs R5--R7 board/firmware observation edges.
- S5 executes the complete four-profile route/state replay.  Any newly found
  chip-internal gap transfers to its earliest hardware-unit T; it is not a
  T503 patch target.

The retained physical Model-40 READY/change relation and mechanical service
latencies remain source-gated boundaries.  They are neither missing logical
connections nor permission to estimate a value.
