# M5 T415: DeskPro Page-Walk Locality Bridge

T415 isolates paging physical accesses from ordinary CPU data accesses. It will
determine whether committed page-table reads and writebacks can use the existing
Core external-memory locality bridge with original D4 page-mode values and a
generic page key, without asserting CPU paging-pipeline overlap, the original
row/bank decoder, BWAIT, DMA/refresh arbitration or physical/L3 timing.
P1 implements the bounded generic-AT receiver and repairs prefetch invalidation
on CR0 translation-mode and CR3 directory changes. The real 80386 paging corpus
is the regression proof; exact D4 phase behavior remains transferred.
P2 independently closes T415. It corrects the earlier broad PE/PG wording to
CR0 PG plus CR3 only, retains all D4 physical phases, and transfers CPU/DMA
page-retention behavior to T416.