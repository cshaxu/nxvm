# Project Status

## Current Work

| Task | Status | Scope |
| --- | --- | --- |
| T528 | Between S tasks | [M5 VM-machine owner cleanup](../proposals/m5-vm-machine-owner-cleanup.md): S1--S3 accepted; the remaining control/lifecycle and profile-owner disposition is planned only after its executor evidence is reconciled. |

## Current Technical Baseline

- `vm-0-5-0527` is the current target. Its stripped Release artifacts are
  `nxvm_0_5_0527_x64.exe` and `nxvm_0_5_0527_x86.exe` in both
  `build/output` and `assets/sessions`.
- `src/lib` is the canonical shared host-services corpus. `src/common` is an
  independently buildable, Lib-public-contract-only product-capability corpus:
  `machine`, `session`, `ui`, `xasm32`, and `debug`.

## Recent M5 Closures

| Task | Compact result |
| --- | --- |
| T528 S2 | Accepted at `27b27298`: `vm/product` is the only monitor-text owner; `vm/machine` exposes copied information only; dead Core-product config is deleted and Core contains only `machine`. [Evidence](../etc/evidence/t528-s2-product-boundary-cleanup.md) transfers the executor receiver to T529. |
| T528 S3 | Accepted at `1a1e9e90`: `vm/media` is the sole FDD/HDD geometry, Lib-storage and Core-provider owner; device composition has the one registry bind/freeze route, and no reverse dependency remains. [Evidence](../etc/evidence/t528-s3-media-owner-relocation.md) records 299/299 unit and specialized-gate proof. |
| T527 | Common product-runtime convergence closed at `2c596f2c`: one Common corpus owns xasm32, Debug, session, machine and UI; NXVM retains one VM Core adapter, product policy owner and App composition root. Independent Common verification, 299/299 unit, 42/42 external integration, specialized gates and stripped x64/x86 artifacts pass. [History](../history/M5-T527-common-product-runtime-convergence.md) and [evidence](../etc/evidence/t527-s9-reusable-common-closure.md). |
| T526 | Canonical-library integration repair closed at `98ac51a4`: 299/299 unit, 67/67 specialized gates, 42/42 external integration, actual-diff review and stripped x64/x86 artifacts accepted. |

## Recent Governance

- **M5 Td S166 P1:** queued two owner-approved, unnumbered candidates in
  dependency order: VM-machine owner cleanup, then Common-machine executor
  completion. Both use the audited NXVM/SoftPC two-consumer criterion; neither
  allocates or admits a numeric implementation task.

- **M5 T527 S9 P2:** records independent corpus verification, complete test
  closure, artifact identity, proposal retention, and task closure after
  implementation `2c596f2c`.
