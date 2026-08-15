# IBM PC/AT 5170 Final Model-L3 Audit

## Purpose

Make the final ready/not-ready decision for IBM PC/AT 5170 Model 339/Type 3
only after its capability ledger, selected-device functional closure, and
board/phase-timing closure complete. This is distinct from T372's current-state
audit and may not absorb implementation.

## Required scope

Reconcile the frozen 8 MHz/Rev.3/512 KB/CGA/no-fixed-disk configuration's CPU,
memory/ROM, bus, DMA, PIC/PIT/RTC, input, FDC/floppy, display, NMI, reset,
cancellation, and deterministic ordering evidence. Identify each supported
aftermarket 1.44 MB field-upgrade condition separately from the factory
baseline. Prove that every selected capability is functionally complete and
has a source-backed or explicitly bounded L3 timing contract.

## Non-goals and stop conditions

No repair work, generic AT or clone conclusion, MFM/ATA substitution, physical
cycle-exact/waveform claim, or Windows claim. A reproduced defect returns to
its earliest functional or timing candidate; an unselected product capability
returns to the later current-product closure.

## Evidence standard

Require an independent requirement-to-evidence matrix, a machine/component
closure matrix, cross-device and reset/cancellation replay, applicable current
gate, and an owner-visible final 5170 L3 ready/not-ready decision with every
residual receiver.
