# M5 T141 S1: Standard Executor Creation

## Result

`core_machine_create()` now creates the sole standard executor unconditionally.
The profile enum, `profile_interface.h`, `executor_enabled` state, and public
`core_machine_enable_executor()` selection API are removed. VM, VDM minimal,
and core fixtures provide only `memory_bytes`; that value no longer selects a
machine type.

## Verification

Windows GCC built and ran the focused gate set. Observed markers:

```text
M3:T2:S1:LIFECYCLE:OK
M5:T94:S1:VDM-SESSION-ISOLATION:OK
M5:T73:S1:TWO-SESSION-ISOLATION:OK
M5:T96:S1:CONSOLE-LIFECYCLE:OK
M5:T70:S2:DOS-PROMPT:OK
```

The source scan for `CORE_MACHINE_PROFILE`,
`core_machine_enable_executor`, `executor_enabled`, and `profile_interface`
has no active-source matches. S2, rather than this selection-only step, owns
the remaining core lifecycle migration.
