# M5 Common-Machine Executor Completion

## Purpose

Complete `common/machine` only with executor semantics that both NXVM and
SoftPC require, then remove the residual NXVM VM-runtime implementation that
duplicates that completed Common owner.

This follows the VM-machine owner cleanup.  It is not a license to migrate
Core, MVDM, profile, ROM, device, firmware, media, renderer or platform code
into Common.

## Shared Contract

The completed `common/machine` owns neutral, copied-value execution mechanics:

- bounded request ingress and run-generation retirement;
- input open/close/clear transitions at reset and stop;
- one executor lifecycle: wake, safe-point request consumption, idle wait,
  completion publication and orderly exit;
- ordered lifecycle requests and actual lifecycle facts;
- a paused-only, generation-invalidated x86 debug lease and bounded operations
  that both products can map to their machine boundary.

It does not own any actual machine state.  Each product supplies an adapter:

```text
common/machine contract
    |- NXVM vm/machine -> Core machine
    `- SoftPC host/machine -> MVDM/CCPU
```

`common/session` remains the overall control reducer and monitor-policy
coordinator.  `common/ui` remains the raw Console/Window coordinator.  Neither
knows a CPU, profile, firmware or product machine object.

## SoftPC Evidence

SoftPC currently implements equivalent behaviour in `app/input_queue`,
`app/runtime`, and its `host/machine` adapter: one executor consumes copied
input at a safe boundary; stale input is retired by run; wake and completion
are explicit.  The work must reconcile the contract with those real consumers,
then allow both projects to consume the same Common source unchanged.  No
SoftPC source is copied into NXVM without a separate approved source review.

## Subtask Plan

1. **S1 - two-consumer contract ledger.** Compare NXVM and SoftPC lifecycle,
   input, wake, safe-point, run-generation, completion and paused-debug
   semantics.  Define a finite contract ledger identifying exact common facts,
   product adapter responsibilities, and excluded product-specific behaviour.
2. **S2 - Common executor completion.** Implement the minimal Common executor
   and completion contract missing from the existing FIFO/lease component.
   Use Lib host synchronization only; expose no platform, Core, MVDM, CPU,
   profile or raw machine pointer.
3. **S3 - NXVM adapter migration.** Bind the NXVM Core adapter to the completed
   contract, delete residual VM-runtime queue/runner/wait/lifecycle mechanisms,
   and prove profile/media/Core ownership remains outside Common.
4. **S4 - SoftPC conformance and Common corpus closure.** Independently build
   the Common corpus; add two neutral conformance configurations and a SoftPC
   adapter conformance consumer.  Verify byte-identical Common source use,
   no product terms in Common, full NXVM unit/integration gates and stripped
   x64/x86 artifacts.

## Exit Criteria

- `common/machine` owns the complete shared executor mechanism and has real
  NXVM and SoftPC adapter consumers; no API depends on either product.
- NXVM has one Common-driven lifecycle/input/safe-point route and no residual
  parallel VM runtime control mechanism.
- Common contains no Core, VM, MVDM, CCPU, ROM, profile, device or platform
  type/header/API; each product adapter owns its own machine implementation.
- The contract ledger exhausts every compared mechanism with proof, explicit
  non-applicability, or a named later receiver.  Full required tests and task
  artifacts pass.
