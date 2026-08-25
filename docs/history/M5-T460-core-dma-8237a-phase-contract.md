# M5 T460: Core DMA 8237A Phase Contract

T460 completes the frozen 8237A logical service-phase contract through the
existing Core DMA, transaction, arbitration and selected IBM AT topology
owners. The coverage universe is exactly `DMA-R1`--`DMA-R4`,
`DMA-F1`--`DMA-F7` and `DMA-T1`--`DMA-T5` in the T450 checklists.

## Planned Subtasks

1. **S1: source, reference and owner reconciliation.** Consume the qualified
   16-row ledger, inspect current production paths, and freeze the complete
   implementation and proof batches. No runtime behavior changes.
2. **S2: request-to-service state contract.** Give the existing DMA owner the
   one logical SI/S0/service/release lifecycle, including DREQ/DACK polarity,
   arbitration handoff, cancellation and reset, without a second scheduler.
3. **S3: transfer-mode and cascade completion.** Complete demand/single/block/
   cascade, EOP/TC/auto-init, M2M and compressed/address-latch logical phases
   through the same owner and transaction route.
4. **S4: selected AT binding integration.** Consume the one existing Core
   transaction/time owner for selected dual-controller, page, refresh and
   five-clock terms; retain any unavailable producer as a named fallback rather
   than inventing board time.
5. **S5: closure audit and product proof.** Reconcile all 16 rows to focused
   regressions, run the current gate, rebuild the T460 artifact, and prove no
   duplicate DMA state, time owner or consumer path remains.

## Completion Standard

Every row must have direct Manual-L3 proof, an explicit maintained fallback or
a named earliest receiver. The task cannot claim completion with an
undispositioned DMA state/mode/board row, an emulator-only requirement, a
second DMA/transaction/time owner, or L4 waveform work.

## Accepted Progress

S1 is accepted at `12aa90d3`. Its
[reconciliation evidence](../etc/evidence/t460-s1-dma-contract-reconciliation.md)
preserves all 16 Intel/IBM rows, rechecks the rendered scanned manual, retains
the Td S144 emulator observations strictly as corroboration, and identifies
the actual sole production path: `dma.c` state, board binding, and T449
scheduler/transaction ownership. It records the immediate cascade completion,
atomic M2M copy and inert command bits as the concrete implementation gaps;
S2--S5 are bounded accordingly. No runtime behavior or artifact changed.

S2 is accepted at `434411d4`. Its
[logical-service evidence](../etc/evidence/t460-s2-dma-logical-service.md)
records the one private DMA acknowledgement/service-release state and the
centralized entry/release helpers. It removes repeated ISR-clearing paths,
does not expose a pin API or change T449/board ownership, and preserves the
focused DMA/transaction/FDC regression group.
