# PC110: C-Segment ROM And Shadow Contract

## Admission And Dependencies

Unnumbered M5 candidate created by T573's bounded real-firmware trace. Allocate
a numeric T only after the source gate admits a complete device contract. It
depends on [T573 S1 evidence](../etc/evidence/t573-s1-pc110-warm-reset-trace.md),
the existing App-local PC110 composition, and the approved external TFT asset
pair. External firmware remains BYOB and is never committed.

## Objective

Establish one truthful, App-local VL82C420 C-segment ROM/decode/shadow contract
which allows the unchanged PC110 firmware to make its own video-BIOS transfer
during POST. The receiver must prove reset/default state, the CPU-visible
configuration interface, each required transition, the backing storage and
read/write behavior, then validate the real execution path on both widths.

## Non-goals

No firmware-address dispatch, BIOS/interrupt interception, automatic BDA warm
signature, static option-ROM alias, PC/AT facade, second executor, synthetic
POST/Setup/DOS service, raw-ROM commit or release claim. PC110-EMU and
PC110-QEMU may provide labelled L2 comparison only; their special completion
paths are not implementation sources.

## Source And Proof Gate

The receiver requires primary chipset documentation or a sufficiently complete,
reproducible live trace covering the exact reset/default mapping and register
writes used before the C000 transfer. Existing broad reverse-engineering facts
alone do not admit a guessed register model. If that gate cannot be met, record
the absent facts and transfer rather than fabricate behavior.

## Expected Scope And Verification

Expected ownership is the PC110 App's existing VL82C420 route and its matching
unit/diagnostic tests; a demonstrated generic Core defect may name its own
receiver. Lib, Emulator, unrelated Apps and external assets remain excluded.
Verify both host widths through the ordinary executor, preserve immutable
firmware ownership, prove C000 execution follows the genuine firmware path,
and run affected units/diagnostics plus structural gates. No PC110 EXE is
published until the separate product-qualification candidate is admitted and
its own guest corpus passes.
