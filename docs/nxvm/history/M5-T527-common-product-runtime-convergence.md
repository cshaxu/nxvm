# M5 T527: Common Product-Runtime Convergence

## Outcome

T527 closes the requested NXVM product-runtime refactor.  The retained Core
layout is `core/machine` plus `core/product`; reusable x86 assembly and Debug
are `common/xasm32` and `common/debug`; the neutral Common runtime is exactly
`common/session`, `common/machine`, and `common/ui`.  NXVM owns only its Core
adapter in `vm/machine`, product policy in `vm/product`, and composition in
`vm/app`.

No old VM events/session/presentation/executor route remains alongside the
Common path.  Common has no product, Core, VM, native platform, firmware,
profile, or machine-layout dependency.

## Subtask record

- S1 froze the route/ownership ledger and prohibited a fabricated
  `common/runtime` component.
- S2 extracted the sole assembler/disassembler owner to `common/xasm32`.
- S3--S4 made Common session and machine the sole ordered control and
  safe-point bridges.
- S5--S6 moved the table-driven Debug grammar to Common, retained its DOS
  semantics, and restored the bounded copied inspection result path.
- S7--S8 made Common UI the sole Lib presenter binding and reduced NXVM to
  product policy, Core adaptation, and App composition.
- S9 independently built and verified the complete Common corpus, added two
  neutral adapter configurations, removed stale integration control routes,
  and completed task-level verification.

## Closure evidence

Implementation P1 is `2c596f2c`.  The final corpus and verification details,
including artifact hashes, are recorded in
[S9 reusable Common closure evidence](../etc/evidence/t527-s9-reusable-common-closure.md).
The retained task proposal is
[M5 T527 proposal companion](M5-T527-common-product-runtime-convergence-proposal.md).

T527 is closed.
