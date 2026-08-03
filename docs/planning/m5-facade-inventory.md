# M5 Residual Facade Inventory

> Historical baseline: rows 19--20 describe the transient TLS scopes present
> during T75. T129--T132 subsequently removed them from production paths.
> `RULES.md` is the forward rule; this inventory remains evidence of the
> migration rather than a permission to restore TLS selection.

## Purpose

This ledger is the T75 baseline for removing process-global facade state from
the retained NXVM path. A facade is in scope when module-global mutable state
implicitly selects a VM session, provider, host surface, or guest service.
Immutable lookup tables and short-lived local parser state are not facades.

T75 only records and constrains the baseline. T76--T81 replace or delete the
listed paths while preserving the retained NXVM Console, debugger, boot flow,
and FDD DOS-prompt gate.

| Family | Current state and production use | Required explicit owner | Task |
| --- | --- | --- | --- |
| Core keyboard facade | Deleted after T76 proved the provider-slot route is the only production route. | Provider slot. | Complete |
| Core display facade | Deleted after T77 proved QDCGA uses its context provider slot. | Provider slot. | Complete |
| Display generation | Stored on `vm_composition_live_machine` after T77. | The owning VM display context. | Complete |
| Debug target | T78 removed lifecycle-wide target binding; T129 later made debugger invocation state explicit. | The composition-owned debug target passed explicitly into the retained debugger UI. | Complete |
| Wait provider | T131 later replaced the transient scope with explicit propagation. | VM run-context wait scope passed explicitly. | Complete |
| VM input sink | Deleted; explicit transport is the only path. | VM session input transport. | Complete |
| VM execution sink | Deleted; explicit transport is the only path. | VM session execution transport. | Complete |
| VM platform mode | Stored in the VM platform run context. | VM platform run context. | Complete |
| Display frame mailbox | Deleted global mailbox; each VM owns a presentation mailbox. | VM host-display adapter. | Complete |
| Host window and Console state | Handles and keyboard-state callback live in the run context. | VM platform/window or Console object. | Complete |
| Block facade | Deleted; firmware uses explicit `*_from(slot, ...)` calls. | Provider slot. | Complete |
| Debugger UI working state | `core/product/debug/debug.c` has retained parser/UI state. It does not itself select the current machine, but must be rechecked when the target becomes explicit. | Retained single NXVM debugger UI until T78 completes. | T78 |

## Separate Executor Debt

The minimal `core_machine_create/run` scaffold and the real
`vm_composition_live_machine` executor are separate machine paths. This is not
a facade removal; T82--T86 make `core_machine_run()` the only guest execution
path after Story 1 closes.

## T75 Static Gate

`tools/VerifyFacadeOwnership.ps1` rejects every retired facade in production
sources. It proves Story 1 closure only; it does not claim multi-session
product support or resolve the parallel executor debt.
