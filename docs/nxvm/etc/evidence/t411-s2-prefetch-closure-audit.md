# T411 S2 Prefetch-Prerequisite Closure Audit

`M5:T411:S2:PREFETCH-CLOSURE:OK`

## Completed prerequisite

T411 S1 (`a9be57f8`) supplies one Core-private 15-byte prefetch window. The only
production code-reader is `_s_read_cs`; it consumes a valid window or uses the
existing classified logical-memory path. `_kdf_skip` records the next
sequential CS:EIP-derived linear address. `ExecInit` invalidates a window when
that address changes, so near/far control transfers, exception delivery,
interrupt entry and task transitions all select a refill without individual
transfer hooks. Preview operates on a copied execution context.

CPU context initialization, CPU state initialization and the actual CPU reset
path clear the window. The focused smoke resets, overwrites reset code and
proves a fresh refill. Existing T359 S4 and 80286 timing-ledger regressions
pass, retaining the current instruction-time publisher. HOLD/HLDA remains the
existing transaction lifecycle; T411 introduces neither a second transaction
owner nor a CPU/DMA route.

## Transfers

| Receiver | Tier | Status after T411 |
| --- | --- | --- |
| D4 2 KiB row/page decoder and read wait selection | original | Not implementable from logical adjacency; requires an explicit source-backed external-cycle row/bank contract. |
| CPU prefetch overlap/retirement relationship | original/reference-derived | The window exposes refill and consumed fetch, but no overlap phase or hardware width/queue depth claim is published. |
| D4 writes, DMA/refresh and BWAIT arbitration | original where available, otherwise reference-derived/generic-AT | Unchanged receiver; requires an explicit signal/phase input before binding to the sole Core transaction owner. |
| Display, storage, keyboard/NMI and BYOB phase receivers | tier-labelled per proposal | Unchanged; none is implied by CPU window completion. |

Therefore T411 closes only its prefetch prerequisite. The next bounded task is
the existing DeskPro physical-cycle proposal, which may use the established
three-tier source policy but must not turn this generic Core mechanism into a
DeskPro physical or Model-L3 claim.