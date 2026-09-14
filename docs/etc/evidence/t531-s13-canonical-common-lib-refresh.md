# M5 T531 S13 Canonical Common and Lib Refresh

## Scope and provenance

- The owner admitted S13 on 2026-09-14 to import only the latest committed
  shared corpus from the owner-controlled SoftPC repository.  The frozen
  source is commit `54845ac02342fe183b3ab308c92ad3c4704d1d84`.
- The import was made with `git archive` from that commit, not from a working
  tree.  It contains exactly `src/lib/` (94 files), `src/common/` (36 files),
  `test/lib/` (39 files), and `test/common/` (16 files).
- No SoftPC app, host, MVDM, firmware, media, asset, product test or
  uncommitted content was copied.  This is an owner-authorized project-source
  import; it introduces no third-party source or binary asset.

## Direct cutover

- Each of the four destination trees was replaced from the archived corpus.
  A post-import `git diff --no-index --quiet` comparison against the staged
  archive returned zero for all four trees.  NXVM therefore has no local
  fork, compatibility copy, or altered canonical source in this scope.
- The imported update changes the Common machine/session/UI contracts and the
  KVM mailbox wake-selection behavior.  NXVM removed its retired explicit
  debug-lease invalidation call: Common now invalidates the lease through its
  own public lifecycle transitions.  The NXVM smoke test now proves that path
  through `common_machine_stop()`.
- NXVM's aggregate unit target now builds the three new canonical tests, and
  its repository-specific CMake/ledger gates were updated only for renamed or
  retired public corpus paths.  The Common corpus remains independently
  verified by its imported tests; its internal C-runtime choices are not a
  NXVM product C-facade exception.

## Verification

| Check | Result |
| --- | --- |
| Four archived-tree equality comparisons | all four passed |
| Full repository-only x64 unit suite (`ctest -L unit -j 4`) | 381/381 passed |
| Specialized current gates | 67/67 passed |
| Clean x64 Release build | passed |
| Clean x86 Release build | passed |
| x64/x86 0531 artifact copies | each build/output and assets/sessions pair has identical SHA-256 |
| Diff whitespace check | passed |

The stale `LastTestsFailed.log` in the build directory predates this run
(08:48); the final 09:03 `LastTest.log` records all 381 selected tests as
passed, including the four tests named in that stale file.

The x64 copies hash to
`F289100312012818C03CF24AC860906915ECB25F1BFDEA163578B688B2B15634`;
the x86 copies hash to
`A5B3AB6C608C2794629ED67277C33B92CAC9DC455BB7CFEF91DFA40CACFC9741`.

## S13 disposition

P1 is `4e7b4a26`, which contains the exact corpus refresh and direct NXVM
adaptations.  This P2 records independent acceptance of that delivery. T531
remains open: this S does not claim that SoftPC's product runtime has been
integrated or that the larger two-product convergence task is complete.
