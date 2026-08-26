# M5 Global Function And L3 Interface Completeness Audit

## Purpose

Perform one bounded, evidence-led global audit after AT fixed-disk
standardization and IBM VGA capability closure.  The audit determines, without
claiming completion by inspection, whether the selected 8086, 80186, 80286 and
80386 instruction functions, controller functions, Core timing/deadline
mechanisms, profile inputs and VM pacing consumers are complete, correctly
classified, and connected through one-way L3-capable interfaces.

## Required Scope

The audit reconciles the current source, accepted instruction ledgers,
controller ledgers, profile declarations and tests.  Every audited row must
name its primary source, implementation owner, producer-to-consumer data path,
test/probe evidence and one precise disposition:

| Level | Required meaning |
| --- | --- |
| L3 | Primary manual/model fact, or a sole-owner external L3 input, determines the implemented function or timing. A stated number or formula is not downgraded merely because it is arithmetic. |
| L2 | A clearly labelled proportional/estimated internal timing or bounded fallback remains after the causal function exists. |
| L1 | Only causal/logic ordering exists; no usable ratio or quantitative timing contract exists. |
| L0 | Required functional logic or integration is absent. |

The audit includes instruction execution and exception/interrupt paths; CPU
retirement and transaction timing; PIC, DMA, PIT, RTC, KBC, 8272A FDC, VADP
CGA/EGA/VGA and fixed-disk controller behavior; deadline composition; board
timing/personality data; Core time progression; and Standard/Turbo's VM-only
consumer policy.  It distinguishes an unselected but real Core capability from
a profile claim, and an L3-capable input contract from an unproven input value.

## Dependencies And Completion

Follows the AT fixed-disk-controller standardization and VADP IBM VGA L3
capability candidates.  It consumes their final ledgers rather than reopening
their finite source research.  It precedes the remaining profile/YAML, XT and
Windows candidates because they must consume an honest capability catalogue.

Completion is an audit result, not a declaration that NXVM is globally L3.
It requires a complete finite census, row-to-owner/path reconciliation,
cross-source conflict disposition, interface-direction review, test-coverage
map, and an ordered, non-duplicated transfer list for every L0/L1/L2 or
source-blocked L3 row.  The task closes only when no unrecorded gap, duplicate
state owner, hidden host-to-Core time injection, or false profile capability
claim remains in the defined universe.

## Planned Subtasks

1. **S1 -- freeze the complete audit universe.** Reconcile prior CPU and device
   ledgers into one de-duplicated row index, with exact source provenance,
   selected profiles and explicit exclusions.  Verify no instruction form or
   controller row is silently omitted by a renamed ledger.
2. **S2 -- instruction-function and CPU-timing audit.** Trace 8086/80186/80286/
   80386 decode, successful retirement, faults, interrupt delivery, bus and
   memory/I/O transaction timing from documentation row to owner and test.
   Classify discrepancies and eliminate only audit-local duplicate records.
3. **S3 -- controller-function and phase-timing audit.** Reconcile every
   selected controller row, including the newly standardized HDC and VGA, to
   ports/memory, IRQ/DRQ, Core deadlines, board/personality inputs and
   snapshot/media receivers.  Verify that a manual number/formula is L3 and
   that only internal estimates remain L2.
4. **S4 -- Core/VM interface and pacing audit.** Prove Core remains the sole
   guest-time owner; inspect deadline/observation publication, immutable
   profile-to-Core input copying, Standard host pacing and Turbo no-wait policy.
   Record any reverse dependency, mutable exposure, mirrored state or second
   production path as a concrete repair receiver.
5. **S5 -- independent reconciliation and transfer.** Cross-check sources and
   implementation evidence, run the relevant structural and focused gates, and
   publish one compact audit evidence record plus a dependency-ordered queue or
   TODO transfer.  Do not implement unrelated repairs under this audit task.

## Architecture And Minimalism

This task adds no runtime framework, compatibility path, wrapper, state or
public ABI.  It reduces complexity by consolidating duplicate ledger entries
and by identifying the unique owner and direct data flow of every covered fact.
Any later repair must replace its duplicate or obsolete path at that owner; the
audit may not authorize a new adapter merely to make a checklist pass.

## Non-goals And Stop Conditions

The audit does not claim L4 physical reconstruction, import external source or
firmware, turn an emulator implementation into a normative source, or use
Windows booting as a substitute for row-level evidence.  It does not implement
the transfers it discovers.

Stop and record an explicit source-blocked or profile-blocked disposition when
the required manual, board identity, corpus or legal authority is unavailable.
Stop and report to the owner if the finite universe cannot be bounded without
an architectural scope decision; do not silently treat an unknown class as
complete.
