# DeskPro FDC Operation Observation Contract

## Purpose

Provide a Core-owned, copied and immutable observation of completed generic FDC
operations, then bind one selected Model-40 VM consumer without exposing FDC,
DMA, media or session layouts. This is a prerequisite investigation for a
future C1 semantic checkpoint, not a checkpoint itself.

## Required Scope

Freeze the complete FDC result-publication universe: transfer completion,
READ-ID, seek, reset/abort, DMA terminal and media/geometry failure. Define
one copied result schema, sequence/lifetime/reset/failure rules, publication
point and caller sweep. The VM consumer may retain only copied values and must
name an exact finite semantic condition plus a project-owned regression.

## Completion Standard

The task closes only after every frozen publisher is proven covered or
transferred, a bounded Core/VM contract is implemented and tested, and the
selected consumer is either proven finite or explicitly transferred. It makes
no CPU physical-time, board timing, firmware compatibility or L3 claim.

## Boundaries

No private FDC pointer/layout publication, VM-specific Core callback, firmware
or media import, raw trace retention, ROM catalogue, timing allocation, or
physical qualification. Local assets and 86Box/PCjs are read-only research
inputs only; no third-party code is imported.
