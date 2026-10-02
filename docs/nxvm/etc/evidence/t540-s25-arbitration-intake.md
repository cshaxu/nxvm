# M5 T540 S25 Arbitration Source Intake

S25 is a source-only intake before editing the mixed arbitration path.
`machine_scheduler.c` still owns one Core `elapsed_ticks` publication and
calls arbitration, readiness and peripheral in that order. Its arbitration
body cannot move wholesale to an IBM-PC board without transferring Core
transaction and CPU prefetch authority. No production source, ABI, test,
profile, asset, INI or product executable changes in S25.

## Exact original order and owners

1. Advance DMA, PIT and auxiliary PIT clock domains. These rational clock
   accumulators currently live on `core_machine` but consume board timing
   configuration. Their returned tick counts are values, not a new time
   owner.
2. Capture D4 refresh pending, request/acknowledge Core HOLD, begin/commit
   the Core refresh memory transaction, then clear/increment D4 board
   pending/address only on success. Failed HOLD or begin leaves the pending
   request for a later attempt; HOLD release remains Core-owned.
3. If the board DMA wait contract is nonzero, iterate the converted DMA
   ticks: check the chip request, honor bus-ready gating, advance the wait
   counter, then perform one Core-held board DMA grant and clear that counter.
   Otherwise use the current 286/386 Core HOLD branch for all ticks, or
   the direct board DMA advance branch. Both `core_machine_dma_grant_advance`
   and the bulk branch ultimately call `dma_bus.c`'s existing
   `core_machine_dma_advance_transaction`, whose device callback uses the
   bounded S20 Core memory-cycle operation. A new second DMA path is not
   permitted.
4. Evaluate CPU prefetch reservation after DMA using both the refresh state
   captured before the attempted service and the updated pending state,
   chip request, Core transaction owner and HOLD owner. This CPU execution
   effect is Core-owned; board code must only provide copied request facts.
5. Emit DMA trace; advance shared and optional auxiliary PIT; emit PIT
   trace; refresh PIC master/slave and emit PIC trace. These are board
   chip/wiring effects at the tail of arbitration, before readiness.

The resulting bounded sequence is S26 D4 refresh request/completion around
Core transaction, S27 DMA clock/request/grant around Core HOLD/wait and
prefetch, S28 PIT/PIC post-prefetch board effects, then S29 CPU PIC INTA and
DMA-HOLD locality. S30 owns mixed plan/reset, S31 neutral Core relocation,
and S32 onward the proven IBM-PC board extractions. This is the minimal
owner split: no generic event bus, provider hierarchy, copied mutable
controller state or additional guest clock.

## Regression receivers

- D4 refresh HOLD, parity and transaction smoke, including failed-HOLD
  retention and reset state.
- DMA channel/grant/wait, single Core memory-cycle, request arbitration,
  primary-only/paired topology and terminal-count smoke.
- CPU prefetch locality and transaction trace tests across 286/386 and XT.
- PIT output/IRQ0, auxiliary PIT, PIC phase/cascade and Core scheduler
  time-observation smoke.
- Four fixed-profile external boots, once per width, after every code
  receiver; eight optimized 0540 products and complete dual-width units per
  code S. This source-only S retains the accepted S24 **469/469** per width,
  all specialized gates, **8/8** boots and eight Release products as its
  unchanged baseline.

S25 changes documentation only. Documentation governance and diff hygiene
are checked at acceptance. T540 stays open for S26 onward.
