# MyNES Emulator Performance: Measured Rendering and Execution

## Outcome And Boundary

Improve MyNES's measured execution and presentation cost for its supported NES
scenarios while preserving guest-visible timing, audio, controls, graphics,
mapper behavior, snapshot continuation, and both Windows host widths. Use
Nesticle as a historical architectural comparator, not as a compatibility
specification or a source of product code.

Owner admits M7 T44 on 2026-10-06, requesting a current performance baseline
and correctness-preserving efficiency work even though Windows gameplay is
already smooth. Current holds the active packet; the
[task ledger](../history/M7-T44-measured-emulator-efficiency.md) freezes the
coverage and dispositions. Existing 0043 artifacts are the pre-task baseline;
0044 is qualified only through normal paired delivery. No Shared production
or sibling source change is implied by this admission.

## Existing Lead

- `src/app-mynes/core/driver.c` rebuilds an indexed 256x240 frame on each
  publication. For each pixel it linearly searches the palette assembled so
  far; when distinct RGB colors exceed 256 it traverses the frame again for
  cube quantization. A bounded sample-to-palette lookup is the first concrete
  candidate, provided raster-order palette assignment, duplicates, overflow
  fallback, and exact published bytes remain unchanged.
- `src/app-mynes/core/ppu.c` advances every PPU dot and fetches background and
  sprite data through the cartridge path. Pattern decoding, row/pixel work,
  mapper lookups, and sprite sampling are profiling candidates. Any cache may
  memoize pure derived values only; it must not omit a required bus access,
  A12 edge, IRQ transition, mid-frame register effect, or pixel decision.
- The shared Window renderer already resolves indexed colors, compares pixels,
  and tracks a changed rectangle. Measure it before proposing further host
  damage detection. DOS planar VGA and segment optimizations do not apply to
  MyNES's Windows product.
- Historical Nesticle 0.2 source suggests decoded 8x8 pattern tiles, dirty
  nametable surfaces, and fast tile/sprite composition. Its provenance and
  limited copyright notice do not authorize copying or transliteration. The
  owner-tested x.xx binary is a different version; no exact-version mechanism
  or speed claim follows from that source alone.

## Proposed Work Sequence

1. **Bounded baseline and comparator.** Fix project-owned test scenes and
   approved owner-local game routes, input scripts, graphics/text output modes,
   x64/x86 builds, frame counts, warmup, timer method, and CPU budget. Measure
   CPU/bus, PPU, cartridge/mapper, APU, frame conversion, Common publication,
   and Window presentation separately. Record guest frames produced and host
   frames displayed; do not count frameskip, dropped frames, changed video mode,
   or skipped emulation cycles as a speedup. A Nesticle comparison is optional
   and must identify its binary, settings, video mode, and comparable workload.
2. **Host conversion candidate.** Prototype direct sample/color lookup and
   bounded palette indexing in the MyNES Core driver. Preserve the exact
   indexed frame, palette ordering, emphasis colors, >256-color fallback,
   graphics/text switching, and publication lifecycle. Adopt only if a paired
   profile shows a repeatable whole-frame benefit without a material regression.
3. **PPU computation candidates.** Rank hot work from the baseline, then test
   one bounded representation or cache at a time, including decoded pattern
   pixels or reusable row calculations where proven safe. Keep PPU dot/bus
   scheduling and mapper-visible reads intact. Invalidation covers CHR ROM/RAM,
   mapper bank changes, CIRAM/name/attribute/palette writes, scrolling,
   masking, sprites, reset, and snapshot restore as applicable. Reject any
   candidate whose correctness or memory/lifetime cost cannot be bounded.
4. **Integrated acceptance.** Compare the selected code against the fixed
   baseline on both host widths and both KVM output modes, including moving,
   dense-sprite, scrolling, palette/emphasis, mapper-4 IRQ, pause/resume, and
   snapshot-continuation cases. Report per-stage and complete-frame time,
   memory delta, guest/displayed frames, regressions, and any rejected ideas.

These are decision gates, not preallocated S identifiers. The coordinator
admits one bounded S at a time using the previous gate's evidence; a material
scope or risk expansion returns for owner approval.

Owner revises the unclosed S1 on 2026-10-06 to include baseline/reference and
frame-local palette optimization together. S2 is the evidence-led Mapper
candidate; S3 is integrated qualification. IRQ and PPU condition cleanup are
observation-only, not admitted optimizations. Split
only through new consecutive numeric S identifiers when evidence requires it;
no letter suffix or first-failure scope drift.
The existing RGB table is already precomputed for all 512 output samples;
baseline the actual per-pixel palette search, not an imagined repeated RGB
formula. Component microbenchmarks and instrumented attribution are diagnostic;
only paired complete-frame results support adoption.

S2 begins with an owner-requested design audit against accepted S1. Prefer
constant-domain simplification for the strictly bounded MMC1/UxROM profiles,
no NROM/CNROM performance cache, and a small MMC3-only derived PRG/CHR window
offset table if measured benefit justifies it. Registers remain authoritative;
construction, bank writes and validated snapshot restoration rebuild offsets.
Do not serialize offsets or change reset behavior to maintain a cache. The
[S2 audit](../etc/evidence/m7-t44-s2-mapper-design.md) records the existing CNROM
restored-bank bounds gap and the required disposition before implementation.

## Acceptance And Stop Conditions

Accept a code candidate only with repeatable improvement in the fixed paired
workload, byte-identical applicable frames and audio/event traces, unchanged
PPU/CPU/mapper timing checkpoints, passing repository-only unit suites and
owner-managed integration on x64/x86, and no material regression in another
fixed scenario. The task closes with a measured before/after report and an
explicit disposition of every investigated candidate; do not infer a
Nesticle-equivalent or universal speed claim from a synthetic microbenchmark.

Stop a candidate on output or timing divergence, unbounded cache invalidation,
unacceptable memory growth, unsupported source provenance, or a speed result
that disappears in the complete-frame comparison. If no safe candidate improves
the fixed workloads, report that evidence and return any remaining idea to the
queue or TODO under its owner rather than changing hardware semantics to meet
a speed target.

## Ownership And Research Controls

MyNES Core owns NES hardware execution and its output adaptation; App owns
product routing; Shared Lib/Common remain neutral and require separate
admission with all receiving consumers if changed. Do not change sibling
MySMB, import third-party implementation, alter ROM data, or commit raw traces,
protected frames, local paths, or build outputs. Any Nesticle source inspection
records exact revision, provenance, purpose, rights boundary, and independently
verified conclusions under the MyNES source-and-research policy. Retain bounded
raw probes only in ignored build storage and remove them under the execution
rules.
