# T399 S1 Reference-Derived Ledger Acceptance Audit

## Independent Reconciliation

The coordinator reviewed T399 S1 P2 `dc4e3fad`, the actual five-row ledger,
prior T396--T398 boundaries, the Queue ordering, and the source/research
policy. The ledger contains exactly the five proposal batches and gives each
one D3PE/current-owner constraints, exact-reference qualification status,
bounded input/checkpoint/reset-or-cancellation handling, a disagreement mask
and a single disposition.

| Acceptance question | Result |
| --- | --- |
| Exact-machine provenance is distinguished from generic AT behavior | Pass. PCjs has an explicit DeskPro model/configuration; 86Box has an explicit September-1986 DeskPro configuration but no accepted route output; unavailable MAME/Bochs are not substituted. |
| Reference behavior is isolated from physical claims | Pass. No tick, delay, scalar, phase or L3 physical result is selected. T398 physical not-ready remains unchanged. |
| Every batch has an honest next state | Pass. B2--B4 reserve only asset-free project-owned observation contracts; B1 remains non-eligible and B5 is rejected without an exact CECG reference. |
| Source, firmware and media containment | Pass. The ledger records revisions and neutral findings only; it imports no third-party source, ROM, configuration, media, trace or binary. |
| Core/VM ownership boundary | Pass. Current owners remain unchanged; S1 adds no scheduler, interface or runtime behavior. |
| Similar stale-queue issue | Pass. The completed T394 proposal is archived and no longer appears as an active Queue candidate. |

## Acceptance

S1 is accepted as `M5:T399:S1:REFERENCE-LEDGER-FROZEN`. Its completion does
not accept a timing value or a runtime repair. A continuation may consume only
one complete B2, B3 or B4 observation-contract batch after a new active packet
specifies an asset-free project-owned probe and focused regression. B1 and B5
remain outside that continuation unless new qualifying evidence changes their
ledger dispositions.