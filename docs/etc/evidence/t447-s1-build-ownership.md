# M5 T447 S1 Build Ownership

## Frozen Ledger And Disposition

S1 consumes the CMake ownership batch from Td S127: the stale
`VM_RUNTIME_SOURCES` aggregate, the current artifact's independent compilation
of `machine_info.c`, and the unconditional Win32 GUI libraries on
`vm-composition`.  The whole current VM build declaration sweep is limited to
source aggregates, `add_library`/`add_executable` owners, artifact construction,
and `target_link_libraries` host edges.

| Ledger member | Before | Disposition |
| --- | --- | --- |
| `VM_RUNTIME_SOURCES` | Unconsumed union of Core product and VM profile sources. | Deleted; it had no target consumer. |
| `machine_info.c` | Owned by `vm-composition` and also compiled directly by every current artifact. | Retained only in `VM_COMPOSITION_SOURCES`; the artifact compiles `main.c` and links its owner target. |
| Win32 GUI libraries | `vm-composition` publicly named `user32` and `gdi32` on every host. | `vm-platform` owns both under `WIN32`; `vm-composition` has only a private dependency on that adapter. |
| Remaining VM aggregates and targets | Potential duplicate source or host owner. | Reviewed against their `add_library`/`add_executable` and link declarations; no second production source owner or unconditional host-native edge remains. |

The generated CMake graph records `vm-composition -> vm-platform` as a dotted
private edge. `vm-platform` alone has the direct `gdi32` and `user32` edges.
This preserves final static-library link closure without promoting platform
libraries or the adapter implementation into the composition interface.

## Proof

- Fresh MinGW configure passed.
- `verify-t447-build-ownership` passed with marker
  `M5:T447:S1:BUILD-OWNERSHIP:OK`; it rejects the removed aggregate, any
  composition-public platform/native dependency, a missing `WIN32`
  platform owner, and an artifact that directly compiles another VM source.
- The current artifact and the focused `vm-platform-host-surface-smoke` and
  `vm-platform-run-handle-smoke` targets built. Both smoke executables passed:
  `M5:T89:S1:HOST-SURFACE-LEASE:OK` and `M5:T194:S3:RUN-EVENT:OK`.
- The first full smoke run exposed a `vm-dos-prompt-smoke` host-timing false
  negative. The smoke had slept for a fixed three seconds before its first
  display observation. It now polls the already-existing prompt observation,
  retains the pause-and-recheck safety condition, and uses a bounded five-second
  host-startup allowance. Five focused CTest repetitions passed; the full
  293-test `current-gate` CTest run at parallelism four also passed, including
  `current.vm-dos-prompt-smoke` in 4.59 seconds. This changes no product path
  or guest assertion.
- `build/output/nxvm_0_5_0447.exe` SHA-256 is
  `2D11FAB64183514AB1CFC7609C5A063403B42DDC7E171D0642B3394C49E3DB29`.
- All 67 configured specialized verification targets passed individually. The
  aggregate Ninja parent on this host exited after its early child output, so
  individual results are retained as the truthful proof rather than inferring
  later completion from aggregate startup.

## Minimalism And Review

The counted source/test change is 57 added and 16 removed lines across
`CMakeLists.txt`, `CMakePresets.json`, `vm_dos_prompt_smoke.c`, and the 35-line
ownership verifier (tracked-diff counts plus the new file), net +41. The
verifier is the only added mechanism: it makes the finite source/edge ledger
regressible. The test change replaces a fixed-delay observation with the
existing condition observation; it adds neither product state nor a test-only
interface. The implementation deletes the stale aggregate and duplicate compile
route; it adds no wrapper target, fallback artifact path, compatibility branch,
or new runtime state. `git diff --check`, documentation governance, all
specialized gates, and current smokes pass.
