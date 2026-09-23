# M5 T531 S18 - Common-Machine Composition Ownership

## Result

`vm/app/composition` now is the only NXVM owner that constructs and destroys a
`common_machine`.  `vm/machine` creates the product adapter and describes its
single value-only `common_machine_driver`; it retains only a non-owning bound
reference while Common is alive.

The App construction order is:

```
vm_machine_create -> describe driver -> common_machine_create -> bind driver
-> common_session_create -> common_ui_create
```

The inverse teardown is explicit and dependency-safe:

```
common_ui_destroy -> common_session_destroy -> common_machine_destroy
-> revoke VM binding -> vm_machine_destroy
```

No VM profile, Core behavior, Common/Lib implementation, or second executor
path changed.  Startup media setup now uses VM-owned direct setup before the
App-owned Common lifecycle exists; runtime removable-media requests continue
through Common.

## Test Adaptation

The four unit fixtures that intentionally exercise lifecycle, debug, display
cadence, or runtime removable media now explicitly build the same driver and
Common-machine relation as App composition.  This removes their former hidden
assumption that `vm_machine_create` owned a worker, while keeping their test
scope local and their VM/Core assertions unchanged.

## Verification

- `cmake -DPROJECT_SOURCE_DIR=O:/repos.hobby/nxvm -P
  cmake/verify_vm_machine_owner.cmake` passed.
- `cmake -DPROJECT_SOURCE_DIR=O:/repos.hobby/nxvm -P
  cmake/verify_vm_machine_lifecycle.cmake` passed.
- Focused lifecycle/media/debug/display fixtures passed.
- `ctest --test-dir build/mingw-gcc-x64-release -L unit -j 8
  --output-on-failure`: **325/325 passed** in 15.78 seconds.
- Stripped `nxvm_0_5_0531_x64.exe` and `_x86.exe` were built and architecture
  verified in both `build/output/` and `assets/sessions/`.
  - x64 SHA-256:
    `B126FFE55ACC28440B60D278C3165EA449DFE7E96E32C1BFB75AC1AE8D92F427`
  - x86 SHA-256:
    `04D180B982F8783EECCFD0242334C670F458CBE26F500F4360F868315A854BA4`

The host's ordinary Ninja worker can stall without a compiler child.  The
accepted artifacts were therefore compiled/linked through the generated
toolchain commands, then architecture-verified; this does not change source
or build inputs.

S18 is accepted after actual-diff review.  T531 remains open for later,
separately admitted convergence work.
