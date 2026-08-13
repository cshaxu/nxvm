# M5 T344: Build-Quality Reproducibility

## Task Record

T344 followed the completed four-profile CPU program and made the build and
current-gate evidence consumed by later Windows readiness work reproducible.
It did not broaden compiler policy, claim inherited source coverage, change
device or CPU behavior, or manufacture a generic test framework.

## Completed Subtasks

| Subtask | Closed outcome |
| --- | --- |
| S1 | Restored fresh GCC configuration by reconciling the T337 VM86 `#UD` owner inventory; repaired the shared TF post-interrupt scheduling condition and stale RF expectations revealed by rebuilding. |
| S2 | Generated a direct Ninja-command matrix for production/current-gate sources and mechanically verified 305 rows: 130 retained strict, 54 deferred production, and 121 deferred owner-test rows. |
| S3 | Replaced aggregate duplicate suppression with one canonical 218-target current-gate union and mechanically verified target/CTest registration and labeling. |
| S4 | Classified all 53 remaining direct historical machine constructors: 22 exact private lifecycle tails converge through `tests/support`, while 31 retained shapes have explicit semantic/admission boundaries. |
| S5 | Removed the repeated strict declaration for `core-platform-presentation-mailbox-smoke` and added configuration plus generated-matrix checks that reject duplicate target-local strict options. |

## Closure Audit

The original request required a full-tree build-quality audit and admission of
the resulting four improvements. T344's evidence proves clean configuration,
truthful direct-source strictness, unique current-gate registration, and
bounded fixture-shape ownership. S5 is a necessary bounded corrective found
by S2's complete duplicate sweep; it completes, rather than expands, that
audit.

The final fresh GCC configure, 55 specialized verification gates, direct
strict matrix, and 218/218 current-gate passed. Documentation governance and
`git diff --check` passed. The detailed baseline and fixture disposition remain
indexed in [T344 evidence](../etc/evidence/t344-code-quality-baseline-audit.md)
and [fixture-shape inventory](../etc/evidence/t344-historical-fixture-shapes.md).

The retained [T344 proposal](M5-T344-build-quality-reproducibility-proposal.md)
preserves the admission rationale. Inherited executor warning cleanup,
xasm/debug capacity redesign, and non-equivalent device/fixture setups remain
separate bounded debts; no such item is mislabeled as direct strict coverage.
