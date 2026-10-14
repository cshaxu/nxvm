# Selected 486SX Instruction Semantics

## Admission And Dependencies

Admitted as M5 T559 on 2026-10-13 after owner acceptance of T558's research
delivery 7ea9084fe. Removed from the candidate queue; Current owns active S1.
Consumes T558 sources/checklists. S1 must settle the selected CPU feature
decisions before affected implementation; research closure is not proof of
every optional feature or permission to guess missing behavior.
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

S1 is admitted for current-code/form inventory, source-conflict decisions and
the finite convergence ledger. It publishes the numbered follow-on S plan
after inspecting actual code; no implementation is hidden in this planning S.

The [S1 convergence ledger](../etc/evidence/t559-s1-instruction-inventory.md)
freezes I1--I11, their failure/alias/profile dimensions and system handoffs.
The reviewed sequence is S2 identity/decode/BSWAP, S3 XADD and inherited LOCK,
S4 CMPXCHG, S5 system-instruction boundaries, S6 CPUID/flags, S7 whole-delta
qualification. Current admits only one batch at a time. System-dependent
instructions cannot be implemented as successful no-ops; the ledger states the
explicit non-runnable disposition until their actual hardware-state owner exists.

These are planned boundaries, not concurrent admissions. Split an oversized
batch before admission; never use letter-suffixed S numbers or an endless
first-failing-BIOS-instruction repair sequence.

### Owner-Approved Batch Efficiency

Implement a complete related instruction family before validation, including
its operand/address widths, aliases, privilege/LOCK rules and failure paths.
Do not turn each opcode, width or test case into its own build/full-suite cycle.
At admission, combine simple related groups into one S when source review
confirms the same ownership and manageable proof matrix; the suggested table
does not require one S per instruction. Reconcile the ledger and subsequent
linear S numbers before admitting a combined batch. Do not merge unresolved
cache/TLB effects into a success-only implementation to obtain a larger batch.

Use this order for each implementation batch:

1. Freeze the complete form/failure matrix and implement source and tests.
2. Build only affected CPU test targets; run the focused instruction/profile
   regression set. Repair failures locally without rebuilding every App.
3. Review the entire actual diff and same-owner paths before qualification.
4. Once source and tests are stable, run one sequential x64/x86 complete unit
   qualification and the required artifact/integration checks. Reuse the
   original Ninja graphs and persistent ccache; shared targets are built once
   per architecture/configuration and reused across the four PC products.

Do not overlap builds with running qualification or restart a live job merely
to obtain fresh output. Reuse valid results for unchanged inputs. If final
review or qualification requires a code correction, rerun its affected proof
and the closure gates required by Execution; record the reason rather than
counting an earlier green run as qualification of changed code. Full external
boot coverage remains a T exit gate, not a per-opcode development loop.

## Verification And Exit

All frozen forms have direct unit proof and truthful feature gating; former CPU families retain behavior. Remaining reset/system/timing obligations are named in the next CPU proposal, not declared complete.

Use direct owner tests plus the real production connection, full repository
unit gates and required external integrations under Execution. Reuse the two
existing Ninja/ccache graphs, rebuilding each shared target once per host
architecture/configuration. Rebuild and deploy every affected admitted App pair;
no PC110 artifact claims runnable qualification before real-firmware proof.
Record actual source/test diff, source grades, review, unresolved scope and
artifact identities; preserve owner INI, ROM masters, disks and snapshots.
