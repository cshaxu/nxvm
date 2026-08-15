# M5 T371: Selected-Profile Model-L3 Phase Refinement

## Task Record

T371 receives T369's physical bus/availability transfers and T370's selected
device-duration transfers for the IBM PC/AT 5170 Model-339 baseline. It must
establish a source-labelled model phase representation before any 5170 L3 audit.

## Active Progress

### S1: phase owner and source ledger

S1 inventories every selected CPU, transaction, bus, device, trace and reset
boundary before selecting a phase scalar or behavior. Its [ledger](../etc/evidence/t371-s1-model339-phase-owner-ledger.md)
retains deterministic ordering but transfers every unqualified physical mapping
to a bounded later S.

S1 is accepted at `e230aceb`. S2 receives CPU retirement/prefetch and
memory/I/O phase vocabulary with the same no-guessed-value constraint.
