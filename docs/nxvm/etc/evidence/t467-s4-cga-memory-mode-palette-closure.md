# T467 S4 IBM CGA Memory, Mode And Palette Closure

Rendered IBM CGA manual pp. 18--22 defines B8000h's 16-KiB self-contained
buffer, 320/640 graphics colour-select bits, and video-enable mode bit. The
existing VADP provider and even/odd-row decode already implement the selected
16-KiB B8000h layout through one copied-snapshot path. This S closes two
actual digital omissions in that path: 3D9 bit 4 now selects the intensified
320x200 palette set, and disabled video publishes a blank text snapshot rather
than leaking text cells while graphics was blacked by its existing palette.

No memory mapping, renderer, mode mirror or second state was introduced. The
320/640 address formulas remain VADP-local and source-backed. 160x100 still
lacks a complete source programming tuple and physical/composite output lacks
a digital Core consumer, so both remain exact L2.

The focused CGA/VADP CTest group passes 4/4 after rebuilding
`core-machine-cga-graphics-port-smoke`; its regression covers both palette
sets, high-resolution interlace memory, and disabled-text output.

Coordinator review measures source/test changes as `+23/-3` (net `+20`): the
sole production owner changes by `+14/-3`, and its focused regression adds nine
assertion lines. No parallel path or state remains.
