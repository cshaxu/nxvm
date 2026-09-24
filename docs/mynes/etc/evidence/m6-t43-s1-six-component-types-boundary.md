# M6 T43 S1: Six-Component Types Boundary Ledger

Date: 2026-09-24
Scope: `src/{lib,common,x86}` and `test/{lib,common,x86}`.

## Rule Applied

`src/lib/types/` is the sole owner of external ISO C, compiler, Win32 and
Linux vocabulary. A component's own public symbols (`lib_*`, `common_*`,
`x86_*`) are not external vocabulary and remain valid component-owned uses.
The audit deliberately includes test code; a passing production gate does not
make a test-side native include compliant.

The following deterministic inventory query is the line-level ledger source:

```powershell
rg -n '^\\s*#\\s*include\\s*[<"](assert|stdio|stdlib|string|stdint|stddef|stdbool|limits|time|windows|unistd|sys/|pthread|mman)' src/lib src/common src/x86 test/lib test/common test/x86 -g '*.[ch]'
```

## Frozen Universe And Results

| Root | C/H files | Direct external headers | Disposition |
| --- | ---: | ---: | --- |
| `src/lib` | 90 | 30 | All 30 are in `src/lib/types/`, the declared external-vocabulary owner. |
| `src/common` | 15 | 0 | Types boundary clean. |
| `src/x86` | 10 | 0 | Types boundary clean. |
| `test/lib` | 43 | 51 | 34 `assert.h`; 17 other direct native/C headers. Open receiver. |
| `test/common` | 16 | 16 | 15 `assert.h`; one `windows.h`. Open receiver. |
| `test/x86` | 6 | 8 | Four `assert.h`; four other direct C/Win32/POSIX headers. Open receiver. |

Production source therefore has no bypass. The test side has 75 direct
external-header rows, all of which are classified below rather than treated as
implicit exceptions.

## Types-Owned Production Declarations

The 30 direct system includes occur only below `src/lib/types/`: standard C
declarations in `types_interface.h` and `file.h`; Win32 declarations in the
`types/win32/` groups; Linux declarations in the `types/linux/` groups; and
the compiler atomic declaration in `atomic.h`. The production corpus gates
confirmed no direct system header exists outside this owner.

## Open Test Receivers

### Assertion Header Rows: 53

`assert.h` is directly included 34 times in `test/lib`, 15 in `test/common`,
and four in `test/x86`. It is an external macro/function vocabulary bypass
under the literal six-root rule. It has no existing `lib/types` declaration
or approved test-only exception. Receiver decision required: either define a
minimal shared test assertion boundary, or formally retain an explicitly gated
test-only exception. S1 makes neither change.

### Native/C/POSIX Header Rows: 22

| File and line | Direct header | Required later disposition |
| --- | --- | --- |
| `test/common/machine_fixture.h:5` | `windows.h` | Use an existing/new Types contract or a documented native-fixture exception. |
| `test/lib/cleanup.h:7,10` | `stdio.h`, `windows.h` | Same; current cleanup fixture directly owns `FILE` and Win32 values. |
| `test/lib/console_blocking_gate_smoke.c:2` | `windows.h` | Same. |
| `test/lib/console_broker_display_smoke.c:4` | `stdio.h` | Same. |
| `test/lib/console_broker_smoke.c:9` | `windows.h` | Same. |
| `test/lib/kvm_frame_lock_smoke.c:2` | `windows.h` | Same. |
| `test/lib/kvm_console_retirement_barrier_smoke.c:7` | `windows.h` | Same. |
| `test/lib/lib_console_event_gate_smoke.c:7` | `windows.h` | Same. |
| `test/lib/kvm_window_modal_smoke.c:1,3` | `windows.h`, `stdio.h` | Same. |
| `test/lib/linux_storage_contract_smoke.c:5` | `stdio.h` | Same. |
| `test/lib/linux_storage_lock_smoke.c:5-7` | `stdio.h`, `stdlib.h`, `unistd.h` | Same. |
| `test/lib/storage_file_writer_binary_smoke.c:6-7` | `stdio.h`, `stdlib.h` | Same. |
| `test/lib/win32_presentation_smoke.c:5` | `stdlib.h` | Same. |
| `test/x86/debug_machine_smoke.c:5` | `stdio.h` | Same. |
| `test/x86/xasm32/xasm32_bounds_smoke.c:7,9-10` | `windows.h`, `sys/mman.h`, `unistd.h` | Same. |

The affected files also directly call the native/C facilities supplied by
those headers (for example Win32 event/thread calls, C streams and POSIX file
or mapping operations). Thus a header-only cleanup would be false compliance:
the later repair must migrate both declarations and use sites, or record a
narrow test-harness boundary with an enforceable static rule.

## Existing Static-Gate Evidence

| Gate | Result |
| --- | --- |
| Lib production Types layout / corpus checks | Pass on x64 and x86. |
| Common production corpus, negative and manifest checks | Pass on x64 and x86. |
| x86 production corpus, negative and manifest checks | Pass on x64 and x86. |
| Test-corpus manifest checks | Pass on x64 and x86. |

These gates prove the production result. They intentionally do not yet reject
ordinary test-source native vocabulary, which is the concrete repair receiver
identified by this ledger.

## Verification And Delivery

| Check | Result |
| --- | --- |
| MyNES x64 complete repository-only unit suite | 53/53 passed. |
| MyNES x86 complete repository-only unit suite | 53/53 passed. |
| Shared six-root x64 selection (`library|common|x86`) | 75/75 passed. |
| Shared six-root x86 selection (`library|common|x86`) | 75/75 passed. |
| 0043 x64 artifact | `assets/mynes/mynes_0_0_0043_x64.exe`; PE x86-64; SHA-256 `ACC2B18A59CCB5541AB79837355F2D87E473BF0BAD5013D94314FA89F1C28CDD`. |
| 0043 x86 artifact | `assets/mynes/mynes_0_0_0043_x86.exe`; PE i386; SHA-256 `CFFFD846CB844B12522DEFC82405C5BAE69278BABA8F2E06F3199195794277A5`. |

## S1 Conclusion And Next Decision

No Shared source or test code changed in S1. Production is compliant. The
test-side 75 direct external-header rows are a bounded, complete repair batch;
they prevent T43 from claiming six-component compliance. The owner must
approve the receiver policy before a later S changes Shared tests or expands
`lib/types`.
