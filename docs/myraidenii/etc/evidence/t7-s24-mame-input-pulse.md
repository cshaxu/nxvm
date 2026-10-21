# M1 T7 S24 — MAME Input-Pulse Black-Box Probe

## Scope And Grade

This is an L2 external black-box observation, not a local implementation
model.  It uses MAME 0.289's documented Lua I/O-port-field API and visible
screen API.  It imports no source, table, ROM, PAL, memory, audio, trace or
image payload into this repository.

The owner-supplied `raiden2.zip` remained read-only at the recorded inventory
identity.  The two already approved PAL inputs remained read-only under
`<external-assets>/media-myraidenii/raiden2/roms`. Each run sent
MAME configuration, input, NVRAM, snapshot and state output only to the owned
temporary `<external-assets>/media-myraidenii/t7-s24-mame` root.
That root and both transient Lua scripts were removed after the observation.

## Method

MAME's public Lua API exposes I/O-port fields by name and lets a script set a
digital field active with a nonzero value, then force it inactive with zero.
`clear_value()` instead removes the override. The probe first enumerated field
names after the assumed Start name failed. It identified these actual fields:

| Control | MAME port / field |
| --- | --- |
| Coin 1 | `:COIN` / `Coin 1` |
| Start 1 | `:SYSTEM` / `1 Player Start` |

The paired 600-frame runs used MAME 0.289, the same read-only ROM/PAL roots,
`-video none -sound none -nothrottle -skip_gameinfo -noreadconfig
-nowriteconfig -nonvram_save`, and a Lua frame-done callback.  The callback
kept pixels only in-process long enough to compute an Adler-32 summary and
then asked MAME to exit.  The pulse run set Coin 1 active on frame 300 through
329 and Start 1 active on frame 360 through 389.  The baseline run made no
programmatic input change.

## Results

Each case was run twice.  Within each case, both runs produced the same
visible-screen geometry and checksum:

| Case | Frame | Visible geometry | Adler-32 |
| --- | ---: | ---: | ---: |
| Baseline | 600 | 320 x 240 | `3666394548` |
| Coin/Start pulse | 600 | 320 x 240 | `1254578711` |

Thus this reference configuration has a repeatable visible difference after a
finite Coin/Start pulse.  This validates the test's *external input schedule*
as meaningful; it does not identify the resulting title/credit/game state,
provide a local Board register contract, or prove MyRaidenII should have the
same screen checksum. Local video, sound and gameplay qualification remain
open. This experiment establishes no missing hardware receiver or root cause.

## Disposition

S23's local Machine-driver pulse is electrically complete but visually neutral
under its existing bounded output summary. It presses Coin and Start together
at instruction-budget checkpoints; this external experiment presses them
sequentially at frame checkpoints. These are not equivalent stimuli or elapsed
guest times. No cross-emulator defect or missing COPX consumer follows from
the different observations. A controlled local baseline and sequential-input
observation is needed before assigning a production cause. This does **not**
justify a MAME-derived fallback, a ROM-specific screen test, a guessed COPX lookup, or
a second input route.

API references: [MAME input fields](https://docs.mamedev.org/luascript/ref-input.html)
and [screen devices](https://docs.mamedev.org/luascript/ref-devices.html).
Input identities are inherited from the existing inventory, not freshly
rehashed by this experiment. No production or test source changed; existing
deployed executables therefore require no rebuild for this documentation batch.

## Regression And Review

Both stable Release trees passed all 697 currently registered repository-only
unit tests, executed as the existing 652-case selection plus 45 MyNES cases.
The initial cache disabled MyNES; it was enabled in both existing trees and
its `mynes-unit-test-binaries` target was built before that added selection.
No source configuration, product EXE or asset was changed by this correction.

- x64: 651/652 initially passed in 328.60 seconds; the cleanup selftest could
  not create its junction in the restricted process. The unchanged test passed
  with the required filesystem permission (1.69 seconds). MyNES passed 45/45
  in 43.96 seconds.
- x86: 652/652 passed in 325.77 seconds; MyNES passed 45/45 in 40.71 seconds.
- Documentation governance and `git diff --check` passed. Actual-diff review
  found only this evidence, its index and the active packet changed. Production
  and test-source added/deleted/net lines are all zero.

These results do not include native desktop/RDP, physical audio, full external
integration or local gameplay acceptance. G04/G05 remain unqualified. In
particular, previous records' missing-COPX inference is not a demonstrated
cause of the unchanged local input observation. The next diagnostic must
establish a concrete receiving-path discrepancy before implementing a fix.
