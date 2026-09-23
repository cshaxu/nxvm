# T370 S1: PC/AT Device-Service Owner Ledger

## Scope and source boundary

This is the pre-implementation ledger for the IBM PC/AT 5170 Model 339 / Type
3 baseline locked by T366: 8 MHz 80286, 512 KB planar RAM, CGA, the selected
FDC/IRQ6/DMA2 path and compatible TEAC 1.44 MB field upgrade, with no fixed
disk. It consumes T369's logical availability boundary and does not convert
the deterministic timeline into a physical clock.

Primary source categories are the [IBM PC/AT Technical Reference (September
1985)](https://www.bitsavers.org/pdf/ibm/pc/at/6139362_PC_AT_Technical_Reference_Sep85.pdf),
the [Intel 8237A data sheet](https://www.pcjs.org/documents/datasheets/intel/INTEL_8237A_DMA.pdf),
the [Intel 8259A data sheet](https://www.cs.umb.edu/cs341/Intel8259/I8259APIC.pdf),
the [Intel 8254 data sheet](https://www.cs.cmu.edu/~410/doc/8254.pdf),
the [Motorola MC146818A data sheet](https://www.ardent-tool.com/datasheets/Motorola_MC146818A.pdf),
the [Intel UPI-41/42 manual](https://www.ceibo.com/eng/datasheets/Intel-8041-Manual.pdf),
the [NEC uPD765A documentation](https://cpctech.cpcwiki.de/docs/upd765a/necfdc.htm),
and IBM Options and Adapters material for the selected CGA surface. They
determine controller protocol/topology only unless the device clock and its
mapping to a project domain are separately qualified.

86Box, MAME and PCjs are not used for a numeric S1 result: their available AT
defaults/configurations do not identify this locked 512 KB Model-339 plus field
upgrade configuration, and no primary-source conversion has yet selected a
project clock domain. They remain qualitative cross-check candidates only if a
later primary contract leaves an eligible same-profile range or no range.

## Complete owner matrix

| Selected device | Command/state/externally visible event owner | Current timing and lifecycle route | Determinate source fact | S1 timing disposition and next receiver |
| --- | --- | --- | --- | --- |
| 8254 PIT / IRQ0 | `pit.c` owns ports `40h--43h`, counter state and channel-0 output; `pic.c` owns IRQ0 publication. | Arbitration callback advances DMA, then PIT, then PIC refresh; machine reset/finalize resets/finalizes both owners. | 8254 has three programmable counters and a programmed counter output; IBM assigns the PC/AT timer/PIC topology. | The core clock-domain ratio is deterministic but not yet a source-labelled Model-339 timer conversion. S2 owns PIT source-to-domain reconciliation and output/ack trace proof. |
| Dual 8259A PIC | `pic.c` owns IRR/ISR/mask/priority/cascade, logical vector acknowledgement and ports `20h/21h/A0h/A1h`. | IRQ providers assert/deassert source handles; arbitration refresh follows DMA/PIT; reset clears both controllers. | 8259A selects/prioritizes a vectored interrupt and cascades; logical acknowledgement is not a physical INTA waveform. | S2 owns command/acknowledgement lifecycle and source-to-visible ordering. Physical INTA spacing/waveform remains phase refinement. |
| MC146818 RTC/CMOS / IRQ8 | `rtc.c` owns ports `70h/71h`, register-C acknowledgement, calendar/event state and IRQ8 source. | Readiness callback advances RTC after FDC/HDC refresh; reset/finalize deasserts its PIC source. | MC146818A defines calendar, periodic/alarm/update event state and register-C acknowledgement; IBM assigns the PC/AT RTC/CMOS route. | S2 owns selected RTC event/domain reconciliation. No host-clock or retirement-tick conversion is admitted. |
| Dual 8237A DMA | `dma.c` owns request binding, controller selection, transfer validation/publication and termination; `transaction.c` owns transaction lifecycle. | Arbitration callback owns DMA opportunity; T369 supplies logical 80286 HOLD request/ack/release; reset/finalize clears controllers/bindings. | IBM supplies dual-controller/cascade topology and 8237A supplies DREQ, EOP, mode and terminal-count semantics. | S3 owns service/terminal-count visibility and FDC DMA2 interaction. Pin-level DACK/AEN/HRQ/HLDA timing remains phase refinement. |
| 8042/KBC and keyboard | `kbc.c` owns ports `60h/64h`, command/data FIFO and IRQ1; selected keyboard protocol enters through its bound controller owner. | Peripheral callback advances KBC after readiness; reset/finalize deasserts IRQ1/IRQ12 and clears owner state. | IBM/UPI documentation determine host-status/command interface and self-test/keyboard protocol context. | S4 owns command response, FIFO/IRQ visibility and controller-clock admission. Exact controller/keyboard response delays are not presently mapped. |
| Planar parity / NMI | `machine.c` Model-339 planar-parity state owns selected parity latch, port-`61h` clear/control and CPU NMI delivery input. | Transaction/memory state triggers parity refresh; cold reset clears latch and mask-related state. | T366 selects planar RAM parity only; CMOS bit 7 masks but is not a producer. | S5 audits commandless fault visibility/reset as a device-service boundary. Adapter I/O-channel-check parity remains unselected. |
| CGA/VADP digital display | `vadp.c` owns selected CGA aperture/register/raster/copy state; product consumes copied frames only. | Peripheral callback advances VADP after KBC; reset/finalize clear adapter state and copied consumer boundary stays outside guest time. | IBM adapter material determines selected digital CGA register/aperture context. | S5 owns raster/status and command-to-visible-state ledger. Scan cadence, display enable, bus contention and analog/composite behavior remain explicit transfers. |
| uPD765-compatible FDC / IRQ6 / DMA2 | `fdc.c` owns ports `3F0h--3F7h`, command/result phase, DRQ binding and IRQ6 source; `dma.c` owns DMA2 transfer. | Readiness callback runs FDC advance then refresh; later arbitration can observe the DMA request; reset/finalize deassert DRQ and IRQ6. | IBM selects IRQ6/DMA2 topology; uPD765A defines command/result status and controller timer classes. | S3 owns command-to-DRQ/IRQ state machine and a source-labelled controller-domain proposal. T366 already prohibits translating its millisecond timers through retirement ticks without such a conversion. |

## Explicit exclusions and transfers

| Surface | Reason and receiver |
| --- | --- |
| ATA/HDC | Current product capability but not a selected Model-339 device. It transfers to the later current-product device-capability L3 candidate; it is not IBM MFM/ST-506. |
| IBM fixed disk MFM/ST-506 | No selected fixed disk in this baseline; retains the existing MFM/ST-506 TODO admission. |
| PPI/speaker channel 2, serial, parallel, game port, unselected ISA adapters | Not selected Model-339 components; retain their existing TODO/corpus admissions. |
| Physical READY/wait, INTA, DREQ/DACK/AEN/HLDA waveforms, prefetch and bus phase | T369 deliberately leaves these source-undefined physical facts to selected-profile phase refinement after device service work. |
| Host wall-clock pacing and protected guest media | Prohibited by the task and source policy. |

## Similar-route sweep and S sequence

The S1 static sweep is:

```text
rg -n "core_machine_(pit|pic|rtc|dma|kbc|fdc|vadp|planar_parity)_(initialize|reset|finalize|advance|refresh|bind|request|.*irq)|core_machine_(arbitration_tick|readiness_tick|peripheral_tick)|CORE_MACHINE_TRACE_(PIT|PIC|RTC|FDC|KBC|VADP|DMA)" src/core/machine tests/core tests/machine
rg -n "core_machine_(pit|pic|rtc|dma|kbc|fdc|vadp|planar_parity)_(initialize|reset|finalize|advance|refresh|bind|request|.*irq)" src/core/machine/machine.c src/core/machine/pit.c src/core/machine/pic.c src/core/machine/rtc.c src/core/machine/dma.c src/core/machine/kbc.c src/core/machine/fdc.c src/core/machine/vadp.c
```

It finds one implementation owner for every selected device and no device-local
parallel scheduler. Focused existing smokes cover the controller, port,
lifecycle and deterministic-order boundaries; they do not establish physical
duration.

The bounded continuation sequence is:

1. S2: PIT/PIC/RTC command/event-domain reconciliation and copied trace proof.
2. S3: dual-8237A/FDC request, terminal/result, reset and controller-clock-domain reconciliation.
3. S4: 8042/keyboard command/FIFO/IRQ timing-domain reconciliation.
4. S5: planar-parity NMI and CGA/VADP visible-state/raster ledger, then a task-level transfer audit.

Every unit must retain a source-undefined conversion as a transfer instead of
selecting a project tick, reference-emulator delay or host time.
