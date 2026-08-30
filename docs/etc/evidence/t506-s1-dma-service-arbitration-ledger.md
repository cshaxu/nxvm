# T506 S1: DMA Service And Arbitration Ledger

## Sources and coverage boundary

The selected universe is exactly the existing bound 8237A routes: PIT1 refresh,
FDC DMA2 and IBM XT Xebec DMA3, plus the primary/secondary cascade required by
the selected AT topology. It does not absorb FDC/HDC command timing, DRAM
electrical refresh, ATA/Compaq/WD PIO personalities, or pin waveforms.

Intel `231466-005`, *8237A High Performance Programmable DMA Controller*, was
rendered and visually checked; its usable source facts cover DREQ/DACK, mask
and priority, HRQ/HLDA, transfer modes, terminal count/EOP, reset and cascade.
IBM 5160 and 5170 technical references provide the selected board wiring.
The manual has electrical timing values, but no conversion into the current
Core time axis; those values therefore do not authorize a service deadline.

Available-local 86Box, Bochs and PCjs DMA implementations were previously
read and reconfirm the request/priority/cascade ordering only. MAME and QEMU
are unavailable locally. External implementations are corroboration, never a
timing source or imported dependency.

## List 1 / List 2 ledger

| ID | Required relation (List 1) | Current owner/path (List 2) | Disposition |
| --- | --- | --- | --- |
| D1 | Bound provider DREQ is retained until acknowledged; mask and priority select the eligible request. | `core_machine_dma_request_{assert,deassert}()` and `core_machine_dma_set_drq()` own the opaque binding; `dma_pending_requests()` selects it. | Manual-L3 function/order. |
| D2 | Selected request raises HRQ; accepted HLDA starts service, and service releases through the existing transaction boundary. | `machine_scheduler.c` owns the HOLD request/acknowledge/release; `dma.c` owns phase and transfer. | Manual-L3 order; duration needs board-axis evidence. |
| D3 | One service follows 8237A phases (including READY/command rules) before provider/memory effect; compressed timing is a phase selection, not a new route. | `core_machine_dma_advance_transaction()` and `core_machine_dma_advance_one()` are sole service paths. | Manual-L3 logical state; Core tick placement is lower-tier. |
| D4 | Demand, single, block and cascade modes preserve the documented request/termination rules. | DMA mode state, completion and cascade derivation remain in `dma.c`; primary-to-secondary is derived mask-aware. | Manual-L3 function/order. |
| D5 | TC/EOP and auto-init finish/reload/mask exactly through one completion lifecycle. | `dma_complete_transfer()` owns TC, EOP, auto-init and provider terminal callback. | Manual-L3 lifecycle. |
| D6 | PIT1 drives the selected refresh request route; refresh is not a second DMA state or media transfer. | Board configuration binds the PIT callback to the immutable refresh channel. | Manual-L3 routing; board cadence remains L2 without a qualified axis. |
| D7 | FDC DMA2 owns byte-ready DRQ and terminal callback; DMA owns only request/service/transfer. | FDC uses its bound opaque DMA request and existing terminal callback. | Manual-L3 route; FDC deadline remains FDC-owned. |
| D8 | XT Xebec DMA3 owns its request/release and terminal response; non-Xebec fixed-disk personalities do not receive a DMA alias. | Xebec owns the binding; ATA, WD1003 and Compaq/WD have no DMA provider. | Manual-L3 route; HDC service duration remains HDC-owned. |
| D9 | Reset/master-clear and provider cancellation withdraw requests and leave no service/cascade state. | DMA reset/finalize and bound-provider reset paths retain the only withdrawal route. | Manual-L3 lifecycle; S2 must sweep every selected provider. |
| D10 | No selected source maps a complete DMA service cadence to the current Core axis. | Scheduler treats a pending DMA request as an L1 compatibility candidate while normal arbitration can advance it only with its existing clock input. | **Gap/receiver:** S2 must determine whether an already available DMA-clock conversion can publish a real next deadline; otherwise retain L1/L2 explicitly, without a guessed delay. |
| D11 | Physical DREQ/DACK/HRQ/HLDA/EOP waveforms are electrical constraints. | No pin-level model exists. | Outside T506; no L4 claim. |

## S2 boundary

S2 consumes D1--D10 as one owner batch. It must inspect all selected board
clock contracts and the Core deadline composition in one pass, then either
publish a source-qualified DMA deadline through the existing DMA/Core owners or
leave D10 as an explicit lower-tier relation. It may not repair one provider
with a private timer or shift responsibility into FDC, HDC, VM, or a profile.

## S1 verification

The manual was rendered; text extraction was used only for navigation. The
source/owner sweep covered `dma.c`, `machine_scheduler.c`, board construction,
and the FDC/Xebec/PIT request callers. Complete repository-only unit passed
312/312 with `ctest -L unit -j 8`; the documentation-governance target passed.
