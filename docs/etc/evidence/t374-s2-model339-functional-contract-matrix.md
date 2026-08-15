# T374 S2: Model-339 Functional Contract And First Repair Selection

## Source Boundary

The principal contract is IBM's *Personal Computer AT Technical Reference*,
revised edition, March 1986, document 6183355 / 6280099. The preserved manual
index identifies it as released with Model 319 and Model 339. Its controller
description says that command-port `F0h`--`FFh` pulses output-port bits 0--3
low, a zero command bit selects the pulse, and output-port bit 0 is connected
to system reset. It says the pulse is approximately six microseconds.

Stable locations: [March 1986 IBM 5170 technical reference](https://www.minuszerodegrees.net/manuals/IBM/IBM_5170_Technical_Reference_MAR86.pdf)
and [manual-edition index](https://minuszerodegrees.net/51xx_manuals.htm).
No manual, firmware, ROM, media, image or source is imported.

## Contract-To-Implementation Matrix

| Selected component | Primary functional contract and S1 comparison | Disposition |
| --- | --- | --- |
| Memory/ROM/reset and planar NMI | The baseline retains an abstract Rev.3 external-only slot. S1 proves selected descriptor/composition, mapping and parity test boundaries, but not a full selected board contract. | Later T374 functional matrix/repair; board reset phase remains timing work. |
| PIC/PIT/DMA/RTC/CMOS | IBM documents observable ports/topology; S1 maps core owners and focused lifecycle smokes. | Later T374 functional work; arbitration/cadence/service phase transfers to T375. |
| 8042 status/buffers, keyboard path and IRQ1 | IBM defines status 64h, output 60h, input writes 60h/64h, command byte and output-port access. `kbc.c` implements these paths plus D0h/D1h output-port read/write and IRQ1. | Baseline behavior present, but pulse-output commands are missing; select first repair. AUX remains excluded. |
| **8042 F0h--FFh command-port pulse** | `core_machine_kbc_write_command()` handles 20h, 60h, AAh, ABh, ADh, AEh, A7h, A8h, A9h, D0h, D1h and D4h, then ignores F0h--FFh. `core_machine_kbc_apply_output_port()` already owns bit-0 reset delivery. | **T374 S3:** decode pulse commands at KBC owner boundary. When command bit 0 is zero, request the same reset operation as D1h output-port bit 0 low; preserve output-port/A20 state. Other pulse bits have no selected NXVM consumer and remain no-op/unobservable. |
| FDC/aftermarket 1.44 MB and CGA | S1 maps selected state, IRQ6/DMA2 and VADP routes plus focused tests, but not full controller/drive or CGA mode/register/reset contracts. | Later T374 functional work; completion/rotation/DMA and cadence/contention transfer to T375. |

## First Repair Contract

S3 changes only `core/machine/kbc.*` and cohesive KBC smoke coverage. It must
prove command-port `FEh` (bit 0 clear) requests one CPU reset; `FFh` requests
none; the pulse does not persistently alter D1h output-port state/A20; and
existing D0h/D1h, keyboard IRQ1 and generic AUX behavior remain unchanged.
No tick value or microsecond conversion is introduced: pulse duration is later
timing work. This neither imports a ROM nor claims full 8042 or Model-339
functional closure.
