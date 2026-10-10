# T548 S5 Active Test-Identity Ledger

## Scope And Method

This ledger reviews active unit source paths, CTest target identities and
success-marker text only. Historical evidence is deliberately out of scope.
An identifier is task-derived only when it contains an explicit task-shaped
token such as `_s14`, `_t242` or `M5:T375:S2`; technical terms such as
`tss32`, `sreg` and source citation identifiers are not task markers.

The inventory was taken after S4 at committed baseline `3d54512ac`:

- 26 active unit source paths contain an explicit `_sNN` or `_tNN` token;
- 67 active unit source files emit an explicit `M*:T*:S*` success marker;
- 92 CMake files contain a broad `T`/`S` lexical match, but most are either
  ordinary CMake, a technical term, or a historical verifier whose semantic
  rename requires individual review.

No rename is authorized by this ledger alone.

## S6 Disposition: Model 40/D4

S6 renamed the twenty MyDeskPro386 paths in the first row to behavior-based
identities, updated their CMake/CTest identities, and removed the matching
task provenance prefixes from active success output.  The Model 40/D4
predicates and fixed-profile context were not changed.  All twenty replacement
targets pass on x64 and x86; the Core test-boundary verifier also passes.
Historical evidence remains unchanged.

## S7 Disposition: Default PC

S7 renamed the six Default-PC source paths in the second row to behavior-based
identities, updated their CMake/CTest registrations, and removed task
provenance from every active Default-PC unit-test output marker.  The six
replacement executables pass on x64 and x86; all changed Default-PC test
sources compile on both widths.  A full-graph build independently exposed the
pre-existing `integration-session-ini-support` reference to retired
`app_composed_machine.machine`; it is not part of this identity-only change.

## Path Rename Batches

| Scope | Count | Current pattern | Required later receiver |
| --- | ---: | --- | --- |
| MyDeskPro386 | 20 | Model 40/D4 source filenames with `_sNN` suffixes | One MyDeskPro386-only S: retain Model 40/D4 subject names, remove only the task suffix, then update every local target/manifest reference. |
| NXVM Default PC | 6 | `pcat_*_sNN`, `machine_*_sNN`, `fdc_t242_*` | One NXVM-only S: choose descriptive board/machine names from asserted behavior, then update local target/manifest references. |
| My5160 / My5170 | 0 | No active task-derived unit source path | Retain paths; their output markers are handled separately. |
| Core / Lib / Emulator / Product | 0 | No active task-derived unit source path | Retain paths; do not infer a rename from incidental source text. |

## Output-Marker Batches

| Scope | Files | Disposition |
| --- | ---: | --- |
| NXVM Default PC | 38 | Replace task provenance in ordinary success output with behavior identities in the same S as any renamed source/target. |
| MyDeskPro386 | 24 | Replace task provenance with Model 40/D4 behavior markers in the MyDeskPro386 rename S. |
| My5170 | 4 | Replace task provenance with Model 339 behavior markers in a My5170-only S. |
| My5160 | 1 | Replace task provenance with XT behavior markers in a My5160-only S. |
| Core / Lib / Emulator / Product | 0 | No active explicit task-formatted success marker found by this scan. |

## S8 Disposition: My5170

S8 replaced task provenance in the four currently active My5170 source files:
the Model 339 CGA topology, firmware/FDC topology, clock-contract, and
composition tests. The first three contain eleven success markers; composition
contains one failure diagnostic. This confirms the baseline ledger's four-file
count without changing a source path, predicate or registration. All four
targets pass on x64 and x86.

## S9 Disposition: My5160

S9 replaced the eight task-provenance success markers in the XT 5160 profile
test with behavior-only identities. Its source path and assertions remain
unchanged, and the target passes on x64 and x86.

## CMake Registration Debt

Task-shaped CMake helper names are not renamed mechanically. They fall into
three kinds:

1. semantic checks whose target name should become behavior-based after a
   direct review (`verify_t359_instruction_timing_inventory.cmake` is a known
   example);
2. historical compatibility/shape guards whose current scope must be decided
   before renaming; and
3. lexical false positives, including ordinary test/support files.

The next rename S must take one finite target-scoped group, prove every old
CTest identity is absent after migration, and preserve its exact predicate.
It must not rewrite evidence documents merely to erase historical task IDs.

## S10 Disposition: CMake Task-Shaped Identities

The remaining CMake matches were reviewed by execution role rather than by
their spelling.  No active CTest unit name or test-source path still contains
an explicit task identifier.  The task-shaped entries below are all CMake
configuration, verification, or integration helpers.

| Group | Current role | Disposition | Reason |
| --- | --- | --- | --- |
| `T317`, `T332`, `T337`, `T344`, `T345`, `T382`, `T388`, `T435`, `T447` | Active unit/corpus verification helpers and custom targets | Semantic rename receiver | These names describe current test qualification behavior, not a preserved external or historical contract.  A later finite CMake-only batch may rename each group to its behavior (for example, strict CPU smoke coverage, undefined-opcode disposition, fixture-shape verification, or timing-inventory verification) while preserving every predicate. |
| `T264`, `T330`, `T331`, `T338`, `T359`, `T360` | Active ownership, construction, profile-metadata, or timing-ledger verification helpers | Semantic rename receiver | They are live verification targets, not evidence files.  Their historical IDs should remain only in the evidence documents that their predicates consume. |
| `T515`, `T533` | External-asset and Console integration registration helpers | Retain, out of S10 unit scope | These names register or classify integration routes, not unit CTest identities.  Their eventual rename belongs to an integration-specific receiver so this unit-only S cannot accidentally alter external-asset behavior. |
| `T296`, `T314`, `T345` comments, and all task IDs embedded in verifier diagnostics | Historical rationale or a diagnostic provenance string | Retain | A textual task ID alone does not name a unit test.  Rewriting it would erase audit provenance without improving source/test ownership. |
| `pit825x`, `int13`, `sreg`, `tss32`, fixed-width integer tokens | Lexical false positive | Retain | The matched letters/digits are technical identifiers, not task provenance. |

The next finite CMake rename batch is therefore limited to live *unit/corpus*
verification helpers.  It must not absorb the `T515`/`T533` integration
helpers, historical evidence paths, comments, or diagnostic provenance.  The
batch must retain all custom-target dependencies and all failure predicates;
it changes technical identity only.

## S14 Disposition: Fixture-Shape Verification

The live fixture-shape verifier now uses behavior names for its file, target,
variables and messages. Its 101 retained historical fixture identities remain
unchanged. A current-tree audit corrected 31 stale renamed source paths,
removed duplicate paths introduced by the verifier's Glob plus explicit list,
and classified the two later direct Core constructors for DMA route rollback
and firmware capability. The verifier now checks 135 unique direct
constructors and passes on x64 and x86. This is CMake/test verification only:
no production source, CTest route, external asset or executable input changed.

## S15 Disposition: Strict-Declaration Uniqueness

The live strict-declaration uniqueness verifier now uses behavior identities
for its CMake file, custom target, variables, generated matrix and diagnostics.
It retains the exact target-local option-record predicate and passes with 532
records on x64 and x86. The distinct direct-compilation matrix and its T345
consumer remain unchanged for a separately scoped receiver. No production
source, CTest route, external asset or executable input changed.

## S11 Disposition: CPU Qualification Verification

S11 renamed the live CPU qualification group without touching CTest routes or
CPU assertions:

| Previous identity | Behavior identity |
| --- | --- |
| `project_configure_t317_strict_cpu_smokes` | `project_configure_strict_cpu_smokes` |
| `verify-t317-test-type-vocabulary` | `verify-test-type-vocabulary` |
| `verify-t317-strict-cpu-smoke-coverage` | `verify-strict-cpu-smoke-coverage` |
| `verify-t332-cpu-fixture-lifecycle` | `verify-cpu-fixture-lifecycle` |

The associated three verifier filenames, two type-vocabulary fixtures,
generated inventory names and internal CMake variables now use the same
behavior vocabulary.  During the rename, the fixture-lifecycle verifier was
shown to contain a real stale-reference defect: it named five retired test
paths and compared full inventory paths against chip-relative classifier
entries.  It now references the active paths and normalizes only the local
comparison key.  The existing 44-owner predicate is unchanged; it now checks
the intended current files.

Both x64 and x86 configurations passed all three affected targets:
fixed-width vocabulary, strict CPU smoke compilation and CPU fixture
lifecycle.  This S changes CMake/test verification only; it does not change
production behavior, CTest count, assets or executable inputs.

## S12 Disposition: Undefined-Opcode Verification

The configuration-time undefined-opcode verifier now uses behavior names for
its source discovery, delivery/terminal disposition sets and helper functions.
It still discovers the same registered unit targets by their source markers,
then enforces exactly the same terminal, real-delivery and explicit
non-delivery requirements.  The verifier intentionally remains a CMake
configuration check rather than gaining a second custom target.  Both x64 and
x86 CMake configurations passed, so the live registration graph has executed
the unchanged predicate.  No CTest route, source assertion, production input
or artifact changed.

## S13 Disposition: Unit Registration Verification

The live unit-registration verifier and generated input names now describe
their behavior rather than the historical task.  Its CTest route/count
predicate remains unchanged.  Exercising it exposed two real stale inputs:

- `ibmpc-build-smoke` was an auxiliary entry with neither a target nor a CTest
  route; it is removed instead of being recreated as an empty test;
- the verifier read `test/core/CTestTestfile.cmake` twice, creating a false
  duplicate-route failure; it now reads that child registration once.

All other auxiliary entries retain a real `unit.*` route.  The verifier now
reports its actual 334 registered routes on both x64 and x86.  No production
source, test assertion, external asset, CTest route or executable input
changed.

## S16 Disposition: NXVM Integration Composition Adaptation

The direct strict-compilation matrix exposed a stale integration-only helper:
`integration_ini_session_restart()` still read the retired
`app_composed_machine.machine` field.  The current composition contract owns
the borrowed machine through `app_composed_machine.composition.machine`.

The receiver initializes its local `app_composed_machine` to zero before
composition, preserving its existing null/failed-composition handling, then
reads the current member.  It does not change the integration route, restart
flow, assertions, production contract or CTest registration.

The unchanged 500-row direct strict-compilation matrix passed on x64 and x86:
497 retained strict rows and three explicitly deferred rows.  This is a
test-only compatibility repair; generated executable changes from the build
graph are discarded rather than published.

## S17 Disposition: Direct Compilation And Deferred Ownership

The active direct-compilation and deferred-ownership verifier group formerly
exposed historical task identities through CMake filenames, custom targets,
variables, generated matrix names and diagnostics.  S17 renames only those
live interfaces to their behavior identities:

- direct-compilation matrix;
- deferred direct ownership;
- owner-test strict classification;
- safe-production strict classification; and
- residual direct classification.

The matrix semantics are unchanged.  While executing the renamed ownership
verifier, the pre-existing residual source ledger was shown to be empty even
though the direct matrix has three deferred `core-product` sources.  The
verifier correctly rejected that mismatch.  The finite ledger now explicitly
records `factory.c`, `ini.c` and `startup.c` as the three Core Product residual
entries, so the existing exact-key predicate can verify rather than remain a
permanent failing route.

Fresh x64 and x86 configuration graphs each verify 500 direct-compilation rows
(497 retained strict, three deferred), 151 ownership rows (148 owner tests and
three exact residual product entries), the duplicate-row negative self-test,
and a 151-command deferred-warning audit.  No production or C test assertion
changed.  The existing build directory's Ninja log recompaction was not used
for this evidence; temporary clean configuration graphs avoid that host-local
metadata failure, and any generated executable changes were discarded.
