# M5 T317: Test-Corpus Quality History

## Accepted Boundary

S1--S4 closed only the admitted quality boundary for the 47 T316-added CPU smoke
owners listed by `PROJECT_T317_STRICT_CPU_SMOKE_INVENTORY`.  It establishes
target-local strict GCC command coverage, an inventory-bounded project type
vocabulary check, and private test-fixture lifecycle consolidation.  It does
not claim CPU, 80386, x87, device, timing, product, ABI, target-policy, or
test-behavior capability completion.

## Package Evidence

| Package outcome | Concrete implementation and evidence |
| --- | --- |
| S1 strict target-local compilation | `5fdfbe8137660941395634f3b7254e8b9ba2aa25` adds the 47-entry CMake inventory and applies `-Wall -Wextra -Wpedantic -Werror` privately to exactly its GCC/Clang smoke executables. `00a09d1d9d85f767979f6d6e317ca83766310cc1` strengthens the retained FPU #NM real-mode frame test. The accepted record is [S1 evidence](../etc/evidence/t317-s1-strict-gcc-evidence.md); acceptance is `58b6b299`. |
| S2 project type vocabulary | `4e968c45d519a97ba5ee188a837ed95dc0542b36` migrates direct fixed-width spellings only in the exact inventory plus `tests/support/core_machine_cpu_fixture.h`, and adds the inventory-bounded positive/negative verifier. [S2 evidence](../etc/evidence/t317-s2-test-type-vocabulary-evidence.md) records the scope and token baseline; acceptance is `1124eb3c`. |
| S3 fixture mechanics | `1f4c9af92d903922028f948460f4e86ef0324a3d` and `3cb1bd4b843ee5bfc4594b0c974b794d55d51819` centralize the mechanically identical lifecycle setup in private `tests/support` helpers. [S3 evidence](../etc/evidence/t317-s3-test-fixture-consolidation-evidence.md) lists the 35 full-helper callers, six lifecycle-tail callers, real-mode/captured-observation boundary, and retained owner semantics. Acceptance is `2d9ceefb`. |
| S4 closure audit | This record independently rechecks the generated inventory, actual Ninja command audit, vocabulary gate, support boundary, deferred raw-type admission path, documentation governance, diff, and full current gate. It adds no runtime or test behavior. |

## S4 Independent Audit

Fresh `mingw-gcc-x64` configuration generated a 47-entry, 47-unique target and
source inventory. `verify-t317-strict-cpu-smoke-coverage` passed its actual
Ninja command audit for all 47 target-local commands; it proves all four
strict flags on the executable compile command, not transitive or global
coverage. `verify-t317-test-type-vocabulary` passed for the same 47 owners and
the one explicit support header with zero prohibited direct fixed-width tokens.

The S3 caller inventory is factual in the governed set: 35 sources call
`test_core_machine_fixture_create_bind_freeze_reset`, six call
`test_core_machine_fixture_bind_freeze_reset`, 44 call the private real-mode
preparation helper, and all 47 call the copied-observation helper. The six
sources without a lifecycle shape retain only shared-machine setup. No governed
source directly calls bind/freeze/reset, and `rg -n "tests/support" src`
returns no production-source inclusion.

Bounded `core_machine_run` interpretation remains source-local because the
instruction image, run budget, expected stop/diagnostic classification, and
observation point are family-specific. Moving those into generic support would
violate T317's no-framework and no-implicit-instruction-semantics boundary.

## Verification

- Fresh configuration: `cmake -S . -B build/mingw-gcc-x64`.
- `cmake --build build/mingw-gcc-x64 --target
  verify-t317-test-type-vocabulary verify-t317-strict-cpu-smoke-coverage`:
  passed, respectively reporting 47 owners plus one header and 47 target-local
  Ninja compile commands.
- `verify-documentation-governance`, `git diff --check`, and the production
  support-include search passed.
- `cmake --build --preset current-gates-gcc` passed all 54 static/governance
  targets and 194/194 CTests.

## S5 Corrective Extension

The owner later admitted S5 as a repository-wide fixed-width vocabulary repair.
It does not expand T317 into a CPU or product capability task: it replaces
lexical type spelling through the single `src/type.h` facade while retaining
the original S1--S4 smoke, strict-command, and lifecycle evidence. The factual
details and the 0.5.0317 artifact hash are recorded in [S5 evidence](../etc/evidence/t317-s5-global-type-vocabulary-evidence.md).

## S1--S4 Deferred Boundary

The type-vocabulary verifier deliberately does not scan inherited tests or the
repository at large. [TODO: inherited test raw-type vocabulary](../TODO.md)
requires a separately approved inventory, token baseline, equivalence review,
and scoped negative/positive verifier before such a migration may be claimed.
The existing inherited strict-compilation admission remains separately bounded
in the same TODO ledger. Neither debt alters the closed T317 47-source scope.
