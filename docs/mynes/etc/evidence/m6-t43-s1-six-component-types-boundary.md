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

## S2 Repair And Verification

Correction: S3 review disproved S2's complete-compliance claim. Its passing
test gate missed ten native pointer references; a manual relative-root call
could also scan no files. The S2 results below are historical, not proof that
the full Types migration was complete.

S2 resolves the complete S1 test receiver rather than making a test-only
exception. `lib/types/test.h` owns the neutral assertion vocabulary and the
Win32/Linux test aggregates collect the exact platform declaration groups used
by tests. The owner groups also gained only the missing neutral aliases for
the existing test uses (including Win32 scalar, pointer, callback, memory and
message declarations, plus Linux memory declarations). No product behavior,
platform implementation, or test expectation changed.

All ordinary C/H sources below `test/{lib,common,x86}` now consume that Types
vocabulary. Deliberately invalid source inputs below `test/lib/fixtures/` stay
outside the rule: they are data for negative static-gate tests, never compiled
as Shared corpus source. `library.test-types-boundary` enforces this scope;
each of `src/lib`, `test/lib`, `test/common`, and `test/x86` now has manifest
revision `shared-m6-t43-s2-p1`.

One white-box failure-path test intentionally intercepts two Types-owned
Win32 functions. It captures the original Types aliases before installing its
test macros, so its own rescue and observation paths continue to use the
official Types declaration rather than restoring a raw native bypass.

| Check | Result |
| --- | --- |
| Test source external-header scan, excluding deliberate fixture inputs | 0 rows |
| Shared test Types boundary gate | Pass |
| Types declaration layout gate | Pass |
| `src/lib`, `test/lib`, `test/common`, `test/x86` manifest gates | 4/4 pass |
| Shared x64 unit selection, serial | 76/76 passed |
| Shared x86 unit selection, serial | 76/76 passed |
| MyNES x64 repository-only unit suite | 53/53 passed |
| MyNES x86 repository-only unit suite | 53/53 passed |
| Documentation governance | Pass |
| Diff whitespace check | Pass |

The Shared selection is deliberately serial: existing negative verifier tests
create temporary fixture files and are not concurrency-isolated. Serial
execution proves the full suite without accepting a timing-dependent fixture
collision as a product failure.

| Artifact | Result |
| --- | --- |
| `assets/mynes/mynes_0_0_0043_x64.exe` | PE x86-64; SHA-256 `0FF46E1958EFCDD9D3AFBB2F94386F22ADD648E7FB71B82329D27EACE70F6796` |
| `assets/mynes/mynes_0_0_0043_x86.exe` | PE i386; SHA-256 `CB0927B5DEB92871884FAA063FDD6C99185D42BA0E9969C810E83F20867010D4` |

## S3 Pointer Batch And Gate Correction

Baseline: `938899d2b`. The complete reported batch was ten uses in
`console_broker_display_smoke.c` (2), `console_broker_reader_failure_smoke.c`
(1), `kvm_window_capture_contract_smoke.c` (3 signatures / 4 uses), and
`lib_console_io_contract_smoke.c` (4). These are eleven type uses across ten
source lines; the original report counted lines, not individual tokens.
Existing Types records plus explicit pointers preserve SDK signatures,
including const RAWINPUTDEVICE input; the pointer repair adds no aliases.
The wider uppercase-token and native-call sweep also found LONGLONG, three
SC_* constants and eight native functions in the Lib tests. These now have
one-to-one declarations in the existing Types scalar/window/console/sync/process
owners. There is no new platform implementation or product-facing API.

The gate now rejects the seven pointer spellings and normalizes relative roots.
Missing, empty or source-free roots fail. Its isolated self-test rejects all
eleven spellings from the owner's report, the four additional type/constants,
the eight additional native calls, a native header/function and an empty scan;
it accepts Types pointers and raw words in comments/strings.
The relative-root self-test exposed and verified the zero-file-scan repair.

The six-root P*/LP* and LONG/LPCWSTR/SIZE_T/SHORT sweep excluded only the Types
declaration owner and intentional invalid fixture data. After removing comments
and string literals, remaining P-prefixed tokens are x86-owned instruction,
prefix, condition and operand names (PUSH/POP/PREFIX/PTR and PE/PF/PL/PO).
There are no remaining native pointer hits in that batch. The test-only
declaration route remains `lib/types`; no parallel implementation exists.

Final verification: complete configured x64 and x86 suites each passed
130/130 (77 Shared plus 53 MyNES); all six manifests, documentation governance
and diff checks passed. Shared implementation `f5a9bd34a` was exported with
`git archive` to `build/t43-s3/committed-corpus`; all six manifest checks and
the test Types boundary passed against that committed export. This is the
fixed import baseline, independent of the working tree's newline conversion.

Import reproducibility correction: S2 manifests hashed CRLF working files
although Git's attributes commit LF. The six roots were normalized to LF and
all six manifests regenerated. This changes no source semantics or Git source
content outside the admitted edits; it corrects hashes to the committed bytes.
Each manifest now records `shared-m6-t43-s3-p1`.

Actual-diff review: all eight changed C tests preserve their assertions,
control flow and native signatures. The five Types headers add twelve direct
declarations (one scalar, three constants, eight functions). A second scan
cross-referenced the raw right-hand-side names of every Types macro against
all three test roots after removing comments/strings; it returned zero hits.
The only remaining capitalized tokens outside owner-prefixed vocabulary are
SDK record member names and character literals, neither a separately owned
type/function/constant declaration.

Code-size accounting uses `git diff --numstat` for `src/lib/types` and
`test/lib`, excluding manifests, plus the new self-test: 87 added, 24 removed,
net +63 lines. The increase is declaration ownership and executable negative
proof. The runtime implementation remains unchanged.

Final rebuilt artifacts (PE architecture read from the executable header):

- x64: `assets/mynes/mynes_0_0_0043_x64.exe`, machine `8664`, SHA-256
  `7C5F71D39C1D4C7D8629BB1682286A6DF5B65C38AD811635D8850279F8733105`.
- x86: `assets/mynes/mynes_0_0_0043_x86.exe`, machine `014C`, SHA-256
  `D984EB5679FB3980B4FB6ABD141903ADF63C6917C9658745B5714164D5F7678A`.
