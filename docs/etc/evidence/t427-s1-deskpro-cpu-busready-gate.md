# T427 S1: DeskPro CPU BUSRDY Completion Gate

`M5:T427:S1:CPU-BUSRDY-GATE:OK`

## Evidence Tier And Source Boundary

Tier: `original`. The transiently consulted 1986 Compaq DeskPro 386/16 D3PE
processor material identifies CPU memory/I/O wait states controlled by
`BUSRDY`, and its D4 memory description identifies `NAM*`/`MRDY*` as the
CPU-cycle completion protocol. The source file, rendered pages, extracted text,
paths and temporary archive were deleted after inspection. No firmware, media,
source text or asset enters this repository.

## Delivered Owner Contract

Core owns the new discrete CPU BUSRDY capability beside its existing DMA
BUSRDY capability, without merging their independent signals. When a selected
profile enables the CPU gate and lowers it during an already pending chargeable
external CPU-memory-cycle completion wait, Core advances deterministic
non-retiring guest time while preserving that pending wait. Releasing the gate
allows the existing wait drain and the existing retirement observation path to
complete. Cold reset restores ready. The operation rejects profiles that did
not select the capability.

The Model-40 composition selects this Core capability. IBM 5170 Model 339 does
not. VM introduces no transaction, execution, scheduler, or timing owner.

## Verification And Sweep

`core-machine-prefetch-locality-smoke` proves chargeable external-cycle wait,
BUSRDY-low non-retiring preservation, release, normal retirement and cold-reset
ready restoration. `vm-model40-private-composition-s7-smoke` proves Model-40
selection; `vm-ibm-5170-model-339-composition-smoke` proves 5170 isolation.
The sweep covers all Core external completion waits, both BUSRDY signals,
construction validation, cold reset, and every selected Model-40/5170
configuration. DMA retains its own existing gate and no device becomes a
fabricated CPU-BUSRDY producer.

## Transfer

This bridge is not a complete BWAIT waveform, CLK16/BCLK calibration, ISA
availability/peripheral binding, D4 PAL row/bank model, external bus-master
phase, asynchronous CPU prefetch producer, or DeskPro physical/L3 acceptance.
Those receivers remain explicit in the DeskPro physical-cycle proposal.