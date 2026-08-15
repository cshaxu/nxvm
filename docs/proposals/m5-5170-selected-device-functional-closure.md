# IBM PC/AT 5170 Selected-Device Functional Closure

## Purpose

Close functional completeness, before physical timing refinement, for every
device selected by the accepted IBM PC/AT 5170 Model 339/Type 3 capability
ledger. This candidate follows that ledger and precedes the 5170 board/phase
timing candidate; it does not itself make a Model-339 L3 claim.

## Required scope

Repair or explicitly remove each selected functional gap in the platform and
transaction foundation (memory/ROM, reset, NMI, PIC, PIT, DMA and bus-facing
state), AT keyboard/8042 input, FDC and selected floppy path, and CGA display.
For each selected component, establish its registers, observable state machine,
command/error/reset behavior, IRQ/DRQ visibility and acknowledgement, and
consumer path before assigning exact service duration or board wait values.
The FDC scope includes the selected 3.5-inch 1.44 MB aftermarket field upgrade
only as a controller/drive compatibility configuration; it must not be labelled
an original Model-339 shipment. The baseline retains no fixed disk, so ATA/HDC
and IBM MFM/ST-506 are not admitted here.

## Non-goals and stop conditions

No generic PC/AT expansion, EGA/VGA, AUX/mouse, ATA/HDC, MFM/ST-506,
pin-level waveform, unselected adapter, ROM/media import, or synthetic timing
value. Stop and transfer a gap if the ledger cannot show that the component is
selected, or if it needs a shared owner earlier than this candidate. A repaired
state machine is not evidence for a service-time or board-wait value.

## Evidence standard

Require one selected-device functional matrix, register/state and reset traces,
IRQ/DRQ and cancellation proof, a caller/owner sweep, focused regressions, and
an explicit handoff of every remaining availability/service/phase question to
the 5170 timing candidate.
