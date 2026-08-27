# T492 S2 8272A / IBM 5160 FDC And Media Function And Timing List 1

`M5:T492:S2:8272A-LIST-1:IMPLEMENTED-PENDING-GOVERNANCE`

NEC source pages 5-11 and 5-17 and IBM printed page 1-167 were visually
checked.  The finite selected unit is the uPD765-compatible controller, the
5160 adapter and logical media semantics; drive mechanics and host files are
not silently included.

| ID | Function or timing obligation | Authority | Level |
| --- | --- | --- | --- |
| F1 | Access one read-only main-status and one bidirectional data register, selected by A0. | NEC 5-11 | Manual L3 |
| F2 | Use RQM and DIO together to determine when and in which direction a data byte transfers. | NEC 5-11 | Manual L3 logic order |
| F3 | Report four drive-busy bits, command busy and non-DMA execution in main status. | NEC 5-11 | Manual L3 |
| F4 | Execute command, execution and result phases in that order. | IBM 1-155 | Manual L3 logic order |
| F5 | Implement the selected common corpus: Read Data/Deleted/ID/Diagnostic, Write Data/Deleted/ID, Specify, Seek, Recalibrate, Sense Interrupt, Sense Drive Status and three Scan forms. | NEC 5-3, 5-13--5-17; IBM 1-155 | Manual L3 |
| F6 | Treat `Version` as revision-dependent: NEC A/B documents it; IBM 5160 names only 15 commands and no A/B die. | T492 S1 reconciliation | Explicit L0 selected-board behavior |
| F7 | Preserve command parameter/result ordering and returned ST0--ST3/CHRN fields for each applicable command. | NEC Table 4; IBM command summary | Manual L3 |
| F8 | `Specify` selects SRT/HUT/HLT and DMA versus non-DMA operation. | NEC 5-17 | Manual L3 |
| F9 | In DMA mode DRQ requests byte transfer; in non-DMA mode processor handshakes the data register and per-byte interrupt behavior applies. | NEC 5-3--5-5 | Manual L3 logic order |
| F10 | Terminal Count terminates Read/Write/Scan transfer as specified by the controller. | NEC 5-4 | Manual L3 logic order |
| F11 | Complete command operations signal INT; Sense Interrupt consumes the seek/recalibrate completion state. | NEC 5-4, 5-17 | Manual L3 logic order |
| F12 | Seek/Recalibrate maintain selected-drive cylinder state, seek-end and recalibrate/equipment-check outcomes. | NEC Table 3/4 | Manual L3 |
| F13 | Sense Drive Status returns ready, write-protect, track-zero, two-side, head and selected-unit state. | NEC Table 3; IBM 1-167 | Manual L3 |
| F14 | Report data/address/CRC/overrun/write-protect/end-cylinder/wrong-cylinder/control-mark/scan result conditions through command status. | NEC Table 3 | Manual L3 |
| F15 | Select up to four drives and use selected-drive readiness, write-protect, track-zero and side inputs. | NEC 5-3--5-5; IBM 1-151 | Manual L3 |
| F16 | Use FM/MFM, head, unit-select, index, read-data, write-data and write-precompensation controller signals at the device boundary. | NEC 5-4--5-5 | Manual L3 interface |
| F17 | Decode IBM adapter data `3F5h`, main status `3F4h` and digital output `3F2h`; no other selected XT FDC port is claimed. | IBM 1-167 | Manual L3 |
| F18 | DOR bits 0--1 select drive, bit 2 releases reset, bit 3 enables INT/DMA requests and bits 4--7 select drive motors; reset clears all bits. | IBM 1-167 | Manual L3 |
| F19 | Route selected FDC data transfers through DMA channel 2 and completion through IRQ6. | IBM I/O/IRQ map and adapter description | Manual L3 interface |
| F20 | Selected adapter supports one or two attached 5.25-inch drives, with a four-drive controller/adapter capacity. | IBM 1-151; NEC 5-3 | Manual L3 board topology |
| F21 | Selected IBM adapter is double-density MFM and supports programmable drive parameters plus write-protect. | IBM 1-151 | Manual L3 |
| F22 | Logical media exposes ID fields, sectors, cylinders, heads, EOT, GPL, DTL and format parameters only through the FDC command grammar. | NEC Table 4; IBM 1-156 | Manual L3 |
| F23 | DOR reset enters idle; ready state during reset produces the documented completion state consumed through Sense Interrupt. | NEC 5-4, Table 4; IBM 1-167 | Manual L3 logic order |
| T1 | Controller maximum clock is 8 MHz; its electrical clock relation is not a Core physical axis. | NEC 5-3--5-4 | L4/out of scope |
| T2 | Ready held during reset produces an interrupt within 1.024 ms. | NEC 5-4 | Manual L3 timing fact |
| T3 | Step rate is 1--16 ms in 1-ms increments, selected by SRT. | NEC 5-14 | Manual L3 timing fact |
| T4 | Head load is 4--512 ms in 4-ms increments; head unload is 0--480 ms in 32-ms increments. | NEC/IBM programming summary | Manual L3 timing fact |
| T5 | WCLK encodes the stated FM/MFM transfer-rate alternatives at 4 MHz and 8 MHz controller clocks. | NEC 5-4 | Manual L3 timing fact |
| T6 | Data-register handshakes, TC termination, command-phase transitions and interrupt ordering are source-defined causal order; the manual supplies no complete board-to-Core absolute schedule. | NEC 5-11, Table 4; IBM 1-155 | Manual L3 logic order |
| T7 | Spindle rotation, head settle beyond programmed values, analog PLL, write precompensation waveform and electrical setup/hold are not Core logical-media behavior. | NEC 5-4--5-10; IBM drive section | L4/out of scope |
| T8 | Host file latency, host cancellation and filesystem persistence are outside the controller/board source corpus. | Ownership boundary | External host boundary |

The 30 rows above are the complete selected FDC/media universe for T492. S3
must map every row to the current Core FDC/media owner or one coherent batch;
it may not create a profile-side controller or media mirror.
