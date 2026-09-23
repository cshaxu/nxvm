# T362 S2: legacy timing normalization acceptance

## Implemented private allocation

Commit `94e970a6` replaces the eligible 80186 immediate-`IMUL` transfer with
one private four-row table in `src/core/machine/machine.c`.  Each row records
the opcode, register/memory source, raw same-profile MAME scalar, inclusive
Intel Table 1-16 bounds, selected clocks, and either `direct` or
`Intel-constrained` provenance.  The helper recomputes the closed-interval
clamp and rejects any table row whose recorded selected value or provenance
does not match it.

| Encoding/source | Raw | Intel domain | Selected | Provenance | Prior route |
| --- | ---: | ---: | ---: | --- | --- |
| `6B /r ib`, register | 22 | 22--24 | 22 | Direct | Existing direct row; no value change. |
| `6B /r ib`, memory | 29 | 22--24 | 24 | Intel-constrained upper clamp | One-tick transfer removed. |
| `69 /r iw`, register | 25 | 29--32 | 29 | Intel-constrained lower clamp | One-tick transfer removed. |
| `69 /r iw`, memory | 32 | 29--32 | 32 | Direct | One-tick transfer removed. |

The only consumer remains
`core_machine_legacy_dynamic_arithmetic_model_cost`, reached before the
generic source classifiers by the existing successful-retirement publisher.
The 80186 table retains MAME's EA-included accounting, so it receives neither
the 8086 EA nor odd-word additions.  Existing segment-prefix ownership adds
only the documented two clocks for a memory source.  Other profiles and
unsupported prefixes retain their existing owners and transfers.

## Regression and source sweep

The former T361 S3 smoke was moved with `git mv` to
`core_machine_legacy_timing_normalization_s2_smoke.c`, and its current-gate
target was renamed accordingly.  It proves all four selected values, both
clamp directions, direct values, an odd-address memory source with no extra
EA/odd-word charge, segment-prefixed memory values of 26 and 34, 8086
non-regression, and divide-fault nonpublication.  Its success marker is:

```text
M5:T362:S2:LEGACY-TIMING-NORMALIZATION:OK
```

The S2 static sweep was:

```text
rg -n "T361_S3|legacy-dynamic-arithmetic-timing-s3|CORE_MACHINE_SOURCE_TIMING_IMUL_IMMEDIATE|CORE_MACHINE_SOURCE_UNALLOCATED_TICKS" src/core/machine/machine.c tests/machine CMakeLists.txt docs/states/CURRENT.md docs/etc/evidence/t362-s1-legacy-timing-normalization-ledger.md
```

It found no retained old test/target identifier.  The remaining
`UNALLOCATED_TICKS` sites are the general/profile/prefix receivers named by
T362 S1; no eligible immediate-`IMUL` route remains in that fallback.

## Build identity and pending closure work

The locally built developer artifact is
`build/output/nxvm_0_5_0362.exe`, SHA-256
`ECB501AD3594979D8AE570479C8401B8614CB5CBEB802B33A5A14DBC6EA363B2`, built
from source commit `94e970a6`.  Its runtime banner reports
`Neko's x86 Virtual Machine [0.5.0362]`.  The focused CTest target passed
1/1 on 2026-08-14.

S3 must independently audit the complete T361-transferred dynamic-arithmetic
set, the residual fallback receivers, source/consumer ownership, documentation
records, artifact identity, and the full current gate.  This remains a
model-L3 allocation only; it makes no physical or cycle-exact claim.
