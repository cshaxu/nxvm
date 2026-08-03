# M5 Unique Owner Convergence

## Objective

Close the remaining gap between the documented session model and the active
implementation. A runtime datum has one owner, a product has one construction
and execution route, and test helpers cannot masquerade as product runtime
layers. This plan preserves the retained NXVM Console, debugger grammar, boot
sequence, and fixture behavior. It does not begin M6 or add a VDM product path.

## Baseline Rules

- `vm_session` is the only NXVM session root. Its `core_machine`, VM device
  storage, platform context, and product contexts are one ownership graph.
- `core_machine` has one production executor. Test construction cannot create
  a second production run semantic.
- A product UI or debugger operation receives explicit state. Thread-local
  data may be a short-lived implementation detail only after it has no public
  or command-level current-object meaning.
- `vdm_session` is the sole future VDM composition entry. VDM presentation
  does not bypass it to reach a machine object.
- Immutable profile descriptions may be shared. A registry stays only when a
  real composition route consumes it.

## Tasks

| Task | Scope | Completion gate |
| --- | --- | --- |
| T119 | Record the unique-owner baseline, the formal product paths, prohibited wrappers, and targeted regression gates. | Documentation review and current structural scans. |
| T120 | Give VM initialization exactly one owner and make repeat initialization explicit/idempotent or invalid. | Console lifecycle, boot, debugger, and DOS-prompt gates. |
| T121 | Make the core executor the only production `core_machine_run` state model; isolate minimal construction to tests or a declared test mode. | Core executor/lifecycle/trace and VM boot gates. |
| T122 | Remove product-source test models: move CPU probe and alternate Console/debugger facades into test support or delete them. | Console/debugger, CPU probe, and two-session gates. |
| T123 | Replace command-level TLS current-object facades in debug/wait/Console with explicit operation state and restore-safe nesting. | Debugger, pause, nested scope, and retained Console gates. |
| T124 | Make VDM minimal machine ownership and presentation flow pass only through `vdm_session`; rename its VDM-owned API. | VDM minimal/profile/presentation gates. |
| T125 | Remove or converge unused runtime profile descriptor and registry layers after proving the actual profile composition contract. | Profile and default-firmware gates plus no dead runtime consumer scan. |
| T126 | Delete remaining no-op or ownership-ambiguous platform/machine facades and document the host Console lease as a process resource. | Platform, Console/window, and two-session gates. |
| T127 | Independently re-audit all module roots, CMake targets, main entries, test-only paths, statics, and TLS against this plan. | Full structural scan, focused CTest suite, and a final evidence matrix. |

## Stop Conditions

Stop the active task if it would change NXVM Console grammar, debugger UI,
startup sequence, host display behavior, or FDD/HDD boot outcome. Do not
replace a wrapper merely because it has test coverage: first prove whether it
belongs to the formal product graph. Each runnable-path task publishes one
`nxvm_0_5_NNNN.exe` artifact and records its hash and behavior evidence.
