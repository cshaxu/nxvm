# T580 S8 — PC110 display receiver selection

## Evidence reviewed

The following inputs were read as research only; no third-party source or
asset bytes are imported:

| Input | Revision | Relevant observation | Grade |
| --- | --- | --- | --- |
| Open-Source-PC110 `Discovery/65535/readme.md` | `02ccf2e8e19b298a2cf04c5d430278eb087cc368` | Identifies PC110 U51 as F65535, reports live XR00=`C1h`, standard VGA text mode and the controller's 640x480 flat-panel/CRT role. | L2 external live-capture report |
| PC110-QEMU README | `bae390ebf21665ec97860d28e26ec46f1fe8f4e9` | Documents genuine-ROM use of mode 12h (`640x480x16`) for Personaware output. | L2 external behavior reference |
| PC110-EMU | `81235b5fdf7d17ae180fc5cb6148e44bcbb87b4c` | Has a host renderer and broader synthetic controller behavior. | Research lead only; not an implementation source |
| Current MyPC110 F65535 route | current source | Owns standard VGA ports, `A0000h--BFFFFh` aperture and copied text capture; it does not capture a graphics frame. | Direct repository fact |

## Current boundary

`x86_f65535_capture_snapshot()` deliberately returns `UNSUPPORTED` as soon as
the standard VGA graphics bit is active. That is the missing machine-owned
receiver: the current F65535 component accepts standard VGA register and VRAM
state but has no copied mode-12 scanout. It is not a Lib or Emulator limitation:
the existing X86-NXVM snapshot/frame route already carries indexed pixels,
256-entry palettes and dimensions through `720x512` into the existing Window
graphics frame.

The front status LCD, power rails, panel FRC/dither and physical TFT/DSTN
electrical timing have separate owners and are not included. The first receiver
is only the standard VGA mode-12 copied framebuffer needed for the real PC110
software-visible display path.

## Selected next receiver

The next bounded implementation receiver is **X86-NXVM F65535 standard VGA
mode-12 (`640x480x16`) capture**:

1. Add one X86-NXVM snapshot kind and its explicit geometry; retain all enum
   values and reuse the existing indexed pixel/palette storage.
2. Decode the existing F65535's standard VGA planar state into that copied
   frame, preserving destination-on-failure behavior.
3. Extend the existing F65535 and Machine frame tests, then add a MyPC110
   App wiring proof. No host renderer, LCD electrical model, FRC emulation or
   ROM-address branch is admitted.

This is L2: external reports establish the selected machine/mode, while the
VGA planar conversion is a bounded compatibility model. It does not claim a
physical F65535 scanout or panel-timing reproduction.

## Disposition

S9 may implement the selected receiver in X86-NXVM and MyPC110. The external
front-LCD/power presentation remains outside this receiver. The original-DSTN
firmware input and storage/guest rows remain independently gated.

## Verification

S8 is research-only. The immediately preceding S7 source baseline passes the
complete repository-only unit suite on x64 and x86; S8 changed no source or
build input.
