# M5 Model 40 Session Lifecycle Consolidation

## Objective

Remove the parallel Model 40 VM session construction, rollback, and teardown
path. VM session composition must have one lifecycle owner; the Model 40
profile may contribute only its immutable board topology, controller
personality, and firmware configuration.

## Design And Boundaries

The generic session owner creates, binds, rolls back, and destroys the plan,
media registry, display provider, Core machine, presentation mailbox, and
debugger exactly once. Model 40 supplies its actual D4/EGA/FDC/HDC/CMOS/memory
board description and controller materialization before that lifecycle; it
does not own a second resource transaction. No generic plugin framework,
profile-side mutable device state, second Core machine, compatibility route,
public session API, external YAML, ROM/CMOS/media contract, or default-PC/AT
conditional is admitted.

## Planned Subtasks

1. **S1 - lifecycle inventory and convergence.** Inventory all construction,
   rollback, finalization and board-materialization callers, then retain Model
   40's distinct preparation while routing it through the generic allocation,
   binding, rollback and finalization transaction. Delete the Model 40 storage
   initializer and rollback.
2. **S2 - verification-baseline correction.** Reject stale pre-change binaries
   as evidence and retain only the fresh-build requirement.
3. **S3 - closure sweep.** Prove successful and failed construction for default
   PC/AT, XT, IBM 5170 and Model 40; rerun unit and applicable external-ROM
   integration, then review the complete construction-caller inventory.

## Completion Standard

One production VM lifecycle owns all shared session resources for every
supported profile. Model 40 remains a declarative board/controller contributor;
there is no residual duplicate lifecycle implementation or partial-session
publication on failure.
