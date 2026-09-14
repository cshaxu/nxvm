# M5 T531 S17 — NXVM App keyboard convergence

## Result

`src/vm/app/keyboard.c` is the sole NXVM App owner of hotkey registration,
neutral lifecycle requests, mouse-release requests, and the two guest chord
sequences (Ctrl+Alt+Del and Alt+Enter).  `app/command.c` retains command
grammar and the required single Common callback-context adapter only; it no
longer owns hotkey identifiers, key registrations, or guest input sequences.

The callback adapter is necessary because `common_session_command_provider`
has one product context for all command callbacks.  It contains no policy and
passes the App machine to the keyboard owner.  No Common, Lib, Core or
VM-machine contract changed.

## Static checks

- `git diff --check`: passed.
- `Verify-DependencyDag.ps1`: passed with 10 explicit migration edges.  The
  new `app/keyboard.c -> vm/machine` edge records App product policy using the
  existing public VM-machine input API; the public keyboard header only
  forward-declares `vm_machine`.
- `verify_console_adapter_closure.cmake`: passed.
- `verify_vm_machine_owner.cmake`: passed.
- `verify_vm_provider_composition.cmake`: passed.
- `VerifyProductSessionManager.ps1`: passed.
- `Verify-DocumentationGovernance.ps1`: passed for `vm-0-5-0531`.

## Build and test

- The changed `app/command.c` and `app/keyboard.c` compiled cleanly with the
  optimized x64 GNU build and the x86 Clang/MinGW target.
- Relinked x64 focused `vm-app-console-smoke` and
  `vm-app-console-lifecycle-smoke`; both passed.
- Full repository-only unit suite: **325/325 passed** (`ctest -L unit -j 8`,
  15.56 seconds).
- Optimized, stripped artifacts were architecture-verified and copied to both
  required locations:
  - `build/output/nxvm_0_5_0531_x64.exe` and
    `assets/sessions/nxvm_0_5_0531_x64.exe`, SHA-256
    `E752D6CA6A14EC4BA2EC411DD7786656C58EDA24AC61E6AD4E8BFCDCDD43AF89`.
  - `build/output/nxvm_0_5_0531_x86.exe` and
    `assets/sessions/nxvm_0_5_0531_x86.exe`, SHA-256
    `5D32FC224DA60B9294D4C66AF6A2F1884C480A94FD21034FEAC3CCB24ED7AB11`.

No integration test was needed: this S changes only App-local routing and
retains the existing public Common and VM-machine behavior.
