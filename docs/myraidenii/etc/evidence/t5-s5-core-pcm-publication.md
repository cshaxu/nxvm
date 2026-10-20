# M1 T5 S5 — Core PCM Publication

## Scope

S5 adds the first bounded PCM output to the existing MyRaidenII Core Sound
owner. It is not host audio: no Lib PCM stream, worker, platform handle or
Machine adapter participates. `myraidenii_core_sound_advance()` remains the
only producer and is called only from the Board attachment's guest-tick
callback. Board forwards the copied queue through one read operation; it does
not mirror samples or own an output clock.

## Evidence Grades

| Adopted fact | Grade | Evidence and boundary |
| --- | --- | --- |
| Board time is 16 MHz and sound Z80 derives from 28.636363 MHz / 8 | L2 | Existing selected-board wiring ledger; retained from S2/S4. |
| MSM6295 is a four-channel ADPCM voice device with external sample storage | L3 | Inspected `oki-1994-voice-synthesis.pdf`, MSM6295 description and feature table. This supports channel/sample ownership only, not this implementation's waveform. |
| The selected map uses YM2151 and two OKI devices | L2 | Existing accepted MAME/board layout research; no source or table imported. |
| The initial square/nibble mixer waveform and its gain rule | L1 | Local deterministic diagnostic approximation. No inspected manual PDF supplied the exact analogue/decode formula or constants used to make it silicon-exact. |

No external source, decoder table, ROM byte, firmware byte or generated audio
was copied into the repository.

## Contract

- Fixed output is 48 kHz, interleaved stereo signed 16-bit samples.
- The private ring is 4,096 frames. On overflow it drops the oldest queued
  frames and accumulates the count until the next successful copy operation.
- `myraidenii_core_sound_pcm_copy()` copies and consumes an ordered prefix,
  returning the accumulated dropped count; reset clears queued and drop state.
- `myraidenii_core_board_pcm_copy()` only delegates to that Sound owner.
- The L1 mixer is intentionally diagnostic. It proves end-to-end guest-clock
  publication and state ownership, not faithful YM2151/MSM6295 sound, music,
  effects, gain, pan, filter or physical timing.

## Verification

Focused cached Ninja proof passed on 2026-10-20:

| Width | Build and tests | Result |
| --- | --- | --- |
| x64 | `myraidenii-core-sound-smoke`, `myraidenii-core-board-bus-smoke` | 2/2 passed |
| x86 | `myraidenii-core-sound-smoke`, `myraidenii-core-board-bus-smoke` | 2/2 passed |

The existing Sound smoke was extended rather than creating another harness. It
proves a mapped device state produces ordered non-silent stereo frames, reset
clears them, and a one-second guest-time burst has bounded queue/drop behavior.
The Board unit retains the public forwarding invalid-argument boundary. Full
MyRaidenII unit qualification is recorded by the closing S5 packet after this
frozen source is reviewed.
