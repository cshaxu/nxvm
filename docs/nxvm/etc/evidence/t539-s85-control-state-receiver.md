# T539 S85 Control-State Receiver

## Receiver map

`cpu_control_state_smoke.c` (392 lines) is a complete CPU-only CLTS, SMSW,
LMSW and MOV-CR receiver. It depends only on the established CPU instruction
fixture and was moved to:

`test/x86/devices/cpu/cpu_control_state_smoke.c`

The former NXVM source/target are retired. `machine_control_state_board_smoke`
remains the sole named owner for public board construction and delivery.

## Static-gate repair

The T332 lifecycle inventory is already the unique 44-owner inventory. Its
source values use NXVM-relative paths for App tests and `devices/...` values
for Shared tests. The verifier now resolves the latter under `test/x86/`, so
S83, S84 and S85 Shared instruction receivers are checked by their real files
rather than by stale App paths. This changes no CPU behavior or API.

## Verification

- Focused `x86.cpu_control_state` and its NXVM aggregate registration pass on
  x64 and x86.
- T332 fixture lifecycle and CPU/PIC authority pass on x64 and x86.
- Shared manifest/corpus, documentation governance and diff checks pass.
- Both 438-case unit suites executed. The existing x64
  `unit.vm-runner-error-propagation-smoke` and x86 `x86.cpu_movs` flakes each
  pass immediately in isolated reruns; neither source is in this batch.

No executable input changed, so no product artifact is rebuilt.
