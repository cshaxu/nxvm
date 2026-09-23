# T412 S1 External-Read Locality Bridge

`M5:T412:S1:EXTERNAL-READ-LOCALITY:OK`

T412 adds one Core-owned, opt-in prefetch-locality timing policy.  On a
committed CPU `INSTRUCTION_PREFETCH` external read, it classifies the physical
address by a configured power-of-two page size, charges the configured miss or
hit ticks to the current execution round, and adds that bounded result only to
the existing retirement publication.  Cancelled reads change neither the tag
nor the pending ticks; reset clears both.  Writes, data accesses, page walks,
DMA, refresh, BWAIT and arbitration are excluded.

The selected Model-40 composition configures `{ 2048, 2, 0 }`.  This is a
`generic-at` bridge, not a reconstruction of the D4 PAL: it uses the original
D3PE facts recorded in T408 (2 KiB locality and two-wait miss/zero-wait hit)
but does not assert the unavailable D4 row/bank decoder, idle behavior, bus
width, or prefetch overlap.  The local read-only PCjs DeskPro configuration
confirms a separately composed 32-bit DeskPro profile but supplies no D4 DRAM
wait implementation; it is therefore not used as timing authority.

The focused `core-machine-prefetch-locality-smoke` proves retirement publication
of a committed miss and, at the Core observer boundary, same-page hit,
cancellation and reset behavior.  The established transaction/prefetch and
80386/80286 timing regressions remain green.  The exact original D4 row/bank
mapping, idle/phase semantics, writes, DMA/refresh, BWAIT and physical/L3
acceptance remain transferred to the DeskPro physical-cycle proposal.
The T412 developer artifact is m-0-5-0412 /
uild/output/nxvm_0_5_0412.exe, 3,211,117 bytes, SHA-256
3B4048CD4C177020530D69734D8ABD376E8B909C4AC0AB0369BDFA2F0B75C2C6.
