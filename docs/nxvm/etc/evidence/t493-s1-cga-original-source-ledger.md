# T493 S1 IBM 5160 CGA Original Source Ledger

`M5:T493:S1:CGA-PRIMARY-SOURCES:OK`

## Accepted primary corpus

| Asset | SHA-256 | Review and selected authority |
| --- | --- | --- |
| `IBM_5160_Technical_Reference_APR83.pdf` | `18CD473851FDFE40C5BF2C7CCC870772857D84509D95ADA7880473CD802C63B8` | IBM 5160 board/adapter-selection context. It establishes the selected XT environment; it is not treated as a replacement for adapter register specification. |
| `IBM_Technical_Reference_Options_and_Adapters_Volume_2_Apr1984.pdf` | `B5BF24EA3E63082D5C637DB8B08469C6D4929B4B9F6B7B24C7A211338B42A15F` | IBM Color/Graphics Monitor Adapter primary programming authority for selected ports, B8000 display buffer, 6845 registers, mode/color/status controls, graphics and mode-change sequence. |

## Visual/OCR audit

The Options/Adapters file identifies Adobe Paper Capture as its producer. OCR is
therefore used only to locate pages. Rendered visual inspection confirms its
CGA contents page and printed CGA pages 3, 11 and 17: the chapter enumerates
controller, mode register, display buffer, timing generator, composite,
alphanumeric/graphics modes, mode/status/6845/color registers and change
sequence; page 11 gives the 640 by 200 and 16,000-byte facts; page 17 provides
the 6845 register table. The rendered pages are legible; one blank printed
page is not used as evidence.

## Source boundary

The next ledger may treat documented CGA register/memory/mode facts and stated
numeric geometry as Manual-L3. The physical timing generator, composite phase,
monitor/electrical waveform and undocumented 6845 behavior are not inferred
from this source. They require a named external L3 input or retain their lower
disposition. EGA/VGA are excluded from this IBM 5160 CGA task.

No external source code, firmware, media or local machine path is imported.
