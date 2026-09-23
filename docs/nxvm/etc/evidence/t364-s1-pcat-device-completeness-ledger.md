# T364 S1: Selected PC/AT Device, Chip, Port, And Bus Completeness Ledger

## Scope and method

This ledger reconciles the selected default PC/AT profile only. It consumes the
exact port/route contract in T353, deterministic device/timeline evidence
T346--T354, and the retained primary-manual provenance in those records. It
does not select new hardware, import firmware, or turn deterministic order
into physical bus timing.

| Selected component or surface | Core/profile owner and route | Lifecycle and current timing owner | Evidence and disposition |
| --- | --- | --- | --- |
| CPU memory, A20, RAM and immutable ROM | `memory.c`, `memory_interface.c`, KBC output and port `92h`; checked physical route | Machine transaction/cold reset; immediate publication after validated route | T354 selected-L3 ledger; exact wait/refresh remains bus-timing receiver. |
| CPU port fabric and selected leaves | `port.c`/`port_interface.c`; validated default-PC/AT descriptor | Provider registration/finalize and transaction trace | T353 exact leaf/route contract; no generic chipset claim. |
| 8259A master/slave PIC | `pic.c`; PIT IRQ0, KBC IRQ1/12, RTC IRQ8, FDC IRQ6, ATA IRQ14 | PIC request/source lifecycle and arbitration callback | T349 S4; physical INTA/spurious timing transfers to bus task. |
| 8254 PIT | `pit.c`, ports `40h--43h`, channel 0 callback | Deterministic `DMA -> PIT -> PIC` timeline order | T350/T346; channel-2/PPI/speaker is absent and transferred. |
| CMOS/MC146818 RTC and NMI mask adapter | `rtc.c`; ports `70h/71h`, IRQ8 | Readiness callback after FDC/ATA; register-C acknowledgement and reset owner | T350; parity/I/O-check producer remains absent. |
| Dual 8237A DMA and page leaves | `dma.c`; primary, secondary and page-port leaves; FDC channel 2 | DMA selection/transaction lifecycle before PIT | T348/T353; HOLD/HLDA and DREQ/DACK waveform timing transfers to bus task. |
| 8042 KBC, keyboard and compatible AUX | `kbc.c`; ports `60h/64h`, IRQ1/12 | Peripheral callback after RTC; FIFO/source/reset owner | T351/T346; advanced AUX remains TODO. |
| VADP CGA/EGA digital surface | `vadp.c`; selected sparse CRTC/mode/status leaves and copied frame | Peripheral callback after KBC; VADP reset/capture owner | T352/T346; host pacing, analog/composite and bus contention transfer. |
| FDC and floppy route | `fdc.c`; sparse `3F0h--3F7h`, IRQ6/DRQ2 | Readiness service and DMA binding/cancellation owner | T347/T348/T353; mechanical duration/waits transfer to service/bus tasks. |
| ATA/HDC PIO route | `hdc.c`; `1F0h--1F7h` plus `3F6h`, IRQ14 | Readiness service, task-file and reset/cancellation owner | T347/T353; rotational/cache/bus-master breadth transfers. |
| Profile/session/firmware composition | default profile descriptor and VM session composition | Validated topology precedes machine publication; reset reaches core owner | T353 S2/S4 and T354; no firmware or host-time ownership. |

## Absent or intentionally incomplete surfaces

| Surface | Disposition and earliest receiver |
| --- | --- |
| Port `61h` PPI/speaker and PIT channel 2 wiring | `TODO(Medium)` PC/AT speaker/PPI `61h`; requires selected corpus, IBM contract, deterministic event/reset owner and copied host-audio boundary. |
| Parity/I/O-channel-check NMI producer | Next Queue PC/AT NMI source-ownership task; requires one selected board source, latch/clear/mask/reset/timeline contract. CMOS bit 7 remains mask-only. |
| Serial, parallel and game ports | Existing `TODO(Low)` receiver; each requires a selected controller corpus and hardware route/lifecycle contract. |
| Physical memory/I-O waits, HOLD/HLDA, INTA, prefetch/cache, pin phases | Next Queue bus-timed PC/AT operation; no duration is inferred from deterministic callback order. |
| Device mechanical/service durations | Next Queue device service-timing corpus; existing FDC/ATA readiness is logical service, not calibrated latency. |

## S1 conclusion

Every selected component, leaf, IRQ/DRQ route, reset/lifecycle owner and
deterministic event owner has a named source/evidence record. The two required
prerequisites for later work are now explicit: NMI source ownership must first
choose a board producer, and bus/service timing must consume the existing
transaction and timeline owners rather than adding a parallel clock. No
selected device is unclassified; no absent device is represented as a provider.
