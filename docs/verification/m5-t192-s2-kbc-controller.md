# M5 T192 S2 8042 Controller Verification

## Scope

`core/machine/kbc.*` implements the bounded 8042 controller subset approved
by T192 S1: ports `0x60`/`0x64`, OBF/IBF/status, command byte, commands
`20/60/AA/AB/AD/AE/D0/D1`, keyboard ACK/reset/enable/disable/identify,
IRQ1, output-port A20/reset, and one fixed 16-byte FIFO. It does not migrate
host input; the established direct QDKEYB route remains until T192 S3.

## Evidence

- `core-machine-kbc-controller-smoke` emitted
  `M5:T192:S2:KBC-CONTROLLER:OK`. It probes command/data ordering, command
  byte, OBF consumption, port enable/disable, IRQ1, keyboard responses,
  A20/reset output-port effects, FIFO order, and full-FIFO
  `NTVDM64_STATUS_INVALID_STATE`.
- `cmake --build --preset current-gates-gcc --parallel 1` passed, including
  the new KBC smoke and retained dependency, lifecycle, executor, session,
  provider-composition, and Console-lifecycle gates.
- Retained guest checks with the local developer fixtures emitted
  `M5:T70:S2:DOS-PROMPT:OK`, `M5:T151:S2:DOS-KEYBOARD:OK`,
  `M5:T7:S1:NXVM-SESSION:OK`, and `M5:T13:S8:VM-SESSION:OK`.

## Exit Review

The controller has one core owner and binds only core PIC/RAM/CPU execution
objects. No VM/profile dependency, host handle, thread, second queue, or
Console/debugger behavior change was introduced. AUX/IRQ12, mouse,
translation, scan-code-set switching, controller timing, and break-byte
emission remain outside this slice. T192 stays open for S3, so no task artifact
is produced yet.
