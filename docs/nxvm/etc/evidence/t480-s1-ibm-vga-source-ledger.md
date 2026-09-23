# T480 S1 IBM VGA Source And Cross-Model Ledger

`M5:T480:S1:VGA-SOURCES:OK`

## Sources And Limits

| ID | Source | Use and limitation |
| --- | --- | --- |
| P1 | [IBM Personal System/2 Hardware Interface Technical Reference, VGA Function](https://www.bitsavers.org/pdf/ibm/pc/ps2/42G2193_PS2_Hardware_Interface_Technical_Reference_Video_Subsystems_Sep92.pdf), VGA Function chapter (also text-verified in IBM document mirror). | Primary controller documentation. The available scan/OCR is legible for the selected register, aperture, DAC and Mode 13h facts; a table that cannot be legibly checked remains unclaimed. It describes PS/2 VGA function, not any existing NXVM profile/card route. |
| E1 | Read-only 86Box `src/video/vid_svga.c`. | Corroborates DAC sequencing, sequencer chain-4 selection and graphics-controller aperture decode. It also contains SVGA adaptations, so it cannot broaden IBM VGA scope. |
| E2 | Read-only PCjs `machines/pcx86/modules/v2/video.js`. | Corroborates port names and intended chain-4 plane selection, but its chain-4 implementation explicitly says actual hardware needs confirmation; it never qualifies that detail by itself. |
| E3 | [QEMU `hw/display/vga.c`](https://github.com/qemu/qemu/blob/master/hw/display/vga.c). | Corroborates aperture, latch and plane-mask structure but labels its chain-4 path simplified. It is not implementation authority. |
| E4 | [Bochs `iodev/display/vgacore.cc`](https://github.com/bochs-emu/Bochs/blob/master/bochs/iodev/display/vgacore.cc). | Corroborates retained sequencer/DAC state and port family. It was not used to infer undocumented board timing. |
| E5 | MAME. | No specific IBM VGA controller implementation was located in the available read-only corpus or direct source search. This is an absence, not evidence of a different behavior. |

## Finite Controller Ledger

| Row | IBM controller fact | Cross-model disposition | T480 classification and receiver |
| --- | --- | --- | --- |
| V1 | VGA has four 64 KiB maps, a CRT controller, sequencer, graphics controller, attribute controller and DAC (P1, VGA Function pp. 2-5--2-9). | E1/E3 retain one controller state with registers and VRAM; E2 has a card controller. | Manual L3 controller state. S2 must map it to the one VADP state; no peer VGA object. |
| V2 | Attribute access uses an index/data flip-flop and palette-enable state (P1 attribute-controller programming sequence). | E1 retains one `attrff`; E2 uses the same port family. | Manual L3. S2 audits existing VADP attribute port/reset-read handling. |
| V3 | Sequencer Map Mask selects writable maps; Memory Mode chain-4 selects map by the low two address bits (P1 pp. 2-84 and 2-32). | E1 implements both; E2 names this VGA-only rule but records unresolved real-hardware chain-4 layout. | Manual L3 for selection semantics; exact present-VADP address transformation requires S2 source-to-code proof, not E2 inference. |
| V4 | Graphics Miscellaneous memory-map field selects A0000 128/64 KiB, B0000 32 KiB or B8000 32 KiB windows (P1 pp. 2-85--2-86). | E1/E3 implement those four windows. | Manual L3 aperture selection. S2 audits VADP mapped-memory ownership and rejects a VM-side map. |
| V5 | Read latches and write modes 0--3 are defined controller behavior (P1 pp. 2-84--2-88). | E1/E3 implement latches and write modes; neither changes P1 meaning. | Manual L3. S3 owns the existing EGA-common latch/write path; incomplete rows remain source-labelled rather than replaced by a 256-colour shortcut. |
| V6 | DAC mask, read/write address and three-component palette data use ports 3C6h--3C9h; DAC values are six-bit components (P1 VGA DAC programming). | E1/E3 implement three-byte auto-increment state; E2 names the same port/state sequence. | Manual L3. S4 adds it only inside VADP and publishes palette values through the copied snapshot. |
| V7 | Mode 13h is 320x200, 64,000 sequential bytes at A0000h and displays 256 colours from the DAC (P1 p. 2-22). | E1/E2/E3 have 256-colour paths; their renderer choices do not alter the controller fact. | Manual L3 Mode-13h controller semantics. S4 owns a VADP 256-colour snapshot; no renderer-held framebuffer. |
| V8 | CRTC word/byte and doubleword fields affect map/display addressing (P1 VGA mapping discussion after p. 2-88). | E3 explicitly warns its memory-access simplification differs from real display fetches. | Manual L3 register meaning; S2 must separate CPU aperture semantics from generated-frame geometry. Do not infer a complete raster model from E3. |
| V9 | Dot-clock and sequencer display fields affect derived display timing (P1 p. 2-49). | E1 recalculates internal timings; E3 has host-oriented display timing. | Manual L3 only where P1 supplies a usable controller formula and Core's common time axis can consume it. Otherwise L2/unsupported; no host-frame-rate guess. |
| V10 | IBM documents VGA in PS/2 hardware, not an IBM 5170, DeskPro Model 40 or `default-at` VGA card/ROM route. | No cross-model source changes this selected-machine fact. | Unsupported profile binding. S5 may bind only a later immutable, source-qualified profile/card declaration. |
| V11 | VBE, SVGA extensions, analog waveform/monitor fidelity and option-ROM behavior are outside P1's selected IBM VGA controller surface. | External implementations contain extensions or product-specific paths. | Unsupported by T480. |

## S1 Result

P1 supports a bounded IBM VGA controller contract. It does not support a current
machine claim. S2 receives the one-owner code-gap audit; S3 receives common
EGA/VGA state; S4 receives only V6--V9's source-qualified VGA-specific work;
S5 receives the separate immutable profile/card admission decision.

`M5:T480:S1:VGA-CROSSCHECK:OK`

`M5:T480:S1:PROFILE-BOUNDARY:OK`
