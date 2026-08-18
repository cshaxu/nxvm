# M5 T412: DeskPro External-Read Locality Bridge

T412 accepts a bounded Core prefetch-locality timing mechanism and binds it
only in the Model-40 composition. The Core remains the sole CPU transaction,
external-cycle and retirement-time owner; VM supplies only the generic-AT
2 KiB, miss +2, hit +0 configuration. The public external-cycle observer now
marks whether a cycle is a read or write, so the locality policy cannot charge
a write.

The selected Compaq D4 source supplies the 2 KiB/two-wait/zero-wait facts, but
not a safe production row/bank decoder or overlap/idle phase model. T412
therefore records the implementation as a generic-AT bridge, not original
DeskPro timing or an L3 result. Its focused regression proves committed
miss/hit, cancellation, reset and retirement behavior. Exact D4 row/bank,
writes, DMA/refresh, BWAIT and physical timing remain in the active physical
cycle/phase proposal.

P1 0d2c9edc is independently accepted by the S2 closure audit. T412 is
closed; its generic bridge is complete and every exact-D4/physical receiver
remains transferred. [Closure audit](../etc/evidence/t412-s2-external-read-locality-closure-audit.md).