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
