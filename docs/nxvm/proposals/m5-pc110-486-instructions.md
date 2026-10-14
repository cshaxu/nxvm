# Selected 486SX Instruction Semantics

## Admission And Dependencies

Unnumbered M5 candidate in the [NXVM queue](../states/QUEUE.md), created by the
owner-requested PC110 task split. Allocate a numeric T only at admission.
Depends on: T558 hardware/CPU feature freeze.
Use the [PC110 programme contract](m5-pc110-evidence-and-implementation.md)
and its convergence ledger; the task packet freezes the exact batch and source
baseline before implementation. Research references are not implementation imports.

## Objective And Scope

Consume [S2 CPU contracts CPU-1--7](../etc/evidence/t558-s2-receiver-checklists.md#inspected-cpu-delta-and-timing-contracts).
PRM section 21.3 includes test registers and descriptor-access behavior as well
as six new instruction families; those cannot be omitted from the form ledger.
Audit existing profile comparisons and negative legality paths before exposing
the new profile. Follow the
[single-owner handoff](../etc/evidence/t558-s2-receiver-checklists.md#one-implementation-owner-per-cross-task-contract):
CPU-4--6 effects belong to the system receiver; CPU-2 decode/privilege and
CPU-7 CPUID/ID belong here, while actual cache/TLB effects and SMM/RSM state
belong there. Record those outstanding effects rather than implementing
success-only stand-ins. The profile remains unavailable for board admission.

Implement the selected 486SX instruction and exception delta over the retained 386 core. Freeze opcode forms, operand/address widths, flags, privilege and fault precedence; CPUID eligibility follows the selected stepping, not a generic 486 label.

Expected owners: core/chips/cpu and test/core/chips/cpu; existing Core interfaces/callers only where necessary.

## Non-goals And Stop Conditions

No PC110 board branches, new interpreter, host BIOS hooks or whole-486 timing claim. Keep the profile non-runnable for production until the system-contract task qualifies it.
Stop and report source/model conflicts, missing lawful assets needed by this
batch, or a required Shared-six modification; do not invent an undocumented
fallback. Continue independent in-scope work where possible.

## Suggested S Boundaries

Instruction/feature inventory; cohesive instruction batches; negative/exception and retained-family regression audit.

These are planning boundaries, not allocated S identifiers. Split an oversized
batch before admission; never use letter-suffixed S numbers or an endless
first-failing-BIOS-instruction repair sequence.

## Verification And Exit

All frozen forms have direct unit proof and truthful feature gating; former CPU families retain behavior. Remaining reset/system/timing obligations are named in the next CPU proposal, not declared complete.

Use direct owner tests plus the real production connection, full repository
unit gates and required external integrations under Execution. Reuse the two
existing Ninja/ccache graphs, rebuilding each shared target once per host
architecture/configuration. Rebuild and deploy every affected admitted App pair;
no PC110 artifact claims runnable qualification before real-firmware proof.
Record actual source/test diff, source grades, review, unresolved scope and
artifact identities; preserve owner INI, ROM masters, disks and snapshots.
