# T522 S6: Complete Shared-Capability Ledger

This ledger supersedes the earlier UX-only interpretation of T522.  The owner
requires five independent library components.  Each component has a bounded
current receiver; no component is an empty directory or a product-policy
container.

| Component | Current candidate | Shared contract and sole owner | Explicitly retained product/Core facts | Receiver and deletion boundary | Regression owner |
| --- | --- | --- | --- | --- | --- |
| `ux` | `lib/ux/*`, `vm/platform/ux_binding.c` | `lib/ux` owns copied presentation, native loops and host-event values; VM binding owns conversion to product input. | Core display state, VADP, session choice, guest input translation. | Completed S1--S5; S6 adds RDP UTF-16 proof. | UX contract, VM binding, Console lifecycle, multi-window and mouse integration tests. |
| `host` | `core/platform/{sleep,wait}.{c,h}`, platform-specific sleep implementations and VM's wait consumers | `lib/host/sync` owns cancellation-aware host wait/yield contracts and native implementations. | Core scheduler/time semantics, VM pacing policy, UI loops and session selection. | S7 moves the generic wait/sync mechanism; removes the replaced Core-platform generic host helper route rather than duplicating it. | Existing Core wait test plus new owner-local host-sync tests and all consumers. |
| `session` | `vm/composition/session/control.*`, the lifecycle portions of `lifecycle.c`, `vm/platform/execution.*` | `lib/session` owns a serial state machine and bounded executor callbacks; it has no machine pointer in its public API. | VM session construction, Core reset/stop, debug refresh, profile/firmware selection, command policy and display lifecycle. | S8 separates state transitions from VM callbacks, migrates callers, then deletes the superseded VM control/transport state mechanism. | New lib-session state tests plus VM session lifecycle and product-console integration tests. |
| `storage` | `vm/machine/media_save.c`, YAML/session copy-on-write overlay support | `lib/storage` owns byte-image staging, overlay and atomic commit/rollback. | FDD/HDD geometry, CHS, controller media protocol, sidecars, media topology and session selection. | S9 migrates generic persistence/overlay operations, preserving one device-media consumer path; deletes `media_save` after callers move. | New storage tests plus existing FDD/HDD/media lifecycle and external-YAML integration tests. |
| `observability` | copied VM fault/start outcomes and bounded Core trace provider delivery | `lib/observability` owns a bounded copied outcome publication primitive; producers retain their typed facts. | Core trace event meaning, CPU diagnostics, VM debugger/Console formatting and pause policy. | S10 gives VM/Core an adapter to one copied outcome channel, removes generic duplicate transport helpers, but retains typed trace/fault owners. | New observability contract tests plus debug/fault/session regressions. |

## Similar-Issue Sweep

The initial source sweep covers all tracked `src/core` and `src/vm` files whose
path or symbol names contain `wait`, `sleep`, `thread`, `sync`, `session`,
`control`, `lifecycle`, `image`, `overlay`, `media`, `file`, `trace`,
`outcome`, `fault` or `diagnostic`.  Hits above are migration candidates.
Controller media implementations, Core scheduler and transaction trace,
profile/firmware construction, VM debugger policy and native UX loops are
explicitly excluded because each owns product or hardware semantics rather
than a reusable host/value mechanism.

## RDP Proof

`test/vm/platform/vm_platform_win32_host_action_smoke.c` now sends a UTF-16
character through `ux_win32_keyboard_submit_utf16()` and the existing binding
sink.  The test accepts only the resulting VM guest key make/break events.
It does not call a Core keyboard API, mutate a queue or add an RDP path.

## T522 Completion Predicate

S7--S10 must each replace their complete receiver mechanism, delete its old
generic route, retain only the documented product/Core facts, and pass their
owner regressions.  T522 may close only after the final source sweep shows one
production route for all five components, full unit and external-YAML
integration pass, and fresh stripped x64/x86 artifacts are verified.
