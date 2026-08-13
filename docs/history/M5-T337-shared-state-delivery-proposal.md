# Shared CPU State And Delivery Foundations

## Objective

After the four-profile audit identifies the exact residual rows, close the
shared architectural state transitions on which instruction forms compose:
real/protected/VM86 exception, IRQ, and NMI delivery; 16/32-bit gate and frame
selection; privilege transitions; TSS stack switching; restart and fault
atomicity; and instruction-created interrupt inhibition where applicable.

## Dependency and scope

This candidate follows the audit because it must use its ledger rather than
assume which paths remain incomplete. It may repair a shared mechanism only
with caller, write, validation-to-commit, and fault-order sweeps. It does not
turn device PIC behavior, x87 execution, VME/PVI, or later CPU extensions into
CPU-completeness claims.

## Completion standard

Every assigned delivery row has a mode-appropriate focused proof or an explicit
Intel boundary, and later profile candidates can rely on one documented state
transition contract rather than per-opcode approximations.

## T336 Audit-Derived Breakdown

Admission begins with the current form ledger and may refine these bounded
subtasks, but must retain their dependency order:

1. establish one real-mode vector-6 `#UD` IVT policy and migrate or explicitly
   retain every affected owner fixture;
2. reconcile `#DB`, INT1/INT3, TF, hardware breakpoint, IRQ/NMI priority, and
   their 16/32-bit frame/restart effects where the audit identifies an
   unproven row; and
3. close the remaining shared producer-to-delivery/return rows, including
   task/reset outcomes only when Intel classifies them as common delivery
   behavior rather than profile-specific state.

Each subtask audits every producer and delivery consumer before a shared change;
an already-proven bounded route is regression evidence, not permission to
rewrite it.
