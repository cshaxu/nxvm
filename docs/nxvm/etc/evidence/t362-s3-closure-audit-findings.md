# T362 S3: closure-audit findings

S3 audited the T362 S2 source and record boundary at baseline `3f2d90a9`.
The four immediate-`IMUL` encodings have one private table consumer, preserve
the existing successful-retirement publisher, and retain the S1 accounting
boundary: the 80186 scalar already includes EA, memory segment override adds
two clocks once, and no odd-word addition applies.  The focused T362 smoke
passes.

The required full `current-gates-gcc` run built the suite but failed two T344
static verifiers before the CTest execution phase:

| Verifier | Exact finding | Timing impact | Required receiver |
| --- | --- | --- | --- |
| `verify-t344-current-gate-registration` | Its parser only accepts historical `add_test("current...`)` output, while current CMake emits bracket-quoted `add_test([=[current...]=] ...)`. | None; `CTestTestfile.cmake` visibly contains the registration and `current-gate` label. | T362 S4 verifier grammar repair. |
| `verify-t344-historical-fixture-shapes` | The fixed inventory has 60 sources but the current source scan finds 66 direct constructors. The six unlisted sources are T359 S2--S6 and the T362 normalization smoke. | None; the T362 rename preserved one direct fixture rather than adding one. | T362 S4 inventory reconciliation. |

This is a gate-maintenance finding, not an instruction-timing allocation or
machine behavior finding.  In accordance with the T362 S3 stop condition,
S4 is admitted solely to repair the verifier grammar and source inventory,
then rerun the full gate.  S3 does not close T362 and makes no physical or
cycle-exact claim.
