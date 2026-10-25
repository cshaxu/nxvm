# T580 S9 — F65535 mode-12 copied capture

## Scope

This S owns a bounded L2 compatibility receiver for standard VGA mode 12:
four planar VRAM planes become one copied 640 by 480, 16-colour indexed
snapshot.  It is selected because PC110-QEMU documents the admitted genuine
firmware using mode 12 for Personaware output, while Open-Source-PC110
identifies the PC110 controller as F65535 with standard VGA behavior.

The receiver does not model the F65535 front panel, LCD electrical timing,
FRC/dither, TFT/DSTN panel behavior, or a physical scanout clock.  It makes no
claim that the real TFT firmware has reached mode 12 or completed POST.

## Contract

The receiver accepts only the standard geometry it implements:

- graphics enabled through the existing VGA graphics controller;
- 80 CRTC characters, yielding 640 pixels;
- 480 displayed scanlines; and
- a 40-word CRTC offset, yielding an 80-byte planar scanline; and
- VGA graphics-controller memory map 1, yielding the standard 64 KiB A0000h
  aperture.

The CRTC start address is a word address and is converted to its byte offset.
For every copied pixel, bit 7 through bit 0 from each of the four existing
planes forms the indexed colour.  The existing RGBI palette is reused.  Any
other F65535 graphics or panel state remains `UNSUPPORTED`; failure happens
before the destination snapshot is modified.

## Verification plan

The existing F65535 contract test programs that state through the public VGA
ports and writes a four-plane pixel through the public A0000 aperture.  The
existing MyPC110 display-route test repeats the same route through the actual
board/Core bus connection.  Existing Machine-frame coverage proves indexed
graphics uses the pre-existing Window graphics conversion path.

## Results

Both stable Ninja/ccache trees built the changed F65535, Machine conversion
and MyPC110 board-route targets. On x64 and x86 respectively, the focused
six-test set passed: Types layout self-test, F65535 contract, Machine frame,
MyPC110 display route, and both X86-NXVM manifests. The broader affected unit
regression also passed on both widths: 97 X86-NXVM unit tests and eight
MyPC110 unit tests.

The result proves copied standard-VGA mode-12 delivery through the existing
routes. It does not establish real TFT BIOS mode selection, LCD timing or
panel appearance, original-DSTN support, POST, Setup or guest boot.
