# T582 S23 — Instruction preparation and dispatch

## S26 boundary and prefetch attribution

An x64 scratch build sampled the existing `ExecBoundary`, `ExecInit` and
`_s_read_cs` calls separately at one in 1,024 calls at the fixed startup
checkpoint. Their call/sample/TSC totals were 10,977,351/10,720/1,839,740,
10,971,241/10,714/10,297,547 and 25,105,467/24,517/1,736,496 respectively.
The checkpoint remained 10,000,000 instructions, 79,964,294 ticks and PC
`0003dbc5`; the instrumented run took 21,322 ms. Boundary is nested inside
Init and the intervals cannot be added. Timer overhead, compiler layout,
preemption and deterministic sampling limit attribution. This suggests that
the entire initialization cost should not be attributed to the full CPU
snapshot alone; it does not establish the cost of an individual copy.

A separate counter-only x64 run at the same checkpoint observed 10,003,409
ordinary `ExecInit` calls, with 6,663,917 entering the prefetch refill path;
971,303 preview calls included 598,067 refills. A second counter run attributed
6,444,676 ordinary calls to a valid prefetch whose actual PC differed from the
expected PC, including 5,594,568 whose prior instruction had a REP prefix.
These predicates overlap and a REP prefix alone does not prove a continuation.
Instrumentation adds work, counts include the stopping interval, and neither
run measures the Game workload or x86. The prior [REP continuation
experiment](t582-s11-performance-observation.md#rep-continuation-prefetch-probe)
changed fetch/timing behavior and reduced observed guest-clock throughput;
these new counts do not justify retrying that shortcut. Branch, write, mapping
and observation semantics can require invalidation. No production code, media
or release artifact changed, and no speed improvement is claimed.

The S20 World Clock probe indicated that CPU instruction execution was the largest sampled refresh stage, but did not identify its sub-operations. A scratch-compiled CPU probe sampled one in 64 calls to instruction preparation (`ExecInit`) and its selected opcode handlers. With the private PC110 boot workload at 10,000,000 retired instructions, both widths reached 79,964,294 ticks and PC 0003dbc5. On x64, preparation accumulated 123,681,214 TSC units over 156,268 samples; individual dispatches accumulated 108,798,485 units over 328,295 samples. On x86 the totals were 140,921,341/156,332 and 122,896,286/328,361 respectively. Multiple prefix dispatches can occur in one instruction, so counts are not interchangeable. This fixed early-boot workload is not a World Clock measurement; nested timers, compiler layout, possible preemption and deterministic sampling affect the numbers. `ExecInit` includes boundary state preservation, prefetch and diagnostics. None may be skipped without preserving their actual semantics.

A scratch-only candidate specialized the existing two instruction-prefetch copies for lengths 1, 2 and 4 through constant-size `lib_memory_copy`, with an arbitrary-length fallback. It preserves the same copies and uses no alignment assumptions. At the same checkpoint, x64 baseline/candidate were 20,708/21,441 ms and x86 were 25,971/24,915 ms, a single sequential pair per width. The opposing signs and host variance provide no consistent cross-width gain and do not warrant the added helper or a long product run. It was rejected; no production, public API or EXE changed. Equality of these checkpoint values does not prove all instruction, failure, debug or timing cases.

Scratch summaries SHA-256 (stages, fetch-copy): `C9A68EA6A5644AF8AFC41045F3C42F08B06FF84591EC91D702D04308140E2866`, `2DDAB1B4B3C555BE1940BB17280805E0FE43BB4F23D8A9900BDBDD126057EE4C`. S19's 712/712 unit gate remains the last complete repository-only run per architecture. The real PC110 World Clock performance gap remains unresolved; these microprobes are not desktop or RDP validation.
