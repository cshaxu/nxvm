# T415 S1 Page-Walk Locality Bridge

M5:T415:S1:PAGE-WALK-LOCALITY:OK

The original D4 material records a two-wait initial/page-miss and zero-wait
sequential same-2048-byte-page CPU-memory condition. T415 admits the remaining
Core CPU physical accesses that T409 already classifies as PAGE_TABLE_READ and
PAGE_TABLE_WRITE. A committed PDE/PTE read or accessed/dirty writeback now uses
the same bounded Core page-key receiver as committed prefetch and data accesses.
Model-40 selects 2 KiB, miss +2, hit +0.

This is generic-AT behavior using original D4 values, not a reconstruction of
the D4 row/bank comparator. It does not claim the 80386 paging pipeline,
prefetch/page-walk overlap, idle behavior, BWAIT, DMA/refresh arbitration, or
physical/L3 timing.

The focused locality smoke proves cancellation has no effect, then proves a
committed same-page page-table read and writeback charge miss + hit and reset
clears the receiver. The independent real 80386 paging corpus now passes too.
While running it, T415 found and repaired a T411 lifecycle defect: MOV CR0
across PE/PG and MOV CR3 could retain bytes fetched through the prior
translation context. Those writes now invalidate the persistent prefetch
window, preserving the Core CPU-executor boundary.

Developer artifact: vm-0-5-0415, build/output/nxvm_0_5_0415.exe, 3,211,636
bytes, SHA-256 F57D7A70EBD78307786C4B0C98BB0D3EEA715070DFCDA025284C3439D94BE5B8.

Exact D4 row/bank mapping, idle/overlap, BWAIT, DMA/refresh arbitration,
locked/RMW breadth and physical/L3 acceptance remain transferred.