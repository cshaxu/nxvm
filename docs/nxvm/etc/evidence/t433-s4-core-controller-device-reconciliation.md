# T433 S4: Core Controller And Device Reconciliation

`M5:T433:S4:DEVICE-LEDGER:OK`

| ID | Current owner and lifecycle | Focused proof | Disposition |
| --- | --- | --- | --- |
| `CTRL-PIC` | `pic.c` owns IRQ source, priority/cascade, logical acknowledgement and reset. | `core-machine-pic-lifecycle-s4-smoke` | Conformant logical lifecycle; INTA waveform/spacing is explicit L2 transfer. |
| `CTRL-DMA` | `dma.c` owns binding, request, transfer and terminal state; `transaction.c` owns lifecycle. | `core-machine-dma-binding-token-smoke` | Conformant logical route; DREQ/DACK/HLDA phase is L2 transfer. |
| `CTRL-PIT` | `pit.c` owns counters/ports/output; arbitration advances it before PIC refresh. | `core-machine-pit-irq0-s2-smoke` | Conformant deterministic clock contract; oscillator phase is L2 transfer. |
| `CTRL-RTC-CMOS` | `rtc.c` owns CMOS/event/Register-C acknowledgement and IRQ8; readiness callback advances/reset deasserts. | `core-machine-rtc-cmos-s3-smoke` | Conformant selected logical route; host/physical clock conversion is L2 transfer. |
| `CTRL-KBC-NMI` | `kbc.c` owns 8042 FIFO/commands/IRQ1/12; `machine.c` owns selected NMI bridge and reset. | `core-machine-kbc-controller-smoke`, `core-machine-kbc-serial-cadence-smoke` | Conformant selected cadence/lifecycle; physical keyboard/controller delay is L2 transfer. |
| `CTRL-FDC` | `fdc.c` owns command/result/DRQ/IRQ6; DMA binding and readiness advance are Core-owned. | `core-machine-fdc-smoke` | Conformant selected logical service; flux/rotation and DRQ electrical phase are L2 transfer. |
| `CTRL-HDC` | `hdc.c` owns ATA PIO phase/IRQ14 and reset; media remains provider-owned. | `core-machine-hdc-smoke` | Conformant current logical ATA route; physical disk/ECC/MFM semantics are L2/TODO transfer. |
| `MEDIA-BACKING` | `media_interface.c` owns checked provider registry and controller binding lifecycle. | `core-machine-media-provider-smoke` | Conformant logical provider boundary; physical media identity remains L2 transfer. |
| `DISPLAY-VADP` | `vadp.c` owns CGA/EGA/CECG registers, aperture/raster state and reset; machine peripheral callback advances it. | `core-machine-vadp-text-status-smoke` | Conformant selected digital/raster route; monitor, contention and analog timing are L2 transfer. |

Current `machine.c` initializes and cold-resets every listed owner, and orders
arbitration, readiness and peripheral callbacks without a device-local
scheduler. T370/T404 supply the retained owner/capability corpus; the focused
replay above confirms the current routes. No contradiction, runtime change,
ABI change or artifact change was found.