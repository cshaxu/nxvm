# M5 T314: Quality Boundary And Coverage Package

T314 closes the admitted VADP safety, current-gate registration, and strict GCC
coverage-boundary package. It adds no CPU, xasm, Win32, media, public-ABI, or
M6 runtime capability.

| Owner outcome | Closure evidence |
| --- | --- |
| EGA CRTC 13h bound | VADP keeps `crtc[]` as the only offset state source, expands it to 20 entries, bounds every supported variable subscript, and asserts constant bounds at C11 compile time. `M5:T314:S2:EGA-CRTC-BOUNDARY:OK` verifies 13h port read/write, display-kind effect, and mode/color/adjacent-state isolation. |
| Existing xasm smoke in the current gate | `core-product-xasm-smoke` is registered once in `PROJECT_CURRENT_SMOKE_TARGETS`; CTest exposes `current.core-product-xasm-smoke` and it emits `M5:T129:S2:CORE-PRODUCT-XASM:OK`. No xasm source or API changed. |
| Strict GCC scope and CRTC closure | [T314 strict GCC matrix](../etc/evidence/t314-strict-gcc-matrix.md) inventories every production library and current artifact. S6 restores the historical target-local strict sets for `core-machine`, `vm-profile`, the current artifact, `core-platform`, `vdm-machine`, and `vdm-composition`, while retaining the three S4 selected libraries (`core-utils`, `core-product-session`, and `vm-request-transport`); actual Ninja commands prove all four flags, without a transitive-coverage claim. `verify-ega-crtc-boundary` is a current-smoke dependency that admits only the known predicate-guarded dynamic CRTC subscripts and compile-time-asserted constants. |
| Inherited governance debts | `TODO.md` records exactly the approved `TODO(Medium)` xasm capacity/failure-semantics and inherited-NXVM strict-compilation admissions. |

- Artifact: `build/output/nxvm_0_5_0315.exe`.
- SHA-256: `6B97C2738C3C8F8983FCF216D3841840973CCFE8B17AAB97DBEF60E742C7D454`.
- Final verification: `verify-current-artifact-target` selected
  `vm-0-5-0315`; 52 static/governance targets and 147 current CTests passed
  under `current-gates-gcc`; documentation governance and `git diff --check`
  passed.
- Similar-issue/package audit: re-ran the S2 dynamic-index and S3 registration
  searches, reviewed the S6 restored target-local options, CRTC verifier,
  matrix, and debt records, and found no residual removed strict option, no
  unclassified dynamic CRTC subscript, duplicate registration, generic
  exemption, or scope drift.
