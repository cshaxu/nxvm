# M1 T5 S8 — Core-local OKI ADPCM State

## Scope

S8 replaces the prior direct nibble-as-amplitude diagnostic path inside the
existing MyRaidenII Core Sound owner. Each of its existing four-voice OKI
instances now owns a signed decoded signal and bounded adaptation step. Board
continues to supply the only guest time axis; PCM publication continues to read
the same one bounded queue.

## Evidence Grades

| Adopted fact | Grade | Evidence and boundary |
| --- | --- | --- |
| Each decoded nibble changes a clamped signed signal and a bounded 49-step adaptation state; step magnitude follows a 16 × 1.1^step family and the low three nibble bits select the documented transition class. | L2 | Independently inspected MAME revision `9071ca1a6d1f1625045054c6f1fd0346a7e5a511`, path `src/devices/sound/okiadpcm.cpp`, on 2026-10-20; BSD-3-Clause. It was used only as a behavior reference; no source block or lookup table is copied. |
| The concrete local `Q32` recurrence, bit-arithmetic delta derivation and per-call execution order. | L1 | Independently written deterministic implementation selected to avoid importing a decoder table. It is not a physical-chip, analogue-path or timing claim. |
| Phrase command and attenuation semantics. | L3/L2 | Unchanged S7 basis: the inspected OKI manual directly establishes attenuation steps; code ordering/reserved values remain L2. |

## Ownership And Contract

- `myraidenii_core_sound_oki_voice` is the one owner of its position, nibble
  phase, decoded signal and adaptation step.
- Starting a voice and reset both reinitialize signal and step. Stopping or
  reaching the phrase end suppresses it from the mixer without adding a second
  state route.
- `myraidenii_core_sound_oki_advance()` alone consumes nibbles and updates the
  decoder. The existing PCM mixer reads the current decoded signal and retains
  the S7 L1 gain mapping.
- No decoder table, external source, ROM byte, host stream, thread or public
  interface is retained or added.

## Regression Ownership

The existing `sound_smoke.c` executable gained a small actual-route sign
check: two synthetic one-byte phrases are started through the local Z80 map and
their copied PCM has opposite signs. It does not duplicate the decoder formula
as a test oracle. Existing reset, phrase selection, status, stop, attenuation,
queue and Board-forwarding tests remain their owners.

## Verification

On 2026-10-20, cached Ninja rebuilt the affected Sound and Board paths on both
widths. Focused Board/Sound rows passed 2/2 per width, then the complete
MyRaidenII unit selection passed 6/6 on x64 and 6/6 on x86. Documentation
governance and `git diff --check` passed. No external archive, host audio,
desktop or RDP test was run or counted.

## Limits

The current PCM publication occurs after a bounded board advance, so this S
does not claim sample-accurate interleaving of Z80/device execution and PCM
output. Resampling alignment, host delivery and actual selected-set music/effect
qualification remain later T5 receivers. This model is L2 even if those later
tests pass.
