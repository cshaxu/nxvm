# T414 S1 CPU Data-Read Locality Bridge

M5:T414:S1:DATA-READ-LOCALITY:OK

D3PE defines D4 page mode for CPU memory cycles, including initial/page-miss
two-wait and sequential same-2048-byte-page zero-wait behavior. T414 extends
the existing Core generic bridge to committed CPU data reads. Model-40 retains
the 2 KiB/miss+2/hit+0 selection. This is a generic-AT page-key bridge backed
by original cycle values, not the D4 row/bank comparator or overlap model.

The focused smoke executes a CPU MOV AL,[moffs] data read and proves the
prefetch miss plus data-read miss surcharge is published at retirement. It
continues to cover write, cancellation, reset and same-page behavior. Page-table
reads are deliberately excluded: their paging-side effects and physical cycle
shape remain a separate receiver.

Developer artifact: vm-0-5-0414, build/output/nxvm_0_5_0414.exe, 3,211,636
bytes, SHA-256 B5CB21F73AC06EDD70B4919368609EDF20BE52C9FEFF20B45734111AC9577D2D.

Exact D4 row/bank mapping, idle/overlap, page walks, BWAIT, DMA/refresh,
arbitration, locked/RMW breadth and physical/L3 acceptance remain transferred.