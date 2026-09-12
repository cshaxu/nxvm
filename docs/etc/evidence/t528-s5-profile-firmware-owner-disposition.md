# M5 T528 S5 Profile/Firmware Owner Disposition

Baseline: `247b67ca` after accepted T528 S4.  This S moves only demonstrated
non-machine owners.  It changes neither the YAML grammar nor profile topology,
firmware bytes/mappings, media behavior, Core execution, the Common
FIFO/paused-debug lease, or NXVM's bounded-quantum runner.

## Owner Map

| Former location | Final owner | Reason |
| --- | --- | --- |
| `vm/machine/request_factory.*` | `vm/app/request_factory.*` | It is the sole session-request to `vm_machine_create()` composition entry. `vm/app` is downstream of `vm/machine`; moving it to `vm/profile` would create a profile-to-machine dependency. |
| `vm/machine/runtime/rom/external_pc_at.*` | `vm/profile/default_profile/external_pc_at_rom.*` | It is a generic PC/AT profile firmware provider. Its ROM-size constants now belong to that provider's profile header, so the provider has no machine-private include. |
| `runtime/model40_composition.*` | retained in `vm/machine` | It mutates the one machine composition object's Model-40 FDD observation, selected media/provider ids and frozen Core plan. Moving it to `vm/profile` would require that profile code to receive or own mutable `vm_machine` state, reversing the declared dependency. |

The request factory's public behavior and the external firmware mapping order
are unchanged.  `vm-machine` continues to depend on the profile target, while
`vm-app` is the declared composition root that may depend on `vm-machine`.

## Dependency-Gate Correction

`Verify-DependencyDag.ps1` previously returned from a pipeline scope when it
encountered either a non-owned source or a non-project include; this silently
stopped the rest of that scan.  It now skips only that item and resolves a
caller-supplied repository root before deriving relative paths.  The gate now
observes all project includes and reports exactly 17 explicit migration edges.
The obsolete external-ROM edge was removed; the two deliberate `vm/app`
request-factory-to-machine edges were recorded explicitly.

## Verification

- Focused CTest: PC/AT topology, default-PC/AT ROM materialization and Model
  40 private-composition smokes passed.
- `verify-current-specialized-gates` passed all 69 gates, including the
  dependency DAG, VM provider composition, default-PC/AT profile closure,
  external-ROM closure and T345 ownership inventory.
- Full repository-only unit suite: **299/299 passed** with `ctest --parallel 8`.
- The two media tests that had one non-reproducing failed full-suite observation
  passed serially, in the succeeding full suite, and in ten concurrent repeats;
  their existing distinct CTest work directories remain the one fixture route.
- Exit sweeps find no `vm/machine/request_factory`, `runtime/rom/external_pc_at`,
  old provider symbols or machine-owned PC/AT capacity constants.

`git diff --cached --numstat -- src test CMakeLists.txt cmake tools` records
**89 added, 88 removed, net +1 line** across the implementation, tests and
verification surface.  The small net addition is the repaired full-scan
dependency gate; the retained production routes are one request factory in
`vm/app`, one generic PC/AT firmware provider in `vm/profile/default_profile`,
and state-coupled Model 40 plan composition in `vm/machine`.

## Retained Execution Boundary

Common owns copied requests, run generation, wake and the paused-debug lease.
NXVM retains the Core bounded-quantum runner; SoftPC retains its CCPU/timer
rendezvous runtime.  This S creates no generic executor worker or second Core
execution path.
