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

S3 is accepted at `2ae6457a` with the final in-scope correction in the later
S5 delivery: M2M has one read then write phase through the existing transaction
owner, cascade is a delegated slot rather than a false terminal transfer, and
the source and destination each use their own address-direction mode bit.
The [mode-phase evidence](../etc/evidence/t460-s3-dma-mode-phases.md) and the
DMA channel smoke cover the sequence and failure release.

S4 is accepted at `d08e38d5`: PIT counter 1 reaches the sole Core-owned DMA1
refresh request binding, and FDC selection rejects that reserved channel. The
[AT binding evidence](../etc/evidence/t460-s4-dma-at-refresh-binding.md)
retains the uncalibrated five-clock/3 MHz conversion as an explicit L2 timing
boundary rather than inventing a board clock.

S5 records the final M2M direction regression and source-backed compressed
timing transition: normal service is `S1 -> S2 -> S3 -> S4`, and `TM` removes
exactly `S3`. Its [closure audit](../etc/evidence/t460-s5-dma-closure-audit.md)
retains the only remaining selected L2 boundary, the IBM AT numeric
3 MHz/five-clock conversion, pending its existing Core timing-plan receiver.

T460 closes with all 16 ledger rows disposed: the manual-backed logical
service states, compressed `TM` transition, M2M source/destination phases,
mode release rules and selected AT binding have direct focused proof; the one
uncalibrated numeric conversion remains an explicit L2 boundary. The final
serial current gate is 294/294 and the stripped `nxvm_0_5_0460.exe` artifact
has SHA-256 `29FB7AC3D715B45D60A82F4D32F3B4D17C8B4A8601C60FBE482DBB332CE0AF62`.
