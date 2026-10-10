# M5 T549 S1 — Live Test Identity Inventory

## Method

The inventory searches tracked live sources and registrations under
`test/lib`, `test/emulator`, `test/product`, `test/core` and the four PC App
roots.  It searches task-shaped labels only when they have an explicit task
prefix (`M<n>:T<n>` or `T<n>:`/`T<n>_`), so ISA names such as `INT13`, fixed
width constants such as `UINT32_MAX`, and chip names such as `pit825x` are not
false positives.  MyNES-private tests and historical documentation/evidence
are outside this task.

The scan found no task-shaped source filename, Ninja target or CTest name.
It found 132 live source references in 31 files.  These divide into the
following receivers.

## Mapping And Disposition

| Scope | Files / references | Disposition | Receiver |
| --- | ---: | --- | --- |
| Shared Lib, Emulator and Product | 0 | Already behavior-owned. | S2 records the clean sweep; no mechanical rename. |
| Core CMake manifest paths | 5 | Retain: these are historical timing-ledger evidence filenames, not test identities. | S3 verifies they remain evidence-only references. |
| Core test source | 3 files / 18 references | Rename `t330` cross-width task-switch helpers and the `t305` software-gate DPL fault helper. | S3 |
| PC App unit sources | 4 / 10 | Rename task-bearing helper symbols, fixture path/constant, comment anchor and success marker to the asserted machine behavior. | S4 |
| PC App integration registration | 3 files / 8 references | Rename `project_add_t*` helper names to behavior-owned integration registration helpers. | S5 |
| NXVM integration support and probes | 20 files / 83 references | Replace task-prefixed diagnostics, helper symbols and markers with stable behavior identities.  Preserve every diagnostic field, assertion, timeout and input. | S5 |
| Model 40 retirement capture | 1 file / 48 references | Replace task prefixes in its live capture diagnostics with `MODEL40:` behavior labels; retain the Model 40/C0/C1/D4 facts because they are the asserted capture subjects. | S5 |

The remaining 437 broad lexical hits from the first scan were rejected as
false positives: standard/Lib integer constants, x86 interrupt names, chip
names and cited historical evidence filenames.  No collision exists among the
current CTest names, and no identity is shared by MyNES.

## Old-To-New Rules

The exact replacement is local to each owning file, never a global alias:

- `M5:T<n>[:S<n>]:<behavior>` and `T<n>:<behavior>` become the file's
  existing App/behavior prefix followed by `<behavior>`; the dynamic values
  and diagnostic field order do not change.
- `vm_t<n>_*`, `t386_s<n>_*`, `VM_T<n>_*` and analogous task-shaped fixture
  names become the file's existing behavior prefix, for example
  `vm_windows31_setup_*`, `vm_fdc_read_track_*` and
  `vm_model40_cecg_*`.
- `project_add_t<n>_*` integration registration helpers become
  `project_add_<behavior>_*`; callers retain their existing CTest route.

This establishes a one-to-one replacement rule without changing a test's
source pathname, test selection, assertion, fixture or external media.

## S1 Exit Evidence

- No task-shaped active test filename, Ninja target or CTest name was found.
- All remaining task-shaped live references have one owning receiver above.
- Historical timing manifests remain cited as evidence rather than being
  renamed out of their established records.
- The inventory is documentation-only; S34's repository-only x64/x86 unit
  qualification remains the source baseline.  No test source changed in S1.

## S2 Shared Corpus Sweep

The three Shared test roots (`test/lib`, `test/emulator` and `test/product`)
have no active task-shaped filename, target, CTest route, success marker,
diagnostic or helper symbol.  Their broad lexical hits are solely `LIB_UINT*`
constants and the negative verifier's literal forbidden-token fixtures.
Those fixtures intentionally contain standard integer spellings to prove the
Types boundary and are not test identities.  S2 therefore makes no source,
CMake or manifest change.

## S3 Core Receiver

S3 renames the two 16-to-32/32-to-16 cross-width task-switch helpers in both
their Core X86 and CPU-state tests, plus the board interrupt entry helper that
proves software-gate DPL fault delivery.  The test files, targets, CTest routes
and their predicates are unchanged.  The five `t435`/`t512` manifest filenames
remain cited only as the approved timing-ledger evidence inputs; they are not
test identities and changing them would break that evidence chain.
