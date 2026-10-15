# Project Status

## Current Work

T560 remains open after accepted S7 implementation 0059bfd51. S7 is closed
after one-session coordinator review of the actual pushed source, tests,
interfaces, artifacts and source/model limits. Its C11/C12 functional batch
has direct CPU and Core proof: SRESET, physical SMM/RSM, special-cycle RDY,
HOLD/LOCK ordering, Stop Grant/qualified CLK and held/deferred FLUSH.

Final full repository units pass 604/604 per width in the original
Ninja/ccache graphs (45.70 s / 49.56 s). The subsequent fixture-only output
cleanup is separately rebuilt/rechecked, 2/2 per width; it is not another
full-suite execution. Both complete specialized aggregates and all eight
profile/width integration/release groups pass. Eight verified stripped 0560
PC EXEs are deployed. MyNES remains at 0044; public-six, MyNES source/binaries
and owner INI/media/ROM/snapshot inputs are unchanged.

The [S7 evidence](../etc/evidence/t560-s7-smm-system.md) records the complete
event matrix, actual diff, source/model qualifications, failures and final
artifact identities. Reserved save-image encoding, ambiguous DR7 and relocated
selector behavior retain explicit L2 limits. Physical pin/PLL timing stays
with S9 and board qualification; no fitted PC110 silicon claim is inferred.
No manual RDP or Windows 3.1 GUI qualification is claimed.

S8 is admitted below for the complete inherited semantic inventory and its
cohesive repairs. Source/form reconciliation precedes production edits.
The [S8 matrix](../etc/evidence/t560-s8-inherited-semantics.md) partitions both
opcode spaces into twenty families. Initial ALU/BCD/group-3 SX selection,
12,288 arithmetic boundary executions and 40,000 valid-decimal cases pass
their focused owner test on both widths. A dword carry/borrow test-oracle
overflow is corrected with existing lib_u64. Those initial test cohorts left
production unchanged; remaining source/form proof and complete S8 qualification
are still open. This is not whole-C15 acceptance.
Further S8 proof covers 4,320 multiply/divide boundary/failure executions,
both widths' shift/rotate, extension/scan and inherited decode cohorts. An
initial rotate failure was traced to test array-index-based timing selection;
explicit family selection preserves every old timing assertion and passes
the final rechecks. The [S8 evidence](../etc/evidence/t560-s8-inherited-semantics.md)
records source-qualified undefined ranges and all initial/final outcomes.
Stack, near/far/branch and five string-instruction owners now also execute
their retained assertions on SX and pass focused x64/x86 tests. The S8
evidence explicitly separates terminal-shutdown fixtures from exact-vector
proof; remaining family/mode/failure reconciliation is still open.
A subsequent real-IDT test exposes raw RPL leaking into selector exception
error codes. Original-source review and same-class inventory precede 27 exact
masking replacements at the existing instruction owner, without API/state
changes. Precise far GP/NP, expanded port/provider-prefix proof and all 99
CPU-owner regressions pass both widths after rebuilding 93 CPU targets each.
This is not final S8 qualification: the eight deployed EXEs remain accepted
S7 artifacts and must be rebuilt/qualified for the completed production batch.
Descriptor queries, BOUND, ARPL, LSS/LFS/LGS and descriptor-system modern forms
now replay SX with retained old-family assertions. New real-handler proofs
cover 480 BOUND signed-range executions and 80 precise LLDT/LTR failures;
their final six-owner cohort passes 6/6 per width. Remaining C15 disposition
and whole/artifact qualification stay open, as detailed in the S8 evidence.
Task/outer-return profile proof now includes SX; a fail-first word-TSS test
also confirms that local DR7 enables incorrectly survived a 16-bit task load.
The common task owner now selects debug clearing by CPU family rather than
incoming TSS width, with no new API/state. Final task/return checks pass 5/5
per width; flag-transfer/direct-flag/SETcc checks pass 3/3 per width. These
focused checks do not complete C15 or replace the required final eight-EXE
publication. The S8 evidence retains sources, failed reproduction and limits.
GPR MOV/moffs/LEA/XCHG and INC/DEC now also have actual SX selection and
expanded alias/flag-edge proof; the final focused cohort passes 6/6 per width,
with a separate passing mode-precondition recheck. The evidence records an
initial VM86 test-wiring correction and does not treat those initial passes
as mode proof. Segment forms and complete C15/final release remain open.
Segment MOV/LES/LDS and external-shadow selection now replay SX. A precise
actual-LLDT-0 test exposes lost selector errors for unusable LDT references;
the existing LDT descriptor reader now rejects them before generic GP(0).
The evidence retains the initial invalid reset-derived probe and corrected
before/after proof. All 99 CPU-owner checks pass per width after rebuilding
93 executables each, followed by a separate passing LDTR/TR assertion check.
No new interface/state or Shared change is involved; S8/artifacts remain open.
Bit/IMUL/segment-stack proofs now also cover SX; the IRQ fixture uses the
existing special-cycle callbacks required by its HLT handler. A fail-first
observer test exposes DR4/DR5 aliases accepted by execution but rejected by
preview. Existing metadata now owns SX alias admission; generic memory-form
and older-family rejection remain intact. All 99 CPU-owner checks pass per
width after rebuilding 93 targets each (11.03/6.04 s). Evidence and both Core
manifests are updated. This is not full C15 or release qualification.
Further reserved-form review exposes CR1/CR5--7 and C0/C1 /6 observer
admission inconsistent with actual execution. Existing metadata branches
now own index/extension validity and family minima, without new interfaces,
state or executor behavior. Fail-first, an initially shadowed Group-2 fix,
the 98/99 failed aggregate and final dual-width focused passes are retained
in S8 evidence. Positive Group-2 lexical controls also pass separately.
No final all-CPU, whole-repository or eight-EXE claim is inferred for this
latest revision. Remaining C15 source/form disposition stays open.
ASCII-adjust continuation adds actual ADD/AAA and SUB/AAS valid-digit
proof plus base-ten AAM/AAD value/defined-flag matrices; the existing ALU
owner passes x64/x86 in 0.28/0.24 s with no production change. Original
Table 9-3 explicitly distinguishes reserved encodings that do not generate
UD, so the earlier Group-2 lexical fix is only current-policy consistency
proof, not silicon alias qualification. The S8 evidence corrects that limit
and keeps historical encodings and invalid-BCD source dispositions open.
The D6 SALC gap now has a source-qualified L2 repair at the existing CPU
owner: original non-UD classification plus independently traced 486SX
reference wiring support AL from CF, with unchanged other state. Metadata
admits selected SX only; older policy and LOCK rejection are retained.
The existing ALU test proves 33,792 executions. Dual-width related checks
pass, and x64's 99 CPU-owner cases pass inside a 101/102 aggregate whose
old board-preview assertion is then corrected/rechecked separately. Exact
logs/grades and the remaining F1/internal-alias/BCD scope are in S8 evidence.
No final S8 or updated-EXE qualification is claimed.
Final x86 CPU-owner rebuild/recheck passes 99/99 in 5.82 s, separately from
the earlier x64 combined aggregate and both widths' three-board rechecks.
Final manifests, Types, fixture lifecycle, documentation and whitespace
checks pass. These remain bounded S8 progress, not final C15 acceptance.
Two more non-UD classes now have explicit L2 models: selected-SX Group-2
/6 uses the existing SHL /4 operation and F6/F7 /1 uses TEST /0. Metadata
is the sole lexical family owner; older execution policy is retained.
Paired actual CPU and independent result checks cover mode/attribute/value
dimensions; existing RMW/failure and Core preview owners include the forms.
The final focused cohort passes 5/5 per width (3.03/0.49 s), without new
API/state/helper/decoder. F1 subsequently gains its distinct privileged DB
trap at the existing owner, with no new DR6 cause or state. Actual mode/gate,
failed-entry, RF/shadow and outgoing-VM86 task proofs expose and repair the
adjacent real-mode DB frame width, VM86 gate admission and task-plan PE guard.
Final CPU plus three board-owner checks pass 102/102 per width (13.71/5.83 s).
The evidence retains failed intermediate cohorts and L2 source limits.
The subsequently reproduced incoming-VM86 task defect now reuses the existing
real segment constructor in the two task selector owners. Actual JMP/CALL,
task-gate and NT-IRET cases prove cache/address formation; new-task EIP/LDTR
faults remain precise. Six-owner dual regression and final task-owner checks
pass as separately logged in evidence. Internal encodings, BCD, whole
qualification and releases remain open.
The subsequent stable CPU/board-owner aggregate passes 102/102 on both
widths (14.93/6.52 s), including all new incoming-task failure cases. Types,
fixture lifecycle, manifests, documentation and whitespace checks pass;
this is still bounded S8 progress, not whole-repository/artifact acceptance.
The next reserved-form batch adds selected-SX UMOV's L2 ordinary-MOV alias
contract by reusing four existing MOV handlers, with metadata/scanner and
Core preview reconciliation. Register and independent memory/failure proof,
required fetch and older/LOCK rejection pass focused dual-width cohorts.
Primary 82 already has the correct shared byte Group-1 route; only its
independent eight-operation proof is extended. Source limits for LOADALL,
invalid BCD and complete/final qualification remain explicitly open.
Final stable UMOV/82 CPU plus board-owner regression passes 102/102 per
width in 12.33/6.35 s after complete owner rebuilds. Both Types gates,
manifests, fixture lifecycle, documentation and whitespace checks pass.
No partial implementation commit or updated-artifact acceptance is claimed.
Historical LOADALL rejection now has a source-qualified L2 normal/SMM
boundary, separate from unmodeled ICE hardware; old A-step slots retain
selected-SX UD. Actual handlers and frames pass both widths. Invalid BCD
inputs expose AAA/AAS AX carry/borrow and DAS original-flag defects; existing
SX handlers are corrected at explicit retrospective L2 grade, preserving
old-family behavior and undefined flags. Complete AX/AL input, all radix,
real zero-divisor and LOCK-priority proof passes. The stable production
CPU/board aggregate passes 102/102 (14.21/6.42 s); the final test-only LOCK
addition passes separate owner rechecks. Finite C15 reconciliation and
whole-S8 qualification/eight artifacts remain open, as detailed in evidence.
The following descriptor-query audit removes erroneous Present requirements
from LAR/LSL/VERR/VERW and rejects LAR interrupt/trap gate types per the
original tables. Actual segment/task load checks remain intact. Two existing
test owners add 65,536 type/presence/privilege/form executions. An initial
101/102 aggregate on each width exposes four stale non-present expectations
in the guest-bootstrap selector test; the corrected complete cohorts pass
102/102 per width (3.02/3.01 s). Evidence retains initial failures and source
limits. No Shared, MyNES, interface, persistent-state or artifact change is
introduced; finite C15 and whole-S8 release qualification remain open.
F11 now has complete condition truth tables, counter/size/wrap and byte-
register alias proof on actual SX, replacing weaker modern witness loops.
Required-suffix and target-fetch owners also replay SX while preserving
all original 286/386 timing assertions; SX still explicitly lacks allocated
timing until S9. Final CPU/board cohorts pass 102/102 per width (2.89/3.14 s).
The evidence records 326,656 direct matrix executions and the corrected
test-only transfer build mistakes. This is functional F11 reconciliation,
not complete C15, whole-repository or updated-artifact qualification.
F13 now adds 9,072 actual page-fault/guest-PTE-repair/IRET/recovery cases
and 9,072 zero-count controls across all five string operations. REP IRQ
boundary proof now also selects SX. Final CPU/board snapshots pass 102/102
per width; the last zero-count-only addition passes separate 2/2 owner
rechecks. Evidence retains the corrected test-only backward-pointer mistake.
The compare-fault EFLAGS question is now resolved as an explicitly bounded
retrospective L2 model: selected SX restores initial instruction flags on
GP/SS/PF/AC, while retained 386 policy stays unchanged. Two transient private
CPU fields carry that image through repeat iterations; no public interface,
second executor or timing-state dependency is added. Actual PF recovery now
asserts every saved flag. Additional real/protected GP/SS/AC, wait/pause,
debug patch, reset/entry, NMI/step and SMM tests cover its lifetime boundaries.
Both complete CPU/board snapshots pass 102/102 (14.58/6.85 s); the final
test-only real-mode expansion then passes separate 5/5 owner rechecks per
width (0.29/0.34 s). These are not full repository or artifact qualification.
Shared-six, MyNES, owner inputs and accepted S7 EXEs remain unchanged.
The next F17 selection sweep adds actual SX execution to PUSHF/POPF, real
IRET and software INT owners, preserving old-family and precise assertion
boundaries. Protected POPF includes 2,048 SX privilege/flag combinations;
software INT now also inspects its real saved frame. Final three-owner
rebuild/rechecks pass 3/3 per width (0.29/0.27 s); no production change is
needed. Evidence records source, actual diff and the remaining finite-C15
and final repository/release obligations.
The final CPU/board cohort after those test changes passes 102/102 per width
(2.92/3.17 s). Remaining descriptor/address/segment/VM86 owner reconciliation
is explicitly retained; this is not whole-Core or full repository acceptance.
The next F16 continuation replays descriptor-table/task-register forms on SX,
checks successful table stores at CPL0--3/VM86, and adds 128 actual privileged
load exception entries across both modern profiles. Original-versus-later
source qualifications remain explicit; no production change is needed.
Final four-owner checks pass 4/4 per width, followed by 102/102 CPU/board
cohorts (2.85/2.99 s). Remaining address/segment/prefix/VM86 disposition and
complete repository/integration/artifact qualification stay open in S8.
Operand-span, protected addressing and eleven prefix groups now also replay
SX at their existing owners, retaining old-family behavior and failure proof.
The span owner includes 1,025 SX boundary/provider scenarios; protected address
tests still use real guest mode-entry instructions. Legacy prefix rejection
runs once. Final CPU/board cohorts pass 102/102 per width (2.93/3.13 s), with
no further production change. Evidence records actual scope and source/model
limits. The remaining twelve-owner investigation index is not a defect count;
finite C15 and whole-repository/integration/eight-artifact qualification stay
open. Shared-six, MyNES, owner inputs and deployed S7 artifacts are untouched.
VM86 entry/return and protected data owners now also execute SX, including
the real IRQ rebind, inner-stack frames, paging and RF cases. An incomplete
IRET frame must demonstrate actual shutdown and unchanged stack, not merely
unchanged CPU. A test-oracle error that confused DB diagnostic cause with a
GP stack error is corrected without production changes. Final four-owner
checks pass 4/4 per width; full CPU/board cohorts pass 102/102 (3.09/3.39 s).
Evidence retains the failed intermediate run and explicit qualification
limits. Remaining source/owner disposition, complete C15 and final S8
repository/integration/artifact qualification are still open.
The remaining owner investigation now has explicit dispositions rather than
blind enum widening. Actual SX reset/observer/debug snapshots and segment
guest-bootstrap groups pass; an uncovered Group-1 dword-immediate form is
added at the existing ALU owner with both CF inputs. 286 and old timing rules
remain distinct. The expanded CPU/board selection, now explicitly including
the separately registered context case, passes 103/103 per width (2.87/3.28 s).
No production change is needed in this continuation. The evidence maps the
five retained old-model witness files to their actual selected-SX owners;
final F01--F20/C15 reconciliation and full S8 qualification/publication remain
open, without Shared, MyNES, input or deployed-artifact changes.
S9 timing and S10 final Core admission remain
mandatory. Production 486 construction stays closed. The persistent complete
486SX implementation objective is not yet achieved.

The prior accepted cache/bus baseline remains supporting evidence:
The [S5 source/route investigation](../etc/evidence/t560-s5-cache-bus.md)
records the original cache/test-register contracts and concrete A20,
per-access paging-attribute and external-write coherence gaps.
Ordinary CPU accesses and TR3--5 commands now share actual cache state,
with mode/privilege/fetch/failure, replacement, PCD/PWT, A20, route and snoop
proof on both widths. INVD/WBINVD deliver ordered Core cache operations.
Page-table A/D updates use authoritative second reads and a single memory
LOCK boundary shared with explicit RMW and implicit memory XCHG. Real Core
tests prove DMA exclusion, changed-entry targets, cached-entry reconciliation,
split-page failures and retained accepted effects. All accepted LOCK handlers
have been reviewed against their common RMW entry; no second arbiter exists.
The accepted S5 snapshot passes 601/601 full repository units per width in the
original Ninja/ccache graphs. All eight final profile/width integration groups
pass and eight verified 0560 PC EXEs are rebuilt and deployed. C16 pin/clock
fidelity remains in the timing batch; the current remaining obligations are
summarized above. No complete CPU or production 486 admission is claimed.
S4 whole units passed 600/600 per width, all eight final profile integration
groups pass and eight verified 0560 EXEs are deployed.
The [complete CPU proposal](../proposals/m5-pc110-486-system.md) and
[convergence ledger](../etc/evidence/t560-s1-cpu-convergence.md) retain all
remaining CPU obligations. Production 486 admission remains closed.

The owner permanently approved routine task-version synchronization of build
targets, presets and release configuration on 2026-10-14. This is not approval
for unrelated Shared functionality or APIs; the [task history](../history/M5-T560-complete-486sx-cpu.md)
records the exact boundary. Do not ask again for equivalent version updates.


| Task | Progress |
| --- | --- |
| T560 S8 | Admitted: reconcile the entire C15 inherited opcode/form universe against original manuals and actual dispatch/metadata/preview/execution; establish the complete gap matrix before grouped repairs. No S8 qualification or production admission yet. |
| T560 S7 | Accepted and closed: 0059bfd51. C11/C12 source-qualified SMM/RSM and system/power proof; final 604/604 units per width plus fixture-only rechecks, both specialized aggregates, eight passing integration groups and eight verified deployed 0560 PC EXEs. Exact timing, full inherited semantics and final admission remain S8--S10. [Evidence](../etc/evidence/t560-s7-smm-system.md). |
| T560 S6 | Accepted and closed: 926bfe185. Selected feature/VME/debug/exception/SX-FPU and carried C05 entry proof at documented grades; no new public API or persistent field. Final 602/602 units per width, both complete specialized aggregates, eight passing integration groups and eight verified deployed 0560 PC EXEs. S7--S10 and production admission remain open. [Evidence](../etc/evidence/t560-s6-feature-exceptions.md). |
| T560 S5 | Accepted and closed: 33cb2efe1. Actual cache/TR/operations/snoop/page-control, second-read/locked A/D and implicit/explicit LOCK proof; 601/601 full units per width, eight passing integration groups and eight verified deployed 0560 PC EXEs. [Evidence](../etc/evidence/t560-s5-cache-bus.md). Remaining CPU obligations stay in T560. |
| T560 S4 | Accepted and closed: c32103a24. Translation/test-command and invalidation proof, 600/600 full units per width plus final affected-test recheck, eight passing integration groups and eight verified 0560 artifacts. [Evidence](../etc/evidence/t560-s4-translation-cache.md). Internal L2 grades and remaining C04/C16 bus/CPU obligations remain explicit; production 486 admission stays closed. |
| T560 S3 | Accepted and closed: cb52586a9. Full units pass 599/599 per width, all eight final PC integration groups pass and eight verified 0560 EXEs are deployed. [Batch evidence](../etc/evidence/t560-s3-alignment-rmw.md). S4--S10 and C13-linked exception-entry qualification remain in T560; production 486 admission stays closed. |
| T560 S2 | Accepted and closed: e7c3f4e78 / ddb260095. Base system tests pass 597/597 per width; all eight integration groups pass and eight 0560 EXEs are published. See the [S2 evidence](../etc/evidence/t560-s2-base-system.md). Complete CPU qualification and planned S3--S10 remain open in T560. |
| T559 | Closed by owner approval: instruction-delta qualification, 597/597 units per width, eight final external groups and eight verified 0559 PC EXEs. Remaining CPU system semantics transfer to the approved next task. |

The [delivery evidence](../etc/evidence/t559-s7-instruction-qualification.md)
and [task history](../history/M5-T559-486sx-instruction-semantics.md) record
actual-diff review, sources, hashes and verification. Shared-six, MyNES and
owner inputs are unchanged. No manual RDP qualification is claimed.
The [instruction proposal](../proposals/m5-pc110-486-instructions.md) and
[finite ledger](../etc/evidence/t559-s1-instruction-inventory.md) retain the
bounded T contract; the [system receiver](../proposals/m5-pc110-486-system.md)
still owns real cache/TLB/test commands, reset/control, AC/VME/SMM and timing.
Production 486 admission stays closed. The persistent full implementation
objective is not complete.


## Active Packet - T560 S8 Inherited Instruction Semantics

The final stack audit has additionally repaired a real SX SP=1 explicit-PUSH
shutdown omission, proved first against a valid VM86 SS receiver; 34 selected
PUSH cases fail before the one-predicate repair. Both profiles retain a working
SS receiver control, and focused stack/inventory checks now pass 5/5 per width.
The first full unit attempt is 601/605 on x64 (four inventory registrations,
not a timeout); x86 was not run by that fail-first aggregate. Every changed
decoder mask is now reconciled with source-qualified Group-2/reserved-CR rules,
and both duplicated runner targets are explicitly rebuilt. The corrected
full rebuild/unit execution now passes 605/605 on each width, 258.33/81.26 s;
this precedes the final unary/reserved-form continuation and does not qualify
that later snapshot. Its FE operand-read fix and final source/form candidate
now pass 103/103 CPU/board checks per width. Complete delivery qualification
is in progress, including a one-token correction in the existing NXVM LSL
source gate for its renamed parameterized test. S8/C15 remain active; any
newly built worktree EXEs are intermediate until the full eight are qualified.
Final qualification is now complete: 605/605 whole units per width,
251.65/67.99 s, both complete specialized aggregates, all eight profile/width
integration groups (48 rows per width without duplicated MyNES runs), two
restored-default identity checks and eight PE/hash-verified 0560 artifacts.
No test row is skipped. Final C15 functional dispositions and source/model
limits are recorded in the S8 evidence. The executor is ready for its complete
P and coordinator actual-change review; S8/C15 are not yet marked accepted.
Manual RDP/Windows GUI qualification and S9/S10 remain outside these results.

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation T560 S8 after accepted S7 implementation 0059bfd51 and acceptance 8b6d48888; next unused S, one-session coordinator/executor roles. |
| Admission And Approval | Owner requires complete 486SX semantics and permanently authorizes sequential Core/chips/x86 work, necessary Core contracts and accuracy upgrades. Target NXVM only; Shared-six, MyNES and owner inputs excluded. |
| Objective | Consume the complete C15 batch: reconcile both 256-entry opcode spaces, all group extensions and inherited arithmetic/BCD, multiply/divide, shifts/rotates, bit/scan, data/stack, near/far control, strings/I/O, descriptor/task, flags/interrupt and system forms. Repair confirmed classes at their existing CPU owner with real execution and retained-family proof. |
| Non-goals | No second decoder/executor, speculative rewrite, per-BIOS checkpoint repair, unrelated x87, PC110 board/App, timing-model rewrite, production SX construction or Shared/MyNES changes. C16 timing and C17 admission remain S9/S10. |
| Reference Baseline | Clean 8b6d48888 after S7 acceptance. Original pinned Intel 1992 PRM instruction/compatibility/exception chapters, selected SX source applicability, accepted T559 delta and T560 S2--S7 direct evidence; source identities in T558/S1. |
| Candidate Proposal | [Complete CPU proposal](../proposals/m5-pc110-486-system.md), C15/form universe in the [convergence ledger](../etc/evidence/t560-s1-cpu-convergence.md), and [T559 inventory](../etc/evidence/t559-s1-instruction-inventory.md). |
| Files And ABI Surface | Existing src/core/chips/cpu instruction/decode/preview/state owners and actual Core connection only when required; corresponding test/core owner cases/registration/manifests, existing NXVM LSL source-gate token, NXVM evidence and eight affected 0560 EXEs. Preliminary tests +400--900/-0--100; production count and exact file set refined from complete matrix before edits, not a quota. The source-qualified compare-fault repair may retain initial EFLAGS and validity across iterations at the existing CPU owner; no second repeat executor or timing-state dependency. No planned Shared/public raw-state API or new persistent owner. |
| Applicable Rules | NXVM guide/Task Reading Set; Execution coverage, complete P, actual-change acceptance and artifact rules; shared Architecture/Coding/Document and NXVM architecture/layout/source policy; local architecture-governance and coding-governance skills; PDF skill for original instruction tables. Preserve original handler style and older CPU semantics. |
| Verification | Enumerate actual dispatch, metadata, preview and profile predicates; classify documented/reserved/undefined forms against primary sources. Reconcile 8/16/32-bit operands, code/address sizes, memory/register, aliases, prefixes/LOCK/REP, real/protected CPL0--3/VM86, flags/count/divide boundaries, required fetch/access failures and restart/publication. Reuse named proof only for unchanged mechanisms; add actual CPU/Core tests for missing classes. Stable closure requires whole x64/x86 units, complete applicable static/specialized gates and affected integration/release qualification using the original Ninja/ccache graphs. |
| Expected Markers | Complete finite C15 matrix with source, handler and regression owner for every disposition; no undocumented success-only stub, mismatched decoder/preview admission or lost accepted side effect. No complete claim from coverage counts, successful boot or a first terminal alone. |
| Asset Needs | Read existing original manuals and lawful build inputs only; ignored research/probe outputs remain under build. Preserve all INI/ROM/media/snapshot and MyNES inputs. Rebuild/deploy eight 0560 PC EXEs if executable inputs change; test/document-only work retains qualified hashes. |
| Reporting Requirements | Before production edits report full class inventory, confirmed gaps, owner design, exact files and refined +/- estimate. Report grouped proof, similar-issue sweep, actual source/test counts, complete terminal verification and artifact identities; do not commit a partial implementation P. |
| Stop Conditions | Required Shared change, essential source contradiction, undeclared consumer, accuracy downgrade or owner-input mutation. Record exact unresolved fact and continue independent in-scope reconciliation; do not invent silicon semantics or silently exclude an applicable form. |
| Exit Criteria | Every C15 member has a source-qualified disposition and direct actual-owner proof or explicit architecturally undefined/non-applicable reason; all confirmed applicable defects repaired, full dual-width qualification and affected artifacts complete, complete P pushed and actual diff accepted. S9/S10 and T560 remain open. |
| Original Owner Request | Implement complete 486SX instruction semantics under governance/code-quality rules; automatically continue bounded Core batches, group related instructions and reuse cached builds instead of per-opcode full runs. |
| Similar-Issue Sweep | All dispatch tables/group extensions, metadata/preview handlers, CPU profile comparisons, operand/flags/publication helpers and retained instruction tests. Classify each hit against the actual contract; include old-family distinctions and undefined encodings rather than mechanically widening 386 predicates. |

### Accepted Delivery — M5 T559 S6 CPUID And Flags

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation T559 S6 after coordinator acceptance of S5 P1 8e6159de4. |
| Admission And Approval | Owner-admitted T559 and automatic sequential Core-only implementation; group I7/I8 together. No Shared or MyNES changes. |
| Objective | Freeze source-qualified selected SX CPUID/ID behavior and implement it with the existing flags owner; reconcile PUSHF/POPF, IRET and task-load masks without duplicated profile state. |
| Non-goals | No production 486 admission, invented zero-feature silicon, unproved VME advertisement, SMM/cache/TLB hardware, PC110 board, Shared or MyNES changes. Real AC/VIF/VIP hardware effects remain mandatory system gates. |
| Reference Baseline | 8e6159de4; S1 I7/I8, pinned selected-SX data sheets and original Intel manuals. Audit exact stepping applicability and current flags paths before implementation. |
| Candidate Proposal | [Selected 486SX instructions](../proposals/m5-pc110-486-instructions.md), [finite ledger](../etc/evidence/t559-s1-instruction-inventory.md), and [system receiver](../proposals/m5-pc110-486-system.md). |
| Files And ABI Surface | Core CPU existing instructions, flags definitions and profile reset only where source-qualified, directly affected CPU tests/registration, Core manifests and NXVM evidence. Preliminary production +60--160/-10--60 and tests +200--400/-0--30; refine after source/form audit. No planned public API or persistent-state addition. Rebuild eight PC EXEs if executable inputs change. |
| Applicable Rules | NXVM guide; Execution lifecycle/coverage/artifacts; Document; Architecture/Coding and their skills; NXVM architecture/layout and source/research policy. |
| Verification | Full I7 leaf/GPR/flags/privilege/LOCK/fetch/old-profile matrix; I8 PUSHF/POPF, real/protected/VM86 IRET and task-load masks, 16-bit upper-half preservation and privilege rules. Focused whole batch, actual-change review, then full x64/x86 repository units and affected artifact/INI identity checks. |
| Expected Markers | CPUID feature and ID behavior match the named candidate source contract; one flags-mask owner, retained earlier-family semantics and no production admission before system qualification. |
| Asset Needs | Existing original manuals and admitted external build inputs only; preserve owner INI/media/ROM/snapshot and MyNES. |
| Reporting Requirements | Before edits report source/stepping contract, existing flags owners, complete form matrix and refined diff. Delivery records actual diff, direct proof, system gates, artifacts and whole-batch qualification. |
| Stop Conditions | Required Shared/new cross-owner API, unresolved source contradiction or absent evidence needed to choose feature values. Record exact prerequisite and continue independent scoped work; do not guess silicon capability. |
| Exit Criteria | I7/I8 applicable instruction contracts have direct proof and retained-family regression, system-only effects remain explicit, full dual-width units and applicable eight artifacts pass, actual diff reviewed and complete P pushed. No full CPU/PC110 qualification claim. |
| Original Owner Request | Implement 486SX instruction semantics under governance; implement related groups together and avoid per-opcode full validation. |
| Similar-Issue Sweep | Audit every reserved/writable flags-mask consumer, CPUID dispatch/metadata/preview and profile-specific identity path; classify reset/control/VME effects at their real owner. No global enum widening or CPU rewrite. |

S5 is accepted and closed after actual review of pushed 8e6159de4. Its
[evidence](../etc/evidence/t559-s5-system-instruction-boundaries.md) records
retained TR transfer/form/privilege/fetch proof and 596/596 units per width.
Complete I4--I6 unavailable instruction/effect contracts remain with the system
receiver, including the existing passive 486 TR6/TR7 limitation. No production
source changed; eight S4 0559 EXEs remain current. Shared-six, MyNES and owner
inputs are unchanged. S6 is admitted for the complete CPUID/flags batch; source
review precedes edits. T559 and production 486 qualification remain open.

The proposal's batch-efficiency order applies: complete family implementation,
focused CPU validation and actual-change review before final whole-suite
qualification. External guest boot remains a T-level exit requirement.

S6 executor delivery is complete, pending coordinator acceptance. The candidate
CPUID handler and one profile-aware flags mask have focused x64/x86 proof.
The new CPU-owned test covers leaf outputs, independent code-size/prefix
combinations, old-profile rejection (without confusing 8086 POP CS with a
two-byte CPUID), truncated preview, failed instruction fetch without register
mutation, reset ID/signature, PUSHF/POPF and real/VM86 IRET AC/ID transfer.
Existing protected IRET and five task-switch completion paths now include
486 AC/ID patterns; their updated tests pass on both host widths. Existing
IRET/POPF/task regressions also passed in the preceding focused batch.
The [S6 evidence](../etc/evidence/t559-s6-cpuid-flags.md) records source-contract
reconciliation, direct proof and actual-diff review. All 597 unit cases per
width now have passing evidence: x64 first passed 594 and then passed its three
affected rechecks after correcting one old metadata assertion and its manifest;
x86 passed 597/597 in one complete run. Eight PC EXEs are rebuilt/deployed and
pass real-product INI identity, PE-width and version checks. Core manifests,
documentation and whitespace gates pass. Shared-six, MyNES and owner inputs
remain unchanged. No production 486 admission, VME hardware qualification,
manual RDP or external firmware boot result is claimed. Whole-T S7 remains.

## Closed Task — M5 T558

Owner accepted S2 delivery 7ea9084fe and approved T closure on 2026-10-13.
The nine source/checklist receivers and their bounded unresolved decisions are
accepted as research delivery. This does not assert fully measured hardware:
CPU feature/source decisions transfer to T559 S1 and the system candidate;
board, input/power, display, storage, audio and asset decisions remain mandatory
gates in their named queued receivers. Production and ten EXEs are unchanged.
The retained [history](../history/M5-T558-pc110-evidence-and-implementation.md)
records the closure review and accepted limits.

### Accepted Delivery — M5 T558 S2 PC110 Implementation Evidence And Contract Freeze

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation T558 S2, after accepted S1 research/planning delivery feadb8235. |
| Admission And Approval | Owner directs continued PC110 audit/design so every later implementation task has evidence, a checklist and original manuals or reference code. Owner subsequently selects original DSTN and upgraded TFT as two compiled variants, with mypc110_x86.exe, mypc110_x64.exe, mypc110_tft_x86.exe and mypc110_tft_x64.exe; this supersedes the briefly proposed INI model/BIOS selection. All ROMs remain external build inputs embedded in EXEs. This admits design/research, not production implementation or source import. |
| Objective | Establish per-receiver implementation evidence, finite capability/checkpoint inventories, hardware/firmware baseline and explicit unresolved decisions; audit completion against all nine planned receivers. |
| Non-goals | No production/test/Shared changes, EXE refresh, external ROM acquisition/import, BIOS bypass, guessed hardware qualification or silently excluded device. |
| Reference Baseline | NXVM feadb8235; three external pinned references in S1 ledger, revalidated unchanged at S2 admission. |
| Candidate Proposal | [PC110 programme](../proposals/m5-pc110-evidence-and-implementation.md), its nine linked receivers and S1 ownership audit. |
| Files And ABI Surface | NXVM planning, evidence, status and history only; no ABI. |
| Applicable Rules | Guide, Execution coverage/closure rules, Document rules, architecture ownership and source/research policy; architecture-governance skill. |
| Verification | Inspect source bytes and reference code, original manual identity and relevant sections; reconcile every receiver to sources, current implementation, finite checklist and proof criteria; run documentation/link/diff checks. |
| Expected Markers | No receiver relies solely on a repository name or boot screenshot; unavailable raw evidence and conflicts are explicitly classified; every unresolved decision has an owner and admission condition. |
| Asset Needs | Read existing external candidates and record roles/size/hash; no copies into source or release admission inferred from availability. |
| Reporting Requirements | Report concrete evidence changes, exact unresolved facts, completeness per receiver and unexecuted hardware/runtime verification. |
| Stop Conditions | Incompatible source derivation, unapproved acquisition, production/Shared changes or essential hardware decision lacking defensible evidence; continue independent research. |
| Exit Criteria | All nine receivers have reviewed evidence/checklists/proof gates; exact baseline choices are justified or explicitly blocked with bounded resolution, without claiming unknown behavior implemented. T closure separately reviews the programme's hardware/contract freeze requirements. |
| Original Owner Request | Execute PC110 audit/design so each later implementation task has evidence, a checklist and original manuals or reference code. |
| Similar-Issue Sweep | Verify referenced raw evidence exists; distinguish original versus reconstructed manuals, implemented versus placeholder models, firmware dumps versus runtime shadows, CPU variant claims and actual bytes. |

S1's inventory, queue plan and ownership review are accepted as design delivery,
not complete hardware freeze. S2 completed its research goal. Current ten artifacts remain
unchanged because no executable inputs change.

S2 research delivery: all nine proposals consume the reviewed
[source record](../etc/evidence/t558-s2-source-availability.md) and
[receiver reconciliation](../etc/evidence/t558-s2-receiver-checklists.md#final-research-reconciliation).
Their source/checklist/ownership/proof coverage is reviewed; exact unresolved
hardware and asset choices retain named admission gates. No implementation,
firmware preparation, runtime qualification or new artifact is claimed.
The owner accepted this bounded delivery; remaining hardware decisions transfer
explicitly to the corresponding receivers rather than being counted as proven.

### Accepted Delivery — M5 T558 S1 PC110 Research And Gap Inventory

| Field | Required record |
| --- | --- |
| Identifier Mode | New T558 S1 after T557 closure; next NXVM numeric task. |
| Admission And Approval | Owner selected PC110 on 2026-10-13, assigns all current work to M5, then requests a multi-T queue plan. T558 is revised to research/contract freeze; nine explicit queued receivers retain its implementation/product goal. Owner also authorizes external reference checkout updates and external ROM embedding into committed PC110 EXEs. |
| Objective | Establish a source-tiered PC110 hardware, asset and current-code gap ledger, current ownership map and dependency-ordered multi-T implementation plan. |
| Non-goals | No production/test code, Shared-six changes, external source/ROM import, BIOS interception, asset edits or claims of a working PC110. |
| Reference Baseline | NXVM 8f00b1bce; external research revisions and dirty-source distinction recorded in the ledger. |
| Candidate Proposal | [PC110 evidence and implementation](../proposals/m5-pc110-evidence-and-implementation.md). |
| Files And ABI Surface | NXVM proposal, status, queue, roadmap, history and research evidence only; no ABI or executable input changes. |
| Applicable Rules | Guide, Execution lifecycle/coverage rules, Document rules, architecture ownership and source/research policy; architecture-governance skill. External sources are read-only research after the explicitly authorized checkout refresh; no code import. |
| Verification | Inspect current CPU/profile/device interfaces and external source provenance; distinguish original authority from reconstruction; check documentation structure, links, actual diff and clean scope. Design-only work does not manufacture runtime tests. |
| Expected Markers | Every inventoried domain has source status, current owner, gap, next receiver and proof gate; no M6 allocation among current candidates. |
| Asset Needs | Inventory external filenames and references only; lawful profile asset set is not yet admitted. Do not copy firmware/media or treat research dumps as release inputs. |
| Reporting Requirements | State facts versus hypotheses, exact CPU absence, external-reference limitations, proposed batches, unknown revision/assets and unexecuted hardware/runtime validation. |
| Stop Conditions | Source licensing/import, undocumented hardware behavior requiring an owner choice, Shared change or expanding S1 into implementation. Continue safe research on unresolved sources. |
| Exit Criteria | Reviewed initial source/gap ledger and bounded next research/implementation gates; each implementation receiver has a proposal, dependency, owner, proof and stop condition; M5 queue reconciled, documentation checks passed and complete documentation P pushed. T558 still needs hardware/contract freeze; no PC110 runtime qualification claimed. |
| Original Owner Request | Admit entry 3: IBM PC110 research and implementation; all currently queued work belongs to M5, not M6; begin research. |
| Similar-Issue Sweep | CPU enum versus historical 486 claims; stale Common/x86/chips/ibmpc paths in PC110 plan; roadmap/queue milestone allocations; reconstructed/emulator evidence versus original sources; owner-requested four-App/Core boundary audit and PC110 ownership receivers. |

The [initial ledger](../etc/evidence/t558-s1-pc110-source-and-gap-ledger.md)
is the T convergence inventory, not a complete register specification. Original
manual, board-revision and lawful asset qualification remain research gates.
Current artifacts remain the verified T557 PC pairs and MyNES 0044 pair;
S1 changes no executable input and creates no PC110 artifact.
The [ownership audit](../etc/evidence/t558-s1-core-app-ownership.md) adds actual
four-App/Core inspection and the proposed PC110 split without admitting code
changes; current manifest and dependency checks pass, with no dynamic retest.

The owner accepts T556 S11 and requests the new PC publication/INI task.
T556's NXVM work is closed. The owner subsequently closed S10's diagnostic
delivery with its unresolved MyNES input repair transferred to
[MyNES TODO](../../mynes/states/TODO.md#rdp-short-input-repair).
`3dc750be2` is not a production fix. No MyNES implementation is
part of T557's original S1; S2's approved receiver exception is recorded below.
Prior S9/S11 packets below are historical delivery evidence.

## Closed Task — M5 T557

| Task | Status |
| --- | --- |
| T557 | Closed after accepted S2 and task-level evidence review, before owner-selected T558 admission on 2026-10-13. |

Accepted delivery: Shared `5f5f43c8c`, MyNES `08bdaf06d`, NXVM `97d3cf948`.
Both widths passed 592 repository units, 36 PC integrations and 12 MyNES
integrations, with all ten EXEs deployed. The [S2 evidence](../etc/evidence/t557-s2-cursor-ownership.md)
retains the contract, review, hashes and verification limits. This closure only
updates governance; production and executable inputs are unchanged, so the
verified binaries remain current without a rebuild. The [proposal](../history/M5-T557-pc-publication-and-cursor-proposal.md)
is archived; both original naming scope and cursor extension are covered.

## Delivered Task — M5 T557 S1 PC Publication And INI Identity

| Field | Required record |
| --- | --- |
| Identifier Mode | New T557 S1, next numeric NXVM task after accepted T556 S11 and explicit MyNES residual transfer. |
| Admission And Approval | Owner requests four product-specific lowercase EXE/INI identities, explicit App INI injection and full affected qualification. This naming contract supersedes the old machine-key artifact naming in Execution for these four Apps only. |
| Objective | Publish my5160, my5170, mydeskpro386 and nxvm release pairs and load each App's own adjacent lowercase INI through the existing Core parser. |
| Non-goals | No SoftPC access/import, Shared-six changes, MyNES changes, configuration-content/media/ROM/snapshot mutation, fallback names or machine-key renaming. |
| Reference Baseline | ed75cf572, clean after separate MyNES diagnostic publication; accepted PC implementation 6b0cb13e8. |
| Candidate Proposal | [PC publication and INI identity](../history/M5-T557-pc-publication-and-cursor-proposal.md). |
| Files And ABI Surface | Four App build/main declarations; Core Product startup signature and version; NXVM build/deployment/gates; affected Core/App tests, current documents and eight assets. Root PC preset target version changes use a separate Shared commit, with no MyNES preset change. |
| Applicable Rules | NXVM guide and Architecture/Coding/UI/source policy; shared Execution/Architecture/Coding/Document rules; architecture-governance and coding-governance skills. |
| Verification | Existing x64/x86 Ninja/ccache graphs; affected startup/config/Console tests, all repository units, four-PC integrations and static/manifest/docs gates. Prove explicit INI selection, no wrong-name fallback, INI-relative media paths, lowercase release identities, PE widths and deployment hashes. |
| Expected Markers | Eight version 0.5.0557 release files with product-specific prefixes; four matching lowercase INIs, byte-identical contents; unchanged display names and internal machine keys. |
| Asset Needs | Read existing external inputs only; create and verify new eight artifacts before removing replaced PC EXEs; preserve MyNES and owner media/configuration bytes. |
| Reporting Requirements | Per-file scope/diff and naming map, actual dual-width results, release identity proof, missing manual/RDP verification; never infer desktop acceptance from unit tests. |
| Stop Conditions | Required Shared implementation, format/behavior change beyond naming, unexplained input mutation or conflict with other work. |
| Exit Criteria | Source/build/runtime/test/docs identities agree, required tests and eight deployments pass, old duplicate releases removed, reviewed target-scoped commits pushed; await owner review. |
| Original Owner Request | Close current S and admit a new T for four PC product-named EXE/INI publication and explicit runtime configuration loading. |
| Similar-Issue Sweep | All four App declarations/callers, integration/setup fixtures, release gates/scripts/presets and current normative documents; historical text stays historical. |

### S1 Delivery

Implementation and qualification are complete: 592/592 repository units and
36/36 four-PC integration executions per width. Eight 0.5.0557 product-named
EXEs are deployed and the replaced eight filenames removed. Four renamed INIs
retain their original hashes. Both specialized gate aggregates, manifests,
registration, INI/publication and documentation checks pass. The
[delivery evidence](../etc/evidence/t557-s1-publication-ini-identity.md) records
actual diff, final hashes and limits. Shared-six and MyNES are unchanged.
S1 is delivered for owner verification; T557 remains open.

## Accepted Task — M5 T556 S11 Default Text Raster

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: next unused S11 after the MyNES S10 admission; do not execute or modify that product's pending work. |
| Admission And Approval | Owner requests default NXVM 80x25 full 8x16 text, DeskPro verification only unless a defect is proven, dual-width qualification and deployment. |
| Objective | Repair the default guest BIOS text-mode register/font geometry so boot and graphics-to-mode03 return produce 640x400, with correct cursor and raster timing; establish whether DeskPro 640x350 is correct. |
| Non-goals | No Shared change, forced Window height, host BIOS interception, external ROM or INI/media mutation; preserve MyNES and other pending changes. |
| Reference Baseline | Current master 7bfefb2fc, plus preserved S10 documentation/diagnostic work; review b2ee99b1c and b25e2ca8d and subsequent firmware changes. |
| Candidate Proposal | [T556 proposal](../proposals/m5-app-component-layout.md), owner-added S11. |
| Files And ABI Surface | Default firmware/video tests and native size diagnostic; owner-added Core CPU retirement selection and existing capture boundary, timing tests/gates and Core manifests. Timing selector becomes CPU-private; no Shared API change. Docs and affected four-PC dual EXEs. |
| Applicable Rules | NXVM guide, architecture, coding, UI and source policy; shared Execution/Architecture/Coding/Document rules; architecture-governance and coding-governance skills. |
| Verification | Real guest execution at boot and return from graphics; complete font halves, cursor, vertical active/retrace/exit; snapshot-to-Window 640x400; actual client dimensions. Both existing Ninja/ccache graphs, full repository units, all four PC profile integrations including default DOS/Windows and required static/manifest/doc gates. Actual RDP DPI proof is separately reported, never inferred from a synthetic frame. |
| Expected Markers | Default 80x25, glyph height 16 and 640x400; coherent CRTC overflow/total/display/blank/retrace and cursor registers. DeskPro retains 14/350 if sources and selected board agree. |
| Asset Needs | Existing external manuals and build inputs read-only; refresh affected four-PC 0556 x64/x86 pairs and validate PE identity after the Core repair. Preserve MyNES, adjacent owner inputs and unrelated EXEs. |
| Reporting Requirements | Record sources and timing grade, before/after reproduction, actual diff, dual-width results and release hashes; distinguish native client, outer frame and untested RDP scaling. |
| Stop Conditions | Required Shared/API change, unsupported EGA register semantics, unproven DeskPro defect, or overlap with unrelated work. Report before extending scope. |
| Exit Criteria | Correct default guest BIOS and regression proof, required qualification/deployment complete, actual-diff review and scoped commits pushed; await owner desktop acceptance, T remains open. |
| Original Owner Request | Investigate/fix default Window text geometry from guest BIOS and verify rather than normalize DeskPro 640x350. |
| Similar-Issue Sweep | Check all default text-mode initialization/restore paths, glyph upload, BDA geometry, cursor and raster counters; inspect selected DeskPro BIOS/board without changing external ROM. |

S11 implementation and final automatic qualification are complete: 592/592
units and 32/32 four-PC profile integration executions per width. All eight
PC EXEs are rebuilt and deployed; MyNES and public six remain unchanged.
The default BIOS now presents all 16 font rows. Its exposed Core TIME fault
is repaired by selecting completed-instruction timing before interrupt entry
reuses decoder state. The actual-Core regression fails against admission
objects and passes with the repair. DeskPro retains its correct 14/350 mode.
Native remote-session observations no longer show the TIME failure, but the
diagnostic pause acknowledgement and owner phone-client DPI/interaction are
not qualified by automation. The owner has accepted this scoped delivery;
S11 is closed. Preserve the separately retained MyNES S10 pending work. See the
[S11 evidence](../etc/evidence/t556-s11-default-text-raster.md).

## Delivered Task — M5 T556 S9 Audit Repairs And Qualification

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: S9 follows completed S8 audit and accepted S1--S7. |
| Admission And Approval | Owner's 2026-10-13 request assigns necessary audit repairs to S9. Targets are NXVM and MyNES, with separate commits. The proposal's existing Shared root-build receiver covers only three PC preset target names changing from 0555 to 0556; public six and shared rules remain excluded. |
| Objective | Flatten DeskPro source and matching unit tests under the owner's S9 amendment; repair A1--A5 under that revised ownership, retain A6's separate Td receiver and complete five-App qualification. |
| Non-goals | No Shared-six or shared-rule edit, unrelated hardware rewrite, new runtime manager, persisted-format change or owner-input mutation. |
| Reference Baseline | S8 audit `a28cac691`; accepted S7 implementation `307cf1bd6`. |
| Candidate Proposal | [Five-App component layout](../proposals/m5-app-component-layout.md). |
| Files And ABI Surface | Core test registration/owner gates; all DeskPro source moved to its App root and units to its unit root, with includes/guards/symbols/build/gates/docs repaired; moved MyNES Machine symbols/callers; corresponding product version/artifact receivers. No Shared API change. S9 actual diff is recorded against the amended flat file map. |
| Applicable Rules | Product guides; shared Execution, Architecture, Coding and Document rules; both product architecture/layout authorities; source policy; architecture-governance and coding-governance skills. |
| Verification | Correct labels/names without reducing cases; reject incomplete D4 attachment with rollback proof; run dual-width full units and every admitted profile/MyNES integration in original Ninja/ccache trees; verify manifests, dependency/doc gates and affected release pairs. |
| Expected Markers | No absolute-path or mislabeled Core tests; no DeskPro chips/machine/product source directories or matching unit nesting; one D4 state owner and one Core attachment owner; missing alias rejects atomically; documents match current source; complete per-route results and ten qualified artifacts. |
| Asset Needs | Existing BYOB inputs read-only; update affected executable pairs only, preserving INIs, media, ROM originals and snapshots. |
| Reporting Requirements | Report per-target actual added/deleted/net lines, retained owners, exact test coverage, artifact identities/hashes, failures and unexecuted desktop checks. |
| Stop Conditions | Required Shared-six/rule change, incompatible saved data, unexplained behavior change or newly expanded hardware scope. Report rather than patch outside this packet. |
| Exit Criteria | A1--A5 repaired with evidence; A6 explicitly transferred; required dual-width builds/tests and artifact checks complete; actual changes reviewed, per-target P commits pushed, clean worktree; T closure still requires final acceptance. |
| Original Owner Request | Admit S8 for comprehensive governance, documentation and code-quality audit; admit S9 for necessary repairs. |
| Similar-Issue Sweep | Search all five App owners and Core plus current authorities for the same stale ownership, registration, vocabulary and contract defects; retain historical terminology as history. |

### S8 Audit Delivery

The [audit ledger](../etc/evidence/t556-s8-quality-audit.md) records six
confirmed classes and their S9 disposition. Both widths pass the 18 selected
static gates; both documentation checks and the production dependency DAG pass.
No new full dynamic qualification is claimed. Production, tests and assets
remain unchanged. Shared-rule vocabulary retains a separate Td receiver.

| Task | Progress |
| --- | --- |
| T556 S9 | Delivered for owner review: 592/592 units and 44 profile/MyNES integration executions per width, both specialized gate targets, documentation/dependency/manifest checks and ten release artifacts. T556 remains open. |

## Retained S9 Delivery

The [S9 delivery report](../etc/evidence/t556-s9-quality-repair-delivery.md)
records exact scope, actual diff, logs, artifact hashes and unperformed manual
acceptance. Four PC Apps retain 0.5.0556 under their flat asset roots; MyNES
retains its independent 0.0.0044 pair. The PC target is `vm-0-5-0556`.
MyNES receiving commits are `7fcd7b1d9` and `c5ca06e85`; the NXVM commit
containing this packet/report identifies its delivered source and eight EXEs.
Root preset names use a separate Shared-target commit; the public six are unchanged.

## Historical Evidence

Prior packets remain in the [retained status archive](../history/M5-T556-retained-status.md).
They are not concurrent active tasks. The [T556 history](../history/M5-T556-app-component-layout.md)
records accepted layout work, S11 closure and the explicit MyNES residual.
