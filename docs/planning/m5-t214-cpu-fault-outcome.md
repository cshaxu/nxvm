# M5 T214: CPU Fault Outcome Handoff

## Goal

Make a core CPU or machine fault a stable, session-owned outcome instead of a
generic runner stop. Preserve the core fault and its diagnostic until an
explicit cold reset; let the retained NXVM Console and debugger inspect one
copied result without borrowing a running machine.

## Contract

`core_machine_run()` returns `TYPE_STATUS_FAULT` with `STOP_FAULT`, the stored
detail, and the current linear PC whenever the core is faulted. A CPU exception
records its first-fault diagnostic, reports the exception mask as `detail`, and
transitions the core to `FAULTED` before the ordinary stop/reset path can run.
The core owns this live state and never depends on VM code.

`vm_session` owns one copied fault outcome: core run result plus CPU diagnostic.
The runner captures it at its single `core_machine_run()` boundary, exits its
normal loop without requesting another core stop or reset, and leaves it valid
until explicit session reset or destruction. The generic debugger target sees a
read-only fault summary; Console status and debugger entry display the same
detail, location, and available CPU-exception fields.

This task does not add CPU instruction support, exception delivery semantics,
or a second fault queue. A faulted machine is inspectable, but `continue` does
not revive it; the established Console `RESET` path is the recovery action.

## Breakdown

### S1: Core Fault Contract

Make the CPU diagnostic callback promote a CPU exception into `CORE_MACHINE_
STOP_FAULT`; preserve detail and diagnostic, and prove no automatic cold reset
occurs in the same run boundary. Keep explicit non-fault stop/reset behavior
unchanged.

### S2: Session And Debug Surface

Add a session-owned copied outcome, a read-only debugger summary, and retained
Console status/debugger reporting. Do not expose core storage through a new
facade or change normal Console startup, commands, or pause behavior.

### S3: Runner Regression And Closure

Force a CPU invalid-opcode fault through the runner-level path. Verify the
runner returns, session result contains `STOP_FAULT`, detail, and first-fault
diagnostic, the core remains faulted, and explicit reset clears the outcome.
Run the current gate. Only S3 produces `nxvm_0_5_0214.exe`.

## Rules And Gates

Applicable rules: core must not depend on VM; one session owns one copied
outcome; no implicit current-object facade or second machine; command/UI access
only occurs after the runner has returned; no automatic reset of a fault.

Focused gate: `vm-fault-outcome-runner-smoke` with marker
`M5:T214:S3:FAULT-OUTCOME:OK`. Retained gate: `current-gates-gcc` and its
current CTest matrix. Stop for a second fault owner, a queue, a core-to-VM
dependency, diagnostic loss before explicit reset, or any Console/debugger
startup regression.

## Result

Completed. A CPU exception now transitions the core directly to `FAULTED` and
returns `TYPE_STATUS_FAULT`, `CORE_MACHINE_STOP_FAULT`, the exception-mask
detail, and the linear PC before the ordinary stop/reset path is considered.
The first diagnostic remains core-owned until explicit cold reset.

`vm_session` copies that result and diagnostic at the sole runner boundary.
The retained Console status and generic debugger target consume only this
session-owned copy after the runner returns. `RESET` clears both the core
diagnostic and the copied outcome; `continue` does not revive a faulted core.

Evidence: `vm-fault-outcome-runner-smoke` reports
`M5:T214:S3:FAULT-OUTCOME:OK`; `current-gates-gcc` passed; the registered
current-gate CTest matrix passed 51/51 tests. Developer artifact:
`build/output/nxvm_0_5_0214.exe`, SHA-256
`17735838DD571F82486D0F2F8C52775F55B5EEC5E26691B9AA0E0CB2D5A3934E`.
