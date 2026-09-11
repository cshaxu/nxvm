# T527 S4 - Common Machine Cutover

## Outcome

`common/machine` now owns the only bounded copied request FIFO between NXVM
control and the Core execution safe point. The former VM executor queue and
FIFO are deleted; `vm/machine` is its one opaque driver.

| Former route | S4 owner | Disposition |
| --- | --- | --- |
| `vm/machine/executor_queue` bounded storage | `common/machine` | Deleted. |
| `vm/machine/executor_fifo` locking, wake and one-request safe point | `common/machine` | Deleted. |
| Core command-boundary binding | `common_machine_observe_safe_point` | Retained once through the VM driver. |
| paused register/memory/port access | `common/machine` bounded lease plus VM driver | New neutral route; no Core pointer crosses common. |
| old `core_debug_target` callback table | S5 receiver | Retained only for the old Core Debug parser until its approved complete migration. |

## Contract

`common/machine` accepts copied input and lifecycle requests, delivers exactly
one request per safe point, reports close/overflow explicitly, and keeps its
host event private. A driver receives an opaque context and is the only code
that can touch a product machine.

`common/session` remains the sole run-id allocator. At each accepted session
run, the VM binds that value to `common/machine`; the latter uses only this
copied value to stamp omitted request ids and reject explicit stale requests.
It does not allocate, advance, or interpret session generations.

Paused Debug operations are bounded to a register, a 32-byte linear transfer
or a port value. A lease is acquired only while the driver reports paused; its
generation is invalidated before a queued resume/reset/stop is delivered, and
on direct reset/stop/new-run paths. Execution with a stale lease returns
`LIB_STATUS_INVALID_STATE`.

## Source And Test Accounting

- Added: `src/common/machine/{machine,machine_interface}.c`, one owner-local
  smoke.
- Deleted: four VM executor source/header paths and their two owner-local
  smoke tests.
- Migrated all direct safe-point test callers to the new owner. VM Debug
  smoke proves a real paused Core read and stale-lease rejection; common
  machine smoke proves stale-run rejection.
- Retained VM input/display result values because S7 owns the final
  `vm/events` removal; no executor request value remains there.

## Verification

- Focused common FIFO, host-input, KBC, Model-40 and paused-Debug tests: pass.
- Complete repository-only unit suite: 298 passed, 0 failed (`ctest -L unit -j 4`).
- `verify-dependency-dag`, `verify-product-session-manager`,
  `verify-session-readiness`, `verify-unit-separation`,
  `verify-t344-unit-registration`, `verify-documentation-governance` and
  `git diff --check`: pass.

## Boundary

S4 does not alter machine semantics, firmware, media, profiles, native UX or
Debug grammar. S5 is the explicit receiver for moving the former Debug parser
and callback table to `common/debug`; no forwarding compatibility route was
added here.
