# T356 S2: Device and timing receiver plan

## Ordered timing program

1. **Instruction-timed execution** is first because all device clocks consume
   executor elapsed ticks; without an explicit profile/form cost source, a
   wait-state or device latency is only an arbitrary multiplier.
2. **Bus-timed PC/AT operation** follows because selected CPU/DMA transaction
   ordering already exists but has no availability/reservation duration.
3. **Cycle-exact selected profile** follows both: it needs the CPU time basis,
   bus ownership model, and selected device microstate contracts before it can
   model prefetch or phases truthfully.

The three linked proposals retain this dependency and forbid a parallel time
or transaction owner.

## Deferred device/product dispositions

| S1 residual | Disposition |
| --- | --- |
| Parity/I/O-channel NMI; PPI/speaker | Existing medium-priority TODO: requires a selected IBM PC/AT source/corpus and state/IRQ/timeline owner. |
| Advanced AUX; CGA/EGA/VGA/VBE breadth | Existing medium-priority TODO: requires a protocol/mode matrix and guest corpus; no Windows checkpoint selected either. |
| Serial/parallel/game | Existing low-priority TODO: requires one interface-specific controller/port/IRQ/DMA/reset contract. |
| Sparse/WASM HDD, LBA48, extended IDE | Existing high-priority TODO: requires opaque backing and asynchronous host/cache boundary; do not attach it to synchronous controller callbacks. |
| Windows Setup/boot, Mantle/DOS/NXVDM | Existing product roadmap/Queue boundaries; T355's INT13 checkpoint provides no feature selection. |
| x87/80287/80387 execution | Existing low-priority TODO and excluded by the owner request. |

No deferred entry is relabelled as implemented.  The audit adds Queue work only
where the source/evidence ledger establishes a universal predecessor for full
L3 rather than a corpus-gated peripheral choice.
