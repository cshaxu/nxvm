# M5 T142 S5: No Forwarding Core-Borrow Facade

## Scope

Delete the pure `vm_composition_machine_access` forwarding layer that exposed
nearly every `core_machine` object through a VM-owned getter. This is a
structural correction only: it does not add guest state, a VM executor, or a
new Console/debugger behavior.

## Result

- Deleted `src/vm/composition/machine_access.h` and `.c`.
- Deleted `vm_session`'s embedded `core_access_storage` and `core_access`
  pointer.
- Moved the non-owning borrow declarations from private `machine.h` to
  core-owned `machine_interface.h`.
- Root `vm/composition` now calls the core owner directly only for actual
  provider/profile binding, session initialization, and existing debugger
  target construction. VM machine, platform, product, and profile peers have
  no raw-core borrow call.

## Verification

Windows GCC configured and built `nxvm-current-gcc` and
`nxvm-current-gates-gcc` successfully. The focused runtime gates passed:

```text
M5:T83:S2:CORE-EXECUTOR-STORAGE:OK
M5:T73:S1:TWO-SESSION-ISOLATION:OK
M5:T14:S3:VM-DEBUG-TARGET:OK
M5:T45:S1:PAUSE-BOUNDARY:OK
M5:T139:S1:RUN-HANDLE:OK
M5:T70:S2:DOS-PROMPT:OK
M5:T142:S5:NO-FORWARDING-FACADE:OK
M5:T142:S5:COMPOSITION-ONLY-BORROW:OK
```

The latter source audit found no `machine_access`/`core_access` symbol and no
production `core_machine_*_borrow` caller outside `src/vm/composition/`.

The S5 task artifact was `build/output/nxvm_0_5_0142.exe`, SHA-256
`1E76A4D338CED55494245CBCCA5A5112EAAB0EC4D1CC511559B7B8EEFF5E5EC6`.
S6 rebuilt the same task-level artifact and records its current hash.
