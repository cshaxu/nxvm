# T511 S11 D4 Board-Route Phase-Consumer Audit

`M5:T511:S11:D4-PHASE-CONSUMER:NO-HIT`

## Sources

The retained Model-40 D4 board ledger establishes counter-1 refresh as a
board request and the selected Core transaction order. It does not establish a
physical refresh duration, which remains explicitly unmodelled. Existing
source-led L3 causal wiring and the Core-owned L2/L1 boundary are retained; no
external implementation or guessed delay is introduced.

## Owner/Consumer Matrix

| Boundary | Sole production path | Disposition |
| --- | --- | --- |
| PIT counter-1 edge | The D4 output callback receives the current PIT edge, invalidates locality and records one pending refresh hold on the active edge. | Repeated asserted levels cannot queue duplicate holds. |
| Pending refresh | The Core deadline observation exposes exactly the pending request at the next arbitration point. | A prior refresh cannot authorize an unrelated later cycle. |
| Hold/transaction | `core_machine_d4_refresh_hold_advance` alone requests, acknowledges, commits and releases the refresh transaction, then increments the address and clears pending state. | One observed pulse yields one transaction. |
| DMA and reset | Arbitration services D4 refresh before DMA; reset clears pending, pulse and address state in the Core owner. | No profile callback or stale board assertion reaches DMA/CPU. |

## Executed Proof And Conclusion

The focused Debug cohort passed: D4 compatibility/map/parity/SKEY, Model-40
DMA, D4 refresh-hold, D4 platform and competition smokes (8/8). It verifies
edge coalescing, refresh-before-DMA ordering, hold lifecycle and reset cleanup.

No ATA-style stale observation exists. The D4 route has a single callback,
pending bit and Core transaction consumer; no production change is warranted.
