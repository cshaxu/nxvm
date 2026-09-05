# T522 S3 Non-UX Capability Admission Ledger

## Scope And Method

This ledger exhausts the current candidates in `core/utils`, `core/platform`,
`vm/composition/session`, `vm/platform`, and `vm/machine` that could otherwise
be mistaken for a reusable `host`, `session`, `storage`, or `observability`
library. A candidate is admitted only when its complete state, public
operation, failure boundary and caller replacement can be independent of Core,
VM, profiles, firmware, controllers and another library component.

## Dispositions

| Candidate family | Current sole owner | Disposition | Evidence-based reason |
| --- | --- | --- | --- |
| `core_utils_wait_scope` / `core_utils_wait_milliseconds` | Core utils | Retain in Core | It is an injected wait scope used by Core/VM timing. A `lib/host` move would make Core consume a new library root without a second product caller or a complete host synchronization implementation. |
| `core_platform_{sleep,wait,file}` | Core platform | Retain in Core | Sleep/wait semantics are Core timing behavior; file operations are Core's existing neutral platform capability. Neither has a duplicate independent implementation to replace, and extracting only a wrapper would add a path. |
| `vm_session_{lifecycle,control,execution,runner,waiting,provider_lifecycle}` | VM session composition | Retain in VM | These paths bind `vm_session`, Core reset/execution, debug, firmware providers, profile construction, request transport, pacing and the host runner. They cannot expose a neutral executor without retaining the real VM lifecycle beside it. |
| `vm_platform_{platform,execution,host_surface,input_flush,request_bridge,virtual_time}` and native run handles | VM platform | Retain for S4 replacement | They carry Core mailbox/input values plus NXVM display policy and session-run ownership. The newly admitted UX surface is their future one-path receiver; S3 must not fork or partially move them. |
| `vm_machine_media_save` | VM machine media persistence | Retain in VM | Its temporary and backup names, pair-commit policy and callers are VM media persistence semantics. A generic storage component would need a separately admitted cross-product byte/overlay contract, not a renamed save routine. |
| session fault outcomes and `vm_machine_debug` trace | VM session / VM machine debug | Retain in VM | They contain Core machine diagnostics and production debugger decisions. A generic `observability` value cannot own or print them without exposing those product types or creating a second trace route. |

## Result

No non-UX component qualifies for extraction in this repository state. No
`lib/host`, `lib/session`, `lib/storage`, or `lib/observability` root is
created. This is a positive simplicity result: NXVM retains one owner for each
candidate, and S4 remains the sole receiver for replacing the existing product
presentation route with `lib/ux`.
