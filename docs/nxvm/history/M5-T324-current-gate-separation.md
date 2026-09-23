# M5 T324: Current Smoke And Specialized-Gate Separation

## Scope And Result

T324 closes the Queue prerequisite that separates developer smoke execution
from specialized verification without reducing acceptance coverage.

`run-current-smokes` now builds only registered current smoke executables and
runs the full `current-gate` CTest label. `run-current-fast-smokes` retains
the non-media selection. `verify-current-specialized-gates` owns the 46
existing named verifier targets plus its target-graph verifier; the
`current-gates-gcc` preset composes exactly the pure full-smoke root and that
specialized root.

No CTest registration, command, assertion, timeout, media label, artifact
contract, product source, or verifier semantic changed. The detailed
[S1 evidence](../etc/evidence/t324-s1-current-gate-separation.md) records the
exact 209/15/194 CTest partition, graph proof, and separate timing baselines.

## Verification And Acceptance

Fresh configuration, the graph verifier, pure full and fast smoke targets,
the specialized target, and the complete `current-gates-gcc` aggregate passed.
Documentation governance and `git diff --check` passed before acceptance.
T324 adds no runtime artifact and leaves the existing `vm-0-5-0323` artifact
contract unchanged.
