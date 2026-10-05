# M5 T544 Retained CPU Qualification

## Admission And Baseline

Owner approved the first queued candidate on 2026-10-05. T543 is closed at
dd9af8951. This task qualifies retained CPU contracts, not another extraction.
Current owns the active packet; the proposal owns scope and initial S plan.

## Frozen Coverage Universe

The implemented public enum has DEFAULT plus 8086, 8088, 80186, 80286 and
80386. DEFAULT is a selection sentinel, not a sixth CPU. Product bindings are
My5160=8088, My5170=80286, MyDeskPro386=80386 and NXVM=80386; retained CPU-only
callers/tests continue to exercise unused 8086 and 80186.

Coverage consists of each CPU's legal decoder form and its source-defined
operand/address size, memory/register, prefix, repeat, mode, privilege and
success/fault/delivery context. F01-F14 and architectural state/delivery rows
in the existing [List 1](../etc/evidence/t512-s2-five-cpu-function-state-timing-list-1.md)
provide the named family partition, not proof that every expansion is correct.
8088 retains a distinct transfer/bus timing rule despite sharing base forms.

The five timing manifests expand to 4,906 canonical keys: 8086=1,053,
8088=1,053, 80186=616, 80286=771, 80386DX=1,413. This is a timing catalog,
not the number of instructions or complete semantic test contexts.
The larger opcode/ModR/M inventories are an admission mask, not semantic proof.
Each family S must reconcile its whole source-defined form/context batch,
including source-valid forms absent from the current decoder.

## Dispositions And Completion Predicate

- Accepted: direct current source-to-code-to-regression proof with exact context.
- Non-applicable: source-defined absence, with reason and negative admission proof.
- Pending: implementation/source/context reconciliation still required in T544.
- Retained non-eligible: named missing capability with explicit receiver.
- Blocked: authority or implementation prerequisite, reported without a grade claim.

No accepted family may hide a pending form behind a later boot checkpoint.
T closure requires every implemented-family unit to be accepted or source-proven
non-applicable; any approved transfer must name its complete batch and receiver
and narrow the claimed qualification accordingly. Complete units, external
integration and current affected dual-width artifacts are closure evidence,
not a substitute for source/function/timing reconciliation.

## S1 Inventory Batch

Tracked inventory contains nine CPU implementation/header files and 90 CPU
test files (81 C sources, nine fixture headers). File counts identify the
surface to read, not the number of independently proved instruction forms.
The four profile bindings were checked directly, not inferred from EXE names.

| Surface | Actual owner/evidence | S1 disposition and receiver |
| --- | --- | --- |
| Models and fixed bindings | x86/chips/cpu/cpu_interface.h; four App profiles | Inventoried; all five retained, no new model introduced. |
| Decoder/handlers, state and delivery | x86/chips/cpu/cpu_instructions.c and cpu.c; CPU-only test/x86/chips/cpu | Pending shared-boundary S2 and complete family S3-S6 source reconciliation. |
| Timing selection/model | x86/chips/cpu/cpu_timing.c and cpu_timing_model.c | Pending family S3-S6; one selector retained, no board-side CPU model. |
| Time publication and fault non-retirement | x86/core plus test/x86/core and PC composition regressions | Pending S2 cross-family boundary audit. |
| Timing catalogs/results and decoder producers | docs/nxvm/etc/cpu-timing, tools/nxvm, cmake/nxvm/NxvmProduct.cmake; test/ibmpc/board-common/composition | Structurally counted; fresh full units pass on both widths. Family-specific manual/state proof remains S3-S6. |
| Original source identity | [T512 source record](../etc/evidence/t512-s1-five-cpu-source-cross-validation.md) | Prior identity/citation index retained; page/form revalidation pending appropriate family S. No fresh manual-reading claim. |
| Prior state/code gap and tier results | T512 List 2 and [final audit](../etc/evidence/t512-s9-five-cpu-final-tier-owner-audit.md) | Historical accepted results, not current-source whole-family proof. Reconcile after chip/Core ownership changes. |
| 80188 | No implemented public enum; no selected product | Retained non-eligible; independent evidence/implementation requires separate owner admission, not an 80186 alias. |
| 486 / PC110 | No implemented public enum or App | Retained non-eligible; queued [PC110 evidence/component receiver](../proposals/m6-pc110-evidence-and-implementation.md) must select and implement the actual variant. |

### Structural Catalog Observation

Verify-CpuTimingManifestContract.ps1 reports 4,906 keys and 4,092
nonconforming template statuses. These are retained planning/status snapshots:
the exporter marks current_ticks as not-observed. They are not 4,092 demonstrated
runtime failures or L1 instructions. Generated result files and their verifiers
are separate evidence. T512's final historical counts remain historical; neither
observation alone changes current timing grades.

### Fresh Runtime Catalog Baseline

The x64 complete unit run regenerated all five timing/decoder result files.
These counts describe executed catalog recipes, not an independent manual
oracle or proof for unenumerated state contexts:

| CPU | Timing keys | Reported L3 | Reported L2 | Failed/unallocated | Lexeme opcode/ModR/M candidates |
| --- | ---: | ---: | ---: | ---: | ---: |
| 8086 | 1,053 | 989 | 64 (Group 3 model) | 0 | 57,926 |
| 8088 | 1,053 | 989 | 64 (Group 3 model) | 0 | 57,926 |
| 80186 | 616 | 580 | 36 (midpoint) | 0 | 61,530 |
| 80286 | 771 | 771 | 0 | 0 | 61,803 |
| 80386DX | 1,413 | 1,411 | 2 | 0 | 63,021 |

The result schema's L2 labels remain unchanged. No new L1 or tier downgrade
has been demonstrated by this inventory. The executable CPU admission mask,
source-legal forms, timing key contexts and architectural-state/delivery
contexts are distinct universes and must not be conflated.

### Exact Sweep And Planned Proof Boundary

The read-only sweep used `git ls-files src/x86/chips/cpu`,
`git ls-files test/x86/chips/cpu`, `rg --files` for CPU timing/decoder records,
and `rg -n` for profile bindings, metadata admission, timing selection,
source_timing_unallocated and successful retirement in CPU/Core. Actual
public enum, fixed App selections, reset-code-base switch, minimum-CPU
metadata gate, timing selector and Core publication seam were inspected.

There is one chip-owned timing selector and one Core-owned successful-time
publication seam. Core resolves DEFAULT to 80386. Their structural presence
does not qualify every caller: S2 must cover all reset/delivery/retirement
contexts, and S3-S6 must prove every source-defined family expansion.
Historical old paths are retained as historical evidence; only the live
proposal's stale owner/deployment assumptions were corrected. S1 added no
synthetic test/decoder or alternate source table.

### Verification And Review

S1 complete repository-only units pass once per width: x64 506/506 in
63.27 seconds; x86 506/506 in 60.58 seconds. Both used RunTestAggregate.ps1
with the existing default receiving cache, four jobs and a 300-second deadline.
Both generated result sets contain the same five counts and no failed or
unallocated recipe. The catalog structural check, documentation governance,
changed-document relative links, 16-field packet check and git diff --check pass.

Production source, test bodies, manifests, assets, INI and MyNES remain
unchanged; no new EXE is needed for this inventory/design-only S. Source/test
code diff is zero. Full external integration and new artifact production are
reserved for actual runnable changes/T closure, not fabricated design proof.
Incremental baseline caches are retained for the next qualification S.
Executor P1 is 742c31f23, pushed to origin/master. Coordinator actual-change
review checked all five changed documents against the immutable S1 packet:
five CPU enum/bindings, finite catalog/context distinction, exact source/test
owners, deferred 80188/486 prerequisites, initial receiver partition and both
fresh complete suites are present. Only NXVM documents changed. Skills informed
the one-owner boundary and rejection of a duplicate decoder/timing path.
The entire S1 inventory batch is accepted; unresolved manual/state proof stays
explicitly pending in S2-S6, not silently qualified. S1 is closed; T544 is open.
No owned test process remains active. The next S requires its own packet.

## S2 Boundary Audit Delivery

The [S2 evidence](../etc/evidence/t544-s2-cross-family-boundary-audit.md) records
confirmed divide-error return-address, reset-CS-limit, SS/debug/NMI arbitration,
runtime length and outgoing FLAGS-image mismatches against rendered original
manual pages and actual source/delivery chains. It distinguishes remaining
generation/source contexts from those specific discrepancies and records an
8086 IDIV arithmetic lead for S3. Immediate/delayed successful-time publication
and the fault-delivery early return were inspected; no complete physical-time
qualification claim is made. Further original-page review confirms a 386 RF
fault-frame gap and reconciles the retirement observer's pre-qualification
notification as an intentional, tested contract rather than a clock bug.
Fresh S2 complete units pass once per width: x64 506/506 in 60.41 seconds;
x86 506/506 in 60.39 seconds. This is a read-only audit delivery, not
Shared repair approval or whole-CPU qualification.
Further rendered 286/early-family pages establish the 286 10-byte/UD length
contract, 286 NMI/priority requirements, early-family interrupt differences
and early reset selector/offset mismatches. The separately read 80186 reset
table confirms FFFF:0000 and qualifies the status-word/image distinction.
The evidence now maps the finite structural paths to findings and remaining
source/regression receivers; no early/186 peripheral completeness is claimed.

S2's complete boundary inventory is delivered for coordinator review. It
classifies reset, admission, FLAGS images/loads, arbitration, exception
completion and immediate/delayed retirement, naming exact code paths,
existing regression limits and remaining family/repair receivers. Missing
source contexts remain pending inside T544; no implemented-family form is
accepted merely from catalog or boot success. Four coherent mechanisms are
proposed for owner review before Shared edits. No source/test/build/asset
changed; code diff is zero and existing EXEs remain current.

Executor review checked the full evidence against the active S2 scope:
all nine structural rows have an observed disposition and receiver, directly
confirmed claims name rendered original pages/current paths, uncertain contexts
are not called correct, the observer ordering is reconciled with its actual
contract, and the complete units passed once per width. Documentation structure,
changed-document relative links, packet shape and diff checks are the delivery
gates; S2 is not closed until coordinator actual-change acceptance and push.

Executor P1 is be44d5be0, pushed to origin/master. Coordinator switched roles
and reviewed all three actual changed documents and the immutable packet,
rather than treating passing units as qualification. The nine structural rows
map the admitted five-family boundaries to rendered-page findings, current
owners, regression gaps and complete remaining receivers. B07's observer
ordering is explicitly not a defect. Source-uncertain contexts and the IDIV
family lead remain pending inside T544; no timing grade or code is changed.
The packet's inventory/proposal objective and full-unit/documentation gates
are satisfied. S2 is accepted and closed by the governance delivery; T544
remains open. All four proposed Shared repair mechanisms require owner review
and an implementation packet before changes. No additional unit/integration
rerun or EXE rebuild is needed for this documentation-only acceptance.
The ignored research render/extraction directory is retained for the immediate
family reconciliation successor; it is not a source/artifact baseline.

## S3 Early-Family Audit Delivery

The host-arithmetic sweep distinguishes safe early one-bit unsigned shift
loops from three unsafe DX:AX word concatenations. Later-only signed-one
bit masks have a named S6 receiver; SAR's signed-right-shift portability and
Group 2 failed-write rollback remain explicit proof contexts. These are
read-only source dispositions, not runtime failure or Shared repair claims.
The checked-write investigation confirms the existing whole-CPU restoration
owner, so pre-write FLAGS calculation alone is not a defect. Ordered split
writes and selective provider rejection retain a separate proof receiver;
register rollback is not claimed to undo arbitrary RAM/MMIO side effects.
The admission sweep distinguishes blocked later forms from unproved early
undefined-encoding behavior. List 1's 8086-only POP-CS wording differs from
the both-early implementation; negative UD assertions and lexeme LOCK
rejection are not independent hardware authority. These source/policy
contexts remain in the same finite S3 receiver matrix.

S3 consumes F01-F10/F14 for both 8086 and 8088. Its
[working evidence](../etc/evidence/t544-s3-8086-8088-family-audit.md) retains
the complete family partition with unresolved source/code/regression contexts.
Fresh rendered Intel 1981 pages identify an early-IDIV quotient-bound discrepancy:
the common helper permits -128/-32768 rather than -127/-32767. Existing early
Group 3 recipes are acknowledged, but do not cover those excluded minima or
the early type-0 frame. The inspected IDIV/IMUL range literals and word-transfer
addition agree with the cited table; their model choices remain L2, unchanged.
The later cross-edition check does not independently resolve the exact minimum;
this remains a source/code discrepancy, not a demonstrated silicon defect.
A helper-local generation rule is conditional on that source reconciliation;
the coherent S2 delivery repair is also proposed, not implemented.
Source/test diff remains zero. S3 is not closed;
qualification remains pending. The S3 audit inventory and its thirteen
receiver batches are delivered with fresh complete units, 506/506 per width:
x64 61.92 seconds and x86 62.56 seconds. Documentation, link and diff checks
pass. The three NXVM records are the only change surface; Shared implementation,
tests, App configuration and all artifacts are untouched. The active packet
remains in the implementation delivery for coordinator acceptance.
F02's shared arithmetic/forms and base-clock rows are now reconciled in the
working evidence, with original encoding conflicts and test limits retained.
The broader direct decoder-call sweep names thirteen sites across F01/F02/F03/F05,
including LEA's effective-address helper omitted by the narrower first search;
runtime failure proof and coherent repair remain pending. Two selected existing
ALU tests pass per width, not a full S delivery. No Shared code changed.
F03's normal transfer/address/pointer paths and selected base-clock rows are
now partially reconciled. Five existing data/segment/unary tests pass once per
width; source legality, bus/failure ordering and both-early-profile coverage
remain explicit receivers, not qualification claims.
F04/F10 source and normal-path inspection distinguishes stack publication,
PUSH SP generation behavior and direct flag operations from unproved
wrap/provider effects, unspecified image bits and S2 inhibition/FLAGS defects.
Four selected existing tests pass each width; their masks/profile coverage
are recorded rather than treated as full early-family proof. S3 remains open.
F08 adds normal repeat/index/compare reconciliation and the source-defined
early multi-prefix interrupt limitation. Whole-start rewinding and unchecked
below-386 element calls have named delivery/failure proof receivers; the
existing timing owner already distinguishes setup and continuation. Five
string tests pass once per width, with absent 8088 contexts recorded. No
Shared repair or complete family qualification is claimed.
F09/F14 source/code inspection records the early TEST sampling formula and
no-FPU memory ESC read omission, without inventing a second wait/completion
owner. I/O provider completion, HLT wake and XLAT boundary contexts retain
named proof receivers. Four selected tests pass each width; no Shared code,
timing grade or artifact changes and no S3 closure are claimed.
F05 reconciles normal predicates, return addresses and frames while retaining
S2 FLAGS/inhibition and multi-write failure receivers. A source/code
zero-displacement counterexample exposes final-PC-based branch timing
selection; the cross-family selector sweep and one-outcome-owner repair
proposal are recorded. Five existing transfer tests pass each width, without
qualifying that uncovered case or closing S3.
F07 adds normal count/flag and selected L3 formula reconciliation; the actual
rotate test passes both widths but its early matrix omits 8088/extreme counts.
An original 16-bit-offset rule exposes XLAT's unmasked promoted BX+AL sum;
the receiver distinguishes effective-address truncation from memory transfer
wrap and keeps 386 address-size-four intact. Runtime boundary proof and
concrete Shared repair remain pending, not an S3 acceptance claim.

### S3 Coordinator Acceptance

The executor delivery is ebe0603e7. Coordinator actual-change review reads
the three changed NXVM documents, including the new audit, rather than accepting
the unit summary as semantic proof. All eleven early-family partitions have
source/code/regression dispositions; thirteen complete pending receivers remain
inside T544. The review corrected an overstrong IDIV cross-edition statement
before P1: the signed minimum is a source conflict, not an independently proven
silicon-generation rule. No pending context is marked repaired or qualified.

The 16-field packet, original request, finite receiver mapping, source-versus-
deduction wording, same-owner repair proposals, changed-document links,
documentation governance and actual diff checks satisfy the read-only S3
delivery contract. Source/test/artifact changes are zero. Fresh complete units
pass once per width as recorded above; no additional run or EXE rebuild is
required for this documentation-only acceptance. The source/repair proof
requirements and concrete Shared approval remain outstanding in T544.

S3 is accepted and closed as an audit inventory, not whole-family CPU
qualification. Current removes its active packet and retains compact progress.
The next numeric S requires its own admitted packet; the later-family audits
must carry all cross-family receivers forward. The ignored research scratch and
existing receiving caches remain needed for that immediate successor.

## S4 80186 Audit Batch

S4 starts against 946f7a737 and consumes additions plus the inherited
F01-F10/F14 surface, not only the new-opcode list. Its
[working audit](../etc/evidence/t544-s4-80186-family-audit.md) freezes the finite
form/state/timing batch and carries S2/S3 cross-family receivers forward.
First rendered original pages corroborate PUSHA's original-SP order, immediate
sign extension, 186 modulo-32 shifts and full-byte ENTER level. Existing ENTER
regressions already include 186 level 255; no invented missing-path repair is
proposed. Exact stack base clocks match the inspected source rows under their
prefetch/no-wait/even-word assumptions. Complete timing, failure, delivery,
source exclusions and unit verification remain pending. Shared edits and
qualification are not admitted by this partial audit.

Further original-page and selector/oracle inspection reconciles the MUL,
IMUL and BOUND range choices as L2, DIV's exact clocks as L3, and ENTER's
level-dependent formula as L3. Selector origin names are not grade labels;
the legacy records already distinguish exact DIV and midpoint BOUND. The
segment MOV reversed bases also occur in the existing regression oracle and
derived override/odd-word expectations, so green catalogs are not independent
proof. These findings and the attribution/source-conflict receivers remain
in the S4 audit; no Shared repair or full S4 verification is claimed.

Group-2 follow-through reconciles five-bit masking, checked element publication
and selected timing formulas, while recording the missing 186 boundary/FLAGS
matrix and no-consumer undefined-flag metadata distinction. Immediate IMUL's
word arithmetic and existing normal register/memory cases are reconciled;
the later 32-by-8 signed intermediate overflow is assigned to S6, not falsely
reported as a 186 defect. Two corresponding tests pass once each width;
complete S4 inventory and full delivery gates remain pending.

BOUND follow-through includes the existing board-level 186 vector-five/frame
test, passing once each width; source-qualified nonzero/prefix return-IP and
read-failure/wrap contexts remain pending. Its second-word offset increment
is distinguished from the logical helper's within-transfer wrap, widening the
effective-offset receiver rather than claiming that helper proves all cases.
ENTER/LEAVE publication and level-255 test assertions are inspected; selective
transfer failures, full copied-slot image and allocation/wrap contexts remain
explicit stack receivers. No Shared edit or whole-family qualification follows.

REP follow-through distinguishes normal FIRST/CONTINUATION/ZERO modifier tests
from the missing interrupt/restart/publication contexts. The timing identity and
ExecInit-before-observation path stay in one shared retirement receiver. ESC
inspection finds no 186 relocation-bit control input, while the existing board
NM test fixes the CPU to 386. The missing optional 186 control capability has
an explicit admission/delivery and integrated-control-input receiver; generic
CR0 behavior is not accepted as its implementation. Shared changes remain
unapproved, and complete S4 verification/qualification remains outstanding.

Inherited branch review visually checks the unambiguous generic JCXZ/LOOP
rows against the displaced dedicated table, retaining LOOP/LOOPNE clock
conflicts instead of guessing from OCR. The final-PC branch-decision inference
and unchecked below-386 JCXZ calls join the existing cross-family receivers.
FFFF already has an undefined handler; lexical inventory counts are not proof
of all unused-encoding exception frames. These distinctions are recorded in
S4 without Shared edits or a claim of complete unit/source qualification.

ALU/movement/port follow-through reconciles adjustment, exchange, LEA and
ordinary port base clocks. It finds that legacy accumulator-immediate and
immediate-MOV midpoint descriptions omit the dedicated table's 8/16-bit
annotation; width-selected source data must not be mistaken for a latency
range. Concrete byte/word selection and attribution receivers are retained.
Ordinary MOV's generic/dedicated direction conflict and XLAT's prefixed-clock
boundary stay separate from the corroborated segment-MOV defect. No timing
grade or Shared source is silently changed by this read-only finding.

The logical width sweep confirms individual AND/OR/XOR/TEST rows and includes
them in the paired-width receiver. IDIV's genuine ranges and midpoint L2
classification are reconciled separately; its quotient-source, signed-host
bookkeeping and 186 fault/state coverage remain pending. Three existing ALU,
divide/form and normalization tests pass once each width (3/3, x64 0.23s,
x86 0.25s), without implying a completed S4 full-unit or source gate.

Stack follow-through confirms actual 186 complete normal PUSHA/POPA image
assertions and negative-byte immediate PUSH coverage, while preserving wrap
and selective-transfer gaps. Current 186 reset/FLAGS still contradict the
S2 source record; a test expecting cleared high bits does not supersede that
source. Inhibition, return-IP and integrated-control capability remain named
generation-specific receivers rather than inherited assumptions.

S4's complete thirteen-partition audit inventory is delivered for coordinator
review. Each partition names source/code/regression evidence and complete
remaining contexts; none is represented as whole-instruction qualification.
The final generic-table review distinguishes the corroborated three-clock
NEG/NOT bases from the non-repeat MOVS nine/fourteen source conflict. Numeric
width annotations, source direction conflicts and genuine latency ranges
remain separate at the sole timing owner. S2/S3 mechanism receivers remain
inside T544; no missing context is transferred out or erased by a green oracle.

Fresh S4 complete units pass once per width: x64 506/506 in 142.71s and
x86 506/506 in 63.31s, eight jobs with bounded 300-second aggregates. Both
owned process handles are terminal with exit zero. Source/test/artifact diff
is zero; no MyNES, INI, firmware or manifest changes occurred, and no EXE
rebuild is required. The three changed NXVM documents comprise this delivery.
Executor self-review checks the original packet, finite batch, source versus
deduction wording, pending receivers and the full-unit/documentation gates.
This delivers the read-only S4 brief; coordinator actual-change acceptance
and governance push are still required before S4 closes. T544 stays open.

### S4 Coordinator Acceptance

Executor P1 is 3fb252295, pushed to origin/master. Coordinator actual-change
review inspects all three changed NXVM documents against the immutable
sixteen-field S4 packet, original request and task convergence ledger. The
thirteen partitions cover additions and inherited forms with explicitly named
residual contexts; matched clocks, confirmed discrepancies, source conflicts
and unqualified failure/delivery paths are not conflated. No residual member
is hidden by catalog success or transferred outside T544. The source identity,
visual-page record, existing regression limitations and fresh complete units
satisfy the admitted read-only inventory contract, not CPU qualification.

Documentation governance, changed-document links, packet/allocation review
and actual diff checks pass. Source, tests and deployed artifacts have zero
changes; no extra runtime rerun or EXE rebuild is warranted for acceptance.
S4 is accepted and closed by this governance delivery; T544 remains open.
The next family audit requires its own numeric packet and must retain all
S2-S4 repair/source/regression receivers. Shared edits still require concrete
owner review; audit acceptance grants no such approval.

## S5 80286 Audit Batch

S5 starts against ebdb40098 and consumes F12 plus inherited F01-F11/F14.
The [working audit](../etc/evidence/t544-s5-80286-family-audit.md) freezes ten
form/state/timing partitions, including real/protected and privilege/failure
contexts. Source identity/page inspection, handler/publication chains and
actual regression coverage remain to reconcile. All S2-S4 pending receivers
remain inside T544. This admission is not permission to edit Shared source,
tests or ABI, nor a claim that the existing 771 green timing keys qualify 286.

The working query/layout audit now reconciles Appendix B and Chapter 11 with
Figures 6-3/6-4 and the actual query/load/task-cache consumers. It retains the
four-query Present assumption and generation-qualified type/layout/publication
batch, including LTR's unqualified type-one/type-nine mask. Existing query
tests execute on both widths but encode some disputed assumptions; no repair
or complete 286 qualification is claimed. S5 remains active and undelivered.

The far-control follow-through adds source-confirmed conforming CALL/JMP/RET,
local-table selector, ring-specific TSS-stack and outer-return DS/ES receivers.
It distinguishes the legal 286 zero-parameter machine gate fixture from outer
error tests that select 286 but install a 386 TSS. Existing far/outer cases pass
once per width; missing contexts remain explicit in the working audit. This is
additional inventory evidence, not a Shared repair or S5 acceptance.

Task-switch Appendix reconciliation resolves the incoming TSS inclusive 2Bh
limit and confirms TS for a non-present task LDT, SS for an otherwise valid
non-present task stack, and late faults in the incoming task. The current
task16 oracle retains old TR for its LDT failure; changing only its exception
mask would be insufficient. The audit also retains the outgoing static-LDT
overwrite/dynamic-save-span receiver. S5 remains active and read-only.

Interrupt-source reconciliation additionally retains 286 DF/shutdown recovery,
EXT error provenance, NT clearing, task-gate error-word delivery and common
gate stack/selector validation. The inspected DF fixtures select 386; their
green status does not qualify the 286 delivery matrix.

IRET reconciliation adds original-page proof for FLAGS privileges and the
17/31/55/169 plus next-byte timing formula, while retaining outer-return
local-table/conforming admission, SS-before-IP priority and DS/ES invalidation
receivers. The actual 286 real-boundary and board-level same/outer-return
fixtures pass once each width; the dedicated protected CPU fixture is 386.
Legal 286 returns are covered narrowly, not the whole return-context matrix.
The original outer conforming predicate remains a named source discrepancy,
not an OCR guess. S5 remains active with Shared source/tests unchanged.

Asynchronous-source reconciliation now identifies missing NMI-in-service
inhibition through IRET and NMI-before-single-step ordering against the 286
original. Existing five-profile signal tests cover external mask/pending/wake,
not nested service or simultaneous TF/NMI. Their once-per-width passes are
retained narrowly; SS shadow and STI distinctions stay in the shared
delivery/return receiver. No board-specific workaround is proposed.

Inherited MUL/IMUL/DIV/IDIV reconciliation confirms the exact 286 base clocks
and normal word-product rules, while extending the S3 unsafe DX:AX capture
receiver to 286. Actual signed-division arithmetic and its earlier operand
capture have different widening behavior; a safe later calculation does not
erase the earlier undefined shift. Immediate IMUL has real 286 regression
coverage, but the legacy Group-3 matrix selects 8086/186 only. Undefined FLAGS,
signed extrema, memory exceptions and complete #DE delivery remain explicit
contexts; matching timing bases do not close the inherited arithmetic batch.

Stack/frame reconciliation retains the 286 whole-span early-limit and ENTER
final-allocation checks, PUSHA real shutdown/exception distinctions and legal
286 regression contexts. Normal default matrices actually include 286, but
protected-failure fixtures select 386 and can bless earlier external writes.
Original LEAVE B-64 and the t435 source ledger give five clocks; production
and the manifest oracle both use eight. This is a source/producer/oracle
repair receiver, not a reason to downgrade an exact manual term. S5 remains
read-only and active; existing stack cases passed once each host width.

Count/branch reconciliation confirms the five-bit 286 count rule and retains
the distinction between RCL/RCR result-cycle reduction, FLAGS definedness and
raw timing count. Taken Jcc lacks its original next-byte timing term; IP-only
taken inference also misclassifies zero-displacement branches across the
shared Jcc/LOOP/JCXZ receiver. Existing 286 matrices cover ordinary conditions
and selected count forms, not the timing/failed-operand context matrix. Their
once-per-width passes are recorded narrowly; Shared remains unchanged.

String/restart reconciliation confirms the original 286 REP iteration order
and exact printed repeat bases, while mapping unchecked pre-386 ordinary
string calls to the existing propagation/retirement receiver. ExecFinal's
oldcpu restoration prevents claiming the intermediate CX decrement necessarily
survives a fault; failed provider effects and attribution still need proof.
Checked INS/OUTS have distinct memory-before-port ordering and generation-qualified
privilege denial. Existing normal string fixtures include 286, but protected
failure fixtures select 386 and legal port-string success selects 186/386.
The current Appendix-B claim for REP LODS has no row on B-92 or formula on
B-69; retain an exact-authority/model-disposition receiver, not a speculative
illegal-encoding claim or automatic downgrade. Seven existing cases pass once
each width; Shared and product artifacts remain unchanged and S5 stays active.

External-cycle reconciliation adds original-page proof for HLT/WAIT bases,
LOCK privilege/zero-prefix clocks and the MSW MP/TS matrix. It retains WAIT's
atomic completion versus interruptible BUSY ordering, EM-qualified WAIT error
testing and ESC error exemptions, 286/287 operand-transfer/protection delivery,
and physical versus merely transparent LOCK contexts. Abbreviated TS-only
WAIT prose is not used to delete the explicit MP rule. Existing actual 286
handoff/deadline tests and real LOCK execution pass once each width but do not
qualify every external signal interval. Shared remains read-only; the owner
explicitly reaffirmed that all T544 CPU audit work precedes any other task.

Ordinary ALU/decimal reconciliation confirms the original 286 base clocks,
including CMP's distinct seven/six memory directions, and normal width/FLAGS
rules. It retains unchecked AAM/AAD immediate fetches in the shared propagation
receiver and source-undefined/nondecimal contexts without inventing legal forms.
Actual 286 binary/accumulator/decimal fixtures are distinguished from Group-1,
TEST and unary matrices that still select 8086/186. Three existing cases pass
once per width; source/test changes remain zero and S5 stays active.

Movement/bounds/FLAGS reconciliation extends fetch/offset/whole-span receivers
and confirms normal 286 base rows. It identifies a concrete POPF producer/oracle
gap: the pre-386 loader ignores real NT/IOPL preservation and protected
CPL/current-IOPL restrictions, while the existing real-image test expects the
incorrect 286 7002 image. This joins the one generation/mode FLAGS owner,
not a global mask change. PUSHF shutdown/failure propagation and XCHG bus
atomicity remain explicit receivers. Four normal movement cases pass once
each width without qualifying their missing protected/failure contexts.

Simple-FLAGS/hardware reconciliation confirms normal CF/DF/NOP and base rows,
but protected 286 CLI/STI skip CPL/IOPL checking in the pre-386 branch.
They join POPF's generation/mode FLAGS privilege receiver. The independently
inspected hardware original confirms NMI inhibition until IRET, reset image
and explicit/automatic LOCK bus intervals; observing a prefix is not those
intervals. Its doubled-clock HOLD formulas are not instruction costs.
Appendix-B assumptions retain exact EA/odd/read-wait terms separately from
the L2 five-to-ten-percent fetch range. Modifier/reference and neutral Core
wait/overlap attribution still need coherent closure; S5 remains active,
Shared read-only, with no product or artifact change.

### S5 Complete Inventory Delivery

All ten finite partitions in the [S5 audit](../etc/evidence/t544-s5-80286-family-audit.md)
now map to source/code/regression dispositions and complete retained receiver
classes. Admission/reset, query/layout, system tables, segment access,
gates/returns, tasks, delivery/arbitration, inherited instructions, strings/
external cycles and timing are not reduced to newly introduced 286 opcodes.
The 1985 original resolves STI's abbreviated IRQ prose but does not resolve
outer IRET or the programmer/hardware IDT reset conflict. Exact numeric/formula
rules, range models, source conflicts and unknown contexts remain distinct.
No residual is moved outside T544 or hidden by a green oracle; source-producer
and oracle corrections require concrete Shared review before implementation.

Fresh complete repository-only units pass once each width: x64 506/506 in
174.97s and x86 506/506 in 145.33s, eight jobs and 300-second deadlines,
both exit zero. The negative-boundary build fixture is the long item;
no repeat run is used to choose a preferred duration. Documentation governance,
changed-document relative links, the sixteen-field packet and actual diff
checks pass. Source/test/ABI/manifests/App configuration/MyNES/artifacts have
zero changes; docs-only work needs no EXE rebuild. No owned process remains.
Executor delivery completes the inventory brief, not CPU qualification;
coordinator actual-change acceptance/governance push still precedes S5 closure.

### S5 Coordinator Acceptance

Executor P1 is 083a8946c, pushed to origin/master. Coordinator actual-change
review inspects the three changed documents and the immutable sixteen-field
packet against the original request and ledger. All ten partitions have the
original-page, handler/helper/timing and regression-context mapping; the
complete residual classes and source contradictions remain inside T544.
Correct normal mechanisms, confirmed defects, disputed legacy oracles and
underqualified contexts are separate. Neither 771 green keys nor full-unit
success is used as CPU qualification. The source/test/artifact diff is zero
and the entire changed surface is NXVM-owned documentation.

The read-only inventory exit is satisfied, including fresh complete units on
both widths, source identity/visual inspection, exact receiver mapping,
documentation/links/packet/allocation and actual diff checks. S5 is accepted
and closed by this governance delivery. T544 stays open for 80386 and full
convergence; no Shared repair is authorized by acceptance, and no residual
form is silently transferred to another T. No duplicate runtime gate or EXE
rebuild is warranted for purely documentary acceptance. Receiving caches and
original/render research scratch remain needed for the next family audit.

## S6 80386DX Audit Batch

S6 starts against a8527a828. Its [working audit](../etc/evidence/t544-s6-80386-family-audit.md)
freezes the complete F01-F14 80386 form/state/time batch, not just new 0F
opcodes. Size/address/stack combinations, real/protected/paged/VM86 contexts,
task/gate/exception/debug and reference/failure timing must be reconciled.
All S2-S5 confirmed defects, source conflicts and regression receivers stay
inside T544. No Shared code edit is admitted, and the 1,413 timing keys do
not independently qualify either 386 product. The original selected source
is Intel 386DX 1990, order 230985-003, with fresh identity/page inspections
recorded in the audit. Current owns the one active packet.

The finite S6 inventory now maps all fifteen partitions to fresh source,
actual handler/helper/selector inspection and precise existing regression
receivers. Confirmed gaps include gate width versus instruction width,
ring-specific stacks/conforming/LDT targets, incoming-task fault context,
VM86 delivery/FLAGS, real/VM and expand-down spans, post-instruction
endpoint checks, RF/TF/NMI/DF handling, host signed arithmetic and
branch/task/conversion/segment-POP/DIV-width clock selection. The record
also withdraws unsupported POP-SP alias and VM far-pointer suspicions
after reading the entire helper and effective-mode predicate. Originals
with internally contradictory operations/tables and 1986/1990 clock
differences remain explicit source receivers, not automatic code changes.

Fresh complete units on the unchanged production baseline pass once each
width: x64 506/506 in 68.14s; x86 506/506 in 83.88s. These are regression
observations, not qualification of the newly identified missing contexts.
No Shared source/test/ABI, manifest, App/INI/media, MyNES or binary changes
are part of S6. T544 remains open for complete cross-family/source/tier
convergence and concrete mechanism repair review; no gap moves to another T.

### S6 Coordinator Acceptance

Executor P1 is 1fc27c7cd, pushed to origin/master. Coordinator actual-change
review reads all 849 audit lines, the ledger change and the sixteen-field
packet, rather than inferring acceptance from unit results. All fifteen
partitions have source, implementation and regression dispositions; inherited
forms remain paired with 386 width/mode/timing evidence. The withdrawn POP
alias and VM pointer suspicions, selected-edition conflicts and retained
mechanism defects are distinguished explicitly. The 1,413 keys are not
claimed as individually proven execution results.

The finite read-only inventory exit is satisfied. Documentation structure,
relative links, packet allocation and actual diff checks pass; complete
units passed once per width. This governance P accepts and closes S6, not
T544, CPU qualification or any Shared repair. No source/test/ABI, manifest,
App/INI/media, MyNES or artifact change is included. Original/render research
scratch and receiving caches remain needed for cross-family convergence.

## S7 Five-Family Convergence

S7 consumes the accepted S1-S6 audits against 8295ff789. The
[convergence report](../etc/evidence/t544-s7-five-family-convergence.md)
maps the complete early, 186, 286 and 386 batches to eighteen coherent
mechanism/proof receivers. Source conflicts, confirmed code mismatches,
implementation-derived oracles and missing contexts stay distinct; all
CPU residuals remain in T544, with no transfer to another queued task.

Actual producer-precedence inspection confirms successful VM86 FS/GS POP
can return source-unallocated one-tick fallback. Its exact source remains
Manual-L3; this is an existing implementation L1 hole requiring allocation,
not a downgrade of the manual. LFS/LGS/LSS instead reaches its existing
nonprotected cost. The report retains default32 DIV/IDIV width, CWD/CDQ,
primary segment POP, branch-outcome, task-matrix and 186 width/source-label
corrections at the one timing owner. Existing green recipes omit or encode
some of these incorrect assumptions and cannot independently settle them.

Fresh visual 1987 system-guide pages corroborate restrictive PDE.U/S and
VM86 IOPL=3 IRET, avoiding changes based on contradictory 1990 local prose.
Fresh 1991 hardware page corroborates NMI-in-service/IRET and one pending
NMI. Other original-edition and undefined-input conflicts keep explicit
source dispositions; none is averaged into an invented exact cost.

Fresh complete units pass once each width, x64 506/506 in 260.54s and
x86 506/506 in 245.60s, eight jobs and 300-second deadlines. Both generated
catalogs retain 4,906 successful recipes and the original reported tier
counts, without claiming untested contexts. The eight deployed PC PE widths
and hashes still match accepted T543 identities. Audit-only source/test/ABI,
manifest, App/INI/media, MyNES and binary changes remain zero.

All 58 original integration contexts pass once: default 22/22 per width,
AT and Model40 3/3 per width, XT 1/1 per width. Exact durations and the
non-repeated invocation accounting are in the report. Source predicates,
external assets and required checkpoints are unchanged. Documentation,
relative links, sixteen-field packet and diff checks pass; no owned process
remains. Complete audit delivery does not authorize Shared implementation,
qualify the identified defects or close T544. Its eighteen coherent repair
boundaries require concrete review before changing Shared source/tests.

### S7 Coordinator Acceptance

Executor P1 is a3b9951c8, pushed to origin/master. Coordinator actual-change
review reads the full 250-line convergence report and its ledger/packet
changes. It checks the five finite family inventories, actual timing
producer precedence, source conflicts and all eighteen coherent receivers,
not merely green aggregate results. VM86 FS/GS POP source-unallocated timing
is distinguished from exact manual timing and from allocated but incorrect
costs; source uncertainty and implementation-derived test oracles remain
explicit. No residual is transferred out of T544.

The read-only convergence exit is satisfied: full units pass 506/506 once
per width, all 58 original integration contexts pass once, documentation
governance, relative links and diff checks pass. Actual source/test/ABI,
manifest, App/INI/media, MyNES and artifact changes are zero. This P accepts
and closes S7's audit delivery, not T544, CPU qualification or Shared repair
authorization. T544 remains open for concrete mechanism/proof repair review;
no other task is admitted. Required research scratch and build caches remain.
