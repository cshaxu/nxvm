# T317 S2 Test Type-Vocabulary Evidence

S2 consumes the exact 47-entry `PROJECT_T317_STRICT_CPU_SMOKE_INVENTORY`
already established by S1.  It does not discover sources by directory scan.
The only support file in scope is explicitly listed as
`tests/support/core_machine_cpu_fixture.h`; no other inherited test or source
file is scanned or claimed clean.

## Mechanical Migration

| Direct spelling | Project replacement | Governed baseline occurrences |
| --- | --- | ---: |
| `uint8_t` | `type_unsigned_8` | 2,024 owner sources; 5 support-header |
| `uint16_t` | `type_unsigned_16` | 369 owner sources; 2 support-header |
| `uint32_t` | `type_unsigned_32` | 553 owner sources; 10 support-header |
| `int16_t` | `type_signed_16` | 13 owner sources |
| `int32_t` | `type_signed_32` | 19 owner sources |
| `uint64_t`, `int8_t`, `int64_t` | matching `type_unsigned_64`, `type_signed_8`, `type_signed_64` | no governed baseline occurrence; covered by the gate |

The replacements are token-for-token type substitutions only.  They do not
change assertion data, runtime markers, fixture topology, target-local strict
policy, public API, or production code.

## Narrow Gate

`verify-t317-test-type-vocabulary` reads the generated S1 inventory file and
the explicitly generated one-header support list.  It requires exactly 47
unique `tests/machine/*.c` owner sources and exactly
`tests/support/core_machine_cpu_fixture.h`, then rejects direct
`uint8_t`/`uint16_t`/`uint32_t`/`uint64_t` and
`int8_t`/`int16_t`/`int32_t`/`int64_t` tokens only in that set.  Its built-in
self-checks read a clean project-vocabulary fixture and a separate negative
fixture containing each forbidden token.  It makes no
repository-wide cleanup claim.

## Verification Record

- Fresh `mingw-gcc-x64` configuration completed.
- `verify-t317-test-type-vocabulary` passed with 47 owner sources, one support
  header, and zero direct fixed-width spellings.
- `verify-t317-strict-cpu-smoke-coverage` passed its actual-Ninja-command
  audit for all 47 owner targets.
- Documentation governance and `git diff --check` passed.
- The full `current-gates-gcc` gate passed all 194 CTests.
