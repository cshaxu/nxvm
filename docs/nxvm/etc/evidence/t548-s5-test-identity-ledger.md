# T548 S5 Active Test-Identity Ledger

## Scope And Method

This ledger reviews active unit source paths, CTest target identities and
success-marker text only. Historical evidence is deliberately out of scope.
An identifier is task-derived only when it contains an explicit task-shaped
token such as `_s14`, `_t242` or `M5:T375:S2`; technical terms such as
`tss32`, `sreg` and source citation identifiers are not task markers.

The inventory was taken after S4 at committed baseline `3d54512ac`:

- 26 active unit source paths contain an explicit `_sNN` or `_tNN` token;
- 67 active unit source files emit an explicit `M*:T*:S*` success marker;
- 92 CMake files contain a broad `T`/`S` lexical match, but most are either
  ordinary CMake, a technical term, or a historical verifier whose semantic
  rename requires individual review.

No rename is authorized by this ledger alone.

## Path Rename Batches

| Scope | Count | Current pattern | Required later receiver |
| --- | ---: | --- | --- |
| MyDeskPro386 | 20 | Model 40/D4 source filenames with `_sNN` suffixes | One MyDeskPro386-only S: retain Model 40/D4 subject names, remove only the task suffix, then update every local target/manifest reference. |
| NXVM Default PC | 6 | `pcat_*_sNN`, `machine_*_sNN`, `fdc_t242_*` | One NXVM-only S: choose descriptive board/machine names from asserted behavior, then update local target/manifest references. |
| My5160 / My5170 | 0 | No active task-derived unit source path | Retain paths; their output markers are handled separately. |
| Core / Lib / Emulator / Product | 0 | No active task-derived unit source path | Retain paths; do not infer a rename from incidental source text. |

## Output-Marker Batches

| Scope | Files | Disposition |
| --- | ---: | --- |
| NXVM Default PC | 38 | Replace task provenance in ordinary success output with behavior identities in the same S as any renamed source/target. |
| MyDeskPro386 | 24 | Replace task provenance with Model 40/D4 behavior markers in the MyDeskPro386 rename S. |
| My5170 | 4 | Replace task provenance with Model 339 behavior markers in a My5170-only S. |
| My5160 | 1 | Replace task provenance with XT behavior markers in a My5160-only S. |
| Core / Lib / Emulator / Product | 0 | No active explicit task-formatted success marker found by this scan. |

## CMake Registration Debt

Task-shaped CMake helper names are not renamed mechanically. They fall into
three kinds:

1. semantic checks whose target name should become behavior-based after a
   direct review (`verify_t359_instruction_timing_inventory.cmake` is a known
   example);
2. historical compatibility/shape guards whose current scope must be decided
   before renaming; and
3. lexical false positives, including ordinary test/support files.

The next rename S must take one finite target-scoped group, prove every old
CTest identity is absent after migration, and preserve its exact predicate.
It must not rewrite evidence documents merely to erase historical task IDs.
