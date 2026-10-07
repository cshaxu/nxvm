# M7 T44 Measured Emulator Efficiency

Owner admits the queued Nesticle-inspired performance study on 2026-10-06
against fc0d31623, after closed MyNES T43. Current owns active S1; the
[proposal](../proposals/emulator-performance-nesticle-study.md) preserves the
full goal. No performance gain or new product is qualified at admission.

## Frozen Coverage And Disposition

The universe is the retained NROM/MMC1/UxROM/MMC3 support and six owner ROMs,
code-owned blank/repeated/raster-color/emphasis/dense/scrolling fixtures,
Window graphics and Console text, x64/x86, reset/pause/resume/input and
snapshot continuation. Source-native guest cycles, completed frames, PCM and
mapper/interrupt events must survive every accepted optimization. Record
source/build/input identity and observe host display separately from guest
production; frameskip/coalescing is not a computational gain.

| Batch | Required proof | Disposition |
| --- | --- | --- |
| S1 baseline and palette conversion | Repeatable bounded before/after execution and conversion measures; exact palette ordering/RGB duplicates/overflow, text/publish behavior and bounded memory. Distinguish synthetic, game and paced native routes. | Executor complete: both suites 56/56, exact paired identities and qualified 0044 pair; coordinator immutable-diff review pending. [Evidence](../etc/evidence/m7-t44-s1-palette-performance.md). |
| S2 Mapper candidate | Only mapping-register-derived address computation. Preserve bus/A12/IRQ and reset/snapshot reconstruction. | Pending S1 evidence; no persistent cache pre-approved. |
| S3 integrated qualification | Full coverage reconciliation, rejected-candidate dispositions, both output modes/widths, required units/integration and paired artifacts. | Pending previous batches. |

Accepted means repeatable whole-frame benefit with unchanged applicable bytes,
guest timing/state/events and no material regression. Rejected candidates keep
their evidence, not a hidden mode-specific workaround. An unmeasured stage,
unsupported reference license, or needed Shared capability is explicit; do
not invent a timing attribution or silently change the universe. Original
Task completion requires every row's direct disposition and final qualification.

## S1 Initial Source And Boundary Review

driver.c still owns pacing, audio adaptation and both output conversions.
Its immutable 512-entry RGB table already avoids repeated emphasis arithmetic;
the graphics path instead linearly searches the accumulating palette for each
of 61,440 pixels, then makes a second pass on overflow. Preserve raster-order
indexing and equal-RGB deduplication. PPU dot/fetch/mapper costs remain to
measure before prescribing tile/dirty caches. At admission no source had changed;
the completed S1 changes only frame-local graphics palette lookup.

Primary reference metadata identifies athros/NESticle as historical leaked
Bloodlust source, not an MIT grant. Remote HEAD/master is
7ede7dab54bc0cef85774b424025a7d6b8133d28. Only neutral architecture lessons are
admitted; no copying/transliteration, source/binary import or exact x.xx speed
claim. The sibling MySMB study is read-only background and not MyNES proof.
