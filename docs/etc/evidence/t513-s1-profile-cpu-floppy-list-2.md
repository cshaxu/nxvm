# T513 S1: Profile/CPU/Floppy Current Gap List 2

## Current test and execution ownership

| Surface | Existing owner | Disposition for T513 |
| --- | --- | --- |
| default-at FDD geometry and BIOS values | `test/vm/machine/vm_default_pc_at_apply_smoke.c` | Unit proof for all four geometries; it is not an external boot-matrix row |
| IBM 5170 native/360K and 720K/1.44M rejection | `test/vm/machine/vm_ibm_5170_model_339_composition_smoke.c` | Unit proof of selector behavior; retain as the construction oracle |
| Model-40 1.2M FDD media construction | `test/vm/machine/vm_model40_fdd_s18_smoke.c` and BYOB media smoke | Unit proof of fixed geometry; add the missing rejection assertion at the same public session owner |
| IBM 5160 topology and fixed geometry | XT profile unit smoke and `vm_session_create_xt_byob` | Unit proof exists for topology; T513 adds public rejection coverage only if it is absent |
| External boot diagnosis | `test/integration/dos/vm_byob_dos_boot_probe.c` | Diagnostic-only, unregistered and trace-heavy; S2 replaces its duplicated parsing/terminal policy with one normal matrix runner |
| Current CTest integration suite | CMake media target lists | Existing tests cover selected default images and component scenarios, but none owns all 20 T513 rows |

## Finite gap disposition

| Matrix subset | Existing external boot row | Receiver |
| --- | --- | --- |
| IBM 5160 / 8088 / 360K | No registered normal integration row | S2 matrix runner and S3 CTest registration |
| IBM 5170 / 80286 / 360K and 1.2M | No registered normal integration row | S2/S3 |
| default-at / four CPU identities / four FDD geometries | No registered normal integration rows | S2/S3 |
| Model 40 / 80386DX / 1.2M | No registered normal integration row | S2/S3; needs owner-provided BYOB firmware at execution |
| All supported rows | No complete external execution record | S4 executes available rows, repairs any sole-owner defect, and records unavailable owner inputs without a synthetic pass |
| Rejected Model-40 FDD requests | Silent 1.2M substitution instead of rejection | S4 session-owner repair plus focused unit proof |

No integration test may recreate a CPU/profile/FDD selection, infer a geometry
from an image, parse a DOS filesystem, or acquire/copy an external input.  The
new runner receives a row descriptor, invokes `vm_session_create`, and observes
the existing session/Core execution path.  CMake remains the only CTest route
registry.

## S1 verification

- The complete repository-only Debug unit suite passes: 315/315.
- Documentation governance passes against the current 0512 baseline.
- This S adds no production or test source: zero source/test code delta.
