# T582 S1: initial performance audit

Baseline is accepted T581 implementation 59ad6e78b; T582 has no production
changes. This is an interim audit, not a completed performance repair.

## Fresh early-workload evidence

A task-owned probe uses the current public App factory and Emulator Machine,
configured private CMOS copies and overlay copies of the accepted disk inputs.
It starts default DSTN for 20 seconds then stops, shuts down and destroys.
Link-time wrappers count existing board deadline/advance calls; no production
archive is changed and no private live Machine layout is read. Counters are
worker-owned and printed only after shutdown. These counts include construction
and start/stop boundaries, and wrapper overhead prevents treating the figures
as precise uninstrumented product throughput.

| Host width | Deadline calls | L1 / blocked / immediate | Advance steps | One-tick steps | Total advanced ticks |
| --- | ---: | --- | ---: | ---: | ---: |
| x64 | 7,292,995 | 0 / 0 / 0 | 7,291,264 | 714,171 | 56,689,629 |
| x86 | 7,099,789 | 0 / 0 / 0 | 7,098,104 | 712,522 | 55,007,419 |

Both processes exited zero. This rejects persistent L1/blocked fallback as the
cause in this sampled workload; it does not reject every later fallback.
At the configured 33 MHz axis the advanced ticks represent about 1.72 and
1.67 guest seconds, not twenty. No guest timing constant has been changed.

A separate task-owned x64 thread-IP sampler observed 3,304 in-image samples,
13,897 outside-image samples and four failed context reads. Outside-image
samples include waiting threads; they are not attributed CPU costs. Largest
individual in-image buckets were memory_route_resolve.part.0 (388),
cpu_execution_refresh (230), board_dma_advance_transaction (144),
dma_get_signals (99), and cpu_timing_select (92). Sampling perturbs execution;
these are candidate function ranges, not exclusive call-stack cost percentages.
The sampled probe also exited zero and showed no L1/blocked deadline fallback.

## Source findings to investigate

- Turbo already bypasses Standard wall-clock pacing. No evidence supports
  deleting a sleep as the main repair.
- Core scheduler queries device deadlines on every scheduler slice; Board
  advances PIT, PIC, RTC, KBC and configured storage in the existing order.
  Do not batch past device events or replace guest time with host time.
- X86 Core memory read/write resolve a whole RAM range and then resolve each
  byte again to detect a provider contained within it. Route resolution scans
  raw, selected-alias, replacement, ordinary and fallback ownership in priority
  order. This is a measured hotspot with repeated work, but simply removing
  per-byte checks would lose partial-range device ownership. A proposed repair
  must prove contiguous ownership at the existing resolver, retain provider
  decline/failure and address-alias semantics, and avoid a second route cache.
- KBC callbacks recur often, but the earlier S89 isolated repeat-query change
  demonstrated no gain. Do not reapply it on superficial source repetition.

An additional bounded configured-guest x64 run samples only host seconds 5--25
and 300--320, captures published frames every ten seconds, and ends after 360
seconds (450-second outer budget). Its purpose is to distinguish startup from
late-guest hotspots. At this record's creation it is still running and is not
counted as passed. No native Window/RDP or interactive playability is established.

## Temporary evidence

Task-owned source, binaries and logs are under `build/t582`. Existing stable
Ninja libraries supply the production code and link order; no duplicate CMake
build tree is created. Inputs remain private copies. All completed short probes
have exited; the bounded late-guest run remains owned by S1. S1 remains open.

Completed evidence SHA256:

- `measure.py`: `4C2B151A1A310F0294EF658DE5730F75528CDD66ECA54A2E2527808A32DED432`
- `probe.c`: `5F72A8F397A34DEEA6F7A7B240A2FA41023A1380E661CBEC4F8E4D2C166D2F0E`
- `count.c`: `E050A288E6914C99F8B3F142D9DD893B2C0CF340B98318D0D66B1B20FC8D911A`
- `measure.log`: `41B1B79B5B14BBD9AF7AD608B21007396378EE3DAA1949A27800DC391E40DC15`
- `sample.py`: `89BCE4EFF41605B02C7265D4D7584F57DD0199CB825CF70A67E62D09A4B64597`
- `sample.log`: `EC654FF570CC15CA8214EC538B1E334C33CFF8DE595F28E07E185AF8161CF3F9`

## Completed late-guest sample and rejected RAM candidate

The configured x64 guest run completed normally, reaching the full Personaware
menu. Its final PPM hashes to the same accepted T581 frame:
`85A35680232CA0AD623DB7DD18D23A79396931ACB133791581D40C37048AEF41`.
The initial attempt was deliberately stopped after discovering that the sampler
buffered guest stdout in a pipe until exit. The corrected attempt writes guest
output directly to a task-owned file; the interrupted attempt is not a pass or
a product defect. All task-owned processes have now exited.

The corrected sample collected 6,670 in-image observations, 17,793 outside-image
observations and 10,737 failed context reads (retained thread handles can outlive
short-lived threads). Consequently these are indicative function samples, not
CPU-time percentages or a complete all-thread profile. Early memory routing
had 340 samples; late routing 169. Late F65535 snapshot capture had 300, the
largest late bucket, followed by CPU execution refresh at 200. Sampling was
limited to seconds 5--25 and 300--320 to limit observer interference.

A private 22-line RAM-range proof candidate was linked ahead of the unchanged
production archive. It conservatively bypasses repeated byte queries only
without aliases/address masking or any intersecting provider. Production files
and release EXEs remain unchanged. One sequential baseline/candidate pair per
width used the same configured inputs and twenty-second execution window:

| Host | Baseline ticks | Candidate ticks | Baseline/candidate host seconds |
| --- | ---: | ---: | --- |
| x64 | 74,160,650 | 74,311,753 | 20.464 / 20.434 |
| x86 | 56,469,758 | 58,742,165 | 20.157 / 20.167 |

The x64 difference is about 0.2%; the x86 difference about 4.0%. A single
host-time pair, including different guest phase progression, does not prove
an improvement. The candidate is not admitted to production. x64 baseline
made 1,692,237 cache-fill queries but enabled only 71; x86 made 40 and enabled
one. The widths reached different phases, so neither cache count is a universal
profile. A fixed-work comparison is needed before reviving this candidate.

The next concrete candidate is F65535 capture: the existing planar routine
expands every pixel (division/modulo and four plane reads per pixel), even for
unchanged generations. Machine discards the resulting unchanged frame only
after the work. Board's existing early observation belongs to the standard
video adapter, not the PC110 custom provider. Preserve the full snapshot API
contract; do not silently return an empty successful snapshot or add a second
render/cache owner. First evaluate row/byte expansion at the existing chip
owner (eight pixels per four plane bytes), preserving palette, stride, start,
validation and change flags. Any earlier generation admission design must
remain a single observation path and include text cursor phase behavior.

Additional evidence SHA256:

- `guest.c`: `890AF089F64F3517DC1B6A75443CA0439EDF9CD7EC6D799FF2F410DB4EFBA570`
- `sample_guest.py`: `3EEEFDA9C7605DD60894159683F9B867D343FE846BA20833DDB3E704A353D385`
- `guest-sample.log`: `4C52D6BBD0E3FBA7B4632F1561A8F64D66CBB8253DF27C6E0BE7839451FA583A`
- `guest-sample-interrupted.log`: `349A0154638CB365E2DD3A631D0D06EAFB181F37A0738360985424A87F7ED7D9`
- `memory_candidate.c`: `AE62DC7612A8A37D314B8C70EE0EEFB3D670C61FE44CB7FD1F5FFCCA6CC5D329`
- `compare.log`: `BB849CA6E8E65374DA77C2D43F25B9FDB75849F223DEC949C9CE83E1C1D3A9E5`

## S1 design disposition: measured planar expansion

A private F65535 candidate expands one four-plane byte group into eight pixels
using row/column loops. It removes per-pixel division/modulo and repeated plane
loads without adding state, tables, public APIs or changing capture semantics.
Fixed-work benchmark: 1,000 captures per geometry, deterministic complete VRAM,
nonzero start address and padded stride. Complete snapshot FNV-1a digests match
baseline/candidate on both widths (including palette, flags and initialized tail).

| Geometry | x64 baseline/candidate ms | x86 baseline/candidate ms |
| --- | --- | --- |
| 8 x 1 | 4 / 3 | 3 / 4 |
| 320 x 200 | 279 / 21 | 310 / 122 |
| 640 x 480 | 1422 / 69 | 1478 / 643 |

Tiny-frame timing is below useful resolution. The larger fixed-work gains are
substantial enough to admit the local repair; no whole-product speedup is
claimed. Existing capture validations, palette, start/stride, glyph/cursor and
full snapshot behavior remain intact. Source change is limited to the planar
loop in `src/x86-nxvm/chips/f65535/f65535.c`; extend the existing chip contract
smoke to compare nonuniform plane bytes with a straightforward reference at
padded stride/nonzero start. Estimated +16/-10 production lines, about 30 test
lines and two manifest entries. No new fixture framework or benchmark gate.

Coordinator review accepts S1's bounded audit/design outcome under the owner's
automatic S sequencing authorization. S1 does not resolve the overall T582
playability goal. S2 implements and qualifies this measured repair, preserving
remaining memory/CPU/device and unchanged-frame observation investigations;
S3/further numeric batches must not claim full performance qualification from
a microbenchmark alone. All 14 receiver artifacts and full dual-width tests
remain mandatory after the Shared source change.
