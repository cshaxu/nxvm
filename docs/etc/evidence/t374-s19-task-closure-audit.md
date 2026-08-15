# T374 S19: Selected-Device Functional Closure Audit

## Original Request And Decision

The owner required selected-machine functional work before timing/L3 and then
selected raw `.img` as the sole floppy format, explicitly retaining 765
Deleted/Control-Mark and Scan functionality as TODO. This audit therefore
tests whether T374 completed every admitted selected functional repair and
made that owner-approved support boundary visible; it does not redefine raw
IMG as full 765 support or claim Model-339 L3.

## Selected-Scope Reconciliation

| T374 proposal scope | Accepted evidence | Audit disposition |
| --- | --- | --- |
| Model-339 profile, memory/ROM-visible reset and planar parity/NMI boundary | S1 inventory, T365/T366 retained evidence, Model-339 composition proof | Selected logical/reset contract retained; I/O-check NMI remains unselected. ROM/bus/reset phase transfers to T375. |
| PIC, PIT, DMA and RTC/CMOS function | S15 platform-controller audit and its cited focused proofs | Registers, reset, IRQ/DRQ route and acknowledgement are accepted; cadence, arbitration and board phase transfer to T375. |
| 8042/101-key keyboard/IRQ1 | S3 pulse reset, S17 audit, S18 native boundary | Command/reset/FIFO/IRQ1, native Set 2 and controller translation are accepted. AUX/IRQ12 and durations remain excluded. |
| FDC, 1.44 MB field-upgrade compatibility, IRQ6/DMA2 | S4--S7 FDC repair/audit evidence and S8--S10 media decisions | Ready/Sense, invalid VERSION, media lifecycle and selected ordinary raw-sector route are accepted. The field upgrade is not called original equipment. Service/rotation/DMA availability transfer to T375. |
| Deleted Data, normal-read Control Mark and Scan | S8--S10, `TODO.md` hardware/compatibility debt | Explicit owner-approved unsupported boundary: raw IMG cannot persist required mark metadata; Scan has its own future admission. These are not hidden FDC claims and are not a T375 timing receiver. |
| Digital CGA | S11--S14, S16 evidence | Selected ports, direction, retained register rows and no-EGA topology are accepted. Geometry, light pen, frame cadence and bus contention transfer to T375. |

## Cross-Scope And Receiver Sweep

The accepted descriptor excludes ATA/HDC, IBM MFM/ST-506, EGA/VGA and AUX from
Model-339 proof. The closure does not move those boundaries into T374. Every
selected availability, duration, board wait, arbitration, cadence and phase
question remains the exact receiver of the queued IBM PC/AT 5170 board and
device phase-timing closure (T375 on admission). The task does not make a
cycle-exact, board-timed or Model-L3 statement.

S18's final sweep found no old direct Set-1 injectors. Its focused KBC,
host-mapper, topology, Model-339 composition and VDM replays pass. The full
configured build remains blocked by the separately recorded stale
`vm_cpu_probe` and `vm_request_bridge_smoke` fixtures; that failure neither
proves nor hides a T374 selected-device gap.

## Closure Result

T374 may close as **functional selected-device closure only**. The raw-IMG
765 limitation remains a visible TODO with a new-format/lifecycle admission
condition; the task neither removes it nor counts it as implemented. T375 is
the next Queue receiver. Model-339 L3, DeskPro 386, PC/XT and Windows 3.1 are
not ready.
