# M5 T142 S1: Bounded Core CPU/RAM/Port Access

## Result

The VM session raw CPU/RAM/port map is removed. An embedded
`vm_composition_machine_access` records only `core_machine*`; its typed
functions borrow the unique core CPU, instruction state, execution context,
RAM, or port object when composition needs it. The record neither allocates
guest storage nor performs lifecycle operations.

VM composition, firmware binding, Console RAM sizing, tests, and the debugger
target now use that contract. The debugger remains a target adapter: product
UI still receives only `core_product_debug_target` operations.

## Verification

Windows GCC built and ran:

```text
M5:T83:S2:CORE-EXECUTOR-STORAGE:OK
M5:T73:S1:TWO-SESSION-ISOLATION:OK
M5:T14:S3:VM-DEBUG-TARGET:OK
M5:T96:S1:CONSOLE-LIFECYCLE:OK
M5:T70:S2:DOS-PROMPT:OK
```

The source scan for `vm_session` CPU/RAM/port raw fields and their member
accesses is empty. Shared-device aliases are intentionally retained only for
the following S2 migration.
