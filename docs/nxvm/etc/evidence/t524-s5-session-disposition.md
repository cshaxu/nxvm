# T524 S5 Session Disposition

## Decision

`lib/session/state` remains the only shared session capability. It is an
opaque request-and-acknowledgement state machine and depends only on `base`.
It owns neither a host thread nor an execution turn.

`lib/session/executor` is removed. Its former start/stop sink merely forwarded
NXVM callbacks around `state`; no second product consumer demonstrated that
this wrapper was shared. Keeping it would have made an NXVM lifecycle binding
look like a portable library contract.

NXVM now owns its one explicit `vm_platform_execution` value. It carries the
already-owned `lib_session_state` plus NXVM-local run and stop callbacks. The
platform starts the neutral state before it signals startup, then invokes the
product run callback; shutdown uses the product stop callback. Frame-only UX
contexts remain valid without this product execution binding, while an actual
platform start requires it.

## Conformance And Sweep

- `test/lib/session_state_smoke.c` and
  `test/lib/library_consumer_conformance.c` are two independent public-header
  neutral consumers of `lib/session/state`.
- No `lib_session_executor`, `execution_transport`, or `session/executor`
  source/test/CMake path remains.
- `src/lib/session` includes only `lib/base`; its vocabulary sweep is empty.
- A fresh full build exposed `ux-mailbox-wake-smoke` constructing an invalid
  copied frame. The test now supplies the required text dimensions, preserving
  the accepted UX rejection of malformed frames rather than weakening it.

## Verification

- Focused state, platform-frame and platform-run checks: 3/3 pass.
- Complete repository unit: 311/311 pass in 19.33 seconds.
- `cmake -DLIBRARY_ROOT:PATH=src/lib -P src/lib/verify_manifest.cmake`: pass.
- Standalone `src/lib` MinGW Makefiles configure/build and `library.manifest`
  CTest: pass.
- `git diff --check`, manifest, removed-route and session-root direct-dependency
  sweeps: pass.

S8 remains responsible for the whole-corpus vocabulary and Windows/Linux
portable closure; this S makes no claim that those later root batches are done.
