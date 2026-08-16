# T382 S7: Debug Debt Reconciliation

`M5:T382:S7:DEBUG-DEBT-RECONCILIATION:OK`

The current core debug surface has operation-based read/write/patch APIs and
no `core_machine_debug_*_borrow()` declaration or production call. The stale
borrow-retirement TODO is removed. The independent open debt remains: debugger
originated `CR0`--`CR4` mutation currently reaches
`core_machine_debug_write_register()` / `core_machine_debug_patch_registers()`
and needs a separately admitted lifecycle, validation, raw-override, and
regression contract.
