# T324 S1: Current Smoke And Specialized-Gate Separation

## Scope And Mechanical Graph

The prior graph attached 30 `verify-*` targets directly to
`run-current-smokes`, while `current-gates-gcc` independently listed 41
verification targets with the smoke target. That made the developer smoke
entry execute structural/governance work and allowed the aggregate to request
some of the same work twice.

`run-current-smokes` now has only `PROJECT_CURRENT_ALL_SMOKE_TARGETS` as
build dependencies and runs the complete `current-gate` CTest selection.
`run-current-fast-smokes` retains the same CTest label and excludes only the
existing `media` label, with its existing non-media target list. The new
`verify-current-specialized-gates` owns 46 named verifier targets plus the
new `verify-current-gate-separation` graph verifier. The complete
`current-gates-gcc` preset names exactly these two layer roots:
`run-current-smokes` and `verify-current-specialized-gates`.

The graph verifier reads the generated 46-target manifest, queries Ninja for
both layer roots, rejects any `verify-*` prerequisite of pure smoke, requires
every manifest verifier under the specialized root, and parses the preset to
require exactly its two roots. It does not execute a CTest or alter verifier
semantics.

## Selection And Execution Evidence

| Selection or target | Observed result |
| --- | --- |
| `current-gate` CTest label | 209 tests |
| `media` label | 15 tests |
| `current-gate` excluding `media` | 194 tests; exact partition of the full selection |
| `verify-current-gate-separation` | `M5:T324:S1:CURRENT-GATE-SEPARATION:OK` |
| Pure full smoke | `run-current-smokes` passed; incremental elapsed baseline 13.44 s |
| Pure fast smoke | `run-current-fast-smokes` passed; elapsed baseline 4.15 s |
| Specialized verification | `verify-current-specialized-gates` passed; elapsed baseline 13.30 s |
| Complete acceptance | `current-gates-gcc` passed with both layer roots; elapsed baseline 16.82 s |

An earlier direct full CTest run observed 84.57 seconds while media-backed
tests initialized their local inputs. The later incremental pure-target
baseline is retained separately rather than being used to claim a fixed
machine-independent timing value.

## Non-Regression And Transfer

No test registration, command, assertion, timeout, media label, artifact
contract, product source, or verifier implementation changed. The aggregate
continues to include every specialized gate and the full smoke selection; the
change only makes their ownership and timing independently observable.
