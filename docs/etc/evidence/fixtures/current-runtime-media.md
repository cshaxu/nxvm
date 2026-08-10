# Current Runtime Media

This record identifies the owner-provided local media approved for the current
runtime smoke configuration. The media remains read-only, excluded from Git,
and excluded from releases. It is distinct from the historical M1 baseline in
[M1 local image fixtures](m1-local-images.md).

## Current Approved Set

| Logical ID | Size (bytes) | SHA-256 | Declared role |
| --- | ---: | --- | --- |
| `runtime-fdd-msdos622` | 1,474,560 | `065038a5de3d64ebdf85477dd23778e27f6fa50647f335e54a1f3a2ae7e9437c` | Owner-declared MS-DOS 6.22 removable media for current runtime smoke input. |
| `runtime-hdd-ewin31-setup` | 51,609,600 | `61e5cdc0b76151cc65b73eb44094738b9de86052b1b07f20fc03205984cd77e1` | Owner-declared EWIN31 Setup hard-disk media for current runtime smoke input. |

The local aliases `fdd.img` and `hdd.img` select this approved set. Aliases
are not a distribution name, provenance claim, or replacement for the logical
identifiers above.

## Configuration Boundary

Operators supply these files through `PROJECT_FDD_SMOKE_IMAGE` and
`PROJECT_HDD_SMOKE_IMAGE` CMake cache inputs. The repository records no
absolute local path. A fresh configuration must inspect the resolved cache
values before running a media smoke; a missing or mismatched file stops that
run.

## Current Observation

A fresh configured run resolved both aliases through the repository-relative
asset root. The retained current gate passed 135 of 145 tests. Ten FDD/DOS
tests did not establish an MS-DOS 6.22 contract and remain failed evidence:
`vm-dos-prompt-smoke`, `vm-dos-keyboard-smoke`,
`vm-dos-mem-fault-smoke`, `vm-dos-video-port-smoke`,
`vm-cga-graphics-dos-smoke`, `vm-ega-planar-dos-smoke`,
`vm-rom-ega-int10-dos-smoke`, `vm-mouse-driver-dos-smoke`,
`vm-fdc-read-track-dos-smoke`, and `vm-ata-pio-dos-smoke`.

Those tests retain DOS 5 prompt/state assumptions; one prompt run also ended
with `0xc0000374`. This record does not claim DOS 6.22 guest compatibility or
weaken the retained corpus. The follow-up admission is recorded in `TODO.md`.

## Change Control

A hash, size, or declared-purpose change creates a new candidate media set.
It requires owner approval, an updated local-only record, and focused media
smoke evidence before becoming the current set. It cannot silently overwrite
the M1 MS-DOS 5 baseline or turn protected guest media into a repository asset.
