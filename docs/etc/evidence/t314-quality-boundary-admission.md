# T314: Core Quality-Boundary Repair And Coverage Audit

## Admission

The owner approved the Queue candidate and this bounded M5 package on
2026-08-09. The S1 admission commit closes the first allocated T314 subtask;
the repair begins at corrective S2. Coordinated dual-session mode applies: the
coordinator admits and accepts one subtask at a time, and the executor waits
for a replacement instruction after every report.

## Ordered Subtasks

| Subtask | Scope | Deterministic proof and stop boundary |
| --- | --- | --- |
| S1 | Admit T314. | Queue, active-packet, and package evidence pass documentation governance; no runtime change. |
| S2 | Repair VADP CRTC 13h storage bounds and add its port-level regression. | Variable CRTC subscripts route through a predicate containing the array bound; constants have compile-time bounds; 13h round-trips without adjacent state mutation. Stop before other CRTC semantics or display/firmware changes. |
| S3 | Register the existing deterministic `core-product-xasm-smoke` in the current gate. | CTest lists and executes `current.core-product-xasm-smoke`; no xasm implementation or API change. |
| S4 | Produce the production-target strict-GCC coverage matrix and apply options only to targets proved pure ntvdm64 and independently buildable. | Actual Ninja commands contain all four options for each selected target; every remaining target has a precise exclusion and admission condition. Stop instead of splitting a target if that changes behavior or architecture. |
| S5 | Close the package. | Update current artifact to 0314, record SHA-256 and final matrix/debt evidence, pass focused checks, all current gates, documentation governance, diff check, commit, and immediate push. |
| S6 | Restore the pre-S4 target-local strict options and add a narrow static CRTC closure verifier. | Six historical target-local strict sets and the three S4 selected libraries have actual strict Ninja commands; `verify-ega-crtc-boundary` permits only known predicate-guarded dynamic shapes and compile-time-asserted constants. |
| S7 | Close the corrective package. | Update the current artifact to 0315, record final S6 evidence and SHA-256, pass retained markers and all current gates, then return Status to Idle. |

## S1 Invariants

- `crtc[]` remains the only owner of the EGA CRTC offset state.
- Every variable CRTC subscript is dominated by the support predicate, and
  that predicate includes `index < CORE_MACHINE_VADP_CRTC_REGISTER_COUNT`.
- Every direct CRTC constant subscript has a compile-time bound assertion.
- The port test selects index `13h`, writes and reads it through the CRTC data
  port, checks the offset-controlled display result, and proves that mode,
  color, and adjacent CRTC state do not change.

## Deferred Governance

- **xasm API capacity and failure semantics (`TODO(Medium)`).** Future
  admission requires explicit input/output capacities, return and
  failure-atomicity contracts, caller inventory/migration, and bounded
  assembler/disassembler regressions. A local string-operation substitution
  cannot claim closure.
- **Inherited NXVM strict compilation (`TODO(Medium)`).** Admit only when an
  inherited unit is substantively changed or safely separated from a mixed
  target. Record the diagnostic baseline, exact modification scope, selected
  constraints, and regression result.

## Exclusions

T314 does not alter inherited CPU execution, xasm implementation, Win32
platform behavior, public interfaces, media policy, M6 admission, or product
UX.

## Closure Evidence

T314 corrective closure published `build/output/nxvm_0_5_0315.exe` and
SHA-256 `6B97C2738C3C8F8983FCF216D3841840973CCFE8B17AAB97DBEF60E742C7D454`.
`current-gates-gcc` passed its 52 static/governance targets and 147 current
CTests; `verify-current-artifact-target` selected `vm-0-5-0315`.

- S2 retains one VADP CRTC offset state array, a 20-register storage bound,
  bounded variable subscripts, compile-time constant bounds, and the focused
  `M5:T314:S2:EGA-CRTC-BOUNDARY:OK` regression.
- S3 registers the existing `current.core-product-xasm-smoke`, which emits
  `M5:T129:S2:CORE-PRODUCT-XASM:OK`, without xasm source or API change.
- S4 records every production library/artifact target in the
  [strict GCC matrix](t314-strict-gcc-matrix.md), selects `core-utils`,
  `core-product-session`, and `vm-request-transport`, and verifies all four
  flags in their Ninja commands. It records the approved xasm capacity/failure
  semantics and inherited NXVM strict-compilation debts in `TODO.md`.
- S6 restores the historical target-local strict sets for `core-machine`,
  `vm-profile`, `vm-0-5-0315`, `core-platform`, `vdm-machine`, and
  `vdm-composition`, while retaining the three S4 libraries. The matrix makes
  no transitive or whole-product strict-coverage claim. It adds
  `verify-ega-crtc-boundary` to current smokes: the verifier checks the known
  CRTC constants' `_Static_assert` coverage, the bounded support predicate,
  the three approved dynamic access shapes, and a negative bypass fixture.
- The corrective closure sweep found no residual removed historical strict
  option and no unclassified dynamic VADP CRTC access; it also found no
  duplicate xasm current-gate registration or generic strict-GCC exemption.
  No artifact or product behavior beyond the admitted S2 repair changed.
