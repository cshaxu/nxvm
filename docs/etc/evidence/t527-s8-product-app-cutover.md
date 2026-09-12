# T527 S8: NXVM Product And App Cutover

## Result

`vm/product` is the sole NXVM owner of YAML/profile selection, monitor CLI and
Debugger entry, recorder policy, lifecycle policy, hotkeys, guest-input mapping,
title policy and user-visible Console text. `vm/app` is limited to one
composition responsibility: it creates/destroys the NXVM machine adapter and
Common session, then translates copied machine results to copied Common facts
and frames.

| Previous route | Final route |
| --- | --- |
| `vm/events/machine_event.h` | `vm/machine/runtime/event_interface.h`, next to the producer/consumer adapter |
| `vm/events/session_request.h` | `vm/machine/request_interface.h`, the frozen product-to-machine construction contract |
| app command, Debug, lifecycle and input policy | `vm/product/console.c` |
| app machine-result callback | `vm/app/app.c` composition fact/frame adapter |

No compatibility facade or forwarding route remains. The moved contracts are
copied values only and retain no Core, Common-session or native UI pointer.
`vm/machine` remains the sole NXVM Core assembly, execution, media/provider and
copied Core-fact adapter owner.

## Verification

- Focused product Console unit and lifecycle smoke tests pass.
- Focused Model 40 Console integration smoke passes.
- Source/CMake scans find no `vm/events` path or former product-policy
  `vm_app_*` bridge.
- Full x64 Release repository-only unit suite passed: **298/298**.
- The specialized ownership-gate aggregate, dependency DAG, documentation
  governance and `git diff --check` all passed in the isolated x64 build.

Implementation commits: `a8d17a39` and corrective ownership commit
`1a6bb158`. Coordinator closure: S8 P3.
