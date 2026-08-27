# IBM 5160 Xebec Fixed-Disk Controller Phase Contract

## Purpose

Complete the selected XT Xebec/ST-506 fixed-disk unit at the sole Core
HDC/media owner, distinct from ATA and IBM 5170 WD1003 personalities.

## Required sequence

S1 validates original Xebec/IBM selected-controller sources. S2 freezes List
1 for ports, command/status phases, DMA3/IRQ5, CHS/media, reset/cancel and
documented service timing. S3 maps List 2 to the HDC/media owner and its full
transaction/test surface. Implementation consumes the complete gap batch.

## Boundaries

No ATA shim, duplicate CHS/cache state, unsupported mechanical delay estimate
or VM-owned HDC state.
