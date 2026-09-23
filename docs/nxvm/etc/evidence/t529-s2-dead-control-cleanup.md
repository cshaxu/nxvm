# M5 T529 S2 Dead-Control Cleanup

Baseline: `6e7b43a4`. The S1 ledger establishes that `common/machine` already
owns NXVM's copied request/run/safe-point/paused-Debug protocol, while the
NXVM bounded Core runner and SoftPC's CCPU timer rendezvous are distinct
product adapter loops. This S therefore removes only the finite local-dead
batch; it adds no Common mechanism.

## Implemented Batch

`src/vm/machine/runtime/control.[ch]` removes exactly two unconsumed controls:

| Removed item | Former lifecycle | Caller result |
| --- | --- | --- |
| `control_changed` | Event create, five signals, destroy and nulling. | No wait or read existed in `src/` or `test/`; all lifecycle writes were dead. |
| `vm_machine_control_wait_for_completion()` | Public declaration and wrapper around `completion_ready`. | No caller existed outside the declaration and definition. |

The live `completion_ready` event remains. `wait_for_pause()` consumes it;
the runner signals it at its existing pause/completion boundary. The step flag,
pause reason and executor-state acknowledgement also remain unchanged.

## Similar-Issue Sweep

The post-change query was:

```powershell
rg -n "\b(control_changed|vm_machine_control_wait_for_completion)\b" src test cmake CMakeLists.txt
```

It returns no production, test or build hit. The complete exported
`vm_machine_control_*` caller inventory leaves every remaining declaration
with at least its implementation plus a real lifecycle/runner/test consumer.
The only remaining VM-machine `host_sync_event` fields are `completion_ready`
and `execution_started`; `execution_started` has create/reset/wait/signal/
destroy consumers in `runtime/lifecycle.c`. No Common or SoftPC file changes
exist, and no executor route was added.

## Verification

- Built `vm-machine-executor-state-smoke` and `vm-control-lifecycle-smoke`;
  both pass (`2/2`).
- Complete repository-only unit suite: `299/299` passed with
  `ctest --test-dir build/mingw-gcc-x64 -L unit -j 8 --output-on-failure`.
- `verify-current-specialized-gates` passes its source, ownership and route
  gates after the task-record correction.
- The active task still inherits the owner-approved T528 external integration
  exception: 39/42 external-ROM integration rows were last known, with the
  three Model-40/IBM-5170 boot rows transferred to the named TODO. This S does
  not change those inputs or paths and does not claim them green.

## Artifact Record

The optimized stripped `0.5.0528` artifacts were rebuilt and copied to both
required developer locations. `objdump -h` finds no `.debug` section.

| Architecture | SHA-256 | PE format |
| --- | --- | --- |
| x64 | `596FD65168391C5C8D22AE0415D12B57F4C12270042DBB2BE7FED6157CD3F4D1` | `pei-x86-64` |
| x86 | `3EDCC295451381DCB33C01EA9BAA0CCC29A2043B2246C8C673F584312A2682DA` | `pei-i386` |

## Actual-Diff Review

Against `6e7b43a4`, the source portion is **0 added / 20 removed lines**:
one state field, one dead wrapper and every lifecycle operation that existed
only for that dead event. The change leaves no forwarding wrapper, duplicate
state or speculative Common abstraction.
