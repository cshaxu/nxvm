# M5 Common-Machine Residual Audit And Conditional Completion

## Purpose

Audit every remaining `vm/machine` responsibility against the actual
`common/machine` and SoftPC owners. Complete `common/machine` only where the
same neutral mechanism has two real consumers; then remove any demonstrated
NXVM duplicate.

This follows the VM-machine owner cleanup.  It is not a license to migrate
Core, MVDM, profile, ROM, device, firmware, media, renderer or platform code
into Common.

## Conditional Shared Contract

The existing `common/machine` owns neutral, copied-value request ingress,
run-generation retirement, safe-point delivery and paused-Debug leasing. A
later subtask may extend that contract only when the ledger proves a shared
mechanism with identical semantics:

- bounded request ingress and run-generation retirement;
- input open/close/clear transitions at reset and stop;
- a common executor lifecycle: wake, safe-point request consumption, idle
  wait, completion publication and orderly exit;
- ordered lifecycle requests and actual lifecycle facts;
- a paused-only, generation-invalidated x86 debug lease and bounded operations
  that both products can map to their machine boundary.

It does not own any actual machine state. Each product supplies an adapter:

```text
common/machine contract
    |- NXVM vm/machine -> Core machine
    `- SoftPC host/machine -> MVDM/CCPU
```

`common/session` remains the overall control reducer and monitor-policy
coordinator.  `common/ui` remains the raw Console/Window coordinator.  Neither
knows a CPU, profile, firmware or product machine object.

NXVM's bounded Core quantum runner and SoftPC's continuous CCPU/timer
rendezvous loop are not assumed to be the same mechanism. If the S1 ledger
does not prove one neutral executor lifecycle, both remain product adapters
while Common retains only their already-shared protocol.

## SoftPC Evidence

SoftPC currently implements related behaviour in `app/input_queue`,
`app/runtime`, and its `host/machine` adapter. The audit must distinguish
identical copied request/run/wake/lease semantics from product-specific worker,
TLS, heartbeat, media and timing behavior. No SoftPC source is copied into
NXVM without a separate approved source review.

## Subtask Plan

1. **S1 - exhaustive two-consumer residual ledger.** Inventory every remaining
   `vm/machine` production file, state field and public operation. Compare each
   with `common/machine` and the actual SoftPC route; classify it as an existing
   Common duplicate, a proven two-consumer candidate, a distinct NXVM Core
   adapter, or a named later receiver. No runtime code changes occur in S1.
2. **S2 - proven local-dead-path and conditional Common completion.** Delete
   any complete S1-proven local dead/forwarding batch. For a separate batch
   proven identical in both products, implement the minimal Common mechanism.
   Use Lib host synchronization only; expose no platform, Core, MVDM, CPU,
   profile or raw machine pointer. If neither batch exists, S2 is skipped and
   the ledger is the task conclusion.
3. **S3 - NXVM duplicate retirement.** Migrate only an S2-proven Common
   duplicate batch, delete its residual VM path, and prove profile/media/Core
   ownership remains outside Common. The runner itself may move only if S1
   proved an identical two-consumer lifecycle.
4. **S4 - dual-consumer conformance and closure.** Independently build the
   Common corpus and verify source-identical consumer use for every admitted
   mechanism. Prove no product terms in Common, then run the applicable NXVM
   unit/integration gates and dual stripped artifacts.

## Exit Criteria

- Every residual `vm/machine` mechanism is exhaustively classified with source
  evidence; only a proven two-consumer mechanism enters Common.
- Every admitted Common mechanism has real NXVM and SoftPC adapter consumers;
  no API depends on either product. A non-identical executor remains an
  explicitly retained product adapter, not a failed migration.
- NXVM has one route for every admitted Common mechanism and no residual
  duplicate VM implementation.
- Common contains no Core, VM, MVDM, CCPU, ROM, profile, device or platform
  type/header/API; each product adapter owns its own machine implementation.
- The contract ledger exhausts every compared mechanism with proof, explicit
  non-applicability, or a named later receiver.  Full required tests and task
  artifacts pass.
