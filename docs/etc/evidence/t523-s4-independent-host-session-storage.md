# T523 S4 Independent Host, Session And Storage Ledger

## Scope

This record covers the S4 replacement of generic host synchronization,
lifecycle transport and byte-image mechanics. It does not move a Core turn,
machine safe point, controller, profile, media topology or guest state into
`src/lib`.

## Operation Disposition

| Capability | Shared owner and public shape | NXVM binding disposition |
| --- | --- | --- |
| Native wait | `host_sync_event` is opaque; create/signal/reset/wait use no native public type. | The former 20 ms predicate-poll helper is deleted. |
| Cancellation and join | `host_sync_task` owns one native thread and cancellation event; task entry receives only an opaque task. | Win32/Linux platform run handles use two host tasks and join/destroy them; raw `HANDLE`/`pthread_t` routes are removed. |
| Start acknowledgement | `lib_session_executor_start` changes generic state, invokes a copied callback, then enters product run. | The kernel task signals a `host_sync_event` before the product runner starts; presenter startup waits on that event rather than polling `flip`. |
| Stop wake | `ux_mailbox_wake` wakes a native loop without publishing another frame. | Run-handle stop wakes the sole presenter mailbox, allowing its existing loop to observe the terminal lifecycle state and join. |
| Lifecycle | `lib_session_state` is opaque and owns reset/pause/step/lifecycle/flip atomics; executor queries it directly. | The prior product `get_lifecycle`/`get_flip` forwarding callbacks are deleted. The VM runner alone acknowledges pause after its Core-safe snapshot boundary. |
| Image modes | `lib_storage_image` supplies direct-readonly, direct-writable and private overlay byte ownership, plus explicit commit/discard. | FDD/HDD topology, geometry and pair-commit policy remain VM-owned. |
| File mechanics | 64-bit seek and exclusive-create helpers are native-private storage implementations. | No platform headers remain in the public `lib/base` header. |

## Verification To Date

- Table-driven repository-only host event/task cancellation, session lifecycle,
  storage-mode and mailbox lifecycle-wake smoke: 5/5 pass.
- Rebuilt external-YAML `vm-platform-run-handle-smoke`: pass. It verifies
  kernel start, host-key ingress, stop, join, reset and second start through
  the new event/task path.
- Complete library manifest: 56 source files are listed and SHA-256 verified;
  `src/lib` has no NXVM/SoftPC/NTVDM/type-facade vocabulary or product header.
- Full repository-only unit suite: 309/309 pass after the direct-control
  lifecycle entry was made idempotent with the executor-owned state boundary.
- Documentation governance and `git diff --check`: pass.

## Review Record

- The implementation delta is +393 production lines and +83 repository-only
  test lines before this evidence record. The added native host implementations
  replace the deleted generic polling route; no adapter or compatibility layer
  remains.
- Product-vocabulary sweep:
  `rg -n -i "nxvm|softpc|ntvdm64|type\\.h|\\btype_|\\bSTD_|\\bC_VOID|\\bC_INT|\\bC_CHAR|\\bTYPE_" src/lib -g "*.c" -g "*.h"`
  has no matches.
- Peer-root include sweep:
  `rg -n '#include "lib/(ux|host|session|storage|observability)/' src/lib -g "*.c" -g "*.h"`
  shows only same-root includes; each root may use `lib/base` but no peer root.
