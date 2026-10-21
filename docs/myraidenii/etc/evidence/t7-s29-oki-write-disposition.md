# M1 T7 S29 — OKI Write Disposition

## Scope

S26 established that the selected set reaches both local OKI ports while
generating no active voices or nonzero PCM.  This batch does not alter the
receiver, sample data, mixer or Board-to-Sound transport.  It makes the
existing Sound receiver report which existing branch accepted each write.

## Local contract

For each of the two existing OKI devices, Sound now aggregates only these
counts:

| Disposition | Existing receiver branch |
| --- | --- |
| sample command | bit 7 establishes a pending sample |
| voice selection | a valid following volume/mask selects one or more voices |
| stop | no pending sample and bit 3 stops the selected voices |
| rejected selection | a pending sample has invalid volume or sample bounds |
| ignored control | no pending sample and no stop bit |

The counters are private Sound facts copied by the existing Board observation.
They do not expose command bytes, sample indices, ROM data or mutable device
state.  A write has exactly one disposition in addition to the pre-existing
total-write count.

The existing direct-device regression now covers command, selection, stop and
reset.  One additional synthetic Z80 sequence covers rejected selection and
ignored control through the production write decoder; it expects the existing
`INVALID_STATE` result for ignored control rather than changing that behavior.

## Selected-set observation

The existing verified wrapper ran once per stable Ninja host width with its
existing 2,000,000-instruction cap.  The owner archive remained read-only;
each wrapper-owned staging directory was deleted, followed by removal of its
empty parent root.  No payload, command values, screenshots or raw trace were
retained.

| Aggregate | x64 | x86 |
| --- | ---: | ---: |
| Guest ticks | 33,283,441 | 33,283,441 |
| Final frame digest | `a06b9c216e1ce034` | `a06b9c216e1ce034` |
| YM address/data writes | 1,135 / 1,135 | 1,135 / 1,135 |
| OKI-1 / OKI-2 total writes | 373 / 373 | 373 / 373 |
| OKI-1 / OKI-2 sample commands | 0 / 0 | 0 / 0 |
| OKI-1 / OKI-2 voice selections | 0 / 0 | 0 / 0 |
| OKI-1 / OKI-2 stops | 373 / 373 | 373 / 373 |
| OKI-1 / OKI-2 rejected selections | 0 / 0 | 0 / 0 |
| OKI-1 / OKI-2 ignored controls | 0 / 0 | 0 / 0 |
| OKI-1 / OKI-2 voice starts | 0 / 0 | 0 / 0 |
| Generated nonzero PCM | 0 | 0 |

The per-checkpoint frame summaries and timing are also identical.  This is an
L2 observation of the owner-selected archive, not a title, gameplay, audible
audio or hardware-fidelity claim.

## Disposition

S25 already reconciled the Board byte-lane and Sound transport against the
pinned external reference.  S29 finds no local OKI command-decoder
discrepancy: every observed write follows the existing stop branch.  Altering
the decoder, sample mapping or transport merely to produce audio would be an
unsupported behavior change.  Any later audio work must investigate the
upstream Sound-program command/state path or establish a later game-state
observation; it must not treat these counts as proof of an audio defect.

## Verification

- Focused Board/Sound units passed on x64 and x86.
- The complete MyRaidenII unit label passed 12/12 on x64 and 12/12 on x86.
- The selected-set probe built and ran once per width; its summaries above are
  byte-for-byte equal.
- `git diff --check` passed.  No Shared component, public cross-product API,
  executable, protected media or asset release changed.
