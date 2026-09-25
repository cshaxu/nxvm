# M6 T43 S7 Shared Quality Repairs

## Scope And Actual-Diff Review

Owner-approved Shared repair batch based on `f1ce6763e`, executed in one
session with sequential executor/coordinator review. T43 remains open.
No public API, product source, sibling repository, firmware or INI change.

| Batch | Before | Final implementation and proof |
| --- | --- | --- |
| xasm32 labels | Missing definitions could leave placeholder bytes successful. | One temporary resolved flag, existing two matching loops and final output guard; undefined/duplicate/bad operator and forward/backward labels covered in 16/32-bit modes. Existing NOP label marker semantics retained. |
| Audio prefix | Cancellation could hide already released native frames and replay a prefix. | Native acceptance published per chunk; FIFO retires that prefix under its existing generation/lock, independently of cancellation result. Deterministic prefix/suffix byte comparison proves no duplicate. |
| Audio cancellation | Clear or destruction could publish a wake in the wrong order. | Clear interrupts before publishing control; native clear consumes outstanding interruption. Destroy publishes task cancellation before native interruption. Existing blocking-wait teardown regression retained. |
| Audio tests | Callback midpoint was treated as worker completion. | Removed premature fake event; use actual flush/FIFO completion. No physical audibility or mute prerequisite. |
| Common admission | Reset/signal errors ignored; executor could claim before rollback. | Four request kinds serialize admission/claim on existing request lock; failures retract admission and return error without waiting or executing. |
| Common completion | Failed completion notification could strand a synchronous caller. | Existing task cancellation is independent wake, followed by worker join before error return. No retry of completed side effects; ERROR survives terminal cleanup. Dispatch reset failures terminate rather than spin. |
| Corpus consistency | Missing Audio README dependency and duplicate Linux test include. | Corrected dependency and removed duplicate include; six manifest revisions/hashes refreshed. |

Common production diff is +127/-53. All tracked changed C/H files together:
+350/-99, net +251; production +171/-81 (net +90), tests +179/-18
(net +161), measured using `git diff --numstat f1ce6763e -- '*.c' '*.h'`.
The increase is explicit failure transitions and deterministic regressions,
not a second executor, queue, event, parser or public abstraction. The original
assembler tables/loops and existing Audio FIFO/Common worker remain sole owners.
Architecture/coding governance guided the owner-local failure boundary and
preservation of the original assembler implementation style.

## Similar-Issue Sweep And Boundaries

Searched label materialization and all `flag_has_label` / `aasm32_execute`
exits in aasm32; both materialization branches and final copy guard are covered.
Searched `base_sync_event_(signal|reset|wait)` and `base_sync_wait_any` in
Common machine and Audio, plus completion/flush/cancellation in their tests.

All synchronous Common debug/media/read-state/write-state admission and
completion variants share the repaired path. Read-state deliberately retains
its multi-stage armed/ready state because it borrows a stream across executor
safe points; it is not a duplicate request implementation. Tests inject failed
reset/wake/completion, stale completion, exactly-once media effects, and real
task cancellation/join ordering. A failed native join retains the machine.

Simultaneous failure of completion and independent task cancellation, or
inability to prove thread exit, is NOT a recoverability guarantee. Callback
and borrowed stream contexts must remain alive until successful shutdown.
No polling, forced thread termination or unapproved public escape API added.
Asynchronous lifecycle/input/Session wakes and Audio control-completion failure
reporting have different contracts; the complete residual class is transferred
to the named P2 entry in `states/TODO.md`, not claimed repaired by this S.

## Final Verification

Final frozen source was rebuilt after the dispatch-reset repair. MyNES configured
suite includes all 77 Shared tests/gates and 53 product tests; running only its
56 unit-labelled Shared tests would not qualify the receiving product.

- Shared/MyNES x64: 130/130, 156.83 seconds.
- Shared/MyNES x86: 130/130, 158.36 seconds.
- NXVM complete repository-only unit x64: 335/335, 61.40 seconds.
- NXVM complete repository-only unit x86: 335/335, 61.15 seconds.
- All six manifests and Types ownership/test boundary: pass.
- Both product documentation gates and whitespace: pass.
- No new external-ROM NXVM integration, Linux runtime, physical-audio quality
  or owner manual acceptance claim. Earlier runs are not substituted for this
  final source verification.

Commands: `cmake --build build/mynes-gcc-{x64,x86}-release --parallel 8`,
then complete `ctest --test-dir <tree> --output-on-failure -j 1`.
NXVM rebuilt each four-profile target `vm-0-5-0535`, then
`cmake --build build/t41-s8-nxvm-{x64,x86} --target run-unit-tests --parallel 8`.
Reusable build trees remain for open T43; CTest owns temporary diagnostics.

## Receiving Artifacts

Developer identities remain MyNES 0.0.0043 and NXVM 0.5.0535. Both optimized
Release widths are refreshed, PE machine fields checked and objdump confirms
no debug sections. MyNES numeric identity is its CMake target/deployed filename;
its current banner is unversioned. NXVM embeds 0.5.0535. Shared source revision
is S7 P1 `38ed0f26f`; artifact commits
follow without source changes. NXVM's eight hashes are in its
[receiving record](../../../nxvm/etc/evidence/m6-t43-s7-receiving-artifacts.md).

| Artifact | Bytes | PE | SHA-256 |
| --- | ---: | --- | --- |
| `assets/mynes/mynes_0_0_0043_x64.exe` | 246798 | 8664 | `1649F2F029B13EA0AA2E6C56D30ED941B48DCDFE7155DAFF8B6CAF221C82DD31` |
| `assets/mynes/mynes_0_0_0043_x86.exe` | 239630 | 014C | `13856C38536304F6DE56D9CE64DF52B18D4D05C1BB2E1094F2407C7E7250A678` |

The unrelated existing default NXVM.ini edit is preserved and excluded.
Delivery is split into Shared implementation, NXVM receiving artifacts, MyNES
records/artifacts, then coordinator governance acceptance. S closure does not
close T43 or replace the owner's pending hands-on verification.
