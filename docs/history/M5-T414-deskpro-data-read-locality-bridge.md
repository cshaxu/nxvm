# M5 T414: DeskPro CPU Data-Read Locality Bridge

T414 is the next bounded CPU memory receiver in the DeskPro physical-cycle and
phase-timing proposal. It extends the existing Core locality bridge from
prefetch reads and data writes to committed CPU data reads, using the original
D4 page-mode facts only as values while retaining the generic 2 KiB page-key
fallback. It does not encompass page-table reads, exact row/bank comparison,
idle/overlap, BWAIT, DMA/refresh or physical/L3 acceptance.
P1 33a79115 is independently accepted by the S2 closure audit. T414 closes CPU data-read locality and transfers page-walk physical timing. [Closure audit](../etc/evidence/t414-s2-data-read-locality-closure-audit.md).
