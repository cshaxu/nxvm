# M5 T538: Deployed Boot Pairs

Owner admitted 2026-09-25. [Proposal](../proposals/m5-deployed-boot-pairs.md).
S1 inventories and reproduces all deployed pairs; task remains open.

Identifier reconciliation: T537 is already present in immutable NXVM history
(including 630a5b014). The later T41 shared-adoption record is preserved as a
historical cross-product hosting record; neither it nor MyNES T43 resets the
NXVM sequence. This new implementation uses T538, not a reused lower number.

Coverage and results: [ledger](../etc/evidence/t538-boot-pairs.md).

## S1 Executor Delivery

Eight EXE/INI pairs and external inputs are frozen and attempted. Seven pairs
have an observed DOS/installer checkpoint; Model 40 x86 remains unaccepted.
The owner's intermittent 5170 303 report on both widths remains unresolved
despite three additional successful cold starts per width. No repair is claimed.
Full x64 units pass 335/335; x86 passes all 335 via non-desktop/desktop partitions.
The NXVM documentation gate passes. All deployed EXE and INI hashes are unchanged.

Actual-change self-review covers the Windows-only diagnostic observer, its
NXVM CMake target and four task documents. No Shared/MyNES or production source
is changed. The observer uses the real monitor, has bounded observation and
external watchdog containment, and deliberately has no passing CTest assertion.
It does not define a second guest execution path. The new diagnostic C file is
238 lines, plus eight CMake lines (246 added, zero removed, counted as physical
source/build lines excluding documentation/artifacts). This positive increase
provides real deployed-process evidence missing from the old monitor-only test.
Native APIs are confined to this Windows diagnostic executable, not product ABI.
Raw logs remain ignored for the immediately following diagnostic batch.
No product rebuild is required for this test/documentation-only S.

## S1 Coordinator Acceptance

Reviewed the actual six-file `5e6ed82cd` diff against the original request and
S1 packet: scope is baseline diagnosis, all eight are attempted, unresolved
members remain explicit, and neither a green unit suite nor a successful cold
boot is presented as T completion. CMake adds only the diagnostic executable;
it cannot become an automatic boot pass. The native observer's owned process
cleanup and stale Window-handle reset are checked. Documentation links, packet
shape, identifier history and unchanged Queue agree. S1 is accepted and closed;
T538 remains open for the two named residual classes. No Shared authority was
consumed and no product behavior was modified.

## S2 Executor Delivery

Owner explicitly accepts the 5170 keyboard repair as a separate S and transfers
the independently reproduced Console capacity failure to S3. KBC now prevents
unmatched Set-2 breaks from starting typematic and routes repeats through the
same serial/inhibit boundary as native input. This is one controller-owner
repair, not a BIOS or profile exception. Both original widths reproduce 303;
both repaired widths reach DOS Setup under the same 50 ms Return-release input.
Reset/resume and stop/start also reach Setup. Model 40 x86's complete visible
frame and paused memory resolve the earlier capture ambiguity without a guest fix.

The isolated S2 source uses the committed Shared corpus, passes 335/335 units
per width and 20/20 external integration, and rebuilds eight optimized stripped
0538 products. Final isolated AT processes each reach Setup and remain alive
through 180 seconds. The ledger records source/artifact hashes, unchanged INIs,
actual-diff review (+188/-55 across four source/test/build files), and the
same-mechanism sweep. Only NXVM changes belong to this delivery. Pending Shared
and MyNES work is preserved for S3. Whole-task repeat coverage remains pending;
neither the S acceptance nor the green integration suite closes T538.

## S2 Coordinator Acceptance

Reviewed actual implementation 1d80f11d8, not only its test summary: the KBC
change keeps one serial publisher, the new regressions cover unmatched breaks
and inhibited repeats, and the observer never treats its exit code as boot
acceptance. Its native APIs remain test-local. The eight deployed binaries are
the isolated KBC build recorded in the ledger; no Shared or MyNES files are in
this commit and no INI is changed. Old artifacts are recoverable from Git.

Original request, latest owner-directed split, packet, actual diff and retained
coverage agree. S2 is accepted and closed after its successful push. S3 is the
authorized next receiver for Console capacity and the already-preserved Shared
candidate; T538 remains open and the complete repeat matrix is not waived.

## S3 Executor Delivery

Shared 6a3f3cb25 removes viewport-size requirements from the sole raw text
storage path. Full backing storage is preserved without changing fonts or
discarding offscreen rows; actual allocation/output failures still propagate.
Three code/test files total +34/-68 lines. ABI and product source are unchanged.
The [S3 evidence](../etc/evidence/t538-s3-console-buffer.md) records the sweep,
335/335 NXVM units per width, 20/20 integration, 132/132 MyNES tests per width,
six manifests, ten rebuilt receivers and unchanged owner configurations.
This is not completion of the T-level repeated real-process boot matrix.

## S3 Coordinator Acceptance

One-session coordinator review inspected actual Shared 6a3f3cb25, NXVM
7d29f409b and MyNES a60dcb906 changes against the owner request and packet.
This is self-review, not an independent reviewer. The buffer-only owner change
matches the approved semantics; tests retain genuine output-failure detection.
No API, INI, media, guest state or product source change is hidden in delivery.
Ten receiver hashes and architecture verification match evidence, both product
documentation gates and six manifests pass, and every P has exactly one target.
All three commits were pushed immediately. S3 is accepted and closed; T538
stays open for repeated deployed-process qualification. The distinct startup
rollback TODO is neither removed nor claimed repaired.

### S3 Completed Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M5 T538 S3, after accepted S2. |
| Admission And Approval | Owner explicitly requested S2 closure and a separate S for the Console repair on 2026-09-25; complete backing storage with a smaller scrollable viewport is approved. |
| Objective | Remove false Console delivery failure caused solely by an undersized visible viewport while preserving all guest text cells. |
| Non-goals | No font scaling, frame truncation, automatic Window fallback, new API, KBC change, owner INI change or unrelated MyNES source change. |
| Reference Baseline | S2 implementation 1d80f11d8 and acceptance 3922622ea; Shared 75099c178; four frozen INIs. |
| Candidate Proposal | [Deployed boot pairs](../proposals/m5-deployed-boot-pairs.md). |
| Files And ABI Surface | Shared Console broker, two existing Lib tests, README and two manifests. NXVM eight 0538 and MyNES two 0043 receiving binaries/evidence. Public ABI unchanged. |
| Applicable Rules | Shared EXECUTION, ARCHITECTURE, CODING, DOCUMENT; NXVM and MyNES architecture/coding/UI and source policies. Broker is the sole host buffer owner; copied guest frame is not modified to match viewport. |
| Verification | Full/offscreen frame and smaller/scrolled viewport; rejected/ignored buffer growth and clipped native writes; stale lower-row clearing; existing Console handoff. Full NXVM units both widths and integration; full MyNES receiver suites both widths; all six manifests; optimized stripped receivers and documentation/diff review. |
| Expected Markers | Undersized viewport no longer reports IO_ERROR; every active cell exists in backing storage; genuine storage/output failure still propagates. |
| Asset Needs | Existing lawful external inputs only, no master or INI changes. Existing private diagnostic evidence may be reused when source identity matches. |
| Reporting Requirements | Distinguish implemented candidate from deployed artifact and whole-task boot qualification; report both consumers and retained rollback debt. |
| Stop Conditions | New host policy/API, product-state coupling, unexpected receiver regression or edits outside approved targets require review. |
| Exit Criteria | Approved buffer policy and regressions pass; manifests/docs consistent; affected receivers rebuilt and verified; target-separated complete delivery reviewed and pushed. T-level three-fresh-launch matrix remains a later gate. |
| Original Owner Request | Close the 5170 repair S, then admit a separate S to fix Console buffering without shrinking fonts or discarding offscreen data. |
| Similar-Issue Sweep | Inspect raw text delivery, palette-before-geometry, tall-to-short clearing and cooked restoration. The separately recorded partial-startup history rollback remains TODO, not claimed fixed. |

## S4 Admission And Import Review

Owner accepts the current S and requests a new import S; S3 was already closed,
so coordinator admits the next unused S4 rather than reopening or re-closing it.
The [S4 evidence](../etc/evidence/t538-s4-console-import.md) pins SoftPC b79769c1
and records same-path import of six reviewed files, with all 227 files across
six shared roots equal. Normal/failure writes keep one broker owner; committed
cache and retry coverage are distinct facts, not duplicate rendering states.
No public API or product source changes. Shared P1 4ca7e6401 is pushed.
Executor qualification passes: NXVM 335/335 units per width and 20/20 external
integration; MyNES 132/132 per width; all ten optimized stripped receivers,
six manifests, component boundaries and Types gates. Owner INIs and snapshot
are unchanged. Final coordinator acceptance follows delivery; T538 stays open.

## S4 Coordinator Acceptance

Reviewed actual commits: Shared 4ca7e6401 changes only the six approved import
files; NXVM 882e6959a contains eight receiving EXEs and NXVM evidence; MyNES
57f0e79e5 contains two receiving EXEs and MyNES evidence. Each was pushed
immediately. No product source, INI, snapshot, media or sibling files changed.

Final independent path/hash and committed-tree comparisons confirm all 227
files across the six roots match clean SoftPC b79769c1. All six manifests,
both documentation gates and diff checks pass. NXVM units pass 335/335 per
width, external integration 20/20; MyNES suites pass 132/132 per width. Ten
optimized stripped receivers are delivered with recorded hashes. S4 is accepted
and closed; T538 stays open for its eight-pair, three-fresh-launch qualification.
Build trees are retained for that next qualification; no new task is admitted.

### S4 Completed Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M5 T538 S4 after accepted S3; no reopening or duplicate closure. |
| Admission And Approval | Owner requests a new S to import reviewed SoftPC repairs and verify complete six-root equality on 2026-09-25. Allowed targets: Shared, NXVM and MyNES receiving artifacts/evidence only. |
| Objective | Import SoftPC b79769c1 Console output-extent and retry-coverage repair unchanged, and prove all six source/test roots match that committed source. |
| Non-goals | No sibling writes, new API, product source or INI/media changes, font scaling, viewport resizing or whole-task boot qualification claim. |
| Reference Baseline | NXVM 763a70134; SoftPC b79769c1 clean worktree, including its reviewed S8 P2 fix. |
| Candidate Proposal | [Deployed boot pairs](../proposals/m5-deployed-boot-pairs.md), Console output batch. |
| Files And ABI Surface | Six changed files in src/lib and test/lib; other four roots and test/register.cmake verified unchanged. Eight NXVM 0538 and two MyNES 0043 receivers plus target-owned evidence. ABI unchanged. |
| Applicable Rules | EXECUTION, ARCHITECTURE, CODING, DOCUMENT; both product guides, architecture/coding/UI and source policies. Only broker owns native frame storage; cache validity and retained coverage are separate facts; no product policy enters Lib. |
| Verification | Exact six-root path/byte comparison to pinned source and current sibling; all six manifests and component/Types gates through complete suites. NXVM full units x64/x86 and external integration; MyNES full suites both widths; all ten optimized stripped product builds; INI hashes unchanged; both documentation gates and actual-diff review. |
| Expected Markers | Steady 25-row frame writes 25 rows; 50-to-25 clears the old tail; partial writes retain retry coverage; 50-to-30 native shrink with a 25-row frame succeeds; undersized viewport is not a failure. |
| Asset Needs | Existing lawful external assets only; preserve masters, configurations and snapshots. Reuse current build trees for receiver verification and the immediately following T qualification. |
| Reporting Requirements | Report equality per root, pinned source and final sibling state, complete tests, ten receiver hashes, per-target pushed commits and remaining T gate. |
| Stop Conditions | Unreviewed sibling changes, license/API changes, test regressions or unrelated product edits require coordinator review; do not chase a moving upstream silently. |
| Exit Criteria | Reviewed import is byte-identical to pinned six-root corpus; required tests/builds/manifests pass; all receivers and evidence are committed/pushed per target; coordinator reviews actual changes; clean tree. T538 stays open. |
| Original Owner Request | Close current S, admit a new S to import SoftPC fixes, then verify six components are completely identical. S3 was already closed. |
| Similar-Issue Sweep | Reconcile all cache/coverage resets and raw-write extents in Console broker plus native/fake tests; check steady, shrink, stream-write invalidation and failed-write retry. Linux and KVM leaves do not own the Win32 backing rectangle. Existing cooked-history rollback TODO remains separate. |
