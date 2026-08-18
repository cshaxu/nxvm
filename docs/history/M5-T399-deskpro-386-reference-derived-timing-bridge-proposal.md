# DeskPro 386 Reference-Derived Timing Bridge And L3 Re-Audit

## Purpose

Create a bounded, reproducible **reference-derived** timing contract for the
exact 1986 DeskPro 386 Model 40 when no physical hardware observation is
available. This is an owner-authorized evidence tier distinct from physical
hardware validation. It can support a later Model-L3 re-audit only as
`reference-derived`; it can never overwrite T398's physical-L3 not-ready
conclusion.

## Required Scope

Use the retained D3PE source facts to constrain topology, ports and signal
roles. Observe only traceable DeskPro-specific reference configurations, led by
PCjs's explicit DeskPro 386 model. 86Box, MAME and Bochs may corroborate or
contradict a route only where their exact configuration is demonstrably
DeskPro-specific; generic PC/AT behavior is non-evidence.

Freeze five batches, each with named input, checkpoints, reset/cancellation
replay and expected order:

1. CPU retirement / board clock / DMA BWAIT conversion;
2. FDC command to DRQ, DMA2 and IRQ6;
3. KBC, D4, port-61, fail-safe and IOCHK/NMI visibility;
4. Compaq HDC command/DRQ/IRQ14 availability excluding physical-media claims;
5. CECG register/firmware-visible, raster and ISA-availability observations.

Every accepted batch must record reference revision/configuration, D3PE facts,
input sequence, observed checkpoints, reset/cancellation result, disagreement
mask and a project-owned normalized result. It must reuse Core's existing
clock/timeline/transaction/device owners and VM's Model-40 composition.

## Evidence Tier And Boundaries

A single exact DeskPro-specific reference can establish a provisional
reference-derived contract when primary documentation constrains its relevant
topology. A second independent qualifying reference is required when available;
disagreement blocks publication. Absence of a second qualified reference is
recorded as lower confidence, never silently promoted to physical proof.

No emulator source, firmware, ROM, media, trace or configuration file is
imported into NXVM. Observations become neutral requirements only after a
bounded reproducible probe validates them. No host wall-clock, Model-40-local
scheduler, generic AT fallback or Core/VM interface change is allowed.

## Completion Standard

The task first produces a durable five-batch convergence ledger. Each batch is
accepted with normalized reference-derived proof, retained as unavailable or
contradictory with an exact receiver, or rejected for insufficient provenance.
After the ledger is exhausted, a new independent DeskPro L3 audit decides
whether the model is reference-derived ready or remains not ready. Physical L3
remains not ready unless a later physical-observation task proves it.

## Stop Conditions

Stop a batch before implementation if the reference cannot prove its exact
DeskPro configuration, its input/checkpoint sequence is not reproducible, its
behavior conflicts with D3PE facts, or a proposed value would equate reference
ticks with hardware time. A disagreement is an evidence result, not a choice
of the more convenient implementation.