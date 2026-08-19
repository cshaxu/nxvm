# T429 S1: CECG 8-bit Bus Wait Skeleton

`M5:T429:S1:CECG-8BIT-BUS-WAIT:OK`

## Evidence Tier And Boundary

Tier: `generic-at` for the one-tick logical wait. Original Compaq CECG material
was consulted transiently and establishes the selected 8-bit interface and
register port surface only. It does not establish a CECG NOWS, BUSRDY or IOCHRDY
electrical assertion contract. No source copy, firmware, media, OCR, path or
third-party code is retained.

## Delivered Contract

Core now carries one copied, bounded external-access wait-window table. Every
CPU external-cycle lifecycle names either physical memory or I/O port space, so
a profile port range cannot alias a memory address. CPU `IN` and `OUT` publish
BEGIN, COMMIT and cancellation through the existing single CPU transaction
lifecycle; Core adds a selected window's logical wait only at matching commit.
Cancellation and reset clear the pending lifecycle, and no VM scheduler or
second transaction route is introduced.

Model-40 selects six one-tick generic-AT I/O windows for the documented CECG
port surface: 3B4--3BA, 3C0--3CF, 3D4--3DC, 7C6, BC6 and FC6. The existing D4
2 KiB memory-page receiver remains memory-space-only. IBM 5170 selects no CECG
window.

## Verification

`core-machine-prefetch-locality-smoke` proves a selected port wait, an
unselected port, cancellation, reset, and a real CPU `OUT` instruction reaching
the lifecycle before retirement. `vm-model40-private-composition-s7-smoke`
proves the Model-40 binding. Both focused current-gate tests passed on
2026-08-18.

## Transfer

This is not a Compaq timing measurement, a CECG memory-aperture wait policy, or
a NOWS/BUSRDY/IOCHRDY waveform model. CECG raster/status/monitor behavior and
physical-clock/L3 acceptance remain separate receivers.