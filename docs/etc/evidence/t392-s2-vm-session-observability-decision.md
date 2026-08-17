# T392 S2: VM And Session Observability Decision

`M5:T392:S2:VM-OBSERVABILITY-DECISION:OK`

## Existing owner matrix

| Existing state | Owner and boundary | C1 endpoint suitability | Decision |
| --- | --- | --- | --- |
| FDD media presence, geometry and generation | VM FDD object; private session storage/media lifecycle. | Proves insertion/replacement only, not FDC command completion, DMA, BIOS result or boot transfer. | Insufficient; do not expose the mutable FDD layout. |
| FDC configuration and DMA binding | Core FDC configuration selected by Model40 composition. | Topology is static configuration, not completion state. | Insufficient; no new Core callback. |
| Display snapshot | Existing copied Core display snapshot consumed by VM display path. | May change for arbitrary firmware output and has no boot-ready contract. | Non-semantic; do not use as a checkpoint. |
| Session fault outcome / run result | VM control/fault boundary with fault or stop status. | Defines failure only; successful non-stop execution is not a boot result. | Failure boundary only. |
| Session control and input transport | VM composition lifecycle. | Host/session scheduling has no guest boot semantic. | Excluded. |
| Retirement observation | Core-owned copied CPU snapshot. | Already exhausted by T392 S1. | No new endpoint. |

## Design decision

No existing VM-level state is an admissible C1 endpoint. The only safe future
capability, if separately approved, is a **VM-owned immutable device-operation
snapshot** produced at the device/session composition boundary: it would carry
a narrowly selected operation result and generation, have reset/failure
semantics, be consumed by a test-local VM observer, and never expose mutable
FDD/FDC/session pointers or alter Core retirement observation. Its exact device
operation, source contract, lifecycle and regression corpus require a separate
admission.

This task does not implement that capability because no selected device
operation or primary hardware/firmware contract exists. The complete receiver
is therefore a future selected device/session observability design; the
80386 physical-retirement candidate remains blocked. No current source or ABI
is deficient merely because this capability is absent.