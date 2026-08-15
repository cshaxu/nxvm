# Current Runtime Media

This record identifies the owner-provided local media approved for the current
runtime smoke configuration. The media remains read-only, excluded from Git,
and excluded from releases. It is distinct from the historical M1 baseline in
[M1 local image fixtures](m1-local-images.md).

## Current Approved Set

| Logical ID | Size (bytes) | SHA-256 | Declared role |
| --- | ---: | --- | --- |
| `runtime-fdd-msdos500` | 1,474,560 | `fadeb3a27c6a0e1cf582dde0b9aecb7e5d30678f2f967f2f4562f167cc0cb1d5` | Owner-declared MS-DOS 5.00 removable media for current runtime smoke input. |
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

The former MS-DOS 6.22 FDD identity and its 145-of-145 gate observation are
historical only; they do not establish a result for this changed current set.
The current FDD now has the MS-DOS 5.00 identity above, while the current HDD
retains `runtime-hdd-ewin31-setup`. The existing CMake aliases remain the
operator-selected inputs for the current smoke routes. Focused FDD and current
gate requalification are pending before a passing current-media claim may be
made.

## Change Control

A hash, size, or declared-purpose change creates a new candidate media set.
It requires owner approval, an updated local-only record, and focused media
smoke evidence before becoming the current set. The smoke procedure must
preserve an owner-supplied master image or use an owner-approved disposable
copy. It cannot silently overwrite the M1 MS-DOS 5 baseline or turn protected
guest media into a repository asset.
