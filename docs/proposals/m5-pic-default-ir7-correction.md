# PIC Default-IR7 Correction

## Purpose

Correct the omitted `PIC-F8` L3 acknowledgement result: after initialization,
an acknowledgement with no selected request returns the master's programmed
vector base plus IR7. This corrective scope belongs to the retained 8259A
phase contract and does not reopen its L4 physical boundary.

## Scope

The existing PIC acknowledgement owner returns `ICW2 | 7` on its initialized
no-selection path without changing IRR, ISR, `scan`, `peek`, CPU delivery, or
the public interface. Retain zero before initialization has programmed ICW2.
Update the two existing PIC smokes that intentionally exercise this operation.

## Exclusions

Do not model INTA waveforms, electrical request withdrawal, elapsed time,
slave spurious races, topology policy, ELCR/APIC, or a second delivery route.
Those are not needed for this L3 correction.

## Acceptance

The initialized no-request acknowledgement returns the default IR7 vector
without state mutation; ordinary master/slave interrupts remain covered; the
configured build, full current gate, and documentation governance pass.
