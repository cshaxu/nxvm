# T7 S72 — SEI252 Multi-Tile Order

## L2 Basis And Local Change

The local renderer traversed multi-tile descriptors row-major.  Two independent
external emulator implementations describe the selected SEI252 descriptor
stream as column-major: the local MAME snapshot's `raiden2_v.cpp` has SHA-256
`F585712A41366EF4D43FABA049C00F9D1E02AD13C37B42869A65F45E3AA2D558`, and
the local MiSTer snapshot is commit
`27021027c5c3db226c5e77a27c5390022b8aa1f2`.  They are L2 behavioral sources
only; no source, table, comment, ROM, PAL, decryption data or code was copied.

`myraidenii_core_video_sprite_draw()` now derives each source tile as
`x * height + y`.  Descriptor decode, flip coordinate selection, buffer order,
palette, transparent-pixel handling and the sole compositor are unchanged.
The existing graphics smoke adds a 2x3 synthetic descriptor whose second row
must select tile four (green).  The former row-major path selected tile two
(blue), so the assertion rejects the old algorithm without using an external
asset.

## Other Findings From The Whole-Repository Run

The initial full x64/x86 run found no MyRaidenII rendering regression.  It
instead exposed unrelated test failures:

- Native MIDI returned an I/O error on this host. A proposed relaxation that
  accepted every I/O error was withdrawn because it could mask a real failure;
  the original integration contract remains in place and needs investigation.
- `x86-nxvm.verifier-negative` exceeded 30 seconds in the full run; a proposed
  timeout change was withdrawn as unrelated to the sprite-order work.
- The MyNES native-Window integration had a center-pixel failure on x64. A
  proposed sampling change was withdrawn pending stronger evidence of cause.

## Verification

The focused MyRaidenII graphics, sprite-buffer and sprite tests pass on x64 and
x86. The full repository-only unit suite passed 697/697 on each architecture
(x64 117.75s; x86 117.79s). The generated CTest metadata used a temporary
60-second verifier timeout, but each verifier completed in 23.21s or less,
within its original 30-second limit. Earlier complete runs passed 736/736 on
each width with the subsequently withdrawn MIDI relaxation: those runs do not
establish that the original integration suite passes. The focused tests were
rerun after the final graphics fixture edit.

No external ROM/PAL was opened or staged by this S.  No selected-set runtime,
desktop/RDP gameplay, audio playback or artifact-release claim follows from
this unit/integration qualification.
