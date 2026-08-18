# T398 S2 DeskPro Model-L3 Decision

`M5:T398:S2:MODEL-L3-NOT-READY`

## Decision

The exact 1986 DeskPro 386 Model 40 is **not ready for a Model-L3 claim**.
The decision is not a functional regression: the selected deterministic
configuration, topology, mappings, logical controllers, reset behavior and
current test graph remain valid. It is a refusal to relabel those facts as a
source-backed, firmware-visible and physical-availability Model-L3 contract.

## Non-Ready Dispositions

| Matrix boundary | Why it blocks Model-L3 readiness | Exact retained receiver |
| --- | --- | --- |
| 80386 retirement and board clock | No selected physical descriptor or conversion from retirement to 16 MHz/phase exists | DeskPro physical-observable device timing TODO requires qualified conversion and observable contract. |
| DMA/BWAIT, FDC DRQ/IRQ and KBC/D4 NMI visibility | Current Core order is deterministic but has no signal-to-guest-visible physical phase | DeskPro physical-observable device timing TODO. |
| Compaq 40 MB controller/drive | RAW-IMG normal PIO is not ECC, format, physical-sector or drive-service timing | Compaq 40 MB physical fixed-disk media semantics TODO. |
| CECG and monitor/firmware effects | Digital VADP state is not raster/sync/ISA wait or option-ROM/BIOS behavior | Compaq CECG physical, firmware and board behavior TODO. |
| External vendor firmware service | BYOB ROM carrier/mapping is not an executed owner-supplied firmware lifecycle or bounded firmware-visible corpus | DeskPro 386 BYOB firmware-visible lifecycle TODO. |

## Evidence And Model Boundary

T398 S1's eleven-row independent matrix, T386 functional closure, T394 CPU
safety boundary, T396 board closure and T397 device closure supply the direct
proof and transfers. T397's full current gate passed all 68 declared checks;
13 focused Model-40/device tests also pass. None changes the non-ready rows.

Model-339 retains its own IBM deterministic L3 contract; PC/XT retains a
distinct 8088 bus/prefetch contract. Neither is evidence for this DeskPro
configuration. No generic emulator, host clock, imported ROM/media or inferred
scalar is used here.

## Consequence

T398 closes the final DeskPro L3 audit with a durable not-ready decision. Any
future change must be admitted through its exact TODO receiver, repair the
corresponding physical or firmware-visible contract, then repeat an independent
Model-L3 audit. It may not silently upgrade this result.