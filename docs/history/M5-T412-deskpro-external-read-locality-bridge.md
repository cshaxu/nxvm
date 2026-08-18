# M5 T412: DeskPro External-Read Locality Bridge

T412 accepts a bounded Core prefetch-locality timing mechanism and binds it
only in the Model-40 composition.  The Core remains the sole CPU transaction,
external-cycle and retirement-time owner; VM supplies only the `{ 2048, 2, 0
}` generic-AT configuration.  The public external-cycle observer now marks
whether a cycle is a read or write, so the locality policy cannot charge a
write.

The selected Compaq D4 source supplies the 2 KiB/two-wait/zero-wait facts, but
not a safe production row/bank decoder or overlap/idle phase model.  T412
therefore records the implementation as a generic-AT bridge, not original
DeskPro timing or an L3 result.  Its focused regression proves committed
miss/hit, cancellation, reset and retirement behavior.  Exact D4 row/bank,
writes, DMA/refresh, BWAIT and physical timing remain in the active physical
cycle/phase proposal.