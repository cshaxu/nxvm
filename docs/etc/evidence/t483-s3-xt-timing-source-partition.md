# T483 S3 IBM PC/XT 5160-268 Timing And Source Partition

`M5:T483:S3:XT-TIMING-PARTITION:OK`

`M5:T483:S3:XT-AUDIT-CLOSURE:OK`

## Rule Of Interpretation

An exact primary-source number or formula qualifies that *named hardware
relation* as Manual-L3 input to a later owner.  It does not establish that the
current machine-wide elapsed axis, CPU retirement, bus waits and every device
service path have been integrated into one physical-time axis.  In particular,
the 4.77 MHz nominal CPU value is not a license to inject host-derived ticks
into Core or to claim that one guest second equals one host second.

The later XT timing/source closure is the sole receiver for the unresolved
items below.  It consumes construction-only inputs through the existing
Core-owned time/board boundary, while the next XT functional closure consumes
the S2 capability matrix.  Neither receiver creates a second scheduler,
profile-owned clock or VM-to-Core time writer.

## Partition

| Hardware relation | Proven input | Current disposition | Sole later receiver |
| --- | --- | --- | --- |
| CPU and board oscillator | IBM 5160 technical reference: 14.31818 MHz crystal divided by three yields the 8088 4.77 MHz board clock. | Manual-L3 fact.  Core has no 8088 profile or 8088 retirement/bus attachment, so it cannot yet participate in the current Core time axis. | XT timing/source closure: attach source-qualified 8088 retirement and board-cycle inputs to the one Core axis. |
| CPU bus-cycle width | IBM: normal bus cycles are four 210 ns clocks; selected I/O cycles take five clocks. | Manual-L3 relation.  Which present transaction paths use each condition is not established by this audit. | XT timing/source closure: derive bounded bus/transaction inputs; retain an explicit L2/L1 fallback for every condition without a source rule. |
| PIT | IBM: 8254 input is 1.193182 MHz; channel 0 supplies IRQ0 and channel 1 supplies refresh/DMA request. | Manual-L3 controller/board facts.  Current Core PIT ownership exists, but the XT route and its common physical axis are not selected. | XT timing/source closure, coordinated with the functional receiver's single-PIC/single-DMA route. |
| DMA refresh and external channels | IBM: channel 1 supplies refresh and there are three external DMA channels plus the refresh channel. | Logical route is Manual-L3.  This source packet does not establish every DMA transfer-service or memory-refresh phase on the common physical axis. | XT timing/source closure; it may accept only qualified board timing inputs, otherwise retain the classified fallback. |
| PIC, keyboard IRQ and NMI | IBM: timer uses IRQ0, keyboard IRQ1, parity uses NMI and the board has eight IRQ levels. | Delivery relations are Manual-L3; interrupt-service latency/edge placement relative to CPU cycles is not closed. | XT timing/source closure after the functional route exists; no VM or profile IRQ scheduler is permitted. |
| CGA | IBM documents the selected CGA aperture; retained CGA evidence separately carries controller facts. | VADP has a source-backed CGA state/frame path, but no XT profile binding or selected board timing contract. | Functional receiver binds the device; XT timing/source closure qualifies only any board timing input that is actually sourced. |
| FDC and selected drive | Product catalogue proves one half-height drive, not its exact selected mechanism/data rate/service-time contract. | No timing value may be inferred from an AT drive or generic 8272A implementation. | XT timing/source closure first obtains a source-qualified selected drive/controller relation; otherwise the functional receiver remains logically correct without a false physical-time claim. |
| Xebec fixed disk | IBM adapter material proves byte-stream ports, DMA3, IRQ5 and option ROM window, but the selected unit has no factory disk and no drive type/geometry is frozen. | The command route is a functional Xebec requirement; seek/service/transfer and media timing are not proven. | Functional receiver freezes an admissible BYOB geometry; XT timing/source closure accepts only later sourced service timing, never ATA estimates. |
| Keyboard serial interface | IBM proves a board serial keyboard interface, but this packet does not prove byte/typematic/command response timing. | Functional controller binding is required; timing remains source-gated. | XT timing/source closure obtains controller/board source; otherwise retain its nonphysical fallback. |
| Host pacing | T471/T472/T474 retain Core-only progress and host-side ahead-only pacing. | No 5160 physical pacing qualification exists because the complete 8088 retirement, bus and device set above is not yet integrated. | XT timing/source closure may qualify a profile only after those inputs close on the existing Core axis; until then Standard uses the established nonphysical policy and Turbo never waits. |

## Closure Reconciliation

1. S1 selected exactly one IBM 5160-268 baseline, CGA and the IBM Xebec
   expansion boundary without committing firmware/media.
2. S2 exhaustively mapped that bill of materials to current owners or explicit
   absences, with one functional receiver and no AT substitution.
3. This S3 exhausts the time-bearing relations in that selected bill and
   assigns all unclosed timing evidence to one timing/source receiver.
4. No S in T483 changes production source, ABI, runtime behavior, media,
   firmware or profile availability.  `ibm-5160-268` is not yet runnable.

This closes the audit, not XT implementation.  Its successors must keep the
two receiver boundaries disjoint: functional construction first, and
source-qualified physical timing only through the retained Core time axis.
