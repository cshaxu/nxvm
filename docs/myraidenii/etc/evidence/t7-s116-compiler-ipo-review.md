# M1 T7 S116 — Compiler IPO review

## Scope

This S used the existing selected-set startup probe to evaluate compiler
interprocedural optimization (IPO/LTO) as a possible execution improvement.
It retained no production configuration or source change.

## Method

Two disposable Release Ninja trees under `build/.ninja/` enabled CMake's
existing `CMAKE_INTERPROCEDURAL_OPTIMIZATION=ON` setting.  Each built the
normal probe and ran it through the verified read-only archive staging wrapper
with the same VBlank-relative Coin/Start sequence and 8,000,000-instruction
budget as S114/S115.  The ordinary x64/x86 Release trees supplied the matched
reference runs.

All runs retained the selected-set facts: 8,000,000 instructions,
133,667,500 guest ticks, Coin/Start delivery, terminal frame digest
`0364633fdce1196d`, and no Board or COP provider failure.

## Result

| Host width | Ordinary Release | IPO/LTO Release | Disposition |
| --- | ---: | ---: | --- |
| x64 | 20,223.1 ms | 18,325.6 ms | approximately 9.4% faster |
| x86 | 25,476.7 ms | 43,805.5 ms | approximately 72.0% slower |

The x64 result repeats the earlier diagnostic observation that whole-program
IPO can help this host/toolchain.  The x86 regression prevents making it the
repository's ordinary Release policy: a shared build switch must not make one
required delivered architecture materially worse.

## Disposition

No portable owner-local CPU cost was attributed by the attempted GNU `gprof`
route: MinGW produced no usable `gmon` output even when the child inherited a
task-local output prefix.  That profiler setup, both IPO trees, and all staged
inputs were removed.

An x64-only release flavor would create a build-policy exception rather than
remove emulator work, while still leaving the required x86 artifact slower.
It is therefore not retained.  The next performance proposal must identify a
single CPU/Core owner with a cross-width benefit, or T7 should proceed with
its independently required gameplay/release qualification rather than continue
speculative throughput edits.
