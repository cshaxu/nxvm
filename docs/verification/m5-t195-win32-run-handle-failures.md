# M5 T195 Win32 Run-Handle Failure Verification

## Closed Scope

T195 makes each existing Win32 native startup failure branch reproducible in a
disposable test build. The production error handling and ownership sequence is
unchanged: the start call fails, any already-created worker is stopped and
joined, then the backend finalizer releases native resources and clears the
embedded owner.

## Stage Evidence

Eight independent MinGW trees each built and ran
`vm-platform-win32-startup-failure-smoke`:

```text
M5:T195:S3:WIN32-STARTUP-FAILURE:1:OK  display-thread creation
M5:T195:S3:WIN32-STARTUP-FAILURE:2:OK  window-class registration
M5:T195:S3:WIN32-STARTUP-FAILURE:3:OK  window creation
M5:T195:S3:WIN32-STARTUP-FAILURE:4:OK  window renderer creation
M5:T195:S3:WIN32-STARTUP-FAILURE:5:OK  window kernel-thread creation
M5:T195:S3:WIN32-STARTUP-FAILURE:6:OK  Console renderer creation
M5:T195:S3:WIN32-STARTUP-FAILURE:7:OK  Console kernel-thread creation
M5:T195:S3:WIN32-STARTUP-FAILURE:8:OK  Console display-thread creation
```

Each smoke executes its failure branch twice and requires both results to have
an inactive `vm_platform_run_handle` with a null backend. This catches a
retained backend and, on Console stages, a retained host-surface lease.

The stage definition exists only when the CMake cache value is nonzero. The
normal GCC cache recorded `PROJECT_VM_PLATFORM_TEST_FAILURE_STAGE:STRING=0`
and no release compile command contained the definition.

## Retained Behavior

- `current-gates-gcc` passed all configured source and retained Console gates.
- Normal smokes emitted `M5:T194:S3:RUN-EVENT:OK`,
  `M5:T149:S1:MULTI-WINDOW:OK`, and
  `M5:T73:S1:TWO-SESSION-ISOLATION:OK`.
- The FDD guest check emitted `M5:T70:S2:DOS-PROMPT:OK` with the local,
  uncommitted `D:\\home\\repos.hobby\\fdd.img` fixture.
- `nxvm_0_5_0195.exe` printed its `0.5.0195` identity and accepted piped
  `EXIT`. SHA-256:
  `CFF5EBAD5969FC97E7D53C60F89DBE2A047FFC2040D29584C3FB972136D6CBEE`.

## Cleanup And Limit

The eight `build/t195-stage*` trees are disposable test evidence and are
removed after this record. This is Win32 proof only; Linux requires T196 native
POSIX compilation and runtime evidence before support can be claimed.
