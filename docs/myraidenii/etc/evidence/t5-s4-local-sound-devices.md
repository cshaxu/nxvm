# M1 T5 S4 — Local Sound Devices

## Scope

S4 adds the two fixed OKI register paths and the YM register/status/timer path
to the one existing MyRaidenII Core Sound owner.  It does not add a Shared
chip framework, host audio delivery, a second guest clock, a mixing path, or
retained media.

## L2 Model

The independently implemented local model uses the fixed selected-board map
and clocks recorded from the inspected MAME Raiden II driver:

- YM address/data: `0x4008` / `0x4009`;
- OKI sample/status: `0x6000` and `0x6002`;
- YM timer A/B periods in Z80 ticks: `(1024 - A) * 64` and
  `(256 - B) * 1024`;
- two 0x40000-byte sample images; and
- an OKI nibble cadence of 462 Z80 ticks, derived from the selected clocks.

This is a deterministic L2 board model.  It is not a claim of silicon-accurate
YM synthesis, ADPCM decoding, mixer gain, analog output, or game-audio
qualification.

## Retained Ownership

Board remains the sole main-side transport and elapsed-time owner.  Sound owns
the copied Z80 program, copied samples, local register state, device time and
mapped reads/writes.  The selected-set probe supplies sample images through
Board configuration, releases its caller buffers immediately after Board
creation, and retains no ROM payload.

## Verification

- `myraidenii.core/sound-smoke`: x64 and x86 pass, including fixed map,
  YM timer status/IM1 delivery, dual-OKI start/status/stop, bounds and reset.
- All six MyRaidenII unit tests pass on x64 and x86.
- The external selected-set startup probe passes on x64 and x86 with the two
  sample roles staged transiently from the approved external archive.  Both
  runs report the same bounded Board observation; neither asserts audio output.

## Deferred

PCM mixing, Lib audio delivery, YM waveform synthesis, actual ADPCM output,
and observed selected-game music/effect completion remain later bounded work.
