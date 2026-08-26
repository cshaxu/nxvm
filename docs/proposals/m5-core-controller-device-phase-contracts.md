# M5 Core Controller And Device Phase Program

This is shared context, not a Queue candidate. T449 supplies the only Core
transaction, arbitration, reset and observation routes. The following
candidates attach one controller or one inseparable controller-plus-media
chain at a time; no candidate may introduce a generic device framework,
machine-name branch or private time publisher.

## Shared Admission Baseline

This candidate applies the
[Core specification-driven timing design](../etc/architecture/specification-driven-l3-timing.md)
to the earliest controller/device receiver batch in the
[T433 S6 Core L3 admission ledger](../etc/evidence/t433-s6-core-l3-admission-feasibility-ledger.md):
`CTRL-PIC`, `CTRL-DMA`, `CTRL-PIT`, `CTRL-RTC-CMOS`, `CTRL-KBC-NMI`,
`CTRL-FDC`, `CTRL-HDC`, `MEDIA-BACKING`, and `DISPLAY-VADP`. It admits only
source-backed or labelled reference contracts through the plan/transaction
owners; electrical waveforms and unadmitted machine facts remain visible L2
exceptions rather than generic Core behavior.

The [T433 S7 source-sufficiency ledger](../etc/evidence/t433-s7-core-source-sufficiency-ledger.md)
is mandatory for each row. In particular, chip manuals admit semantics but not
unselected clock/wiring/personality facts, and `CTRL-HDC` is blocked pending an
ATA/IDE source admission as well as a selected profile contract.

Every candidate first admits its named original manual and selected board or
personality facts under the source policy. It freezes a finite command/phase
matrix with producer, consumer, transaction or clock relation, cancellation,
reset, source tier and regression. A fact without those sources remains an
explicit L2 fallback or blocked row, never an inferred delay.

## Candidate Order

Each newly admitted controller is its own T and first performs its own
original-source, List-1 and List-2 stages. PIC, DMA, PIT, RTC/CMOS, KBC/NMI,
FDC plus logical media, VADP and fixed-disk personalities therefore do not
share an aggregate implementation task. A later integration task only consumes
their accepted copied contracts. HDC is source-blocked; IBM MFM/ST-506 is not
ATA and remains a separate capability.
