# T413 S2 External-Write Locality Closure Audit

M5:T413:S2:EXTERNAL-WRITE-BRIDGE-CLOSURE:OK

The coordinator reviewed P1 ae3839f1 against T413. The Core owns the only
external-cycle callback, locality state and retirement-tick accumulation. The
Model-40 profile only selects the generic-AT 2 KiB/miss+2/hit+0 values. The
predicate accepts only committed instruction-prefetch reads and committed CPU
data writes; page-table writeback, DMA, BWAIT, arbitration, device timing and
all other access classes are excluded.

The focused owner regression proves actual CPU data write publication at
retirement, same-page write hit, cancellation and reset. T410 transaction,
T359 80386 timing and 80286 timing regressions pass; the full serialized
current-gate succeeds and documentation governance passes for vm-0-5-0413.

T413 closes the bounded CPU-write bridge. Original D4 write hit/miss facts
support its values, but the exact row/bank comparator, hold/idle/system-board
transition, write-CAS phase, CPU overlap, BWAIT, DMA/refresh arbitration,
locked/RMW breadth and Model-40 physical/L3 acceptance remain transferred.