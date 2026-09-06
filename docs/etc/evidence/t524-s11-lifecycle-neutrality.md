# M5 T524 S11: Lifecycle Neutrality

## Former leakage and retained ownership

`lib/session/state` formerly stored an NXVM debugger single-step flag, a
numeric pause reason, a start-toggle `flip`, and a `fault` alias for `stop`.
They are not a neutral lifecycle contract: only NXVM's debugger/control/runner
path consumed the first two, no caller consumed `flip`, and only NXVM used the
fault alias.

The retained path is now:

`NXVM debugger or product request -> vm_session_control -> lib lifecycle -> runner acknowledgement`

`vm_session_control_state` is the sole owner of the atomic step flag and pause
reason. `vm_session_control_step()` sets its flag before generic resume; the
runner caps that turn at one instruction, consumes the flag, and requests a
generic lifecycle pause with the NXVM-only `STEP` reason. Normal continue,
stop, fault and an explicit pause clear the flag; pause reason remains a
control fact. `lib/session` owns only start/stop/reset/pause/acknowledge/resume
state.

## Complete disposition

| Former surface | Disposition |
| --- | --- |
| `lib_session_state_request_step`, `take_step`, `step_requested` | Deleted; NXVM control owns equivalent atomic flag and runner consumption. |
| `lib_session_state_request_pause(... reason)`, `pause_reason` | Replaced by generic pause; reason moves solely to NXVM control. |
| `lib_session_state_flip` / state field | Deleted; whole source sweep found no consumer. |
| `lib_session_state_fault` | Deleted as an NXVM-only stop alias; control now calls generic stop. |

## Evidence

- Current-source focused CTest: `unit.session-state-smoke`,
  `unit.lib-neutral-consumer`, `integration.vm-debug-pause-boundary-smoke`, and
  `integration.vm-unified-debug-backend-smoke` pass 4/4.
- Complete unit replay passes 311/311.
- `cmake -DLIBRARY_ROOT:PATH=src/lib -P src/lib/verify_manifest.cmake` passes.
- A complete `src/lib` and consumer sweep has no old step/reason/flip/fault
  symbol. The public manifest is revision `m5-t524-s11-p1`.
- Current-source optimized Release artifacts pass their x64/x86 PE gates:
  `build/output/nxvm_0_5_0522_x64.exe`
  (`1523B7AA112FB4F74C930FF510E6E6541D9EF9859D052186EF9BB85E165927CF`)
  and `build/output/nxvm_0_5_0522_x86.exe`
  (`CCDB21AEC62FCA2236277B2885D881F43BED2C4388AE5DBA6DE758325F40F1E7`).

## Simplicity result

Tracked implementation and test code changes add 43 and remove 78 lines
(`git diff --numstat` excluding manifests, README and documentation): net
minus 35 lines.
The sole retained production path has one lifecycle owner in `lib` and one
debugger-policy owner in NXVM control; no compatibility wrapper or second
state copy remains.
