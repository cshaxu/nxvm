# T485 S1 IBM 5160-268 Board And Device Phase Source Ledger

`M5:T485:S1:XT-BOARD-PHASE-LEDGER:OK`

This ledger consumes T483's primary-source partition, T484's closed functional
owners, the Intel controller checklists and the current Core timing-plan
boundary.  It records only source-backed inputs and present receivers; it does
not publish a physical clock or host pacing claim.

| Path | Source-backed fact and tier | Present owner/receiver | Disposition |
| --- | --- | --- | --- |
| 8088/ISA | IBM 5160 Technical Reference: 14.31818 MHz / 3 = 4.77 MHz; normal bus is four 210 ns clocks and selected I/O is five. **Manual L3**. | The XT profile selects `ticks_per_instruction = 1`; Core has retirement and external-cycle hooks, but no 8088 physical-retirement or XT bus-cycle mapping. | Blocked: next S must define one sourced CPU/bus conversion; no nominal-MHz tick injection. |
| ROM/RAM/reset | IBM defines 256 KiB and ROM mapping/reset visibility, not a ROM access or reset-duration formula. **Manual L3 construction; no timing formula.** | Core immutable ROM/reset owner. | L2 fallback for access/reset duration. |
| PIC/IRQ/NMI | IBM wires IRQ0, IRQ1, eight IRQ levels and parity NMI. Intel 8259A defines causal INTA/priority order but no controller clock. **Manual L3 ordering.** | Core PIC/PPI/NMI owners; `pic_visibility` accepts only L2 fallback. | L2 visibility latency; do not derive a delay from electrical diagrams. |
| PIT/refresh | IBM supplies 1.193182 MHz PIT input, OUT0 to IRQ0 and OUT1 to refresh/DMA. Intel 8254 defines counter/output phase. **Manual L3 rational clock and phase.** | Core timing plan can accept `pit_clock = SOURCE_RATIONAL_CLOCK`; XT currently leaves it fallback. | Eligible for a copied XT rational PIT clock in the next implementation S. |
| DMA/refresh | IBM supplies one 8237A, channel 1 refresh and three external channels. Intel 8237A defines state phases but not XT board clock/service publication. **Manual L3 logical route.** | Core has one DMA owner and can accept rational clock plus source DMA service phases; XT currently supplies neither. | Clock/service remain L2 until an XT board formula and transaction mapping are both established. |
| PPI keyboard/NMI | IBM/PPI sources establish line and IRQ relations; no selected keyboard byte, typematic, parity-fault or NMI-delivery latency formula. **Manual L3 causality.** | Core XT PPI owner. | L2 service/visibility phase. |
| 8272A FDC/360 KiB drive | Intel 8272A gives command fields and SRT/HUT/HLT units; IBM XT selection fixes 360 KiB route. Neither source proves the selected drive's rotation, ready/motor or host-image service phase. **Manual L3 controller relation.** | Core FDC/media/DMA owners. | L2 for all board/mechanical deadlines; no AT `3F7h` or 1.44 MB substitution. |
| CGA | IBM selects CGA; retained CGA checklist proves controller semantics, but no selected XT scan-to-Core-axis conversion is established. **Manual L3 function.** | VADP owns ports, VRAM and snapshots. | L2 scan/display phase; presentation is non-guest time. |
| Xebec Type-2 | IBM adapter defines ports/DMA3/IRQ5/ROM mapping; prior Xebec ledger establishes logical command/DMA path. No selected controller/drive seek, rotation or transfer-time formula exists. **Manual L3 wiring; Other L3 only for bounded logical corroboration.** | Core HDC/media/DMA owners. | L2 physical service phase; no ATA timing or emulator scheduler borrowed. |

The current Core plan is intentionally narrower than the desired machine result:
it exposes typed L3 inputs only for PIT rational clock and paired DMA
rational-clock/service phases.  PIC visibility remains an explicit L2 field;
CPU retirement and external bus timing require their own qualified mapping.
This is correct ownership, not a reason to fabricate profile-side setters or a
second scheduler.

External implementations (MAME, 86Box, PCjs, Bochs and QEMU) corroborate
controller state/order only where already recorded in the Intel checklists and
T479/T484 evidence.  Their host schedulers, later-machine extensions and
compatibility workarounds create no XT timing fact.  The next S may implement
only the PIT rational-clock receiver after confirming exact ratio semantics;
CPU/bus, DMA service, PIC, PPI, FDC, CGA and Xebec remain separately bounded.
