# M5 T203: Console Adapter Closure

## Contract

The Console machine-provider vtable remains the VM product/composition
boundary. `vm_session_borrow_selected()` is the only composition/session source
that directly borrows the selected session from the core session manager. It
owns no state. Console callbacks preserve their former null, error, pause, and
debugger behavior.

## Evidence

- `verify-console-adapter-closure` checks the whole composition/session source
  set for one direct selected-session borrow, requires adapter use of the typed
  helper, rejects obsolete local wrappers, and checks retained debugger steps.
- `cmake --build --preset current-gates-gcc --parallel 1`: 42/42 CTest smoke
  tests passed and all 17 static/structure gates passed, including
  `M5:T203:S1:CONSOLE-ADAPTER-CLOSURE:OK`.
- `cmake --build --preset current-gcc --parallel 1`; piped Console `EXIT`:
  exit status 0. Artifact:
  `build/output/nxvm_0_5_0203.exe`; SHA-256
  `57038D581B0E2C7D2B866F0FDB5D93F90D14A31D67D67E0C4991FBCB51AFA8F3`.
