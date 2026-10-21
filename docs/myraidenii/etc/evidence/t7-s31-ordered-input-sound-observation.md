# M1 T7 S31 — Ordered Input Sound Observation

## Scope

S28 established the existing Machine-driver route can deliver Coin press and
release followed by Start press and release.  S30 established that the Sound
program services the Board transport during no-input startup.  This batch makes
no source change.  It runs the existing Probe's existing ordered-input mode
against the same aggregate Sound observation.

## Schedule and boundary

The unchanged `--coin-start-sequence` mode uses the existing normalized
instruction-budget schedule:

| Edge | Instruction threshold |
| --- | ---: |
| Coin press | 5,000,000 |
| Coin release | 10,000,000 |
| Start press | 12,000,000 |
| Start release | 13,000,000 |

The existing Probe retained no input payload, guest RAM, command bytes, ROM
data, screenshot or raw trace.  The 20,000,000-instruction cap is containment,
not a conversion to an external frame schedule.

## Selected-set observation

One run per stable Ninja host width completed with all four existing driver
deliveries accepted.  The complete aggregate records match exactly.

| Aggregate | x64 | x86 |
| --- | ---: | ---: |
| Executed / guest ticks | 20,000,000 / 324,088,505 | 20,000,000 / 324,088,505 |
| Final frame digest | `ccad0e0867e39b9c` | `ccad0e0867e39b9c` |
| Frame transitions | 3 | 3 |
| Coin/Start edges delivered | 4 / 4 | 4 / 4 |
| Sound command-0 / command-1 reads | 2,070 / 1 | 2,070 / 1 |
| Sound command acknowledgements | 2,070 | 2,070 |
| RST18 transport accepts | 2,146 | 2,146 |
| YM address/data writes | 11,047 / 11,047 | 11,047 / 11,047 |
| OKI-1 / OKI-2 writes | 3,657 / 3,657 | 3,657 / 3,657 |
| OKI sample commands / selections / starts | 0 / 0 / 0 | 0 / 0 / 0 |
| OKI stop commands | 3,657 per device | 3,657 per device |
| Generated nonzero PCM | 0 | 0 |

The recorded frame checkpoints retain the same Sound-latch aggregate as S28;
this observation identifies neither the values nor a title/credit state.

## Disposition

The existing ordered Coin/Start route does reach the complete existing Sound
transport during the full bounded window, but it does not change the observed
stop-only OKI behavior.  This eliminates neither a later title/gameplay state
nor an unmodelled device dependency; it only says that changing the local
input delivery schedule or repairing the already-active Board transport is not
an evidenced way to create audio.

The owner archive stayed read-only.  All wrapper staging roots and temporary
aggregate logs were removed after both owned processes exited.  This is L2
owner-ROM observation, not gameplay, sound-fidelity or release qualification.

## Verification

- Existing Probe mode parsing and the four delivery edges were exercised on
  both widths.
- Output comparison is byte-for-byte equal across widths.
- Documentation/whitespace review passed.  No source, test, CMake, manifest,
  public interface, executable, protected media or Assets file changed.
