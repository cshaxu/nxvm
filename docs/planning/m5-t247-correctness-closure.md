# M5 T247: Correctness Closure

## Status

**Complete.** T248 S1 is the sole active subtask.

## Scope

T247 closes four proven defects without changing NXVM Console, debugger, boot,
or guest compatibility behavior:

1. Preserve the Win32 `E0` extension marker through both Window and Console
   host-input adapters.
2. Bound the Console and Linux execution-flip startup wait. Timeout requests
   stop, joins every created worker, finalizes the run handle, and returns
   failure. Win32 failure stage 9 injects a kernel that exits without flipping
   execution.
3. Reject every overlapping pair of entry-plan preloads during preflight.
   Caller-order overwrites are not admitted.
4. Compile the current runnable GCC artifact with `-Werror`.

## Boundaries

This task adds no queue, executor, machine/session, host-clock guest-time path,
or NXVM UI behavior change. Linux receives the same bounded lifetime logic;
Windows GCC executes the no-flip injection.

## Similar-Issue Sweep

| Defect class | Production hits | Disposition |
| --- | --- | --- |
| E0 scan-code narrowing | Win32 Window and Console adapters | Both use explicit 16-bit conversion. |
| Unbounded execution-flip wait | Win32 Console and Linux Console | Both use the shared bounded execution helper. Window waits only for display readiness. |
| Entry preload ordering | One core entry-plan preflight loop | Pairwise range overlap rejects before commit. |
| Current artifact warning policy | One `add_current_vm_artifact` function | GCC artifacts receive `-Werror`. |

## Evidence

* S1: `vm-platform-win32-keyboard-scan-smoke` asserts both Window and Console
  conversions preserve `0x0148` and leave `0x0048` unchanged.
* S2: Win32 stage 9 proves a no-flip Console kernel cleans up twice without an
  active run handle. The shared helper polls at 100ms against a 5000ms budget,
  avoiding inflated `Sleep(1)` waits. Linux uses the same helper.
* S3: `core-machine-entry-plan-smoke` rejects overlapping RAM preloads before
  CPU/RAM mutation; the current artifact target compiles with `-Werror`.
* S4: `current-gcc` and `verify-current-artifact-target` select
  `vm-0-5-0247`. The corrected full gate passed all 34 static/ownership checks
  and 85/85 CTest cases. Artifact `build/output/nxvm_0_5_0247.exe` SHA-256 is
  `51D45A24C6C41E3D43D5E53E00CF0C3862FF01250F007D1D33787F09C86FECF4`.
