# M5 T587 S5 - Token-contract import

Owner approved exact SoftPC 4cd56d018abd5328f0578dbfe0b7524f4bcfcce4 import
and receiving migration on 2026-11-18. The S4 audit is the design review.
PC110 now uses vdisplay dstn/tft and pccard insert slot mode common attribute;
quoted paths are lexed only by Emulator. No old grammar compatibility path.
Estimated exact import: 17 files +179/-93. Receiving adaptation: approximately
12-18 source/test files, production net reduction expected from removing
normalization, tokenization, trimming and raw-tail parsing. No extra framework.

## Implementation and actual-diff review

The eight imported trees retain every upstream byte. Lib has no change.
X86 NXVM Product now consumes name/argument tokens for info, speed and floppy;
its old whole-line normalization and path splitting are removed. Keyword case
comparison retains existing PC tolerance without modifying path bytes.
PC110 consumes the same tokens for vdisplay and pccard, removing its local
lexer, trim and pipe separator. Card opening, pending ownership, rollback and
stopped-only admission retain their original owners and implementation.
MyNES removes its second quote parser, validates/copies the decoded ASCII path,
and retains the NES Debug console parser and snapshot/ROM execution paths.

No legacy raw callback, reconstruction bridge, alternate dispatch, new thread,
persistent command state or changed INI format is introduced. Emulator owns
borrowed token storage for the synchronous callback only. Removable-media
Machine copies its path; PC110 opens files before returning; MyNES copies its
path and opens snapshot files synchronously. No receiving path retains tokens.

Existing tests were extended, not duplicated: PC110's production provider now
lexes quoted temporary card paths before the real Storage/Machine operations;
old pipe/equal syntax is rejected without replacing accepted pending media.
MyNES tests distinguish the shared whole-line bound from its own path bound.
The X86 NXVM factory test preserves case-insensitive speed and argument counts.
Upstream tests cover bounded tokens, quoting, malformed input and Debug N paths.

Historical records retain the original syntax. Current UX, help and asset
instructions use the new syntax. The remaining old syntax in active tests is
intentional negative coverage. Protected configuration/media hashes are checked
again at delivery. Dynamic qualification is recorded below when complete.


## Qualification and publication

Both stable Ninja/ccache Release trees built successfully. Each architecture
passes 716 unit and 48 integration cases. Additional profile checks pass on
both widths: My5160 20, My5170 22, MyDeskPro386 23. Only new or changed test
commands/executable hashes are selected for each profile; identical baseline
checks are reused. Default build configuration is restored afterwards.

MyRaidenII's existing opt-in selected-set wrapper passes its 2,000,000-instruction
startup probe and deployed-product lifecycle/Console smoke on both widths.
These four checks are not gameplay qualification. No runtime failure was retried,
no test assertion/budget was weakened, and no required case was skipped.
Automated native Window, Console and audio paths are included where registered;
manual desktop/RDP/physical input acceptance has not been performed.

Fourteen stripped Release EXEs are published in their existing seven asset
directories with correct PE widths and unchanged product version/filenames.
The 29 protected configuration, CMOS, media and snapshot files have unchanged
hashes. No Lib or X86 Core production/test changes were made. Eight imported
trees match the fixed upstream revision byte-for-byte: 296 files, zero
missing, extra or differing files.

Actual package diff (including package README/manifests):

| Scope | Added | Deleted | Net |
| --- | ---: | ---: | ---: |
| Imported eight trees | 179 | 93 | +86 |
| X86 NXVM receiving source/test | 48 | 65 | -17 |
| PC110 receiving source/test | 97 | 101 | -4 |
| MyNES receiving source/test | 30 | 41 | -11 |

Across C/H alone, production is +167/-216 (net -49), tests +160/-71
(net +89). Existing test fixtures own the added assertions; no new test
framework or redundant production dispatch path was added.

The [verification index](../etc/evidence/t587-s5-import-verification.json)
contains suite counts, command outcomes/timings, manifest results, artifact
SHA-256 values and log digests. Build commands retain the repository-local
ccache routing and stable Ninja trees. Aggregate cache counters are not
misrepresented as this task's hit rate.

Coordinator actual-diff review confirms the imported ABI migration is complete
across all receivers. The task owns the syntax change, receiving artifacts and
associated UX/evidence; no unrelated cleanup is included. All owned jobs are
finished before scratch cleanup; stable caches remain intact. S5 is delivered
for owner verification, not closed. T587 remains open.

Delivery commits: Shared import/PC extension adaptation `f060090a4`, MyNES
adapter and receivers `6f6a83af9`, MyRaidenII receivers `3ca9f9052`. The final
NXVM P4 owns PC110 adaptation, ten PC EXEs, provenance and the completed S5
qualification record. Every scoped P is pushed immediately after commit.

## Owner acceptance and S5 closure

The owner accepted the delivered result and explicitly authorized S5 closure,
while keeping T587 open and waiting for further instructions. The coordinator
reviewed P1-P4 through `b11e2c266` against the original request and archived
packet: byte-exact eight-tree import, receiving command migration, fourteen
published EXEs, 1,658 registered tests plus four selected-ROM checks, manifests,
documentation/LF checks and clean publication all have recorded evidence.

This P5 changes NXVM governance records only. No source, test, binary, user
configuration or media changes are required; existing build/test qualification
remains applicable. Documentation governance, LF and diff checks validate this
closure. S5 is closed; T587 stays open without an active S or newly admitted work.
The owner acceptance does not broaden the recorded automated-test coverage into
unperformed manual RDP or gameplay qualification.

## Archived S5 delivery packet

# M5 T587 S5 - Delivered: awaiting owner acceptance

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: T587 S5 after owner-accepted S4 at 7a61c56d8. |
| Admission And Approval | Owner approved exact SoftPC import and unified PC110 syntax on 2026-11-18; cached full build/publication/testing and scoped commit/push follow the standing delivery rules. |
| Objective | Import committed eight trees unchanged, migrate receiving callbacks, and publish/qualify all fourteen EXEs. |
| Non-goals | No changes to SoftPC, Lib forks, legacy callback/grammar compatibility, firmware, INI or user media; no gameplay claim. |
| Reference Baseline | NXVM 7a61c56d8; SoftPC 4cd56d018abd5328f0578dbfe0b7524f4bcfcce4, clean at admission. |
| Candidate Proposal | [S5 plan and evidence](../history/M5-T587-token-import.md). |
| Files And ABI Surface | Shared eight-tree import; X86 NXVM Product, PC110 and MyNES command consumers/tests; fourteen receiving EXEs; affected product UX/help and task records. Existing callback signatures migrate without aliases. |
| Applicable Rules | Execution/Documentation/Architecture/Coding rules; NXVM architecture/layout/UI/source policy; MyNES guide and source/layout rules for its explicitly approved consumer migration. |
| Verification | Exact upstream bytes and manifests; callback lifetime/syntax tests; cached x86/x64 full build, all unit/integration suites and selected-profile/receiver checks; fourteen deployment hashes/PE widths; docs/LF/diff review. |
| Expected Markers | No mismatched callbacks or duplicate lexer; all required tests pass; fourteen current outputs; upstream trees exact. |
| Asset Needs | Existing owner BYOB only; preserve INI/media. Scratch build/t587-s5, 2 GiB budget, 15-minute no-progress review; archive evidence then clean after jobs exit. Stable .ninja/.ccache retained. |
| Reporting Requirements | Actual per-owner diff, removed paths, verification/failures, artifact hashes and commits; distinguish native automated and manual testing. |
| Stop Conditions | Do not weaken tests, overwrite unrelated changes, alter upstream or claim skipped verification. |
| Exit Criteria | Receiving paths migrated; eight trees exact; 1,528 baseline cases, 130 profile cases and four selected-ROM checks pass; fourteen outputs published with hashes; scoped P1-P4 delivery and final gates recorded in S5 evidence. Await owner acceptance; T587 stays open. |
| Original Owner Request | Admit new S, import current SoftPC eight packages unchanged; adopt ordinary token syntax for PC110. |
| Similar-Issue Sweep | All fixed/extension callback declarations, direct test invocations, path copying and help/command fixtures; Debug grammars remain independent. |
