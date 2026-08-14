# T362 S3: legacy timing normalization closure audit

## Allocated and retained rows

The accepted private 80186 immediate-`IMUL` table remains the sole allocation
for every T361-transferred immediate form:

| Form | Raw/domain/selected clocks | Result |
| --- | --- | --- |
| `6B` register | 22 / 22--24 / 22 | Direct model allocation. |
| `6B` memory | 29 / 22--24 / 24 | Intel-constrained upper clamp. |
| `69` register | 25 / 29--32 / 29 | Intel-constrained lower clamp. |
| `69` memory | 32 / 29--32 / 32 | Direct model allocation. |

The source classifier recognizes `69`/`6B`, and
`core_machine_legacy_dynamic_arithmetic_model_cost` dispatches all four
80186 register/memory shapes to that table before the generic classifier can
transfer them to `CORE_MACHINE_SOURCE_UNALLOCATED_TICKS`.  The table's
accounting check recomputes the clamp and matches the selected value to its
direct/constrained provenance.  8086 `F6`/`F7` and 80186 Group-3 rows retain
their accepted T361 direct values; unrelated profiles, non-segment prefixes,
and generic fallback sites retain their named T359/T360 or source-owner
receivers.  No scoped immediate-`IMUL` one-tick fallback remains.

The 80186 raw model total includes EA.  The focused corpus verifies an odd
memory address has no additional EA/odd-word charge and segment-prefixed
memory sources add exactly two clocks.  It also preserves the divide-fault
nonpublication and single successful-retirement publisher.

## Gate maintenance finding and resolution

The first full-gate attempt exposed two stale T344 static-verifier assumptions,
recorded in the [S3 findings](t362-s3-closure-audit-findings.md).  Commit
`316c2334` repairs only those verifiers: it accepts CMake's current
bracket-quoted CTest output as well as the historical quoted output, and it
reconciles the direct-fixture inventory from 60 to the current 66 sources.
This does not change CTest registration semantics, CPU behavior, timing
allocation, or the developer artifact.

The static verifier reports 246 registered current-gate targets and 66 direct
machine-constructor fixtures (22 shared tails and 44 retained shapes).

## Final verification and artifact

On 2026-08-14, the focused
`core-machine-legacy-timing-normalization-s2-smoke` passed, documentation
governance passed, `git diff --check` passed, and the repaired
`current-gates-gcc` target passed.  A subsequent labelled CTest execution
completed all **246/246** current-gate tests without failure.

The task developer artifact remains `build/output/nxvm_0_5_0362.exe`,
SHA-256 `ECB501AD3594979D8AE570479C8401B8614CB5CBEB802B33A5A14DBC6EA363B2`.
Its runtime identity is `Neko's x86 Virtual Machine [0.5.0362]`; source
allocation commits are `94e970a6` and `316c2334`.  It is a local developer
artifact, not release evidence.

T362 closes as a source-labelled model-L3 timing allocation only.  It does
not claim physical bus/prefetch timing, device/service latency, measurement,
or cycle-exact fidelity.  The next Queue candidate is the complete
instruction-timing corpus.
