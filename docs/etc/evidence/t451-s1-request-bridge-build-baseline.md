# T451 S1: VM Request-Bridge Build Baseline

## Frozen Contract And Owner Sweep

`request_bridge.h` is the sole request record declaration. Its current kinds
are `KEY_EVENT`, `MOUSE_EVENT`, `STOP` and `DISPLAY_MODE`; its copied keyboard
payload is `data.key_event`. The bridge is the bounded copied queue owner;
`vm_request_transport` owns locking, close/discard lifecycle and one-request
per execution-boundary delivery. VM session composition is the sole consumer
boundary for keyboard and mouse requests.

The complete source/test/CMake sweep found no `VM_PLATFORM_REQUEST_KEY_PRESS`
or `key_press` reference. The direct bridge smoke, transport smoke, input
producer and session consumer all use `KEY_EVENT/key_event`; no alternate
production request authority exists.

## Build Disposition

The configured build tree is `build/mingw-gcc-x64`, supplied by the
`mingw-gcc-x64` configure preset. The top-level `build/` directory is only a
container and is not a CMake build directory; invoking `cmake --build build`
therefore does not diagnose a source or Ninja failure.

On 2026-08-24, outside the Codex sandbox because CMake must update its build
cache, all of the following passed:

```text
cmake --build --preset current-gcc --parallel 2
cmake --build build/mingw-gcc-x64 --target all --parallel 2
build/mingw-gcc-x64/vm-platform-request-bridge-smoke.exe
ctest --test-dir build/mingw-gcc-x64 -R "request.*(bridge|transport)|(bridge|transport).*request" --output-on-failure
```

The complete build rebuilt 574 work items successfully. The CTest selection
ran `current.vm-request-transport-smoke` and passed 1/1. The direct bridge
executable exited successfully, and its registered target reported no work
after the complete build.

## Disposition And S2 Boundary

The TODO assertion that `vm-platform-request-bridge-smoke` names removed
request fields is stale. The only related current defect is supporting
toolchain documentation that invokes nonexistent `m0-probe-gcc` and describes
removed M1 probe outputs, while `README.md` and `CMakePresets.json` already
declare the current `mingw-gcc-x64` plus `current-gcc` route.

S2 must retire the stale TODO and consumed Queue candidate and make the
supporting toolchain command match that one declared route. It must not change
source, target registration, request ABI, current-gate membership or add a
compatibility path.
