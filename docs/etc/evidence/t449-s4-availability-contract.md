# T449 S4: Availability, BUSRDY And Prefetch Contract

`M5:T449:S4:AVAILABILITY-CONTRACT:OK`

## Selection And Dynamic-State Ledger

| Selected value | Copied-contract consumer | Dynamic owner and clear/reset disposition |
| --- | --- | --- |
| External page timing and access wait windows | `cpu_timing_model.c` is the sole selector/charger at external-cycle commit. | `core_machine` owns pending/page/overlap and per-round counters; cancel, HOLD invalidation and cold reset clear them. Physical READY duration remains explicit L2. |
| DMA BUSRDY gate and wait quanta | `machine_scheduler.c` alone decides whether a pending DMA tick may advance. | `machine_board.c` alone accepts selected board-level changes; `core_machine` owns the current level and wait counter, both restored on cold reset. |
| CPU BUSRDY gate | `machine.c` alone holds a pending CPU retirement before final publication. | `machine_board.c` alone accepts selected board-level changes; `core_machine` owns the current level and cold reset restores ready. |
| CPU prefetch reservation enable | `machine.c` reserves after a successful CPU round; `machine_scheduler.c` releases only while refresh, DMA and transaction/HOLD ownership are absent. | CPU execution owns the queue, expected address and reservation. CPU control changes, stopped physical writes and cold reset invalidate it; HOLD/refresh retain a reservation until it can be released. |

## Boundary Result

The selected Model-40 configuration is a value-only nested initializer in the
copied plan. No profile name, callback or mutable VM/session object reaches
Core. Default PC/AT and Model-339 leave the contract's zero/default values in
their existing materializers.

The reservation deliberately does not create a second asynchronous fetch
producer: CPU instruction execution remains the only fetch/memory transaction
and exception owner. It records a bounded eligibility state, which the
scheduler releases only after the shared arbitration conditions hold. This is
the existing explicit L2 boundary for unsourced physical prefetch/READY/HOLD
waveforms, rather than an invented delay or parallel bus path.

All production reads of the transaction contract, BUSRDY levels and prefetch
reservation are the owners listed above. Test-only reads are observations.
No stale raw selection, duplicate prefetch producer, unpaired wait path or
reset omission was found, so S4 adds no source code.

## Verification And S5 Prerequisite

Focused direct regressions pass the retained external-cycle/prefetch,
DMA-BUSRDY/competition, D4 refresh-HOLD and transaction-lifecycle markers.
Documentation governance and `git diff --check` pass.

S5 may consume this accepted selection boundary to reconcile arbitration and
RAM/ROM/A20/parity access classification. It may not duplicate an availability
consumer or turn an explicit L2 electrical timing transfer into a synthetic
cycle model.
