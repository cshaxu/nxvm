# T520 S1 Session Lifecycle Ledger

## Scope And Method

The searched production scope is `src/vm/**/*.c` and `src/vm/**/*.h`.  The
query covers creation and destruction of the Core plan, media registry, display
provider, Core machine, presentation mailbox and debugger, plus every storage
initialize/rollback/finalize helper.  Direct Core and test fixtures are outside
this VM composition ownership query: they construct their own owner-local test
or Core fixtures and do not publish a VM session.

## Production Disposition

| Resource or operation | Before | Disposition |
| --- | --- | --- |
| plan, media registry, display provider | Generic session and Model 40 storage paths | One generic session lifecycle owner. |
| Core machine, mailbox, debugger | Generic session and Model 40 storage paths | One generic session lifecycle owner. |
| rollback/finalization | Two copied Model 40/generic sequences plus generic finalization | One generic finalization sequence for normal and failed construction. |
| Model 40 topology | Interleaved with Model 40 resource allocation | Retained as Model 40 board preparation only. |
| Model 40 D4/FDC/HDC configuration | Interleaved with Model 40 resource allocation | Retained as Model 40 plan materialization only. |
| default PC/AT, IBM 5170, XT topology and devices | Generic session path | Unchanged generic lifecycle consumers. |
| VDM minimal machine | Separate VDM owner | Not a VM session and not applicable. |

## Resulting Contract

`vm_session_storage_initialize` validates the selected profile, creates and
binds the shared resources, applies the selected topology and CMOS seed,
materializes the board's devices, creates Core, then creates product mailbox
and debugger.  Any stage failure invokes `vm_session_storage_finalize`; no
Model 40-specific resource cleanup remains.

The Model 40 contribution is deliberately narrow: it validates its external
ROM contract, writes a copied topology, and configures its D4 memory and
controllers into the supplied plan.  It neither allocates a plan nor owns a
session resource lifetime.

## Regression Receivers

- `unit.vm-model40-private-composition-s7-smoke`: Model 40 external-ROM,
  D4/EGA/controller, CMOS and reset-facing construction.
- `unit.vm-session-initialization-atomicity-smoke`: generic construction and
  failure nonpublication.
- `unit.vm-two-session-isolation-smoke`: independent lifecycle ownership.
- The prior 304-test executable set predates this source change after the
  CMake reconfiguration and is therefore not T520 evidence. The complete unit
  suite must be rerun only after the configured build regenerates its targets.

S2 will add the full cross-profile construction/failure matrix and external-ROM
integration closure.  This ledger does not claim those task-level results.
