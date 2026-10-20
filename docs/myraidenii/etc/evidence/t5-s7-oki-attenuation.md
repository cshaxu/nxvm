# M1 T5 S7 — MSM6295 Attenuation Contract

## Scope

S7 corrects only the existing Core-local Sound owner's second-byte attenuation
handling. It neither changes the bounded PCM queue nor introduces a decoder,
host delivery route, external payload or Shared API.

## Evidence Grades

| Adopted fact | Grade | Evidence and boundary |
| --- | --- | --- |
| The MSM6295 phrase-selection second byte uses D0--D3 for attenuation from 0 dB through -24 dB in -3 dB steps. | L3 | Inspected *MSM6295 Voice Synthesis LSI* (OKI Semiconductor, 1994), printed page 71, PDF SHA-256 `582368ADD98884F605CF22CAD81FDF450358B5E73EF8BB050E2355E5F728B0AC`. This is the manual's direct attenuation rule; it does not supply a decoder formula or numeric-code ordering. |
| Codes 0--8 select the nine levels and codes 9--15 begin no voice. | L2 | Independently inspected published emulator behavior, without copying source or a table. The manual page establishes only the nine-level rule, so code ordering/reserved-code behavior is not promoted to L3. |
| The fixed-point `181/256` per-level gain ratio used by the existing diagnostic mixer. | L1 | A local deterministic approximation for a nine-step display waveform. It is not an imported gain table, physical output curve or ADPCM reconstruction algorithm. |
| Stop command channel bits are interpreted only when no phrase byte is pending. | L2 | The manual requires two fully latched bytes before phrase synthesis begins; published emulator behavior resolves the overlapping bit-3 encoding. The code retains one local command owner. |

## Resulting Contract

- Values 0--8 create a voice at the selected manual attenuation step.
- Values 9--15 consume the pending phrase selection but create no voice.
- A second byte following a pending phrase is always handled as that phrase's
  selection data before stop decoding, so attenuation value 8 is not confused
  with a stop marker.
- Stop masks use their channel bits only when no phrase selection is pending.
- The existing mixer remains L1 diagnostic PCM. It proves queue/clock/routing
  behavior, not MSM6295 waveform, ADPCM, delay, timing or gain fidelity.

## Regression Ownership

The existing `sound_smoke.c` executable now executes the actual local Z80 map:
it proves reserved attenuation 9 leaves the status quiet and valid attenuation
8 activates channel 1. It also starts channel 2 and stops exactly that channel
through the actual local Z80 map, rejecting the former shifted stop-mask
interpretation. Existing vectors retain mapped start/status, timer, reset,
transport and bounded PCM queue coverage. No second fixture or test harness
was added.

## Verification

On 2026-10-20, cached Ninja builds and focused rows passed on both widths:

| Width | Build targets | Focused rows | Complete MyRaidenII unit selection |
| --- | --- | --- | --- |
| x64 | `myraidenii-core-sound-smoke`, `myraidenii-core-board-bus-smoke` | 2/2 | 6/6 |
| x86 | `myraidenii-core-sound-smoke`, `myraidenii-core-board-bus-smoke` | 2/2 | 6/6 |

The documentation-governance gate and `git diff --check` also passed. No
external-set probe, host audio, desktop or RDP test was run or counted.

## Limits

This record does not qualify real sample playback, selected-set music/effects,
host PCM delivery, desktop output or a playable game. The rendered manual page
was task-local research under `build/myraidenii-s7-research/` and is removed at
S closure.
