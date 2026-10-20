# M1 T4 S37 — Black-Box Graphics Mutation Boundary

> **Superseded causal conclusion:** S40 added the missing no-mutation control
> and found the same two-frame checksum/difference summary with and without the
> selected write. The observation below remains an accurate record of S37's
> unpaired run, but it does not establish that the synthetic byte caused the
> later screen change. See the [S40 control](t4-s40-paused-graphics-control.md).

## Purpose

This packet tested whether the current external MAME reference can provide a
lawful, reproducible *behavioural* graphics-input observation without reading,
recording or importing protected game data.  It does not inspect MAME source,
copy a graphics table, retain an image/pixel payload, or establish an on-board
graphics layout.

## Tool And Inputs

- Reference: MAME 0.289, external to this repository.
- Inputs: the owner-supplied external `raiden2` archive and S35's two external
  PAL files.  Neither input was changed.
- API basis: MAME's published Lua interface documents both its memory-region
  byte writer and its visible-screen pixel API.  The script used the API only
  after the emulated system had started.
- Lifetime: one ignored `build/myraidenii-t4-s37-oracle/` Lua script per run;
  it was deleted after the observation.  No MAME configuration, NVRAM,
  snapshot, log, ROM copy or image was retained.

## Neutral Observations

The reference exposes the following role-sized graphics regions:

| Role | Observed region size | Local immutable role size |
| --- | ---: | ---: |
| Character image | 131,072 bytes | 131,072 bytes |
| Combined background images | 4,194,304 bytes | 2 × 2,097,152 bytes |
| Combined sprite images | 8,388,608 bytes | 4 × 2,097,152 bytes |

At a fixed post-startup frame, the script changed a single in-process byte in
the first combined background region to the synthetic value `0xff`.  It did
not read or retain the replaced byte.  The script calculated Adler-32 values
only in process over the visible pixels and printed only dimensions and the
two checksums:

| Trial | Visible dimensions | Before checksum | Later checksum |
| --- | --- | ---: | ---: |
| 1 | 320 × 240 | `1079166174` | `852859541` |
| 2 | 320 × 240 | `1079166174` | `852859541` |

The identical result establishes that an external reference presentation reacts
deterministically to a bounded synthetic graphics-region mutation.  It also
confirms the existing local role inventory and the native 320 × 240 output
geometry used by the later Core-to-Machine frame path.

## Disposition

This is **not** a graphics-element decoder contract.  A whole-screen checksum
cannot distinguish bitplane ordering, tile row/column stride, tilemap word
fields, layer order, sprite attributes, scroll, priority, decryption or palette
brightness.  Inferring any of those from a reference source/table remains
prohibited by the source policy, while retaining raw screens or ROM/memory
payloads would violate the packet's content-containment boundary.

The next decoder S therefore needs one of the following admissible inputs:

1. original board documentation or measurement that identifies the missing
   element layout/attribute contract; or
2. a separately owner-approved expansion of the black-box method that can
   isolate synthetic tile/palette state and retain only a finite, neutral
   coordinate/value oracle.

Until then, the existing Video owner correctly retains graphics images,
palette and tilemap without claiming it can render them.  No decoder, renderer,
frame publisher or shared-component change is admitted by this evidence.
