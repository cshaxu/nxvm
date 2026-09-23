# M3 Source Consultation

Consulted for the concrete M3 design on 2026-09-21; baseline `2f6126d`.
Direct NESdev opens returned HTTP 403; search-indexed page text was available.
No downloaded document digest or live wiki revision is asserted. Existing
[reference register](hardware-references.md) retains source classification.

| Source | Inspected claim and design receiver |
| --- | --- |
| [PPU programmer reference](https://www.nesdev.org/wiki/PPU_programmer_reference) | Eight register mirrors, separate I/O latch and delayed PPUDATA read buffer; G03. |
| [PPU rendering](https://www.nesdev.org/wiki/PPU_rendering) | NTSC scanline/dot geometry, pre-render fetches, vblank at scanline 241 dot 1; G02/G04/G06. |
| [PPU scrolling](https://www.nesdev.org/wiki/PPU_scrolling) | Shared scroll/address state and pre-render vertical copy interval; G04. |
| [Sprite evaluation](https://www.nesdev.org/wiki/PPU_sprite_evaluation) | Overflow evaluation is an indexed OAM process, not a count of sprites; G05. |
| [Frame timing](https://www.nesdev.org/wiki/PPU_frame_timing) | PPUSTATUS reads near vblank suppress flag/NMI in particular phase windows; G06 must keep dot-specific cases. |
| [PPU power up](https://www.nesdev.org/wiki/PPU_power_up_state) | Initial write suppression distinguishes registers; reset/phase fixtures in G03/G06. |
| [Controller](https://www.nesdev.org/wiki/NES_controller), [electrical reading](https://www.nesdev.org/wiki/Controller_Reading) | Strobe-high reload, serial pad and open/unconnected lines; G09. |
| [DMA](https://www.nesdev.org/wiki/DMA) | OAM halt on read, alignment, 256 get/put pairs and last RMW page; G06. DMC is outside M3. |

Local inspected ABI: Common machine frame/driver/debug lifetime contracts,
Session command provider/handle_hotkey and frame-kind routing, KVM event and
hotkey registry. Current Core bus/CPU, driver/debug/cartridge and App composition
were inspected for actual integration gaps recorded in m3-design. These local
sources, not a presumed SoftPC capability, define the reuse boundary.

The indexed Controller Reading electrical table was additionally inspected to
freeze NES-001 D5-D7 open bus and unconnected D0-D4 zero, rather than leaving the
disconnected-port profile to implementation guesswork.

For T12 S3, the PPU rendering record was rechecked on 2026-09-21. It specifies
background fetch regions 321--336 and 1--256, two-dot nametable/attribute/low-
and high-pattern fetches per tile, transfer into the **high** eight bits of the
pattern shifters on each eighth dot, fine-X selection from the low eight bits,
and one shift per eligible dot. Core records a dot at its starting edge, so the
external eighth-dot transfer is represented at the end of Core's
`dot % 8 == 7` cycle; the local receiver observes the post-shift state. An
earlier low-half loading interpretation was corrected before it became the
pixel path.

For T12 S4, the same indexed rendering text was rechecked on 2026-09-21 for
the display-pipeline convention. It states that pixel selection occurs on every
background-fetch dot, that shifter reloads are reported at ticks 9, 17, through
257, and that physical video output is delayed so the first pixel appears during
cycle 4. The public cycle-zero discussion records that tools use more than one
horizontal numbering convention. MyNes therefore cannot equate its private
`dot == 0` frame-buffer write with the external cycle number without an explicit
translation and delay state. S4's timing-ROM oracle owns that translation; the
current direct sampler is not evidence that it has been implemented.

The public governance-method URLs referenced by repository rules were also
attempted through raw GitHub access and returned cache misses. They are not
claimed as read or applied skills; the readable repository rules govern this
delivery. No missing external guidance is substituted for a hardware source.
