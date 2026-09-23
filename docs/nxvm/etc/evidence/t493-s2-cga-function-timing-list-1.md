# T493 S2 IBM 5160 CGA Function/Timing List 1

`M5:T493:S2:CGA-LIST-1:OK`

## Scope and reading rule

This is the finite selected IBM Color/Graphics Monitor Adapter universe for
T493: the color I/O block, its Motorola 6845 programming model, B8000 display
buffer, IBM-documented text and graphics output, reset-visible state and the
copied-frame boundary. The source is the accepted IBM Options/Adapters asset
from [S1](t493-s1-cga-original-source-ledger.md). Page references below are
the printed CGA pages visually checked from that asset, not OCR text.

`Manual-L3` means the row's guest-visible function or formula/value is stated
by IBM. It does not claim monitor, electrical, composite, or host-wall-clock
fidelity. `L1` records an architecture-required relation for which this source
does not specify a reset value or physical timing; it is not an L3 estimate.

## Function and state ledger

| ID | Capability and required behavior | IBM visual authority | Disposition and later owner |
| --- | --- | --- | --- |
| C1 | `3D4h` is the write-only 6845 index register; its five low data-bus bits select one of the other 18 registers. | CGA 15 | Manual-L3, VADP port/state owner. |
| C2 | `3D5h` writes the register selected through `3D4h`. | CGA 15-16 | Manual-L3, VADP port/state owner. |
| C3 | R0 horizontal total is write-only; supported-mode values are `38h`, `71h`, `38h`. | CGA 17 | Manual-L3 register semantics and values. |
| C4 | R1 horizontal displayed is write-only; supported-mode values are `28h`, `50h`, `28h`. | CGA 17 | Manual-L3 register semantics and values. |
| C5 | R2 horizontal sync position is write-only; supported-mode values are `2Dh`, `5Ah`, `2Dh`. | CGA 17 | Manual-L3 register semantics and values. |
| C6 | R3 horizontal sync width is write-only; supported-mode value is `0Ah`. | CGA 17 | Manual-L3 register semantics and value. |
| C7 | R4 vertical total is write-only; supported-mode values are `1Fh`, `1Fh`, `7Fh`. | CGA 17 | Manual-L3 register semantics and values. |
| C8 | R5 vertical total adjust is write-only; supported-mode value is `06h`. | CGA 17 | Manual-L3 register semantics and value. |
| C9 | R6 vertical displayed is write-only; supported-mode values are `19h`, `19h`, `64h`. | CGA 17 | Manual-L3 register semantics and values. |
| C10 | R7 vertical sync position is write-only; supported-mode values are `1Ch`, `1Ch`, `70h`. | CGA 17 | Manual-L3 register semantics and values. |
| C11 | R8 interlace mode is write-only; supported-mode value is `02h`. | CGA 17 | Manual-L3 register semantics and value. |
| C12 | R9 maximum scan-line address is write-only; supported-mode values are `07h`, `07h`, `01h`. | CGA 17 | Manual-L3 register semantics and values. |
| C13 | R10 cursor start is write-only; supported-mode value is `06h`. | CGA 17 | Manual-L3 register semantics and value. |
| C14 | R11 cursor end is write-only; supported-mode value is `07h`. | CGA 17 | Manual-L3 register semantics and value. |
| C15 | R12/R13 start-address high/low are write-only and select the displayed memory start. | CGA 17 | Manual-L3 register semantics; List 2 must map the word-address consumer. |
| C16 | R14/R15 cursor-address high/low are read/write registers. | CGA 17 | Manual-L3 register semantics; VADP owns cursor state. |
| C17 | R16/R17 light-pen high/low are read-only registers. | CGA 17 | Manual-L3 register semantics; input/latch producer is separate from VADP state. |
| C18 | `3D8h` is the write-only six-bit mode-control register: it selects 40/80-column text, text/graphics, color/monochrome, output enable, high-resolution graphics and blink/background-intensity behavior. | CGA 19-20 | Manual-L3, VADP mode-state owner. |
| C19 | `3D9h` is the write-only six-bit color-select register; it selects the documented color/palette inputs used by medium-resolution graphics. | CGA 18, 10-11 | Manual-L3, VADP color-state owner. |
| C20 | `3DAh` is a four-bit read-only status register: bit 0 reports regen-buffer access without display interference, bit 1 is the light-pen trigger, bit 2 reflects the light-pen switch, and bit 3 reports vertical retrace. | CGA 20-21 | Manual-L3 logical status meanings. The physical phase that produces the bits is outside this source's numeric contract. |
| C21 | `3DBh` address-activated write clears the light-pen trigger; `3DCh` address-activated write presets it. | CGA 15, 21 | Manual-L3 latch operation; external light-pen input is not synthesized. |
| C22 | Mode changes follow: choose the mode; clear video enable; program 6845; then program mode/color and re-enable video. | CGA 21 | Manual-L3 logical ordering. No undocumented inter-write delay is invented. |
| C23 | The display buffer starts at `B8000h`, is 16 KiB, has no parity, and is both alphanumeric display buffer and graphics bitmap. | CGA 5, 22 | Manual-L3 aperture/function; VADP is the sole guest-memory and frame-state owner. |
| C24 | In high-resolution alphanumeric mode, CPU access to the display buffer is restricted to horizontal-retrace intervals; other modes permit concurrent CPU/controller access, with possible interference outside horizontal retrace. | CGA 5 | Manual-L3 contention rule. Exact interval scheduling is L1 until a sourced timing-plan input exists. |
| C25 | Text cells are two bytes: character code then attribute. Low-resolution text is at most 40 by 25, needs 2,000 bytes and uses an 8 by 8 box; high-resolution text is at most 80 by 25, needs 4,000 bytes and uses an 8 by 8 box. | CGA 6-8 | Manual-L3 text geometry/data format. Character-ROM/font-jumper emulation is excluded from selected output work. |
| C26 | Text attributes define foreground RGB/intensity and background RGB; bit 7 supplies blinking foreground. | CGA 6-7 | Manual-L3 logical attribute interpretation; blink cadence is not numerically specified here. |
| C27 | Medium-resolution graphics is 320 by 200 with four colours total; each byte encodes four two-bit PELs. | CGA 9-10 | Manual-L3 geometry and packed-pixel format. |
| C28 | Medium-resolution graphics uses two 8,000-byte banks: even scan lines at `B8000h` through `B9F3Fh`, odd scan lines at `BA000h` through `BBF3Fh`; `B8000h` is the upper-left PEL. | CGA 10 | Manual-L3 memory map and scan-line selection. |
| C29 | The two medium-resolution palette sets are green/red/brown and cyan/magenta/white; the remaining code selects one of 16 background colours. | CGA 11 | Manual-L3 palette relation. |
| C30 | High-resolution graphics is 640 by 200, black-and-white only, requires 16,000 bytes, uses the same mapping as medium resolution, and maps each memory bit to one PEL. | CGA 11 | Manual-L3 geometry, memory and pixel format. |
| C31 | The adapter has a timing generator for the 6845 and dynamic memory, including processor/graphics-controller display-buffer contention. | CGA 6 | Manual-L3 existence only; no clock rate, formula, or waveform is stated on this page set, so physical timing is L1 rather than estimated. |
| C32 | A copied display snapshot is the project presentation boundary: guest port/memory writes update VADP state, then VADP publishes a copy for VM presentation. | Project architecture; CGA 5, 22 establish the guest state being captured. | L1 architecture boundary for this task; it must not become renderer-owned guest state. |
| C33 | Reset must establish one deterministic VADP-owned guest state before capture or port use. IBM's selected CGA pages do not state a power-on register image. | Source absence after CGA 15-22 review | L1 reset requirement; List 2 must distinguish current deterministic reset from any claim of IBM power-on values. |

## Explicit boundaries

- Composite colour generation, monitor/electrical waveform, light-pen switch
  producer, character-ROM jumper/font selection, undocumented 6845 behavior
  and a physical raster-clock rate are outside this finite CGA universe.
- The existing Core timeline may supply a real, source-qualified external timing
  input in a later admitted task. Until then C20, C24, C26 and C31 retain the
  stated logical or L1 boundary; exact arithmetic cannot upgrade an absent
  source premise.
- S3 maps C1--C33 exactly once to port, memory, state, snapshot and test paths.
  S4 then repairs every in-scope List 2 defect in one VADP-owner batch under the
  owner-approved immediate-repair rule.
