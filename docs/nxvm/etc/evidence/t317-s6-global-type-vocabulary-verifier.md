# T317 S6 Global Type-Vocabulary Verifier Correction

## Scope

S6 corrects two post-closure quality findings only.  It changes no runtime
source, ABI, target policy, or test behavior.

- `src/type.h` now states the operative rule: new code uses the project type
  facade rather than raw standard fixed-width names.
- The global fixed-width verifier now calls `git ls-files` for the governed
  `src`, `tests`, `cmake`, and `tools` trees plus tracked `CMakeLists.txt`
  entry points.  It filters the resulting tracked paths to C, header, CMake,
  and PowerShell files rather than discovering files with a working-tree glob.

## Mechanical Proof

The verifier fails if tracked root `CMakeLists.txt` is absent from its scan
set.  It reads that tracked entry point, appends one dynamically constructed
forbidden type token in memory, and requires the same content checker to
reject it.  The existing clean and full-family negative fixtures remain
required.  The repository itself remains clean: only the foundational aliases
in `src/type.h` and the controlled negative fixture contain direct forbidden
spellings.

## Verification Record

- Reconfigured Ninja built the affected strict smoke dependency graph.
- `verify-global-fixed-width-vocabulary` passed and reported 469 tracked
  code/script paths.
- The retained `verify-t317-test-type-vocabulary` dependency and the
  47-command `verify-t317-strict-cpu-smoke-coverage` audit passed.
- `run-current-smokes` completed with 194/194 current-gate tests passing.
- `git diff --check` passed.

This correction prevents the observed discovery and guidance regressions; it
does not claim wider scalar-type policy beyond T317's fixed-width family.
