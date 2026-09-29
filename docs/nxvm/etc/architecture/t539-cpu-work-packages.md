# T539 CPU Migration Work Packages

## Owner Amendment

The owner requested bounded, individually traceable S deliveries on 2026-09-29.
This replaces the original single-batch S18 delivery requirement, not the CPU
architecture or verification requirements. S18 remains active and unaccepted;
S19-S32 are reserved planned packages, not concurrently admitted work. Current
is the admission/acceptance authority. T539 and the downstream queue stay intact.

## Recover A Deliverable Baseline First

At the split's admission, the worktree contained uncommitted CPU bus, timing,
observation, allocation and test migrations. Some rebuilt subsets passed, but
the complete unit corpus did not build after the opaque-allocation cutover.
That was the recovery starting point, not the current verification status;
Current and the linked S18 evidence record subsequent results. Nothing here
marks that work accepted.

S18 must first reconcile that work into a green incremental baseline. Preserve
every existing change and its case mapping before selectively staging or
deferring it. Prefer delaying the allocation/layout cutover until S30 while
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
| S21 | Data movement/addressing/stack tests: MOV/MOVX/MOFFS, XCHG/LEA, operand/address prefixes, PUSH/POP/PUSHA, ENTER/LEAVE, segment moves and LES/LDS/LSS/LFS/LGS. Separate chip cases from board composition. |
| S22 | Arithmetic/bit tests: legacy ALU/LOCK, INC/DEC, IMUL, rotate, bit scan/test, double shift, SETcc and sign extension. Keep CPU-private invariants in CPU-owned tests. |
| S23 | FLAGS and string/I/O tests: direct flags, LAHF/SAHF, PUSHF/POPF including their includer, CMPS/LODS/MOVS/STOS/SCAS/REP and port strings/ownership. Retain profile matrices and board IRQ/port assertions. |
| S24 | Descriptor/system tests: ARPL and its includer, BOUND, table-register instructions, LAR/LSL, VERR/VERW, CLTS/MSW and debug-register/TF cases. Use real guest setup for board tests; retain artificial cache cases at CPU owner. |
| S25 | 16-bit protected-mode dependency group: gate base fixture and all six direct includers (external, call gate, outer return, outer IRET and two timing runners). Keep timing consumers buildable while the shared fixture changes. |
| S26 | Protected transfer/return group: call gates, privilege entry, outer-return atomicity with its outer-IRET includer, task switch/TSS and protected data access. Preserve complete cache rollback and stack/descriptor side effects. |
| S27 | Interrupt/VM86 dependency group: CLI/STI with HLT/INT/IRET/composition includers; protected IRET; interrupt-entry; VM86 delivery/IRET/table-load and hardware delivery. Migrate shared fixtures with all includers in one package. Preserve NMI/IRQ priority, masking, fault escalation and frames. |
| S28 | Timing corpus: 8086/80186/80286/80386 ledgers and manifest runners, protected I/O and T359/normalization suites. Preserve all formula/catalog rows and measured deltas; remove generated production catalog dependency. |
| S29 | Remaining-consumer sweep: profile gating, CPU/FPU escape, paging/fault diagnostics and any uncategorized raw CPU consumer. Reconcile the original-case inventory to zero unassigned cases; delete the legacy mixed fixture only after its last caller moves. |
| S30 | Opaque CPU lifetime cutover: create/destroy, prepared entry, reset rollback and board ownership. Remove embedded CPU/decoder layout without exposing a mutable private pointer or maintaining a mirror. Full corpus must still build. |
| S31 | Physical Shared relocation: all nine CPU files, CPU-owned fixtures/tests and generated inputs move to x86; update sole build target, manifests and boundary checks. Delete old App implementations and prove independent tools-off build/test. |
| S32 | Whole CPU receiving audit: cross-width units, required integration, one vendor boot per profile/width, eight 0539 EXEs, actual-diff and complete ledger review. Accept the CPU row only here; separately assess remaining T539 exit criteria. |

S19-S29 migrate consumers before S30; S31 follows S30. Existing in-progress edits
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
Bus edits map to S19; copied observations and board tests to S20; EFLAGS/REP to
S23; interrupt-entry/private rollback to S27; formula and target preparation to
S28/S31; opaque allocation to S30. Other edits are assigned by the S18 inventory,
not dropped. Do not replay an already accepted change in a later P.

Every S closes only after its complete applicable unit suites, fresh affected
builds, boundary/manifests/document checks and actual-diff review pass. Rebuild
affected x64/x86 deliverables under execution rules; no stale binary acceptance.
Each P has one target (Shared or NXVM); MyNES/INI/assets inputs stay unchanged.
T closure still requires full integration. S32 consolidates receiving proof; it
does not defer earlier S unit gates. User-facing progress must state planned,
active, verified or accepted, with remaining cases and evidence links.
