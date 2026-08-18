# T399 S1 DeskPro Reference-Derived Timing Convergence Ledger

## Evidence Tier And Frozen Universe

This ledger records the owner-approved reference-derived tier for the exact
1986 DeskPro 386 Model 40 only. It is not a physical observation and cannot
revise T398's physical Model-L3 not-ready decision. The five rows are the whole
T399 universe; a row may progress only through its recorded project-owned
probe, checkpoint and reset/cancellation replay. Reference elapsed ticks are
never a hardware-time input.

The read-only configuration sweep on 2026-08-17 found PCjs revision
`c7f21b4fa2bdedac3d5c73094a6402fdc8b24c70` with an explicit `deskpro386`
model and a 2 MiB, 80386, 1986-08-19 ROM, 1.2 MB-drive EGA configuration.
The same sweep found 86Box revision
`4fef696a4eead1d55a28d6ac0e5bd2864e5454da` with an explicit September-1986
DeskPro configuration. Neither source, configuration, firmware, media, trace
or output is imported. MAME and Bochs have no local exact DeskPro configuration
in the surveyed checkout set.

PCjs is the sole currently qualified behavior-reference candidate: its chipset
selects a DeskPro-specific model, a second PIT at `48h`, and DeskPro-specific
port-`61h` NMI bits. 86Box qualifies as an exact-machine configuration lead,
but its initializer adds a general AT FDC/common-machine path and exposes no
DeskPro-specific `48h` or port-`61h` observation route in the swept machine
surface. It therefore cannot corroborate any batch until a bounded run proves
that route without a generic-AT substitute.

## Five-Batch Ledger

| Batch | D3PE constraint and current owner | Qualified reference status | Frozen input, checkpoints and reset/cancellation replay | Disagreement mask and S1 disposition |
| --- | --- | --- | --- | --- |
| B1 CPU retirement / board clock / DMA BWAIT | D3PE identifies the 80386/16, DCLK and BWAIT topology; Core owns retirement publication, transaction and DMA arbitration. T396 proves no retirement-to-board conversion. | PCjs identifies DeskPro topology but supplies no qualified CPU-retirement-to-BWAIT signal contract. 86Box exact configuration has no observed DeskPro-specific BWAIT route. | Project-owned probe must execute a named DMA request and capture only transaction publication, DMA grant/release and reset-before-rearm checkpoints; cancellation must clear the request before the next eligible publication. | Any reference tick, host delay, generic AT callback or missing signal checkpoint is a disagreement. **Non-eligible for implementation**; retain the physical-observable timing TODO as receiver. |
| B2 FDC command → DRQ, DMA2, IRQ6 | D3PE fixes the selected 1.2 MB FDC/DMA2/IRQ6 topology; Core FDC/DMA/PIC and VM drive profile own functional behavior. | PCjs exact configuration declares two 1.2 MB drives and has DeskPro FDC branches. 86Box exact configuration declares an AT FDC path, but not a DeskPro-specific observable route. | Project-owned probe must issue one named FDC command using synthetic project-owned input and record command acceptance, DRQ eligibility, DMA2 transfer boundary, IRQ6 assertion/clear, cold reset and mid-command cancellation. | Missing DRQ, generic-AT-only route, reference time value or a different terminal ordering blocks normalization. **Accepted in T399 S3:** project-owned Model-40 and shared-Core replays normalize command acceptance, DRQ eligibility, DMA2 transfer, terminal IRQ6 and reset/cancellation ordering; PCjs is sole qualified behavior reference. No behavior change or physical timing scalar. [B2 contract](t399-s3-b2-fdc-dma2-irq6-contract.md). |
| B3 KBC / D4 / port-61 / fail-safe / IOCHK-NMI | D3PE defines the selected composite port-`61h`, fail-safe and IOCHK/NMI topology; Core KBC/D4/platform/PIT/reset own latches and lifecycle. | PCjs has DeskPro-only port-`61h` NMI-disable and timer-output bits plus its second PIT. 86Box exact configuration does not surface a DeskPro-specific port-`61h` route in the sweep. | Project-owned probe must toggle the named port-`61h` controls, inject only an existing project-owned D4/IOCHK condition, and record latch/read/NMI delivery or suppression plus cold reset and cancellation before delivery. | Any firmware-only side effect, generic port-`61h` behavior, absent D4 distinction or reference tick is disagreement. **Accepted in T399 S2:** PCjs DeskPro-specific port-`61h` facts reconcile active-low IOCHK/fail-safe NMI enables and high-pulse latch clear in the existing Core D4 owner. No electrical pulse, propagation phase or physical timing claim. [B3 contract](t399-s2-b3-port61-nmi-contract.md). |
| B4 Compaq HDC command / DRQ / IRQ14 | D3PE and T386 select the Compaq controller and normal IRQ14 route; Core HDC and VM checked RAW-IMG attachment own only functional normal PIO behavior. | PCjs exact configuration declares an HDC and DeskPro HDC compatibility branches. 86Box exact DeskPro initializer does not bind a DeskPro HDC route; its generic selection is not corroboration. | Project-owned probe must use a synthetic, project-owned HDC command path and capture command acceptance, data-request visibility, IRQ14 assert/clear, reset and cancel. No sector, ECC, format or physical-media claim is permitted. | Media-dependent behavior, generic AT HDC, a reference delay, or unavailable exact input blocks normalization. **Observation contract reserved; physical-media TODO remains receiver.** |
| B5 CECG register / firmware-visible / raster / ISA | D3PE/CECG constrains the selected Compaq Enhanced Color Graphics topology; Core VADP and VM CECG personality own digital register/memory and copied presentation only. | PCjs exact machine uses IBM EGA, not CECG. 86Box exact-machine initialization resets the selected external video but does not prove a CECG device or firmware-visible/raster/ISA route. | No qualifying reference input exists. A future probe requires an exact CECG reference configuration, one named register/firmware-visible checkpoint, raster/ISA availability observation and reset/cancel replay. | IBM EGA/VGA, copied presentation, generic ISA video or firmware import is a disagreement. **Rejected for this evidence tier in S1**; retain the CECG physical/firmware TODO as receiver. |

## Completion Predicate And Next Admission

`M5:T399:S1:REFERENCE-LEDGER-FROZEN` is satisfied: every proposed batch has one
exact configuration disposition, its D3PE/current-owner boundary, a finite
input/checkpoint/reset contract or an explicit absence, a disagreement mask and
a single receiver. No row is accepted as a timing value or production behavior.

A later T399 subtask may admit at most one complete observation-contract batch
at a time, beginning with B2 or B3 only if it can create a project-owned,
asset-free bounded probe. It must preserve the Core/VM boundary and record a
normalized result rather than a raw reference trace. B1 and B5 remain
non-eligible; B4 remains conditional on an asset-free exact input. After all
eligible rows are exhausted, an independent re-audit may decide only whether a
reference-derived L3 tier is ready; physical L3 remains not ready.