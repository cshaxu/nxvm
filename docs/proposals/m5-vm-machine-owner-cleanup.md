# M5 VM-Machine Owner Cleanup

## Purpose

Remove the NXVM `vm/machine` runtime paths that duplicate an already-admitted
Common or Lib owner.  Leave `vm/machine` as the NXVM-specific adapter that
constructs a Core machine from a frozen profile/assets request and translates
between that machine and Common contracts.

This task is subtractive.  It does not invent a new shared runtime and does
not move NXVM-specific machine/profile behaviour into Common.

## Boundary

The retained `vm/machine` boundary owns only:

- frozen NXVM profile, ROM/CMOS/font and media assembly into a Core machine;
- Core-specific device, frame and paused-debug target adaptation; and
- binding those adapters to the existing `common/machine`, `common/session`,
  `common/ui`, and Lib contracts.

The following duplicate categories are removed from `vm/machine` rather than
kept as compatibility paths:

- request FIFO, safe-point wake and request ordering already owned by
  `common/machine`;
- generic lifecycle request/state coordination already owned by
  `common/session` and `common/machine`;
- host wait/event mechanics already owned by `lib/host`;
- frame mailbox, presenter target and native presentation control already
  owned by `common/ui` and `lib/ux`;
- Common debug command/lease state already owned by `common/debug` and
  `common/machine`; NXVM retains only the Core target implementation;
- monitor/status text already owned by the NXVM product callback, not the
  machine adapter.

An NXVM-specific profile compositor, Core display-frame copier, Core debug
target implementation, or media/profile mapping is not a duplicate merely
because SoftPC has a different implementation.  Such code remains in NXVM,
but is relocated to the narrowest correct NXVM owner where necessary.

## SoftPC Cross-Check

SoftPC's `app/control`, `app/input_queue`, and `app/runtime` prove the common
semantics for ordered control, per-run input retirement, executor wake and
completion reporting.  Its `host/machine` remains a product adapter for MVDM
and CCPU, just as NXVM `vm/machine` remains a product adapter for Core.  This
task must not import SoftPC source or turn either adapter into a Common
dependency.

## Subtask Plan

1. **S1 - ownership inventory and cutover ledger.** Inventory every
   `src/vm/machine` production file and public symbol; classify it as retained
   NXVM adapter, existing Common/Lib duplicate to remove, or NXVM code to
   relocate.  Compare every claimed common category with SoftPC's equivalent
   owner and record its consumer.  Establish focused regressions before any
   deletion.
2. **S2 - product-output and stale-Core cleanup.** Move monitor/status text
   out of `vm/machine`; replace its print helpers with one copied information
   query; relocate the NXVM banner to `src/`; and delete the uncalled
   Core-product configuration corpus.  This independent subtraction proceeds
   before executor cutover because S1 proved Common has no executor receiver.
3. **S3 - NXVM media-owner relocation.** Move the generic FDD/HDD
   Lib-storage-to-Core-media adapters out of `vm/machine` into `vm/media`.
   Preserve one Core provider route and all media behaviour; this does not
   depend on the executor decision.
4. **S4 - provider-lifecycle forwarding cleanup.** Delete the demonstrated
   zero-state `provider_lifecycle` facade and have the existing
   device-composition owner directly initialize, reset and finalize FDD/HDD
   providers.  Preserve call order and every Core/Common boundary.
5. **S5 - profile/firmware-owner disposition.** Audit the remaining
   profile/firmware composition for a demonstrated wrong owner, then relocate
   only code that can move without a reverse profile-to-machine dependency.
   Retain copied Core-frame adaptation and the Core debug target bridge.
   Executor/worker ownership is explicitly excluded: Common owns the copied
   FIFO/lease contract, while NXVM and SoftPC retain their distinct execution
   loops unless a separately approved two-consumer design proves otherwise.
6. **S6 - residual owner closure audit.** Reclassify the actual remaining
   corpus after S2--S5, remove a demonstrated local forwarding facade or
   wrong-owner value contract, and prove every retained file is either Core
   assembly, copied Core adaptation, Core-time pacing, or NXVM's bounded
   quantum runner. Common continues to own copied FIFO/run/lease semantics;
   it does not acquire an executor worker.
7. **S7 - task-level closure and artifact verification.** Keep the proven
   Common-protocol/NXVM-runner boundary unchanged; run the complete unit and
   external integration suites, all closure gates, and build the required
   stripped Release x64/x86 `0.5.0528` artifacts.  Review the actual final
   diff and map every task exit criterion to retained evidence.

## Exit Criteria

- `vm/machine` has no duplicate request queue, lifecycle state machine, host
  wait primitive, presenter mailbox/target owner, monitor output path, or
  Common debug state.
- Each retained VM-machine symbol has one NXVM adapter responsibility and a
  documented Core or Common boundary; no retained code is a forwarding facade.
- The ownership ledger covers every former runtime file and every migrated or
  deleted public API, with a focused regression and similar-issue sweep.
- Full repository-only unit and external integration suites pass, and stripped
  x64/x86 artifacts are produced for the admitted numeric task.
