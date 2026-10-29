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
