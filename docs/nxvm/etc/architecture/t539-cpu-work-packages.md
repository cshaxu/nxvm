# T539 CPU Migration Work Packages

## Owner Amendment

The owner requested bounded, individually traceable S deliveries on 2026-09-29.
This replaces the original single-batch S18 delivery requirement, not the CPU
architecture or verification requirements. Current is the admission/acceptance
authority. T539 and the downstream queue stay intact.

S21 intake found 18 mixed chip/board tests totalling 11,335 lines at
`7e7a7e156`. Before implementation, that planned row is divided into nine
bounded packages, S21-S29. Previously planned S22-S32 become S30-S40;
no admitted or accepted identifier changes. Historical S18-S20 records retain
their original prospective numbering; this plan and the revised inventory own
the receiving map. Later large groups must receive the same size review before
admission, retaining includer dependency closures.

At S30 intake, its eleven planned sources exceeded 7,000 lines, including a
single 3,400-line mixed ALU suite. Before changing them, the unadmitted S30
row was divided into S30-S35 by independent instruction groups. The formerly
planned S31-S40 become S36-S45. No accepted S18-S29 identifier is changed;
earlier evidence retains its historical planned numbers.

At S36 intake, its twelve FLAGS/string/port sources total about 7,000 lines.
Before implementation, split that unadmitted row into S36-S39 by distinct
instruction families: FLAGS, string transfer, string scan/compare and port I/O.
The formerly planned S37-S45 become S40-S48. Earlier accepted identifiers and
their historical prospective references do not change.

At S40 intake, the thirteen descriptor/system sources total 7,736 lines.
`arpl_s53` directly includes the base ARPL source; the remaining files have
independent source boundaries. Before implementation, split the unadmitted
S40 row into seven instruction-family packages, S40-S46. Formerly planned
S41-S48 become S47-S54. No accepted S18-S39 identifier changes.

## Recover A Deliverable Baseline First

At the split's admission, the worktree contained uncommitted CPU bus, timing,
observation, allocation and test migrations. Some rebuilt subsets passed, but
the complete unit corpus did not build after the opaque-allocation cutover.
That was the recovery starting point, not the current verification status;
Current and the linked S18 evidence record subsequent results. Nothing here
marks that work accepted.

S18 must first reconcile that work into a green incremental baseline. Preserve
every existing change and its case mapping before selectively staging or
deferring it. Prefer delaying the allocation/layout cutover until S38 while
retaining the original single CPU owner, rather than migrating all remaining
tests just to close S18. This is staging of the existing implementation, not a
second CPU, mirror, private-pointer accessor or compatibility implementation.
Already migrated public consumers may remain only where the baseline supports
them without a new bridge. Deferred edits need an explicit recoverable inventory;
do not silently delete them, commit a broken checkpoint or hide them as accepted.
If that green boundary cannot be recovered safely, report the concrete dependency
and revise the packet; splitting S numbers does not waive the full-unit gate.

## Ordered Packages

All packages retain original scenarios, handler/table style and timing grades.
Each row names a reviewable result, not permission to add new CPU functionality.

| S | Scope and required result |
| --- | --- |
| S18 | Reconcile pending work, recover full unit build/run, inventory carried edits and original cases, and freeze the incremental baseline. No CPU-extraction acceptance. |
| S19 | CPU bus boundary: memory/I/O, INTA and external cycles; board owns routing and transactions. Prove BEGIN/COMMIT/CANCEL order and partial effects; remove the unused firmware hook. |
| S20 | Copied CPU observations and board adapters: entry/current registers, decode/fault/retirement, reset/debug/FPU and board fixture consumers. No private CPU access from migrated consumers. |
| S21 | LEA and MOVX: two files, 686 lines. Preserve effective-address/no-read, sign/zero extension, failure atomicity and the LEA board IRQ case. |
| S22 | GPR MOV and MOFFS: two files, 1,407 lines. Preserve register-width, memory direction, segment override, fault and board IRQ cases. |
| S23 | XCHG: one file, 876 lines. Preserve accumulator/general forms, LOCK, partial memory effects and board IRQ cases. |
| S24 | GPR PUSH/POP, immediate PUSH and PUSHA/POPA: three files, 1,784 lines. Preserve stack addressing, discarded ESP slot, atomicity and IRQ cases. |
| S25 | ENTER/LEAVE: one file, 698 lines. Preserve nesting, widths, protected stack faults and IRQ cases. |
| S26 | Legacy and FS/GS segment stack: two files, 978 lines. Preserve selector/cache checks, protected faults and SS interrupt shadow. |
| S27 | LES/LDS and LSS/LFS/LGS: three files, 1,424 lines. Preserve all overlapping original matrices, memory-only forms, atomicity and IRQ shadow. |
| S28 | Segment selector and SREG MOV: two files, 1,987 lines. Preserve descriptor/cache/query matrices and board IRQ semantics. |
| S29 | Operand/address and prefix attributes: two files, 1,495 lines. Preserve prefix precedence, width, LOCK/REP and port/IRQ composition. |
| S30 | Bit/condition/extension group: bit scan, bit test, double shift, IMUL2, SETcc and sign extension; six sources, about 1,500 lines. CPU-private results move to CPU receivers; board faults/IRQ stay with the board. |
| S31 | Immediate IMUL and rotate: two sources, about 1,500 lines. Preserve width, flags, profile and fault matrices. |
| S32 | Legacy ALU and LOCK: two sources, about 1,600 lines. Preserve original legality, profile and timing assertions; no blanket LOCK special case. |
| S33 | First `core_machine_inc_dec_smoke.c` group: INC/DEC, NOT/NEG, accumulator TEST, MUL/IMUL and DIV/IDIV, through the original divide-fault cases. Move only their CPU-owned assertions; keep the still-unmigrated remainder active. |
| S34 | Second `core_machine_inc_dec_smoke.c` group: TEST rm/reg and ADD/ADC/SBB forms, flags, profile and fault cases. Continue the same single production path and keep the remaining logical/compare groups active. |
| S35 | Final `core_machine_inc_dec_smoke.c` group: OR/AND/SUB/XOR/CMP, decimal adjust, XLAT and Group-1 matrix. Remove its last private board-test access only after all original cases have receivers. |
| S36 | FLAGS: direct flags, LAHF/SAHF and PUSHF/POPF with its includer. Retain all profile and reserved-bit matrices; keep real exception/IRQ delivery board-owned. |
| S37 | String transfer: MOVS and LODS, including REP, direction, address/operand width and timing cases. |
| S38 | String scan/compare: STOS, SCAS and CMPS, including REP termination, flags and interruptibility. |
| S39 | Port I/O: port instruction semantics, string I/O and port ownership. Retain board permission, IRQ and actual port transaction assertions. |
| S40 | ARPL base plus S53 includer: two sources, 910 lines. Keep the includer and its original real/protected CPU and board cases in one change. |
| S41 | BOUND S54: one source, 899 lines. Preserve bounds, width/profile, segment and fault/IRQ contexts. |
| S42 | Table-register instructions: DTTR S61, LGDT/LIDT and SGDT/SIDT; three sources, 1,367 lines. Retain guest table loads, store width, segment/VM86 and IRQ/fault contexts. |
| S43 | Accepted: descriptor-system cases moved to their CPU-only receiver; complete descriptor/table/cache/rollback coverage remains. The source's `SMSW/LMSW/CLTS/MOV CR` control-state block stays once in S45. |
| S44 | Accepted: descriptor queries moved from two 1,748-line mixed sources to CPU-local LAR/LSL and VERR/VERW receivers. The retained 80386 board timing runner owns LSL's 21/25/22/26 page-granularity rows; public receivers retain descriptor-table, fault and IRQ behavior. |
| S45 | Accepted: control state moved from CLTS S62, MSW S63 and the retained descriptor-system block to one CPU-local receiver and one public Core/PIC board receiver. Preserve CR0/TS/PE, CR2/CR3, privilege, memory and interrupt cases. |
| S46 | Debug state: MOV DR S59 and TF/#DB S60; two sources, 1,006 lines. Preserve breakpoint register, trap/fault delivery, RF/TF and IRQ order. |
| S47 | 16-bit protected-mode dependency group: gate base fixture and all six direct includers (external, call gate, outer return, outer IRET and two timing runners). Keep timing consumers buildable while the shared fixture changes. |
| S48 | Call-gate privilege entry: retire `core_machine_call_gate_privilege_entry_smoke.c` (776 lines) into one public board receiver. Preserve outer gate entry, parameter transfer, descriptor rejection and fault-delivery boundaries. The smaller `core_machine_call_gate_smoke.c` remains with its 80286 timing-runner includer in S57. |
| S49 | Control-transfer source: `core_machine_control_transfer_smoke.c` (1,181 lines). At intake, split its independent near/far, protected and fault/IRQ families before changing code if one bounded receiver cannot retain them. |
| S50 | IDT/privilege entry pair: `core_machine_idt_privilege_entry_smoke.c` and `core_machine_protected_privilege_smoke.c` (665 lines). Preserve DPL/CPL, gate and frame publication rules. |
| S51 | Protected far/data pair: `core_machine_protected_far_s1_smoke.c` and `core_machine_protected_data_access_s2_smoke.c` (934 lines). Preserve selector/cache validation, data accesses and atomic fault boundaries. |
| S52 | Outer return pair: `core_machine_iret_outer_s52_smoke.c` and `core_machine_protected_return_atomicity_smoke.c` (1,003 lines, including its includer closure). Preserve return frames and all-or-nothing failure behavior. |
| S53 | Task-switch 16-bit/task-gate half of `core_machine_task_switch_smoke.c` (through the 16-bit and task-gate families). Preserve task-stack, nested-task and task-gate rules without moving the later cross-width rows. |
| S54 | Task-switch 32-bit/cross-width half of `core_machine_task_switch_smoke.c`. Preserve 16↔32 TSS transitions, direct 32-bit TSS forms and their fault/IRQ ordering. |
| S55 | TSS I/O-map port source: `core_machine_tss_iomap_port_smoke.c` (291 lines). Preserve CPL/I/O-map authorization separately from task-switch state construction. |
| S56 | Interrupt/VM86 dependency group: CLI/STI with HLT/INT/IRET/composition includers; protected IRET; interrupt-entry; VM86 delivery/IRET/table-load and hardware delivery. Migrate shared fixtures with all includers in one package. Preserve NMI/IRQ priority, masking, fault escalation and frames. |
| S57 | Timing corpus: 8086/80186/80286/80386 ledgers and manifest runners, protected I/O, `core_machine_call_gate_smoke.c` as the 80286 runner includer, and T359/normalization suites. Preserve all formula/catalog rows and measured deltas; remove generated production catalog dependency. |
| S58 | Remaining-consumer sweep: profile gating, CPU/FPU escape, paging/fault diagnostics and any uncategorized raw CPU consumer. Reconcile the original-case inventory to zero unassigned cases; delete the legacy mixed fixture only after its last caller moves. |
| S59 | Opaque CPU lifetime cutover: create/destroy, prepared entry, reset rollback and board ownership. Remove embedded CPU/decoder layout without exposing a mutable private pointer or maintaining a mirror. Full corpus must still build. |
| S60 | Physical Shared relocation: all nine CPU files, CPU-owned fixtures/tests and generated inputs move to x86; update sole build target, manifests and boundary checks. Delete old App implementations and prove independent tools-off build/test. |
| S61 | Whole CPU receiving audit: cross-width units, required integration, one vendor boot per profile/width, eight 0539 EXEs, actual-diff and complete ledger review. Accept the CPU row only here; separately assess remaining T539 exit criteria. |

S19-S58 migrate consumers before S59; S60 follows S59. Existing in-progress edits
are inputs to these packages, not a reason to rerun or rewrite a verified
algorithm. At admission, each brief lists exact files, original cases, includers
and target-specific P boundaries. If a row proves oversized, subdivide it before
implementation and record the revised numbering; never silently broaden it.

## Carryover And Verification

The [incremental inventory](../evidence/t539-cpu-incremental-inventory.md) names
the deferred edits, recovery artifact, 100 remaining direct private-test
consumers and additional include-dependent consumers by receiving S.

The original [S18 evidence](../evidence/t539-s18-cpu-extraction.md) remains the
chronological record, with narrow test results explicitly distinct from S exit.
Bus edits map to S19; copied observations and board tests to S20; EFLAGS to
S36, REP to S37-S39; interrupt-entry/private rollback to S56; formula and
target preparation to S57/S60; opaque allocation to S59. Other edits are assigned by the S18 inventory,
not dropped. Do not replay an already accepted change in a later P.

Every S closes only after its complete applicable unit suites, fresh affected
builds, boundary/manifests/document checks and actual-diff review pass. Rebuild
affected x64/x86 deliverables under execution rules; no stale binary acceptance.
Each P has one target (Shared or NXVM); MyNES/INI/assets inputs stay unchanged.
T closure still requires full integration. S61 consolidates receiving proof; it
does not defer earlier S unit gates. User-facing progress must state planned,
active, verified or accepted, with remaining cases and evidence links.
