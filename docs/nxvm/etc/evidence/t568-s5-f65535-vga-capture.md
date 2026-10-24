# T568 S5 — PC110 F65535 Standard-VGA And Text Capture

## Delivered boundary

S5 extends the standalone F65535 owner with a deliberately bounded
standard-VGA compatibility surface.  It owns standard VGA register storage,
the four 128 KiB display-memory planes, the selected standard CPU aperture and
one copied text snapshot.  MyPC110 remains only a Core route adapter: it maps
the standard VGA register ports and `A0000h--BFFFFh`; Core keeps the sole
port/memory dispatch owner.

The supported capture subset is alphanumeric text.  It reads the active
standard VGA memory-map selection, odd/even or planar CPU access, display
start, CRTC character geometry, plane-0 character cells, plane-1 attributes
and plane-2 glyph bytes.  It publishes through the already-existing
`x86_video_snapshot`, so it does not create a second guest frame, renderer or
Lib presentation contract.

## Evidence and model grade

The C&T F65535 Revision 2.0 reference establishes F65535 VGA compatibility,
the standard VGA port set and the controller's 512 KiB display memory.  The
Open Source PC110 live dump records PC110 mode 03h using the standard VGA
register region, 80 columns, `CRTC 09h=4Fh` (16 raster rows), a 400-line
vertical display end and graphics-controller `06h=0Eh` for B8000h text
storage.  Those interface facts are L3.

The controller's internal standard-VGA behavior is an L2 compatibility model.
It is informed by the observed register/memory state and used only where this
receiver can directly prove a deterministic text frame.  It does not claim
silicon-accurate latches, write modes, graphics scanout, DAC timing, palette
precision, panel compensation, TFT/DSTN timing, power behavior or a booting
PC110.

## Direct proof

The chip contract programs the observed mode-03 geometry, writes a plane-2
glyph through its normal VGA aperture, writes a character/attribute pair at
B8000h, and proves a copied 80x25, 16-raster-row text frame.  It also proves
generation acknowledgement and rejects a graphics-mode capture rather than
publishing a false text frame.  The MyPC110 route test performs the same
register and B8000h path through Core, proving the App has no private memory
or port shortcut.

Both cached Ninja widths run the chip and App route tests plus the X86-NXVM
manifest/dependency gates.  This is direct component proof only; it is not a
raw-firmware boot, panel, desktop or RDP qualification.

The complete repository-only unit corpus was also run from the stable cached
Ninja trees after the direct proofs: x64 passed 660/660 in 149.69 seconds and
x86 passed 660/660 in 169.77 seconds (four workers).  This confirms that the
bounded controller addition leaves the existing component and App unit graph
intact; it does not substitute for the unimplemented PC110 firmware/panel
integration described below.

## Explicit remaining receiver

The current MyPC110 composition does not yet install a board display provider
that captures F65535 output into the Machine adapter's existing frame
publication route.  That receiver must choose a neutral board/Machine bridge
without pretending the controller is the PC/AT VADP.  It also owns any later
panel mode, graphics scanout and externally supplied firmware validation.
