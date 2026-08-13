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
