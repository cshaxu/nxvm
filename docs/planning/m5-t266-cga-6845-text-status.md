# M5 T266: Digital CGA / 6845 Text-And-Status Closure

## Scope And Rules

T266 closes existing VADP text, CRTC, snapshot, and status behavior. It does
not create another video model, host-clock path, renderer policy, graphics
mode, or VM-side raster state. VADP owns guest CRTC, VRAM observation, raster
phase, and copied snapshot facts. Profile firmware and guest code use the same
frozen port/memory route.

## S1: Contract And Failing Corpus

**Status:** complete.

The admitted CRTC fields are 0Ah/0Bh cursor shape, 0Ch/0Dh display start, and
0Eh/0Fh cursor address. Cursor start/end scanlines are inclusive; the 0Ah
disable bit wins, and a non-hidden one-scanline cursor is visible. Address
fields are normalized on port write/readback. A cursor outside the visible
page beginning at display start is invisible and must not acquire a wrapped or
truncated on-screen coordinate.

Copied snapshot and frame coordinates mean `cursor_x=column` and
`cursor_y=row`, relative to display start. The prior inverted names are an
existing contract defect: `vm/composition/session/display.c` only copies the
payload, while Win32 Console, Win32 window, and Linux Console consumers
interpret the inverted axes. S2 changes those consumers' coordinate use only,
preserving visible placement without changing renderer state or product policy.

3DAh bit 0 reports display enable and bit 3 vertical retrace. VADP phase is
advanced only by the core scheduler's frozen VADP rational-clock delta. Reset,
active-display, blank, and retrace phases are repeatable; a status read is
pure for CGA. Retained EGA attribute-controller flip-flop reset is a separate
port side effect and remains unchanged.

### Similar-Issue Sweep

Defect class: inverted copied cursor coordinate semantics and cursor facts
derived inconsistently from the CRTC image. S1 searched tracked production
source and current tests with:

`rg -n "cursor_x|cursor_y|cursor_visible|CURSOR_TOP|CURSOR_BOTTOM|3dau" src tests`

Production hits: VADP is the source owner; the VM composition adapter copies
the snapshot; Win32 Console, Win32 window, and Linux Console consume the old
axis convention. All are in scope and will be migrated together. No second
guest cursor or raster owner exists. Existing EGA 3DAh attribute-controller
reads are retained, not cursor-coordinate hits.

The core corpus will cover masked CRTC readback, cursor disable and inclusive
shape, visible and off-page cursor coordinates, display-start wrapping, and
pure/repeatable 3DAh phase. A VM system fixture will prove the ROM/guest port
route. Stop if any correction needs raw VADP access outside core, platform
guest mutation, host time, or unbounded 6845 geometry.

**S1 closure:** VADP already owns the only CRTC image, raster phase, text
snapshot, and B8000h route. QDCGA already writes 3D4h/3D5h through its frozen
binding. The audit found three dead public VADP setters that bypassed this
route, plus the inverted snapshot axes and platform consumers that deliberately
compensated for them. The setters have no consumers and are S2 removals; the
copied coordinate consumers are an in-scope compatibility migration, not a
second renderer or state owner. Existing T221/T256 timing probes retain
step/reset/rational-clock replay evidence for this task's status phase.

## S2: Core-Owned Correction

**Status:** complete.

Modify VADP and the required copied-frame coordinate consumers only; QDCGA
remains a port client. The active patch normalizes CRTC fields, removes the
unused setter escape hatches, computes visible cursor coordinates relative to
display start, and leaves 3DAh's retained EGA flip-flop behavior untouched.

**S2 closure:** CRTC port writes now normalize the admitted cursor and address
fields. VADP derives inclusive/disabled/off-page cursor facts from its CRTC
image and tracks the copied facts for dirty detection. The three unused setter
APIs were removed. Win32 Console, Win32 window, and Linux Console now consume
the same copied `column,row` coordinates; no platform module receives VADP,
VRAM, or raster access and visible placement is retained.

## S3: Evidence And Closure

**Status:** complete.

Focused evidence passed:

- `core-machine-vadp-text-smoke` retains the corrected legacy text path.
- `core-machine-vadp-text-status-smoke` proves CRTC masks, inclusive and
  hidden cursor shape, off-page suppression, display-start wrapping, reset and
  pure 3DAh active/blank/retrace sequencing.
- `vm-no-media-video-port-smoke` proves the real default-ROM INT 10h/BDA/CRTC
  route reaches a visible copied cursor at column 0, row 6.

The retained T221 timing-checkpoint and T256 rational-clock corpus cover
step/reset and clock-domain replay; retained EGA controller smoke covers 3DAh
attribute flip-flop reset. `current-gates-gcc` passed **102/102** CTest,
including DOS prompt/video, graphics, keyboard/mouse, FDD/HDD, Console,
debugger, and session regressions. The rebuilt
`build/output/nxvm_0_5_0266.exe` SHA-256 is
`4A9BADD8FFD8F26F4EC2DAA2D576C7D24D0AEA0072F6A5AD2D5ABCC15960928A`.
It embeds the developer identity `Neko's x86 Virtual Machine [0.5.0266]`.
The source commit is `81864ae`.

The post-fix similar-issue sweep found no remaining production consumer of the
removed VADP setter APIs and no remaining inverted cursor coordinate consumer.
Deferred scope remains additional CRTC geometry breadth and light pen.
