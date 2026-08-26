# IBM 5160 8237A DMA Phase Contract

## Purpose

Complete selected XT 8237A channel, refresh, arbitration and device-route
behavior at the sole Core DMA owner.

## Required sequence

S1 validates the 8237A and IBM board references. S2 freezes List 1 for every
selected channel/mode, HRQ/HLDA, DREQ/DACK, refresh, terminal count, reset and
phase rule. S3 maps List 2 to `dma.c`, transaction routes and regressions.
Implementation closes the complete admitted batch at that owner.

## Boundaries

No FDC/Xebec personality implementation, VM DMA state, guessed DMA service
time or duplicate transfer loop.
