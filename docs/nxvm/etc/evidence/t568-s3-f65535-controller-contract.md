# T568 S3 — PC110 F65535 Controller Contract Audit

## Result

The PC110 display controller is a distinct C&T F65535 device.  It must not
be represented by `x86-nxvm/chips/video` plus Board-Base `vadp`: that pair
owns the existing PC/AT CGA/EGA/VGA personalities, their memory vocabulary and
their snapshot contract.  Adding an F65535 selection there would claim that
the different extended-register, memory and flat-panel behavior were merely a
PC/AT personality.

The next receiver is therefore a new, self-contained
`x86-nxvm/chips/f65535` mechanism.  It owns the chip's register file, logical
display memory, reset values and one snapshot/capture result.  MyPC110 owns
only the physical port and memory wiring plus its fixed panel identity.  Core
Machine remains the only I/O/memory dispatcher, and the existing copied-frame
path remains the only host publication path.

This record admits no implementation.  In particular, it does not claim a
working BIOS, panel scanout, a display frame, a product EXE or a guest boot.

## Directly sourced controller facts

| Fact | Grade | Evidence and implementation consequence |
| --- | --- | --- |
| F65535 is a VGA-compatible flat-panel/CRT controller. | L3 | C&T *F65535 Revision 2.0*, introduction and register summary, pp. 1 and 33--34.  The chip must expose normal VGA state as one device, not a separate host-side compatibility layer. |
| Extension index/data are `3D6h`/`3D7h`; the extension index has seven usable bits and bit 7 reserved zero. | L3 | C&T manual p. 81.  The chip owns a seven-bit indexed extension register path. |
| `XR00` is read-only chip version at index `00h`; the PC110 live dump reads `C1h`. | L3 | C&T manual p. 81 and Open-Source-PC110 `Discovery/Live-Dump` PC110 port observation.  The PC110 configuration must report the observed identity rather than a guessed later stepping. |
| `XR02`--`XR05` include CPU/memory controls; `XR04[5]` is the CPU write-buffer control and the register reset values are documented. | L3 | C&T manual pp. 83--84.  S4 must retain raw register semantics even when an exact bus-performance model is outside the display receiver. |
| The standard minimum display-memory option is one 256K x 16 DRAM, i.e. 512 KiB. | L3 | C&T manual pp. 1, 8 and 176--178; Open-Source-PC110's physical reconstruction inventory independently lists original PC110 video memory as 512 KiB.  S4 uses exactly 512 KiB logical VRAM, not the reconstruction board's 2 MiB. |
| The F65535 supports flat panels, panel power sequencing, panel formats and vertical compensation. | L3 for capability, not selected PC110 values | C&T manual pp. 10--16 and 141 onward.  The manual describes the controls but does not prove which values the stock PC110 BIOS programs.  No S4 panel constant is admitted without selected-ROM observation or a labelled L2 model. |

## Existing ownership audit

`x86-nxvm/chips/video` has a PC/AT-oriented public vocabulary: CGA and planar
memory regions, EGA personalities, VADP CRTC/attribute/sequencer/graphics
configuration, and PC/AT board registration.  It has no F65535 extension
register bank, no 512 KiB F65535 display-memory contract and no truthful
flat-panel configuration owner.  Board-Base `vadp` additionally treats that
specific chip as the Board's scheduled display owner.  Reusing either as an
F65535 personality would create false hardware identity and make the new
controller depend on a PC/AT board facade.

No Lib or Emulator frame extension is admitted by this audit.  Existing frame
capacity covers the selected controller's indexed 640-by-480 target.  A
different pixel representation or a second renderer would require separately
demonstrated need and approval.

## S4 implementation boundary

S4 may add only the following X86 NXVM mechanism and matching unit corpus:

1. `x86-nxvm/chips/f65535`: lifecycle, 512 KiB logical VRAM, normal VGA and
   `3D6h`/`3D7h` extension-register ownership, reset and an inspection/capture
   contract.
2. `app-mypc110`: static PC110 port/memory routes and selected revision/panel
   input; it may not reinterpret registers, allocate a second framebuffer or
   choose a host presentation policy.
3. Tests for the sourced reset/identity/index/VRAM facts and route ownership.

The following remain later receivers: exact stock-DSTN/TFT programming tables,
flat-panel timing/compensation, panel power sequencing, guest BIOS mode
transitions and real-firmware graphical qualification.  QEMU, PC110-EMU and
other simulators may inform a visibly labelled L2 model only after the direct
manual/live facts above have been preserved.

## Rejected alternatives

- Do not add an F65535 enum/personality to VADP.
- Do not use the reconstruction board's 2 MiB as a stock-PC110 fact.
- Do not set panel registers from an unqualified BIOS or invent reset defaults.
- Do not publish a graphics frame through a second path or force a Window size.
- Do not commit, runtime-load or infer protected firmware/ROM bytes.

## Verification and remaining proof

This S is documentation-only.  It inspected the original local manual visually
at the extension-register and memory-control pages and reconciled it with the
PC110 live-dump material.  It runs documentation governance only; dynamic
hardware, guest, panel and desktop tests are intentionally not claimed.
