# M1 T4 S38 — Synthetic Graphics Oracle Boundary

## Purpose

S38 tested whether the owner-approved external MAME black-box route can isolate
a finite graphics-element coordinate/value contract without reading, recording
or importing game data. It did not inspect reference source, retain a screen,
RAM or ROM payload, or add a decoder. S40 later supplied the missing control
and showed that the S37 unpaired change was ordinary frame advancement; this
packet's negative layout conclusion remains unchanged.

## Method

The external MAME `0.289` Lua API was used only through its documented
in-process memory-region writer and screen-pixel facilities. At a fixed frame,
the transient script replaced the complete external tile region with zeros. It
then captured one later screen in process, optionally changed only byte zero of
that same region to synthetic `0xff`, captured a second screen in process and
printed only:

- visible width and height;
- changed-pixel count and bounding rectangle; and
- an Adler-32 checksum of the later visible screen.

The captured pixel strings never left the MAME process. The script, its output
and all generated MAME state were disposable; no protected byte, pixel or
address-to-content observation is retained here. MAME was explicitly given the
external `nxvm-assets/media-myraidenii/raiden2/mame-config/` configuration
directory, so no repository-root configuration is a runtime dependency.

## Repeated Observations

Each run used the same external owner inputs and headless configuration. The
two runs with the one-byte synthetic mutation and the two no-mutation controls
all produced the same bounded summary:

```text
320 × 240; changed pixels 51191; bounds 0,0 through 319,239;
later checksum 2536801818
```

The equality of mutated and control summaries proves that the observed change
is dominated by the independently advancing reference presentation. It does
not identify an element footprint, bitplane, row/column stride, tile-word
field, palette field, layer order, scroll, priority or decryption behavior.

## Disposition

The black-box method cannot supply the finite initial decoder contract within
its current containment boundary. Freezing the reference, reading a raw
intermediate frame, retaining multiple screen payloads, or deriving a layout
from MAME source would be a different authority decision and is not admitted by
this S.

The existing Core remains correct to own copied graphics image roles, the raw
palette and tilemap without claiming a renderer or frame publisher. A future
decoder S needs original board documentation/measurement, or a separately
approved black-box method that can isolate a stable synthetic element without
retaining protected payload.
