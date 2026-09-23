# T415 S2 Page-Walk Locality Closure Audit

M5:T415:S2:PAGE-WALK-LOCALITY-CLOSURE:OK

Fresh coordinator review of P1 0651b9f6 confirms one existing Core receiver:
committed external CPU page-table reads and page-table writebacks participate in
the same bounded locality state as committed CPU fetch/data cycles. Model-40
only selects the generic-AT 2 KiB, miss +2, hit +0 values. Cancellation and
reset remain safe, and no Core/VM dependency reversal, second scheduler, or
second CPU/DMA transaction path was introduced.

The review also corrects P1 wording: the prefetch window is invalidated on a
CR0 PG transition and on CR3 replacement, not on every CR0 PE transition. The
real 80386 paging corpus proves both transitions through the existing guest CPU
path. Focused locality, paging, transaction/timing regressions, the fully
rebuilt serialized current gate (288/288), and documentation governance pass
for vm-0-5-0415.

T415 closes only this generic-AT page-walk receiver. Exact D4 row/bank decode,
CPU/DMA page retention, prefetch/page-walk overlap, BWAIT, DMA/refresh
arbitration clocks, locked/RMW cycle shape, and physical/L3 acceptance remain
transferred. T416 is admitted next for a bounded generic-AT CPU/DMA ownership
boundary without claiming those exact D4 phases.