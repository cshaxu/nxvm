# T531 S22 Canonical SoftPC Import Evidence

## Frozen Source And Corpus Identity

S22 imports the clean committed SoftPC revision
`fa14c62bd34182bf9930988e25296bfb7d5be646`.  After the import, each of the
following NXVM trees is byte-identical to its SoftPC counterpart:

- `src/lib`
- `src/common`
- `test/lib`
- `test/common`

There is no NXVM patch inside those four trees.  In particular, the former
NXVM-only Common Debug provider and session reconciler are absent because the
canonical corpus folds their behavior into its current public boundaries.

## NXVM-Only Adaptation

- Root CMake includes all imported Shared test targets in the repository unit
  aggregate, so a normal NXVM unit invocation builds and executes the whole
  imported corpus.
- The integration YAML boundary verifier recognizes
  `test/integration/support/session_yaml.c` as the one provider which parses a
  YAML request and creates the machine.  The verifier still rejects direct
  machine construction in every integration consumer and still rejects media
  copying.
- NXVM's Debug and deferred-ownership gates now reference the canonical
  `command_runtime.h`, `command.c`, and current Session source set rather than
  retired canonical filenames.
- Direct YAML integration fixtures remain deliberately uncomposed: they own a
  bounded VM runner rather than a Common executor.  That runner now has the
  same explicit pause wake/acknowledge path as its public control contract,
  publishes the paused display snapshot, and consumes a one-instruction step
  before re-requesting pause.  The composed production path remains owned by
  Common and is not duplicated.
- Console-product integration now starts the actual paired release artifact in
  an isolated native Console and injects real `KEY_EVENT` records.  It no
  longer fakes stdin redirection, which cannot exercise the native cooked
  reader used by the imported Common/Lib corpus.

## Verification

- Corpus equality: all four `git diff --no-index --quiet` comparisons return
  success.
- `ctest --test-dir build -L unit -j8 --output-on-failure`: **328/328 passed**
  (81.81 seconds process time).
- `ctest --test-dir build -L integration -j4 --output-on-failure`: **40/40
  passed** (516.57 seconds process time), including the prior direct-runner,
  native-console, 5160, 5170, and Model 40 rows.
- A similar-issue sweep changed every direct integration runner which had
  accidentally used the composed-only lifecycle API.  Each now starts and
  controls its declared direct runner consistently; its bounded waits are
  wall-clock deadlines rather than `Sleep(1)` iteration counts.

## Remaining Task Scope

The imported corpus remains exact; the NXVM adapter repairs above are outside
those four shared trees.  T531 remains open for its separate two-product
execution gate.
