# T544 S7 Five-Family Audit Convergence

## Subsequent Owner Disposition

After S7 acceptance, the owner explicitly requested T544 audit closure and
CPU repairs first in Queue. All eighteen receivers below and their linked
family findings transfer intact to the
[CPU repair proposal](../../proposals/m5-cpu-audit-gap-repair.md).
The original audit-time no-transfer/open-task statements remain historical,
not current status. Closure does not repair, qualify or erase any finding.

## Scope And Meaning

Read-only convergence against 8295ff789, carrying the complete accepted
[boundary](t544-s2-cross-family-boundary-audit.md),
[8086/8088](t544-s3-8086-8088-family-audit.md),
[80186](t544-s4-80186-family-audit.md),
[80286](t544-s5-80286-family-audit.md) and
[80386DX](t544-s6-80386-family-audit.md) audits. Those records contain the
original identities/pages, actual helpers/selectors and existing regression
limitations. This document is their common disposition and repair index,
not a replacement for their individual source-to-code comparisons.

Implemented families are 8086, 8088, 80186, 80286 and 80386. DEFAULT resolves
to 386 and is not a sixth implementation. Neither 80188 nor a PC110 486
implementation exists. Their separately named prerequisite is retained, not
silently counted as audited implemented silicon. x87 arithmetic is likewise
not a CPU instruction implementation; CPU ESC/WAIT/interface requirements
are within this audit.

The audit unit is a form plus its size, effective mode, privilege, reference,
success/fault and time-publication context. The 4,906 timing catalog keys and
opcode/ModR/M candidate counts are not an independent enumeration of every
such context. This audit identifies mismatches and unqualified classes; it
does not turn all catalog successes into accepted instruction correctness.

No Shared implementation, test, ABI, manifest, product configuration or
binary is changed. Every CPU finding stays in T544. Read-only batch acceptance
does not satisfy the task ledger's stronger qualification/repair predicate.

## Five-Family Coverage Reconciliation

| Family | Complete audit partition | Source/function/time disposition |
| --- | --- | --- |
| 8086 and 8088 | S3 eleven partitions, F01-F10/F14 including inherited decode, ALU, movement, stack, branch, divide/count, strings, FLAGS, ports and external bus | Both retained profiles have matching ordinary contexts and explicit residual source, decoder, arithmetic, width-transfer, restart and bus classes. 8088 is not qualified by 8086's word-transfer assumptions. |
| 80186 | S4 thirteen partitions: additions plus every inherited family class | PUSHA/POPA, immediate PUSH/IMUL/shift, BOUND, ENTER/LEAVE and INS/OUTS inspected alongside inherited forms. Integrated-control/escape-trap input is missing, not emulated by generic EM/TS. Width-annotated exact rows are distinguished from genuine latency ranges. |
| 80286 | S5 ten partitions spanning ordinary forms, system/query, descriptors, protection, gates, tasks, delivery and time | Protected privilege, frame/selector and fault contexts are not implied by ordinary real-mode rows. Paging/VM86/32-bit forms are non-applicable; leaked later descriptor types are a generation-boundary defect, not supported later capability. |
| 80386DX | S6 fifteen partitions covering inherited/new forms, D/B/66/67, paging, VM86, tasks, debug and external cycles | Ordinary width/address/defined-state mechanisms are identified; task/gate/exception and effective-mode timing defects remain. Default32 is not equivalent to prefix66. A successful test case cannot settle an original's contradictory row. |
| Cross-family state/retirement | S2 boundary inventory plus each family's actual callers | Sole reset/FLAGS, arbiter, exception delivery, decode/fetch, timing selection and Core publication owners exist, but their generation/context contracts are incomplete. Repairs must preserve these owners rather than create per-App exceptions. |

The four family deliveries cover the inherited form partition as well as new
opcodes. Normal mechanisms, confirmed defects, unsupported source claims,
undefined results and absent regression contexts have different dispositions.
No omitted-context count is manufactured from the number of green recipes.

## Current Timing Coverage And Tier Findings

The full selector at `cpu_timing.c` is evaluated in its real precedence,
not by searching for a constant in an unused fallback. 8088, 186 and 286
have their respective chains; 386 tries strings, dynamic multiply/arithmetic,
secondary, privileged, primary, control/stack, family fallback and compatibility.
Each successful evaluator terminates selection, even if it returns an
explicit source-unallocated result. Core's successful-retirement publisher
distinguishes source eligibility from an observation and from elapsed time.

### Confirmed VM86 FS/GS POP Allocation Hole

For a successfully executed `0F A1`/`0F A9` in VM86:

1. It is not a string or dynamic arithmetic/multiply operation.
2. The secondary evaluator cannot allocate it: its ordinary secondary
   switch is for the bit/count/extension forms, not segment POP.
3. The privileged evaluator recognizes it but explicitly returns false
   when incoming VM=1, before its seven-real/twenty-one-protected allocation.
4. The primary/control-stack owners do not allocate these secondary opcodes.
5. The 386 fallback's default marks source-unallocated and returns success
   with `CORE_MACHINE_SOURCE_UNALLOCATED_TICKS`, defined as one. Its success
   prevents the compatibility endpoint from being tried.

This is a reachable order-only timing fallback for legal successful forms,
not merely a missing static key. Source timing still has the Manual-L3
seven-clock real/VM-style contract; the implementation is unallocated.
It must be reported as an existing L1 execution hole requiring upgrade,
not as permission to demote the source or silently bless one tick. Restore
allocation at the existing segment timing owner and extend both FS/GS,
VM/real/protected, operand sizes and pop-failure predicates after Shared review.
LFS/LGS/LSS is not the same hole: effective protected mode excludes VM,
so those forms use the existing seven-clock nonprotected branch.

### Wrong Allocated Numbers Are Not Automatically L1

- 386 CWD/CDQ reaches the primary conversion shape's three clocks; CBW/CWDE
  legitimately uses three, CWD/CDQ requires two. Preserve one selector but
  distinguish the paired opcode contracts.
- 386 DIV/IDIV chooses word/dword cost from prefix66 alone. Default32 without
  66 therefore gets word clocks and default32 with 66 gets dword clocks.
  Dynamic multiply and secondary bit/count selectors already combine CS.D
  with 66; the sweep does not label those correct expressions as defects.
- Protected primary DS/ES/SS POP reaches the control-stack owner's twenty
  clocks for 286 and 386. 386 FS/GS reaches twenty-one. Source-specific
  generation/form rows must be reconciled, not changed by replacing all POP
  constants or inserting a parallel timing route.
- 286 LEAVE's producer and current oracle use eight versus exact manual five.
  Matching the old test is not independent L3 evidence.
- Branch taken identity is inferred from resulting PC versus fallthrough.
  A taken zero-displacement branch is therefore mistaken for not taken;
  some prefixed 386 fallthrough calculations additionally omit prefixes.
  A real execution outcome must feed the sole timing producer.
- Task CALL/JMP/INT/IRET often select one constant from old/new TR or final
  CS, instead of the complete old/new TSS-format and VM matrix. The source
  m/n terms are not interchangeable with task matrices.

These have exact/formula sources but incorrect allocation/context selection.
No downgrade is needed to fix an implementation that uses the wrong L3 row.
Preview failure and legal LOCK combinations can separately reach explicit
unallocated paths; their complete reference/source contexts remain in the
fetch/LOCK receivers, not an invented claim that every LOCK instruction is L1.

### Source And Recipe Labels

Exact values and formulas remain L3 under their stated prefetch, reference,
alignment, no-wait or mode conditions. True ranges, rounded midpoint choices,
the early operand-sensitive multiply/divide model and macro models are L2.
186 accumulator-immediate and immediate MOV byte/word annotations must not
be read as latency ranges merely because the table prints two numbers.
The current midpoint recipe labels and selected byte values need correction
after source reconciliation. Conversely 186 IDIV and immediate IMUL really
have ranges; their existing L2 classification is not a defect.

Undefined FLAGS/destination/encoding behavior is not assigned a invented
exact value to make an oracle pass. A source conflict is not resolved by
averaging two exact values. An allocated source label is not proof that its
mode/default, transfer count, next-form or failure assumptions were satisfied.

## Original-Source Conflicts And Resolutions

The family records retain every original conflict and its exact page context.
The following dispositions prevent inconsistent changes during repair.

| Class | Converged disposition |
| --- | --- |
| Early signed-IDIV minimum | Different compatibility prose and signed-interval presentations remain source reconciliation; do not assume a single five-family endpoint rule. |
| Early POP CS/undefined opcodes | The combined 1981 POP table says CS illegal; current compatibility opcode and old List 1 need independently identified reference evidence. Minimum-family rejection alone does not prove early vector-six behavior. |
| Early ALU/logical/INC/TEST aliases and decimal encodings | Encoding/edition differences and undefined/nondecimal domains remain explicit; no blanket removal or exact extension claim is admitted. |
| 186 MOV direction/MOFFS and non-repeat MOVS | Dedicated and generic tables disagree. Keep opcode/direction and repeated/non-repeated cases separate; REP agreement does not resolve the ordinary MOVS discrepancy. |
| 186 LOOP/LOOPNE and vertical table alignment | Unambiguous generic rows and displaced dedicated entries conflict with existing selected scalars. OCR proximity is not authority; no midpoint substitute. |
| 186 width annotations | Individually inspected arithmetic/logical/TEST/MOV 8/16-bit annotations are width contracts, not merely intervals. Genuine IDIV/MUL/IMUL/BOUND ranges remain distinct. |
| 286 IDT reset | Both programmer editions say 03FF versus hardware FFFF. Retain current programmer-selected 03FF; do not claim a proven reset defect for this field. CS/MSW discrepancies are independently confirmed. |
| 286 reset durations | Programmer processing clocks versus hardware CLK/first-memory-cycle interval have different units/endpoints; no scalar substitution or whole physical-axis proof. |
| 286 expand-down prose | Detailed strict-greater formula and explicit empty FFFF case settle the abbreviated contradictory prose. Do not change the correct non-big empty-range rule. |
| 286 STI | 1985's next-instruction examples corroborate the IRQ shadow omitted in abbreviated 1987 prose. NMI effect remains its separate source grid. |
| 286/386 outer IRET predicates | Contradictory conforming/nonconforming prose across chapters/editions remains unresolved; independent LDT/ring/frame/cleanup defects do not wait for or settle that contradiction. |
| 286 task-JMP nesting | Task-transition chapter distinguishes non-nesting JMP; contradictory instruction continuation is recorded, not copied into NT/busy implementation. |
| 386 mixed PDE/PTE U/S | 1990 table conflicts with 1986 restrictive combination. Fresh 1987 system-guide PDF 48/2-21 explicitly makes every page supervisor when PDE.U/S=0, independently corroborating the restrictive direction used by current code. Do not invert the implementation from the isolated 1990 table. |
| 386 VM86 IRET | Fresh 1987 system-guide PDF 146/9-11 explicitly includes IRET among IF-related instructions allowed at IOPL=3 and trapped below three, corroborating chapter 15. The dictionary's unconditional VM fault is not used to prohibit this documented path. |
| 386 LAR system types | Both inspected programmer editions list the disputed gate types; no selected-edition disagreement resolves hardware/model differences. Current query admission versus detailed original operation remains explicit. |
| 386 TR6 X polarity/00-11 cases | 1986 resolves literal 01/10 polarity; 1986 undefined 00/11 versus 1990 all/none models remain revision-specific. Raw register storage is not functional test-TLB implementation. |
| 386 REP/MOVS/INS/OUTS clocks | 1986 and 1990 have different exact setup/iteration/base values. Preserve explicit selected edition and workload form; do not let one mixed source table claim a single universal L3. |
| 386 task/transfer matrices | Old/new 16/32 TSS, VM and transfer cause are explicit cells. Ordinary branch m terms do not fill omitted task cells. JMP-real-memory exact forty-three source conflicts with a lower current constant, not a range. |
| 386 SETG/SETNA, SGDT size prose, BOUND description | Paired aliases or compatibility/operation text expose internal table/prose errors; use the internally corroborated operation, not known contradictory text. |
| 386 CLTS/WAIT/POPF-RF prose | Privilege/MP-TS and FLAGS chapter versus abbreviated dictionaries remain explicitly reconciled or unresolved in S6; no later-CPU rule is imported silently. |

New originals used only for the named cross-checks:

- Intel 80386 System Software Writer's Guide (1987), SHA-256
  E16C70EC593AE3FBA0C630EDC798A98F65B570E7ADFB0273895E6734791C6FD8.
  PDF 48 and 146 were rendered and visually read, not accepted from OCR alone.
- Intel386 DX Microprocessor Hardware Reference Manual (1991), SHA-256
  5BC8373E6813F2C113DAE416705B87B72C0C0AD13F7B0221CA983F28EB4DE2E1.
  PDF 66/3-28 was rendered/read. It independently confirms architectural
  NMI-in-service blocking, one pending NMI and return at IRET. Its statement
  that every interrupt automatically disables INTR is too broad for trap
  gates; the software gate-type contract remains authoritative there.

The system guide also has abbreviated or erroneous prose (for example
expand-down lower-bound wording). Being another original does not make all
its sentences override the detailed programmer operation. Only the named
corroboration is claimed, not a new full-book qualification.

## Complete Mechanism Repair Index

These are coherent repair/proof batches, not newly allocated S tasks or
Shared approval. Exact source/caller lists and regression names are in the
linked family records. No item is transferred out of T544.

| Receiver | Complete defect/proof boundary and minimal repair design |
| --- | --- |
| Reset and architectural images | One chip reset plus distinct writable/readout/saved FLAGS rules; early FFFF:0000, 286/386 cached limit, 286 MSW and generation-specific outgoing images. Preserve physical first fetch and undefined reset fields. |
| Runtime decode/admission | One fetch/decode failure convention and complete byte limit (286 ten/UD, 386 fifteen/GP); sweep thirteen unchecked early sites, immediate/group/memory-only privilege admission and preview versus execution. Preserve table handlers. |
| Effective address and segment span | Mask offsets by actual address size; XLAT wrap, real/VM limit, unreal cache, expand-down empty range and all aggregate pointer/table/frame widths belong to the existing address/access owner. No caller-local protection stack. |
| Admission versus next fetch | Remove unjustified post-instruction ESP/next-fetch validation at the correct seam; retain legitimate branch-target/frame checks and attribute the subsequent instruction's fault separately. |
| Host arithmetic/count | Widen before signed left shift/multiply; unsigned literal before bit shift; original guest count versus carry-ring result count; undefined shift/result exclusions and SAR compiler semantics. Cover all affected widths/forms, not just one overflow. |
| Stack/frame publication | PUSHA full frame and special stack failures, ENTER operand versus SS.B, LEAVE, ordinary POP alias, discarded slot and fault ordering; no extra guard on restored RET ESP that the source defers until access. |
| FLAGS privilege and return | Generation/mode-specific POPF/CLI/STI/IRET, RF fault image/preservation, TF prior-state semantics and reserved bits at their actual load/image producers. Task flags are not POPF flags. |
| Asynchronous arbitration | One profile-qualified NMI/INTR/debug arbiter, short shadows distinct from external mask/NMI-in-service, actual MOV/POP SS versus LSS effects, expiry/reset/IRET and simultaneous requests. |
| Exception delivery/shutdown | Source-qualified frame return address, complete ordered exception pairs including serial handling, DF zero error/shutdown and late task context. CPU shutdown differs from product stop or explicit board reset policy. |
| Descriptor/query/table | Generation-qualified types/layout, P-independent query, null/invalid LDT, privilege priority and busy/accessed publication; keep LAR/LSL/VERR/VERW tests independent of implementation-derived rejection. |
| Gate and outer-return | Descriptor width, operand width, old/new SS.B, TSS format and target CPL are independent; legal LDT/conforming/ring1/2 paths and DS/ES/FS/GS cleanup in one control owner. |
| Task transition | Admission versus incoming context, CR3/LDT/selector/CPL, dynamic outgoing save versus static fields, busy/backlink/NT, VM/debug and exception commit point; one staged transition and finalizer, not per-INT/CALL patches. |
| Paging and implicit references | Segment-before-page, user/supervisor combinations, full-page span, CR2/error/A/D and task/descriptor/frame accesses. Observation-only preview cannot alter architectural translation/faults. |
| String and port restart | Checked current-element status, completed prior elements, whole prefix/repeat restart and port/memory effects; keep one REP path and distinguish irreversible I/O from CPU rollback. |
| CPU external/NPX/bus | Interruptible WAIT and TEST/BUSY, CPU-mandated ESC references/error rules, integrated 186 escape input, automatic/explicit LOCK interval feeding sole Core arbitration. No host mutex or board BIOS shortcut. |
| Scalar/formula/transfer timing | One successful timing owner: effective widths, 8088 byte/word transfers, odd/reference counts, 186 widths/directions, 286 LEAVE, 386 conversion/segment POP/FS-GS VM allocation, branch decisions and complete task matrices. |
| Retirement/external waits | Preserve completed instruction decode/outcome through asynchronous entry; add source-qualified external waits/overlap exactly once, separate instruction ticks, delivery and compatibility progress. |
| Regression/oracle source | Existing owner-local tests keep useful ordinary proofs; correct false expectations and add absent mode/width/failure contexts only after Shared review. Passing copied values is not original-source proof. |

Ordered external side effects require an explicit bus/provider contract.
Restoring a CPU structure does not undo MMIO, accepted port reads/writes or
the first half of an accepted split write. A universal undo log is neither
minimal nor generally correct. Conversely checked-write ordering alone is
not proof of absent register rollback: ExecFinal already restores saved CPU
state on ordinary failures. Whole-mechanism repairs retain both facts.

## Verification And Qualification Boundary

Fresh full repository-only unit aggregates pass once per width: x64
506/506 in 260.54s, x86 506/506 in 245.60s. Both used eight jobs and a
300-second deadline; the two suites ran concurrently. These are observed
durations, not a new performance claim or justification for rerunning them.

Both regenerated result sets still contain 4,906 successful catalog recipes:
8086/8088 each 989 L3 plus 64 L2:G3; 186 has 580 L3 plus 36 L2:midpoint;
286 has 771 L3; 386 has 1,411 L3 plus two L2. No sampled recipe reports
source-unallocated. This does not contradict the VM FS/GS allocation hole:
that successful effective-mode context is absent from those recipes. Nor
does it override the false width/source assumptions identified above.

All 58 unchanged original integration contexts pass, each aggregate once,
including the actual fixed product binding and external INI/media:

| Product binding | x64 | x86 |
| --- | --- | --- |
| NXVM/default | 22/22, 20.92s | 22/22, 21.14s |
| My5170/AT | 3/3, 41.35s | 3/3, 46.56s |
| MyDeskPro386/Model40 | 3/3, 61.61s | 3/3, 73.72s |
| My5160/XT | 1/1, 21.15s | 1/1, 27.63s |

An initial wrapper invoking multiple aggregates in one PowerShell process
returned after its first default-x64 invocation. The remaining seven groups
were launched separately; default-x64 was not repeated. No predicate, retry
count, checkpoint or timeout was relaxed to obtain these results.

The eight deployed PC 0543 SHA-256 values exactly match the accepted
[T543 artifact table](../../history/M5-T543-four-pc-apps.md); each x64 PE
machine is 8664h and each x86 is 014Ch. The MyNES 0043 pair is untouched.
Existing product artifacts have unchanged executable inputs, so no new EXE
or version is manufactured for audit-only changes. Audit completion means
every finite family partition and retained class is explicitly dispositioned,
with the actual owner and required repair/proof. It does not mean the CPU is
correct, all tests are sufficient, every clock is qualified or T544 can close.

Documentation governance, all changed-document relative links, the complete
sixteen-field S7 packet and `git diff --check` pass. Actual executor review
reads the complete convergence record and both state/ledger diffs, checks
every S2-S6 retained class against its receiving mechanism and confirms the
entire changed surface is three NXVM documentation files. No owned test
process remains; receiving caches and source/render scratch are retained
for the same task's reviewed repairs. Coordinator acceptance follows the
immutable complete delivery, rather than being asserted by this self-review.
