# M5 T80 S1 Verification

## Result

Win32 Console C1 now passes one `vm_platform_run_context` to its kernel,
display, and input work. The context refers to live-machine-owned keyboard and
execution transports; it contains no guest storage. The window and Linux paths
retain their old APIs until their own T80 cutovers.

## Gates

- `nxvm-vm-platform-input-smoke`: `M5:T80:S1:VM-PLATFORM-INPUT:OK`.
- `nxvm-vm-platform-execution-smoke`: `M5:T80:S1:VM-PLATFORM-EXECUTION:OK`.
- `nxvm-vm-dos-prompt-smoke D:\\fdd.img`: `M5:T70:S2:DOS-PROMPT:OK`.
- `nxvm-product-console-smoke`: `M5:T6:S1:NXVM-CONSOLE:OK`.
- `verify-facade-ownership`: `M5:T75:FACADE-OWNERSHIP:OK`.

GCC 16.1 built the retained executable target successfully. T80 is still
active, so no `nxvm_0_5_0080.exe` task artifact is published yet.
