# M5 T80 S2 Verification

## Result

Win32 window C2 now passes one host-only run context to the kernel/display
threads and stores the same pointer in `GWLP_USERDATA`. Window message handling
uses that context for execution and keyboard dispatch. `w32aHWnd` remains for
the renderer only and is deferred to C5.

## Gates

- GCC 16.1 built the retained NXVM target and focused transport/FDD tests.
- Input transport: `M5:T80:S1:VM-PLATFORM-INPUT:OK`.
- Execution transport: `M5:T80:S1:VM-PLATFORM-EXECUTION:OK`.
- Console: `M5:T6:S1:NXVM-CONSOLE:OK`.
- FDD: `M5:T70:S2:DOS-PROMPT:OK`.
- Facade scan: `M5:T75:FACADE-OWNERSHIP:OK`.
- Controlled window FDD smoke: select window display, insert `D:\\fdd.img`,
  run for three seconds, send `stop`, then `exit`; process exited `0` within
  the 15-second budget.

T80 remains active, so no task artifact is published yet.
