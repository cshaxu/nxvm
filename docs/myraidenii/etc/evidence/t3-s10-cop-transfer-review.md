# T3 S10: Transfer Contract And Shared Fault Review

## Current Implemented Boundary

This is the implementation/review record; Current remains the active task
authority. The owner approved the narrowly documented Shared fault-publication
repair after the independent reproduction below. Full qualification and
receiving publication are required before acceptance; S10 is not yet closed.

The three-member functional L2 transfer contract is:

| Command | Source | Destination | Effect |
| --- | --- | --- | --- |
| `5205`, `5a05` | COP address register 0 | address register 1 | Copy four bytes. |
| `f205` | address register 0 plus four | address register 2 | Copy four bytes. |

The pinned MAME research unit `src/mame/seibu/seibucop_cmd.ipp` at
`c1425a1af00b88aa07477b1b23a30ed5e5f856ff` has SHA-256
`B1E84AEF5BFDEBCA53D8F0AF0B1BE481B8218157BA460C1714853D322AC92EFC`.
Its three operation declarations establish these L2 functional leads only;
no reference function body, uploaded program or table is copied into source.
The implementation uses original bounded public Core transactions, not the
reference implementation's memory API. This does not establish physical
latency or a generic programmable COP interpreter.

Runtime upload storage has 256 words in eight-word slots and retains staged
value/mask/trigger metadata. Complete word lanes and metadata are required
before a recognized uploaded trigger is admitted. Unknown, incomplete and
busy commands fail explicitly. No game-uploaded word or fingerprint is
embedded. The model owns only the stated recognized macro effects; arbitrary
alternate uploaded microprogram semantics remain unqualified and this must
not be reported as a complete programmable processor implementation.

One pending command uses the first existing Core advance after the CPU write
commits. The current V30 retirement staging prevents the next instruction
from running before that service point. Source/destination register writes
are refused while the request is pending. Two word reads precede both word
writes, preserving overlap semantics. Read failure writes nothing; second
write failure keeps the accepted first word. No compensating write, second
clock, thread or shadow error state is added. Failures are sent through the
existing public Core report_fault operation, with lib_status in fault detail.

The updated existing Board unit covers all three command locations/forms,
overlap, final upload slot, incomplete/empty/unknown program admission,
partial-byte upload, busy/reset, both read/write refusal positions, high
address bits and pointer overflow. Successful transfer/admission cases pass.
Six asynchronous failure cases originally exposed the same Shared issue on
both widths. After the approved repair, the entire existing Board unit and
Core executor unit pass on both widths without relaxing budgets or assertions.

## Reproduced Shared Defect

After a device callback successfully invokes report_fault(), a run ending on
the same instruction or partial-tick budget overwrites FAULTED with PAUSED
and reports BUDGET/OK. This is independent of COP: a disposable synthetic
V30 NOP probe uses only public Core construction, attachment, memory/debug
setup and run. Its callback reports one fixed neutral fault detail.

| Budget | report_fault | run result | final lifecycle | result detail |
| --- | --- | --- | --- | --- |
| one instruction, no tick cap | OK | OK / BUDGET | PAUSED | zero |
| one instruction, one tick cap | OK | OK / BUDGET | PAUSED | zero |

The same issue occurs in the real Board transfer fixture at each read/write
failure position and both address-failure cases. With an extra instruction
left in the budget, the failure is reported, which is why that weaker fixture
initially passed. The tightened final fixture remains failing; no budget or
assertion is relaxed to accept the defect.

Relevant actual owners are machine.c's pending-retirement/public-run boundary
and machine_scheduler.c's elapsed publication. Eight publication sites inside
run and the paused/deadline advance callers were inspected. No public API,
type, callback, enum, clock or persistent fault field is missing.

## Owner-Approved Minimal Shared Repair

Keep one public run entry and one private bounded execution body. Preserve
all existing eligibility and CPU logic. The scheduler's existing publication
operation returns INTERNAL_ERROR if its callbacks leave Core FAULTED, allowing
existing caller failure branches to stop before overwriting that lifecycle.
The public run boundary normalizes fault results once, using existing fields:

```text
publish_elapsed_ticks(...):
    existing_scheduler_advance(...)
    return FAULTED ? INTERNAL_ERROR : OK

run(...):
    validate_existing_public_arguments()
    start_tick = current_elapsed
    status = bounded_execution_body(...)
    if lifecycle == FAULTED:
        result.reason = FAULT
        result.detail = existing_fault_detail
        result.ticks = current_elapsed - start_tick
        result.elapsed_ticks = current_elapsed
        result.linear_pc = existing_current_pc
        return INTERNAL_ERROR
    return status
```

The private body is the existing execution implementation, not a second
executor or alternate path. The wrapper has the specific responsibility of
publishing a consistent result for every callback-failure exit, including
partial budgets; it is not a forwarding facade. Existing retired-prefix count
is preserved. Normal non-fault returns and early invalid arguments preserve
their contracts. Existing CPU-pending fault accounting must remain distinct
from an already-reported FAULTED lifecycle.

Expected Shared production delta: approximately +25--40/-5--15 in the two
existing Machine files. Extend one existing Core execution unit with a small
table of instruction/tick budgets and before/after advancement injection,
approximately +40--80 test lines; update both manifests. No new public API,
state, compatibility branch or test executable. Required verification after
approval: both full unit suites, MyRaidenII boundary/failure cases, and the
eight affected current PC receiving releases/integrations. Lib/Emulator/MyNES
source remains outside this fix.

## Evidence And Temporary Cleanup

The independent probe source SHA-256 is
`9D19A0FE749198235A5F7CDC9DB25DDBF8750ED978F63B82F7A9725521A36BEE`,
executable `CD306AD5F48C4BD74BF6422986FBF35A3140CCA8A8641100F0485C99533E70B4`,
and neutral summary
`F86C0E26E359AA5F4B1E8C4E75519BA05E69330E8B7F831B1225B718C43F0717`.
The probe completed normally; the initial focused tests failed for the
documented six cases. This preserved reproduction is not a passing result.
The final implementation and qualification are recorded below; no S10
owner-ROM run or gameplay pass is claimed.

The four task-owned temporary files total 698653 bytes. All owned sessions
have terminated; necessary research and probe identities are recorded before
governed cleanup. No ROM or media was used or copied in this phase.

## Final Mechanism And Similar-Issue Review

The approved Shared repair changes only the two existing Machine owners and
their existing execution unit. The scheduler returns INTERNAL_ERROR when
advancement callbacks have reported FAULTED. The sole public run operation
validates arguments and publishes consistent fault reason/detail/time/PC from
existing fields; its private budget body is the unchanged execution path.
There is no second executor, fault flag, clock, callback, public API or layout.
The retired-prefix count is not rewritten. Pending CPU fault accounting is
distinct from a device callback that has already published FAULTED.

All eight run-time elapsed-publication callers now take their existing failure
exits before a budget/pause return can overwrite FAULTED. The paused explicit
advance and next-deadline callers already return the publication status;
bounded L1 compatibility advancement retains its existing INVALID_STATE failure
classification. They require no new wrapper/result because they publish no run
result. Existing CPU failure completion remains responsible for pending CPU
fault work, not a second device-fault owner. Full-unit Intel/SX, retirement,
entry, scheduler and failure regressions retain their distinct timing models.

The existing executor unit adds a four-case before/after-callback by whole/
one-tick-budget matrix. Each checks actual report_fault success, FAULTED,
FAULT reason, unchanged detail, elapsed/result consistency and faulted re-entry.
The existing Board unit adds thirteen transfer/admission/failure rows plus
reset/partial-upload/busy checks. No new test executable or framework is added.
An actual guest command releases its CPU transaction before successful public
DMA admission; neither nested DMA nor paused-debug operational access is used.

The COP's four cohesive private helpers decode register words/addresses,
check uploaded-command admission and publish completed upload lanes. Runtime
program/configuration storage and one pending operation are real local device
state, not copied game data or a queue. Both source words precede writes;
an accepted write prefix is retained on failure. A final comment clarification
removes the misleading word atomic: this is not a rollback transaction. Both
COP object hashes remain byte-identical to the full-unit baseline, and both
rebuilt Board executables pass again.

Counts use git diff --numstat against 574b92492, excluding documentation,
generated data and releases. MyRaidenII production: +165/-7/net158 across three
existing C/H files; its existing test: +160/-2/net158. Shared production:
+28/-7/net21 across two existing C files; its existing test: +74/-1/net73.
Two manifests change +3/-3/net0. The positive delta owns the new L2 transfer
device behavior and explicit failure/budget proof; no unrelated formatting,
Lib/Emulator/MyNES source or test migration is included.

## Full Unit Qualification

Both final runs execute all 687 registered repository unit names with zero
failures/skips: x86 152.11 seconds, x64 135.95 seconds. The unchanged 300-second
aggregate budget and four CTest workers remain. The first x64 attempt ended
at that aggregate deadline after 482 completed passing tests; it is an
incomplete attempt, not a pass. A complete same-graph rerun supplies final x64
qualification. No assertion, per-test budget or inventory was weakened.

The sorted 687-name UTF-8/LF inventory hash is
3A8F7CEA273D8E06C8B059853DD35A195E86EC390561F6FEC50D5BD1AA83EC6E.
The final x64 LastTest log hash before receiving tests is
9EA41CF69D59CD1F8FBDF1AE9AAA8FBD37BB64E5A44C10F220E990539AC5F834.
Focused Core/Board tests also pass on each width before the full runs.
MyRaidenII and NXVM documentation gates, both changed manifests and whitespace
checks pass. Structural checks do not replace the actual code/ledger review.

The final comment-only COP rebuild preserves object hashes
31E5B4369FA762207E20B99F93868EB020659175FF6A981C6FACAB0E7E3F582D (x64)
and 007B0A281415760C8FA32B2027126D20BEF4F03F2D676A0E958109DF4F356CD4 (x86).

## Remaining T3 Boundary

B07/B08 upload/register admission and the complete stated three-command transfer
family receive direct synthetic bus/failure/reset proof in this batch. S9's
real-set upload inventory remains its prior observation, not a new S10 ROM run.
Other numeric, geometry, collision, unresolved trigger and DMA/sort families,
alternate microprogram semantics, real-set startup and full game operation
remain unqualified T3 obligations. A later checkpoint is not evidence that
those classes are complete. Video rendering, sound, desktop/RDP, physical
timing and gameplay are not tested or claimed here. T3 and M1 remain open.

## Receiving Release And Executor Review

Shared P1 f2ca9ae01 and NXVM receiving P2 990ff6eda are pushed. Eight stripped
current0564 PC releases are rebuilt/deployed; PE width and built/deployed hashes
match. Fourteen selected receiving integrations pass per width: identity,
session configuration and boot for each App, plus the default-profile and
Windows3.1 checkpoints for NXVM. DeskPro boot takes145.63 seconds x64 and
191.41 seconds x86 under the unchanged300-second budget. This is selected
receiving qualification, not an entire external/desktop suite.

The [result record](t3-s10-results.json) retains eight artifact identities,
JUnit hashes, two unchanged MyNES releases and manifest identities. All33
protected tracked files and six selected external media-role hashes are checked
unchanged; existing media access modes stay overlay. Profile switching
recompiles zero CPU objects; both persistent graphs finish at default. An
initial temporary receiving-record script incorrectly looked for the deployed
filename inside the build tree; correcting that internal target path reused
the already-passed XT result without rerunning or altering any test.

Executor actual-diff review covers all changed production/test/header/manifest
and task files. One public execution path, original register/table style,
Lib vocabulary, inward dependencies and explicit L2/source restrictions remain.
Architecture/coding governance selects the single fault-publication owner,
not a COP-specific workaround. No additional device framework, recovery path
or cross-App fixture is introduced. Legacy CPU warning exceptions are unchanged;
this is not a claim of waiver-free compilation of all historical source.

All owned sessions terminate; process inspection finds no remaining owned run.
After result/hash archival, the real cleanup tool previews then removes only
35 S10 generated files/55430 bytes and two identified empty old test remnants.
Earlier four-file probe cleanup removed698653 bytes as recorded above. Build
now contains only .ccache/.ninja; global/repository cache, ROM, media, user INI
and unrelated work are retained. Coordinator acceptance follows actual review
of the pushed target-scoped commits, not merely these summaries.

## Archived S10 Admission Packet


| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: MyRaidenII M1 T3 S10; sequential coordinator/executor roles. |
| Admission And Approval | Owner's standing implementation approval and delegated same-T S ordering; S9 accepted at 574b92492. Owner additionally approved the documented Shared x86-core callback-fault publication repair and eight affected NXVM receiving artifacts in S10. |
| Objective | Consume S9's complete upload/register protocol and three-member memory-transfer family (5205/5a05/f205): define an explicit L2 functional contract, preserve runtime upload state and perform real bus effects at the existing Core scheduler safe point, with exact failure/reset behavior. |
| Non-goals | Other COP numeric/collision/DMA-trigger algorithms, generic microinstruction interpreter, imported reference code/table, embedded game upload fingerprint, Lib/Emulator/MyNES source changes, physical COP latency or game qualification. |
| Reference Baseline | 574b92492; S9's 22-ID finite initialization ledger and S8 Board/Video; actual public DMA/report_fault/attachment contracts; pinned MAME unit as L2 behavioral research only. |
| Candidate Proposal | ../proposals/m1-board-cop.md; ../etc/evidence/t3-s9-cop-initialization-protocol.md. |
| Files And ABI Surface | Existing MyRaidenII cop.c/cop.h, Board attachment integration, existing board_bus_smoke.c and records; Shared x86-core machine.c/machine_scheduler.c, existing executor_run_smoke.c and two manifests; eight current NXVM PC EXEs and receiving receipt. No public API change. MyRaidenII estimate production +180--260/-30--70, tests +140--230/-0--15; Shared production +25--40/-5--15, tests +40--80. |
| Applicable Rules | Architecture one CPU/Core timeline, bus transaction and COP state owner; Coding Lib vocabulary and cohesive private helpers, no duplicate error state/queue; source policy factual L2 contracts and synthetic independent oracles, no reference implementation/table import; Execution complete units/actual review/P/cleanup; Document authority boundaries. |
| Verification | Real configured Board guest issues command writes; independent transfer/upload/register/offset/busy/reset and read/write refusal/prefix assertions. Existing Core executor unit covers before/after callbacks at exact instruction/tick boundaries. Both stable Ninja/ccache graphs run complete units; rebuild/deploy eight current PC EXEs, PE/hash identity and selected receiving integrations, unchanged MyNES/INI/media guards. Optional owner-ROM prefix is diagnostic only. Doc/whitespace checks and actual diff review. |
| Expected Markers | Runtime uploaded words never embedded; one pending hardware operation; DMA through public Core after CPU bus release; failure reaches existing Core fault path without shadow error flag; unimplemented IDs still reject. |
| Asset Needs | Owned build/myraidenii-t3-s10 for research/logs/probes and receiving receipts, 64MiB output/5-minute no-progress/90-minute phase budgets; optional ROM probe limited to 2M instructions/120 seconds/64KiB neutral summary. Eight existing PC receiving EXEs use their approved firmware embedding and overlay media; MyNES pair and protected inputs unchanged. MyRaidenII has no executable release yet. |
| Reporting Requirements | Confirm exact operation/transport/failure model and final estimates before code; report actual delta, retained owners, full dual-width units, any real-ROM result separately, evidence hashes and cleanup. |
| Stop Conditions | Stop affected work before any public API or Shared change outside the approved fault-publication repair, reference implementation/table import, raw protected output, copied upload fingerprint, invented hardware-exact time or unrelated Video/Sound expansion. |
| Exit Criteria | Entire admitted protocol/transfer batch has actual dual-width bus/failure/reset proof, final full units pass and artifacts are correctly scoped, actual diff reviewed, evidence/cleanup and P commits pushed. Other B07/B08 obligations and T3 stay open. |
| Original Owner Request | Implement MyRaidenII under single-session dual-role governance with autonomous ordinary same-T planning and minimal cohesive code/tests. |
| Similar-Issue Sweep | All three transfer forms and all register lanes share admission/service/failure/reset; preserve unknown-command rejection and existing RAM boundary tests. No nested DMA, paused-debug operational access or mirrored fault status. |
