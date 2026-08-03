# M5 T142 S6: Execution Context Alias Cleanup

## Scope

Remove the stale CPU/RAM/port aliases from `vm_session_execution_context` and
make its only retained callback owner an explicit typed `vm_session*`.
Repair the CPU-probe smoke support that still referenced removed `vm_session`
raw aliases but was absent from the current GCC gate preset.

## Result

- `vm_session_execution_context` no longer stores `C_VOID*` CPU, RAM, port,
  or generic device state.
- Its callbacks take `vm_session*`, and the context stores that typed owner as
  its sole session reference.
- `bind_machine_state()` and the unused CPU accessor are deleted; the new
  `bind_session()` establishes only the callback owner.
- CPU-probe support borrows CPU/RAM/execution/instruction state directly from
  its test-owned `core_machine`, rather than referencing removed session
  aliases.

## Verification

Windows GCC configured and built `nxvm-current-gcc`,
`nxvm-current-gates-gcc`, and the previously omitted
`nxvm-vm-cpu-probe-smoke` target. Runtime gates passed:

```text
M5:T1:S1:CPU-PROBES:OK
M5:T10:S4:CONTEXT-LIFECYCLE:OK
M5:T73:S1:TWO-SESSION-ISOLATION:OK
M5:T14:S3:VM-DEBUG-TARGET:OK
M5:T70:S2:DOS-PROMPT:OK
M5:T142:S6:NO-EXECUTION-CONTEXT-ALIASES:OK
```

The CPU-probe's intentional invalid-instruction vector emits its historical
`#UD` diagnostic before its passing marker.

The current task artifact is `build/output/nxvm_0_5_0142.exe`, SHA-256
`6F22B9D48869B771DCE2B01AA515DBD5F6057BACE54CE12C28FF14F803ED2547`.
