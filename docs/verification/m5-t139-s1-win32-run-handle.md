# M5 T139 S1: Win32 Run Handle

## Result

`vm_session` owns its embedded `vm_platform_run_handle`. The Win32 Console and
window backends allocate one private handle only through that object. Kernel
and display workers never free renderers, surfaces, run contexts, or their
private handle; `vm_platform_run_handle_finalize()` does so after both joins.

Console retains its synchronous run call: `vm_session_resume()` is its joiner.
Window runs return to the retained Console and `vm_session_stop()` is their
joiner. Both forms first request guest stop. A stopped window handle is also
joined and finalized before a later reset can begin another run.

## Verification

Windows GCC configured and built:

```text
cmake --preset mingw-gcc-x64
cmake --build build/mingw-gcc-x64 --target \
  nxvm-vm-platform-run-handle-smoke nxvm-vm-dos-prompt-smoke \
  nxvm-product-console-lifecycle-gate nxvm-vm-two-session-isolation-smoke \
  nxvm-0-5-0139
```

Observed markers:

```text
M5:T139:S1:RUN-HANDLE:OK
M5:T96:S1:CONSOLE-LIFECYCLE:OK
M5:T73:S1:TWO-SESSION-ISOLATION:OK
M5:T70:S2:DOS-PROMPT:OK
```

The run-handle smoke starts, cancels, joins, finalizes, then starts and cancels
the window path again. Its postconditions require an inactive owner and null
backend pointer. Static source review confirms that `CloseHandle`, renderer
finalization, and `STD_FREE(handle)` occur only in the two backend finalize
functions. Each second-worker-create failure follows request-stop, join, and
finalize for every resource already created.

Artifact: `build/output/nxvm_0_5_0139.exe`, a local developer artifact. Its
banner is `Neko's x86 Virtual Machine [0.5.0139]`; SHA-256 is
`883502B885DE9EA0D49FDE7F2AC6F4D51389DDB922525BA1274D66306DA8F7D4`.

## Limits

Linux still contains its former detached-worker model and is deliberately not
claimed by this result; T140 has the parity conversion and POSIX verification
handoff. The retained NXVM window-close outcome is a stopped, reusable NXVM
session; VDM's future process cancellation result is a separate product rule.
