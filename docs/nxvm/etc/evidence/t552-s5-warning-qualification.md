# T552 S5 — Core CPU Warning Qualification

## Reproduction

`src/core/CMakeLists.txt` had one GNU-only `-w` receiver for both
`chips/cpu/cpu.c` and `chips/cpu/cpu_instructions.c`.  There is no separate
Debug compiler-warning downgrade in the current Core build rules.

Both MinGW compilers were run directly with `-std=c11 -O3 -Wall -Wextra
-Wpedantic -Werror` against the two receivers. `cpu.c` has no diagnostics.
`cpu_instructions.c` was then reproduced with the narrow retained diagnostic
classes listed below on x64 and x86.

## Disposition

| Class | Disposition | Reason |
| --- | --- | --- |
| `-Wunused-parameter` | Retain, CPU instruction source only | Trace-shaped private helpers preserve full handler signatures in non-trace builds. |
| `-Wunused-variable` | Retain, CPU instruction source only | The same trace-disabled paths make local diagnostic values unused. |
| `-Wunused-but-set-variable` | Retain, CPU instruction source only | Trace-disabled diagnostic capture retains a write without a release consumer. |
| `-Wunused-function` | Retain, CPU instruction source only | Private complete segment helper variants are compiled for the CPU source but not selected by the current profile surface. |
| `-Wempty-body` | Retain, CPU instruction source only | Trace macro expansion intentionally produces empty release blocks. |
| `_kec_task_switch` declaration | Repaired | Removed the unimplemented, uncalled private declaration. |
| Control-register opcode condition | Repaired | Parentheses now state the existing range-or-single-opcode grouping. |
| Lexeme byte-count comparison | Repaired | The sum is explicitly evaluated as `lib_u32`, preserving the bounded comparison without signedness ambiguity. |
| CPU read/output temporaries | Repaired | Segment descriptors, SIB/ModRM bytes, task descriptors and FPU opcode temporaries are initialized before their read helpers receive their addresses, closing the compiler-proven possible uninitialized paths without changing successful decoding. |

No global warning policy changed. `cpu.c` no longer has a local exception;
only `cpu_instructions.c` holds the five named GNU diagnostic classes. All
other warnings remain errors.

## Verification

Both x64 and x86 completed:

1. direct strict syntax compilation of `cpu_instructions.c` with only the
   five retained classes disabled;
2. `core-chip-cpu` and `core-machine-cpu-timing-preview-smoke` builds;
3. `core.test-manifest` and
   `unit.core-machine-cpu-timing-preview-smoke` CTest routes.

The four PC Apps were then rebuilt from their existing profile-specific
x64/x86 Ninja directories. Each `vm-0-5-0551` deployment hook verified its
PE architecture and refreshed the eight tracked PC artifacts. MyNES does not
consume Core and was not rebuilt.

The x86 build initially encountered a stale Ninja index. No new build
directory was created; its original `mingw-gcc-x86-release-ccache` directory
was recompacted and then passed the same build and tests.
