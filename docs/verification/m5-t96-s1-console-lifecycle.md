# M5 T96 S1: Console Lifecycle Regression

## Defect

`nxvm_0_5_0091.exe` accepted `device fdd insert D:\\fdd.img`, while
`nxvm_0_5_0092.exe` through `nxvm_0_5_0095.exe` faulted on the first
non-`exit` Console command with `0xC0000005`.

T93 made the Console context a member of `vm_composition_live_machine`. The
retained Console then called `machineInit`, which rebuilt the same live machine
and zeroed that active context. Its argument array and target disappeared
before the first command was parsed.

## Repair

- `vm_composition_live_machine_initialize` is now single-lifetime and
  idempotent.
- `machineInit` continues setup on an already-created live machine instead of
  recreating it.
- `main` does not finalize a session already finalized by the Console target.
- The live-machine finalizer detaches the CPU extension before clearing its
  pointer and is safe to call after a completed teardown.

## Verification

- `nxvm_0_5_0096.exe` accepted `help`, `device fdd insert D:\\fdd.img`,
  `info`, and `exit`; FDD state reported `inserted`.
- `nxvm-product-console-lifecycle-gate` runs the retained Console with real
  `help`, `info`, and `exit` input and emitted
  `M5:T96:S1:CONSOLE-LIFECYCLE:OK`.
- `nxvm-current-gates-gcc`: passed.
- `nxvm-vm-dos-prompt-smoke D:\\fdd.img`:
  `M5:T70:S2:DOS-PROMPT:OK`.
- `build/output/nxvm_0_5_0096.exe` SHA-256:
  `AE7F3F48D80725FE192DD6ADD11D7117B33FC4C93A65CA39AE24837ED564E45D`.

`nxvm_0_5_0092.exe` through `nxvm_0_5_0095.exe` are revoked. Use
`nxvm_0_5_0096.exe` or `nxvm_0_5_0091.exe` for retained Console work.
