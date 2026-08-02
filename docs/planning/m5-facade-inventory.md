# M5 Residual Facade Inventory

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
| Core keyboard facade | `core_machine_keyboard_*` global binding has no production caller after T76; full-PC host requests use their owning provider slot. | Delete the unused legacy facade after Story 1 closure review. | T81 |
| Core display facade | `core_machine_display_*` global callback has no production caller after T77; QDCGA notifies its context's provider slot. | Delete the unused legacy facade after Story 1 closure review. | T81 |
| Display generation | Stored on `vm_composition_live_machine` after T77. | The owning VM display context. | Complete |
| Debug target | `core_product_debug_target.c` stores one process-global target selected by VM composition. | The composition-owned debug target passed through the existing debugger UI. | T78 |
| Wait provider | `core_product_wait.c` stores one global provider and context. | The VM session host-wait transport. | T79 |
| VM input sink | `vm/platform/input.c` stores one keyboard sink and context. | The VM session input transport. | T79 |
| VM execution sink | `vm/platform/execution.c` stores one execution sink and context. | The VM session execution transport. | T79 |
| VM platform mode | `vm/platform/platform.c` owns one global `t_platform` mode state. | A VM platform/session object. | T80 |
| Display frame mailbox | `core/platform/display_frame.c` stores one global frame and lock; composition publishes and platform renderers capture it. | The owning VM host-display adapter, without a platform-to-machine dependency. | T80 |
| Host window and Console state | Win32 state includes static keyboard sink/context, window handle, display thread state, and Console handles. | Explicit VM platform/window or Console objects. | T80 |
| Block facade | `core_machine_block_*` global provider storage has no production caller; explicit `*_from(slot, ...)` calls are used instead. | Delete the unused legacy facade. | T81 |
| Debugger UI working state | `core/product/debug/debug.c` has retained parser/UI state. It does not itself select the current machine, but must be rechecked when the target becomes explicit. | Retained single NXVM debugger UI until T78 completes. | T78 |

## Separate Executor Debt

The minimal `core_machine_create/run` scaffold and the real
`vm_composition_live_machine` executor are separate machine paths. This is not
a facade removal; T82--T86 make `core_machine_run()` the only guest execution
path after Story 1 closes.

## T75 Static Gate

`tools/VerifyFacadeOwnership.ps1` limits each recorded legacy facade to its
known implementation and current production call sites. It rejects any new
call site. The gate is containment only: it does not claim that a facade is
already removed or that NXVM is a multi-session product.
