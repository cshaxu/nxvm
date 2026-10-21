# M1 T7 S33 — Selected-Set Video Configuration Observation

## Scope

This batch adds one read-only Board aggregate for the existing successful
Video boundaries.  It distinguishes an unconfigured local Video owner from a
configured owner whose later frame/state semantics still require work.  It
does not retain guest state, ROM data, RAM data, register payloads, screenshots
or a second rendering route.

## Single owner and counter contract

| Aggregate field | Existing owner | Counted boundary |
| --- | --- | --- |
| `vblank_events` | Board advance | `video_advance()` reports a completed vblank service |
| `crtc_byte_writes` | Video | one accepted CRTC byte write |
| `sprite_flag_byte_writes` | Video | one accepted sprite-flag byte write |
| `sprite_latch_events` | Board | one successful existing sprite-buffer latch |
| `palette_commits` | Video/COP | one accepted full palette load |
| `tilemap_commits` | Video/COP | one accepted full tilemap load |

The aggregate is copied by `myraidenii_core_board_video_observation_get()`.
It is reset with its existing owner.  Failed validation and failed COP paths
do not increment it.

## Synthetic coverage

The existing Board smoke now proves the real production routes rather than a
parallel fixture:

- two mapped sprite-latch writes count two latches and reset to zero;
- ten accepted mapped CRTC bytes count ten, while rejected read/inspect paths
  do not count;
- three scheduled raster deadlines count three vblank services and reset to
  zero;
- existing COP DMA palette/tilemap success counts once, while the established
  invalid transfer paths do not add a count.

The direct Video tests retain their existing byte/array checks and add only
the success/reset checks for their own accepted operations.

## Bounded selected-set observation

The established wrapper staged the owner archive transiently and ran its
existing Coin/Start sequence through the already bounded 2,000,000-instruction
startup window.  It retained only the Probe's neutral aggregate output.

| Aggregate | x64 | x86 |
| --- | ---: | ---: |
| Executed / guest ticks | 2,000,000 / 33,283,353 | 2,000,000 / 33,283,353 |
| Frame digest / transitions | `a06b9c216e1ce034` / 2 | `a06b9c216e1ce034` / 2 |
| Vblank events | 115 | 115 |
| CRTC byte writes | 1,448 | 1,448 |
| Sprite-flag byte writes | 8 | 8 |
| Sprite latches | 110 | 110 |
| Palette commits | 110 | 110 |
| Tilemap commits | 110 | 110 |

Both widths have an identical nonzero configuration footprint.  Therefore the
previous ambiguity is resolved: the selected program does reach the existing
Board/Video configuration owners.  This does not prove a title screen,
accepted credits, gameplay, pixel-perfect rendering, or audio.  Any continued
static/incorrect presentation investigation belongs to the existing local
Video rendering/state semantics, not a missing CRTC/palette/tilemap/latch
attachment.

## Verification and cleanup

- x64 and x86 focused Board smoke passed.
- Complete MyRaidenII unit label passed: 12/12 on each width.
- x64 and x86 selected-set Probe aggregates above match exactly.
- `git diff --check` passed.
- The owner archive remained read-only.  Wrapper staging was deleted after
  each run; transient diagnostic logs are not retained.

This is L2 owner-ROM observation.  It changes no Shared component, external
asset, executable release, or gameplay qualification.
