# T507 S11 Model-40 D4 Refresh-Hold Re-Audit

`M5:T507:S11:D4-REFRESH-HOLD:P2`

## Source And Scope

The selected primary source is the owner-managed Compaq DeskPro 386/16 D3PE
processor material used by the accepted D4 bridge evidence
[`t419-s4-d4-refresh-hold.md`](t419-s4-d4-refresh-hold.md).  It directly
defines counter-1 refresh requests, REFRS arbitration, one refresh cycle and
refresh-before-pending-DMA hold ordering.  It does not define a Core-tick
duration, DRAM row retention, electrical refresh, CPU BWAIT or a host-time
conversion.  No external implementation can promote those missing facts.

## List 1 / List 2

| ID | Source-qualified relation | Current one-owner route and disposition |
| --- | --- | --- |
| D4-1 | A counter-1 refresh pulse requests one refresh arbitration cycle. | The D4 PIT1 callback is installed only by `core_machine_configure_d4_platform()` and queues one private pending hold per low pulse: Manual L3 logical relation. |
| D4-2 | Refresh is a hold/request/acknowledge/cycle/release operation. | `core_machine_d4_refresh_hold_advance()` uses the existing Core transaction owner `REFRESH` and commits one refresh memory-cycle before release: Manual L3 logical relation. |
| D4-3 | A remaining DMA request follows refresh without an intervening CPU transaction. | The existing shared arbitration routine calls the D4 hold before its existing DMA grant route: Manual L3 ordering. |
| D4-4 | Reset cancels pending board state. | Core reset clears the D4 pulse, pending hold and address token through the same machine lifecycle: retained logical lifecycle. |
| D4-5 | No selected document gives a duration or conversion to Core source ticks. | A pending hold blocks blind fast advance and uses the existing Core L1 compatibility receiver only; no deadline, host tick or board callback is fabricated. |
| D4-6 | Generic PC/AT and non-D4 profiles do not acquire Model-40 board state. | The callback is configuration-gated and the non-D4 focused case emits no refresh transaction.  DMA retains its separate owner. |

## Result

The source-to-code route is complete and has no duplicate owner: D4 owns the
pending bit, Core transaction owns the hold lifecycle, scheduler owns its
single L1 disposition, and DMA remains a later consumer at the same existing
arbitration boundary.  No code repair is warranted.  Focused D4 and adjacent
transaction/DMA tests pass 5/5; the complete repository-only unit replay
passes 312/312 in 14.11 seconds.

## P2 Actual-Diff Review

P1 `bbe134aa` is documentation only.  Review of the D4 callback, the existing
transaction hold and the arbitration order confirms a single board request,
one transaction owner and one subsequent DMA route.  No source-qualified
duration exists, so no production change, public contract, generic-DMA state,
VM/profile path or extra scheduler was added.  The unrelated working-tree
proposal change was neither staged nor reviewed as S11 work.
