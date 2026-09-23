# T393 S1 FDC Publisher And Consumer Inventory

## Frozen Publisher Batch

| Class | Current owner/path | Disposition |
| --- | --- | --- |
| Data transfer and scan/format | `complete_transfer` -> `PENDING_COMPLETE` -> `set_result` -> result phase -> IRQ | Selected: one copied terminal-result publication at the final result/IRQ boundary. Covers success, media/geometry failure and DMA terminal. |
| READ ID | immediate `set_result` -> result phase -> IRQ, or the selected transfer-complete failure path | Excluded from the first boot-sector consumer; it must remain distinguishable by opcode if later observed. |
| Seek/recalibrate | pending seek -> `complete_simple` -> result phase/IRQ, then command phase | Not a result-register completion; transfer to a later generic event class rather than forge a seven-byte result. |
| Sense/status/unknown commands | direct result phase only | Not device-operation completion; excluded. |
| DOR/Core reset and readiness notification | cancel/reset controller or ready-change IRQ | Cancellation/reset boundary: publish no successful completion; epoch/reset semantics must invalidate prior VM copy. |

## Contract Decision

The implementable first contract is Core-owned FDC **data-transfer terminal
result** observation. Its copied record needs a monotonically increasing
sequence, command opcode, final ST0/ST1/ST2/CHRN result bytes, selected drive,
and success/failure classification. It is published only after the final result
is formed and before/with its guest-visible IRQ; reset or DOR cancellation emits
no terminal record and invalidates the VM-held copy by reset epoch. The Core
callback receives only the record and opaque context; the VM owns its copy and
may not access FDC, DMA, media or session-private layout.

## Model-40 Consumer Decision

Model 40 owns the FDC topology (ports 3F2/3F4/3F5/3F7, IRQ6, DMA2) but has no
existing copied completion fact. The selected future consumer is the first
successful `READ DATA` terminal result for drive 0 after the accepted C0A
predecessor. It is a diagnostic semantic candidate only: it must be observed in
a bounded owner-managed corpus before it can qualify C1. No result currently
proves firmware boot transfer or physical time.

## Evidence

Project FDC smoke and Model-40 S24 coverage exercise pending completion,
media/geometry failure, DMA terminal, IRQ and reset. Read-only local technical
material confirms the selected platform's DMA/FDC topology; no asset bytes,
paths or third-party code enter this repository.
