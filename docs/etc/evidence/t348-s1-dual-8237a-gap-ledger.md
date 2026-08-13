# T348 S1 Dual-8237A Contract And Gap Ledger

## Authorities And Scope

The primary reference set is the Intel 8237A programming contract and the IBM
PC/AT Technical Reference's dual-controller wiring, page-register, and
word-channel description. The latter fixes channels 0--3 as byte-addressed,
channels 5--7 as word-addressed, channel 4 as cascade, and prohibits address
progression across the corresponding 64 KiB or 128 KiB page boundary. Research
links are retained as requirements references only:

- [Intel 8237A data sheet](https://www.pcjs.org/documents/datasheets/intel/INTEL_8237A_DMA.pdf)
- [IBM PC/AT Technical Reference](https://minuszerodegrees.net/manuals/IBM_5170_Technical_Reference_6280070_SEP85.pdf)

No reference implementation, source, firmware, or guest asset enters NXVM.

## Source Ownership And Visibility

| Boundary | Production owner | Current state | Evidence / next receiver |
| --- | --- | --- | --- |
| PC/AT controller and port decode | `src/core/machine/dma.c` | One paired controller owner; primary `00h--0Fh`, secondary sparse `C0h--DEh`, and board page ports are decoded locally. | S2 must prove every programmable/readable register and exact page mapping. |
| Channel binding | `core_machine_dma_bind_channel` plus frozen machine wiring | Channel 0--3 and 5--7 have opaque bindings; channel 4 is intentionally not bindable. | S3 must prove secondary bindings and cascade isolation. |
| Hardware request visibility | `core_machine_dma_request_assert/deassert` | Request state is controller-owned and reaches DMA only from timeline arbitration. | S3 owns request/mask/priority/cascade completion. |
| One-grant progression | `core_machine_dma_advance_one` called by machine arbitration | One deterministic unit advances per due tick; T346 owns order and copied trace. | Retain; S4 proves all adopted DMA results become visible only at this boundary. |
| FDC consumer | frozen channel-2 binding and FDC callbacks | Retained consumer is bounded and covered by channel/FDC/service probes. | S4 rechecks it after generic DMA repairs. |

## Finite Contract Matrix

| Family | Current implementation / proof | Classification | Required receiver |
| --- | --- | --- | --- |
| Primary register file, flip-flop, software request/mask, mode, master clear | Decoded in `dma_port_read/write`; primary channel smoke covers channel 2, modes, mask, auto-init, decrement, and memory-to-memory. | Implemented but only partial channel/register coverage. | S2 port/register grid. |
| Secondary sparse register file and word transfer layout | Sparse `C0h--CEh` address/count and `D0h--DEh` controls are decoded; `flagWord` shifts the current address and transfers two bytes. No channel-5--7 focused proof exists. | Unproven. | S2 word-channel, port, and page grid. |
| PC/AT page ports | `81h/82h/83h/87h` select primary channels; `89h/8Ah/8Bh/8Fh` select the peer. Secondary page low bit is masked, matching the 128 KiB word-page boundary. | Source-aligned, no complete port/readback proof. | S2 page and readback grid. |
| Page-boundary progression | `IncreaseCurrAddr` increments `page` after a zero wrap; `DecreaseCurrAddr` decrements it after `FFFFh` wrap. | **Defect.** It crosses a PC/AT 64 KiB/128 KiB DMA page. Both byte and word channels share the construction. | S2 must repair the shared address-step mechanism and prove increment/decrement at both layouts. |
| Transfer type/mode | Verify, device-to-memory, memory-to-device, demand/single/block, auto-init, decrement, and memory-to-memory have bounded primary-channel tests. | Partial; secondary and interaction grid unproven. | S3 transfer-state matrix. |
| Cascade and arbitration | Primary DRQ state feeds secondary channel 0; secondary channel 0 selects primary work. Fixed/rotating priority code exists but has no complete contention/cascade proof. | Unproven. | S3 cascade, fixed/rotating priority, controller-disable, and channel-4 isolation matrix. |
| Hardware EOP and signal polarity | `flagEOP` is internal terminal state only; no bounded external EOP producer or DREQ/DACK polarity contract exists. Command bits are stored but not modeled at a machine signal boundary. | Not implemented. | S3 decides a minimal explicit signal contract; if a selected board consumer cannot exercise it, transfer the exact unselected signal behavior to TODO. |
| Physical transaction failure | `Transmission` and memory-to-memory paths discard physical memory read/write status after provider interaction and counter mutation. | Unproven atomicity risk. | S4 must reproduce boundary behavior and define validation-before-provider/counter publication if required. |
| Reset/finalize | Reset clears latch/controller data and masks channels while retaining frozen bindings; finalization is intentionally ownership-neutral. | Partially proven for primary/FDC. | S4 reset/cancellation and stale-callback sweep. |
| Mechanical L3 duration | T346 supplies a deterministic one-grant arbitration boundary, not 8237 bus-cycle duration or general wait states. | Explicit external boundary. | Later L3 bus-timing Queue candidate. |

## Reproduced Defect And Audit Result

The page-crossing defect is both source-established and reproduced by a
discarded owner-local S1 probe: primary channel 2 was programmed with page
`01h`, current address `FFFFh`, inclusive count `0001h`, block device-to-RAM
mode, and an unmasked secondary cascade. Its two bytes appeared at `1FFFFh`
and **`20000h`**; `10000h`, the required same-page wrapped destination, stayed
unchanged. The probe exited successfully only when it observed this defect and
was removed rather than retained as a passing assertion of incorrect behavior.

Both normal DMA transfer and memory-to-memory use `IncreaseCurrAddr` or
`DecreaseCurrAddr`; those helpers mutate the page register at `0000h`/`FFFFh`
wrap. This contradicts the PC/AT address rule for both controller layouts and
cannot be repaired safely as a channel-specific test exception. Existing
`core-machine-dma-channel-smoke` does not place a transfer on either boundary,
so its green result is not contrary evidence.

No second scheduler, storage/provider ABI change, or CPU delivery change is
required by the audit. S2 owns the port/page/byte-word repair; S3 owns the
complete request/cascade model; S4 owns transaction, reset, FDC/ATA, PIC, and
timeline reconciliation. The separate Queue bus-timing candidate owns physical
duration and wait-state modeling. This ledger therefore leaves no unclassified
DMA construction gap while avoiding a false whole-board L3 claim.
