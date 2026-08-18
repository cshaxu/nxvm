# T399 S3 B2 DeskPro FDC/DMA2/IRQ6 Contract

## Result

B2 is accepted as a reference-derived logical availability/order contract, not
physical timing. The exact PCjs DeskPro 386 configuration at
`c7f21b4fa2bdedac3d5c73094a6402fdc8b24c70` selects two 1.2 MB floppy drives
and has DeskPro FDC model branches. Retained D3PE fixes the selected Model-40
1.2 MB FDC, DMA2 and IRQ6 topology. 86Box's exact-machine initializer only
corroborates an AT FDC path, not a DeskPro-specific observable route; it is not
used as behavioral confirmation.

The project-owned Model-40 replay uses synthetic ROM and 1.2 MB raw bytes. It
proves command acceptance; non-DMA data visibility; DMA2 request, transfer and
completion; IRQ6 assertion; terminal observation; cold reset; and an
out-of-range terminal result. The shared FDC media-change regression proves
mid-command DOR cancellation deasserts DMA request and IRQ and returns the
controller to command phase. The focused Core FDC regression covers the same
shared FDC/DMA/PIC owner. No implementation defect was found, so no production
source changed.

The normalized order is: accepted command reaches an FDC execution phase,
which makes DRQ eligible for the existing DMA2 owner; transfer completion
publishes the terminal result and IRQ6; reset or cancellation removes pending
request/IRQ publication before rearm. This is deterministic logical ordering
only. It chooses no FDC clock, command-to-DRQ latency, DACK phase, rotation,
media or electrical timing, and makes no physical-L3 claim.

## Reproducible Verification

On 2026-08-17, these focused CTests passed:
`current.core-machine-fdc-smoke`,
`current.core-machine-fdc-media-change-port-smoke`, and
`current.vm-model40-fdc-s24-smoke`.

The Model-40 test emits `M5:T386:S24:FDC-12MB-LOGICAL:OK`,
`M5:T386:S24:FDC-DMA2-IRQ6:OK`, and
`M5:T386:S24:MODEL40-FDC-BINDING:OK`. These are retained historical markers
whose coverage is reconciled here; no reference trace or third-party material
is imported.