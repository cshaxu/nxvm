# T467 S1 IBM CGA Function And Timing Checklist

## Sources, Form And Cross-Check Method

The primary source is IBM, *Technical Reference: Options and Adapters, Volume
2*, revised April 1984, Color/Graphics Monitor Adapter pp. 1--25 (PDF
pp. 42--66), local file SHA-256
`B5BF24EA3E63082D5C637DB8B08469C6D4929B4B9F6B7B24C7A211338B42A15F`.
It is a Paper Capture scan with an OCR text layer: direct rendered inspection
of PDF pp. 56--63 confirms the mode-control, colour-select, status and
mode-change material. OCR is used only to locate pages; the rendered scan is
the authority. The IBM chapter names the Motorola 6845; the unimported
Motorola MC6845 data sheet is the primary supplement for the chip's register
units and CRTC output relations.

Read-only comparison used 86Box `4fef696` (`src/video/vid_cga.c`) and PCjs
`c7f21b4fa` (`machines/pcx86/modules/v2/video.js`). Both provide an IBM-CGA
device and agree on the five normal port addresses, B8000h 16-KiB storage,
mode bits and 320/640 layouts. 86Box additionally chooses snow, wait-state
and composite models; PCjs deliberately exposes debugger-oriented readbacks
and a host-cycle retrace estimate. These are model decisions, not new IBM
facts. MAME's `mc6845.cpp` is a generic CRTC and has no selected IBM-CGA glue;
Bochs 2.6 and QEMU `hw/display/vga.c` are later VGA implementations. None of
those three can establish an IBM-CGA row. No external source is imported.

`Manual L3` below means the cited IBM/Motorola material defines the selected
digital fact. `Other L3` would require convergent external models for an
otherwise missing bounded fact; no row qualifies in S1. `fallback to L2` means
the source does not define an implementable selected profile/board contract.

## Checklist 1: Frozen Function And Timing Universe

| ID | Primary source and finite requirement | Cross-check result | Final disposition |
| --- | --- | --- | --- |
| CGA-R1 | IBM pp. 15--17, 22--25: 6845 index/data programming is the adapter's CRTC route; Motorola defines R0--R17 units and access class. | 86Box and PCjs retain the index/data route; MAME corroborates only generic 6845 sequencing. | Manual L3 for the CRTC grammar and documented register units. |
| CGA-R2 | IBM pp. 18--20, 22--25: 3D8h is a six-bit write-only mode-control register. Bits select 40/80 columns, graphics, monochrome/colour, output enable, 640 graphics and blink/background intensity. | 86Box and PCjs use the same six control bits. | Manual L3. |
| CGA-R3 | IBM pp. 18--19: 3D9h is the write-only colour-select register; it determines border/background and the documented graphics colour set. | 86Box and PCjs retain border, intensity and palette-set controls. | Manual L3. |
| CGA-R4 | IBM pp. 15--17, 20--21: 3DAh status is read-only; bit 0 reports a display-buffer-access interval and bit 3 vertical retrace. | 86Box/PCjs publish dynamic retrace status, but derive it from their own schedulers. | Manual L3 for observable bit meaning; exact phase conversion is CGA-T2 L2. |
| CGA-R5 | IBM pp. 15--17, 22--25: 3DBh/3DCh control the optional light-pen latch and R16/R17 expose its address. | 86Box models latch controls; PCjs models status-facing pen bits but not an NXVM profile input. | Manual L3 for register/interface meaning; CGA-T3 is the unselected input lifecycle. |
| CGA-F1 | IBM pp. 1--8: 40x25 and 80x25 alphanumeric modes, 8x8 character cells, attributes, cursor and display memory use are defined. | 86Box/PCjs implement text through the CRTC/start/cursor path. | Manual L3 for digital text layout and attribute effects. |
| CGA-F2 | IBM pp. 9--11: 320x200 graphics has four pixels per byte, the documented two 8-KiB even/odd display-bank arrangement, and colour-set rules. | 86Box/PCjs agree on the 320x200 packed/interleaved form. | Manual L3. |
| CGA-F3 | IBM pp. 11--12: 640x200 black-and-white graphics has eight pixels per byte and the documented even/odd display-bank arrangement. | 86Box/PCjs agree on the 640x200 packed/interleaved form. | Manual L3. |
| CGA-F4 | IBM p. 20: 160x100 is an available low-resolution mode only through special programming while the 40x25 alphanumeric setup supplies its base. | 86Box/PCjs choose their own mode-detection and renderer assumptions; IBM does not publish the complete programming tuple here. | Manual L3 for existence/boundary; complete generation falls back to L2. |
| CGA-F5 | IBM pp. 1--2, 22: adapter storage is 16 KiB and processor-accessible; the selected PC aperture is B8000h. | 86Box and PCjs use a 16-KiB B8000h buffer. | Manual L3 for capacity/aperture and digital addressing facts. |
| CGA-F6 | IBM pp. 3--6, 15--17: CRTC counts govern character/raster/display timing and start/cursor addressing; Motorola supplies their defined units. | 86Box recomputes raster from CRTC writes; PCjs derives display geometry from its register state. | Manual L3 for CRTC-to-raster relation; Core tick formula is CGA-T2 L2. |
| CGA-F7 | IBM p. 21: mode change requires disabling video, programming the mode/CRTC state, then enabling video. | 86Box/PCjs honour output-enable as display suppression; their intermediate scheduling differs. | Manual L3 for ordering and output-enable effect. |
| CGA-T1 | IBM pp. 3--6, 20--21: timing generator arbitrates display-buffer access and produces 6845/DRAM timing; status exposes horizontal/vertical observable intervals. | 86Box has a per-card timer; PCjs estimates from host CPU cycles; MAME, Bochs and QEMU do not supply IBM-CGA board timing. | Manual L3 for logical relation. The existing copied profile -> VADP timing declaration is Board-L3-capable; its current unsourced values are L2 data. |
| CGA-T2 | IBM pp. 3--6, 23--25: oscillator/signal and display-rate specifications describe adapter operation, but not the selected Core tick source, ISA arbitration or CPU wait-cycle formula. | 86Box's wait-state table and PCjs's cycle estimates disagree in mechanism; later VGA models are inapplicable. | Board-L3-capable receiver for a sourced external timing declaration; current default data and unmodelled ISA wait formula remain L2. |
| CGA-T3 | IBM pp. 1, 15--17: light-pen hardware interface and latch exist. | 86Box can synthesize a latch; PCjs has no selected NXVM-like input lifecycle. | fallback to L2 until an admitted profile supplies an input source, cancellation and consumer. |
| CGA-T4 | IBM pp. 1--6, 23--25: composite/direct-drive outputs, RF-modulator interface and monitor options are named. | 86Box adds selectable composite/RGB algorithms; PCjs renders to a browser; MAME/Bochs/QEMU do not define IBM monitor electrical behaviour. | Manual L3 for named interface facts; waveform, phosphor/composite and host presentation fall back to L2. |
| CGA-T5 | IBM pp. 22--25: port/memory/card facts are adapter-local, not a new VM/renderer owner. | All useful models retain private video state and a host rendering endpoint. | Project boundary: one VADP state owner and copied snapshot consumer; no external model creates a second owner. |

## S1 Result And Transfer

The frozen universe has 17 rows: 13 Manual-L3 digital/interface relations, no
Other-L3 claim, and one existing Board-L3-capable VADP timing-input path. Its
current default values remain L2 because they have no recorded source
provenance. Complete 160x100 generation (CGA-F4), the unmodelled ISA wait
formula (CGA-T2), light-pen lifecycle (CGA-T3), and physical/host monitor
rendering (CGA-T4) remain explicit L2. S2 audits exactly these identifiers
against the current VADP, memory, clock, profile firmware and copied-snapshot
path. It cannot turn L2 data into L3 from reference-code similarity.
