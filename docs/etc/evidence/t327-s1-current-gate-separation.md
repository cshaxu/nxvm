# T327 S1: Current And Specialized Gate Reconciliation

## Audit And Repair

T324 established the two-root design, but its verifier checked only the full
smoke root and specialized root. It did not mechanically inspect the fast root
or derive the CTest label partition. Its retained observation of 209 full,
15 media, and 194 non-media tests was consequently stale after later current
test admissions.

The T327 audit found one behavioral build-boundary defect: the fast root used
`PROJECT_CURRENT_SMOKE_TARGETS` directly. That base list also contains several
targets that the media classification adds to the complete current set; for
example, `vm-debug-pause-boundary-smoke` was still built by the fast root even
though its CTest execution was excluded by the `media` label. The repair derives
`PROJECT_CURRENT_NONMEDIA_SMOKE_TARGETS` by removing the authoritative media
target set before declaring the fast root dependency. It changes neither CTest
registration nor label, command, timeout, asset, assertion, or product code.

## Mechanical Proof

`verify-current-gate-separation` now receives the generated
`CTestTestfile.cmake` as well as the specialized-target manifest and Ninja
path. It proves all of the following from generated state:

- `run-current-smokes` and `run-current-fast-smokes` have no `verify-*`
  prerequisite.
- Every current CTest executable is a dependency of the full root.
- Every non-media current executable is a dependency of the fast root, while
  no media current executable is a fast-root dependency.
- Every media-labelled CTest belongs to `current-gate`; the full current
  selection is therefore exactly its current-media and current-non-media
  partition.
- Every generated specialized verifier belongs to
  `verify-current-specialized-gates`, and `current-gates-gcc` names exactly
  `run-current-smokes` and `verify-current-specialized-gates`.

The configured T327 baseline is 210 current tests: 15 media and 195 non-media.
Those numbers are generated observations, not an invariant against future test
admission.

## Execution Evidence

Fresh GCC configuration and the focused verifier passed with
`M5:T327:S1:CURRENT-GATE-SEPARATION:OK: full=210;media=15;non-media=195`.
On the same configured tree, the roots passed with these observed elapsed
baselines:

| Root | Result | Elapsed |
| --- | --- | --- |
| `run-current-smokes` | 210/210 current CTests | 13.45 s |
| `run-current-fast-smokes` | 195/195 non-media CTests | 4.18 s |
| `verify-current-specialized-gates` | 48 named mechanical verifiers | 12.78 s |
| `current-gates-gcc` | full smoke plus specialized roots | 16.76 s |

The elapsed values are diagnostic local baselines only. They do not relax any
command, label, test, media requirement, or acceptance coverage.
