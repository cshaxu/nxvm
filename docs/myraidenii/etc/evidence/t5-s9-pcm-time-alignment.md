# M1 T5 S9 — PCM and Device-Time Alignment

## Scope

S9 removes the previous PCM conversion that consumed an entire Board interval
after all device state had already advanced. It retains the single Board-derived
timeline, but emits queued PCM as each existing Z80/device slice advances.

## Clock Derivation

The existing Sound seam derives Z80 ticks from Board time as
`28636363 / (8 × 16000000)`. With a 48 kHz PCM queue, each delivered Z80 tick
therefore contributes `48000 × 8 / 28636363` frames. S9 keeps one fractional
phase accumulator for this ratio. This is an L2 composition of the existing
accepted selected-board clock model; it is not a physical bus-write phase or
chip-clock accuracy claim.

## Resulting Ownership

- Board remains the only source of elapsed time.
- Sound retains the only derived Z80-time/PCM phase and the only output queue.
- The existing device-advance operation now immediately feeds that same PCM
  phase. The obsolete Board-interval PCM conversion path is removed.
- Overflow skipping remains queue-local: device state already advanced along
  the sole timeline and no second device advancement is created.

## Regression Ownership

The existing Sound smoke's new synthetic phrase helper copies PCM from the
same call that begins and advances the local Z80/OKI path. It scans only the
bounded copied frames for the first nonzero sign, proving that a phrase begun
within that interval reaches PCM without requiring a later advance call. It
does not create a timing oracle, a second scheduler or external-media fixture.

## Verification

On 2026-10-20, cached Ninja rebuilt the affected Sound and Board paths. The
focused Board/Sound rows passed 2/2 on x64 and 2/2 on x86; the complete
MyRaidenII unit selection passed 6/6 on each width. Documentation governance
and `git diff --check` passed. No external archive, host audio, desktop or RDP
run was performed or counted.

## Limits

Instruction completion remains the local scheduling boundary. This record does
not claim sub-instruction bus-write phase, actual MSM6295 output cadence, host
audio delivery, selected-set music/effects, desktop output or playable-game
qualification.
