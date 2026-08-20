# Td S121: Evidence-First Legacy CPU Timing Governance

`M5:Td:S121:CPU-TIMING-EVIDENCE-FIRST:OK`

## Decision

The queued [Core CPU instruction-timing program](../../proposals/m5-core-cpu-instruction-timing-program.md) must not begin by auditing current scalar code or repairing the first visible fallback. Its durable convergence ledger is built in three ordered phases:

1. complete 8086/80186 evidence and model research;
2. implementation-gap audit and finite repair plan; then
3. implementation, retirement of obsolete timing paths, and final ledger audit.

This supplements the T433 CPU source-sufficiency records for L2 admission. It does not rewrite their historical L3 finding: an emulator-derived or project-derived model is never an Intel-primary L3 formula.

## Phase 1: Complete Evidence And Model Ledger

Before implementation planning, freeze the successful-retirement universe for each 8086 and 80186 profile: legal opcode/form, operand and address form, repeat/prefix/context distinction where it changes a rule, and the manual functional semantics. Each member receives exactly one timing disposition in this order:

| Evidence situation | Required disposition |
| --- | --- |
| Intel documentation gives an exact constant or formula | Record the direct rule; it is eligible for L3 only under the existing source and profile-contract gates. |
| Intel documentation gives a bounded range | First evaluate a version-pinned, same-CPU 86Box or other mature-emulator model. A model consistent with the manual's functional semantics and interval is a labelled L2 reference-derived model. |
| Reference model conflicts with the manual | Rebuild functional semantics and applicable boundaries from the manual; reuse only separable, explainable timing mechanics from the reference. Record the correction and retain it as L2. |
| The usable reference is fixed rather than operand-dependent | Compare available same/near-profile fixed implementations. When a manual range exists, use a deterministic fixed L2 selection within that range; use the range midpoint only after no more explanatory compliant model is available. |
| Intel documentation has no usable timing range | Evaluate same-CPU, then clearly labelled near-profile reference mechanisms; correct them to manual semantics where necessary. If none is usable, construct a project-owned, explainable model from the instruction's documented execution and transaction structure. |
| No compliant reference or explainable model can be established | Record a named L1 exception. It guarantees ordered functional execution only and must never be represented by an anonymous fallback or one hardware tick. |

For every non-L3 row, the ledger records the applicable manual locus, reference revision/configuration or project model, CPU/profile scope, input variables, range/bound if any, accounting boundary, functional correction, regression vectors, L1/L2 level, and why it is not L3. No third-party source is copied, imported, made a dependency, or treated as authority for a rule that contradicts the manual.

## Phase 2: Current-State Audit And Repair Plan

Only after Phase 1 is complete may the task audit production code. The audit compares every ledger member with its actual decoder, timing selector, retirement publication, prefix/EA/repeat accounting, trace origin, fallback path and regression ownership. It produces a finite repair list that identifies model absence, semantic conflict, accounting mismatch, duplicate/obsolete path, unlabelled fallback, missing test, or incorrect L-level claim. A discovered code value never substitutes for missing Phase-1 evidence.

## Phase 3: Implementation And Closure

Each implementation S consumes a declared ledger batch and removes every obsolete or duplicate timing route in that batch. The task closes only when the frozen ledger has no anonymous `SOURCE_UNALLOCATED` or one-tick successful-retirement result: every member is direct L3, labelled L2, explicit L1, rejected, or not applicable. The final audit rechecks the original evidence/model ledger against production paths and tests; a boot result, host benchmark, or emulator agreement alone is not closure proof.

## Scope Boundary

This decision governs CPU instruction-internal timing. It does not allocate board waits, prefetch availability, DMA/HOLD ownership, device service, exception/IRQ delivery, x87 numerical timing, or physical pin phases; their named Core/VM receivers remain unchanged.
