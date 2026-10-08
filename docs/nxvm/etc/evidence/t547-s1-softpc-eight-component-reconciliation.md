# T547 S1 SoftPC Eight-Component Reconciliation

## Baselines

- NXVM: `766ef2587` before this S's test-only import.
- SoftPC: clean project-owned commit `c6413911`.
- Source authorization: the root MIT project policy permits import of
  project-owned SoftPC shared corpus material; this S imports no third-party
  source, firmware, media, ROM, INI or App asset.

## Current Diff Disposition

`git diff --no-index` was reviewed for all eight shared roots. Paths absent
from the table are byte-identical apart from their independently generated
manifest revision where noted.

| Root | Disposition |
| --- | --- |
| `src/lib` | Identical: no SoftPC correction is missing. |
| `src/common` | Identical: no SoftPC correction is missing. |
| `src/ibmpc` | Identical: no SoftPC correction is missing. |
| `test/common` | Identical: no SoftPC correction is missing. |
| `test/lib` | Imported SoftPC `c6413911`'s native-test isolation: private per-test work directory, `native_desktop` resource lock for the three real desktop tests, and actual-viewport display capture. `MANIFEST.sha256` is regenerated for this NXVM S. |
| `src/x86` | NXVM-only T546 repair set retained: CPU/FP execution, timing, attachment, bus, machine and scheduler routes in 15 paths. SoftPC has no additional path in this diff. |
| `test/x86` | NXVM-only T546 CPU regression set retained in 20 test/gate paths: control transfer, descriptors, execution bus/fault, LAR/LSL, outer return, selector/task switch, VERR/VERW, FPU, paging, timing ledgers, prefetch and negative gates. SoftPC has no additional path in this diff. This S also makes each negative-probe case private, so an interrupted or concurrent prior case cannot supply another case's source to the assertion. |
| `test/ibmpc` | NXVM retains the T546 exact expectations: 8086 retirement's qualified wait source is `3`, not SoftPC's older `6`; 80186 `LEAVE` uses the corrected `5`, not older `8`. The manifest is NXVM's corresponding generated record. |

The non-identical `src/x86` paths are:
`chips/cpu/cpu.c`, `cpu_instructions.c`, `cpu_instructions.h`,
`cpu_interface.h`, `cpu_timing.c`, `cpu_timing_model.c`, `chips/fpu/fpu.c`,
`chips/fpu/fpu.h`, `chips/fpu/fpu_interface.h`,
`core/attachment_interface.h`, `core/cpu_bus.c`, `core/machine.c`,
`core/machine.h` and `core/machine_scheduler.c`, plus the corresponding
manifest. They are mapped to accepted T546 S15-S24 repair commits in NXVM
history, not copied from an older SoftPC state.

No raw SoftPC path is copied merely to obtain parity. The resulting corpus is
the union of SoftPC's current test-isolation correction and NXVM's newer,
source-backed CPU/board repair set.

## Verification Record

- All eight manifests pass their owning verifier.
- `test/common`: 20/20 passed on x64 and 20/20 passed on x86.
- `test/x86`: 182/182 passed on x86. On x64, the first aggregate exposed three
  absent stale-tree executables; after building those exact targets, their
  three tests and the isolated negative gate passed. The earlier aggregate's
  remaining 179 tests had already passed. A subsequent aggregate exceeded the
  interactive 30-second command window after 40 passing cases; it is not
  recorded as a second complete aggregate.
- `test/lib` focused native desktop x64: modal, display and retirement each
  passed. One earlier three-test group saw an intermittent modal component
  failure; a direct x64 modal run then passed. This is recorded as a native
  desktop environment observation, not hidden as a deterministic pass.
- `test/lib` focused native desktop x86: 3/3 passed.
- The Lib x64/x86 aggregates each reached 50/51 passing tests; in both cases
  the only unreported item was the pre-existing `types-layout-selftest`, which
  exceeds this environment's 30-second interactive command ceiling. It is not
  related to the imported work-directory, resource-lock or viewport paths, and
  is deliberately not represented as a pass.
- `test/ibmpc`: 182/182 passed on x64 and 182/182 passed on x86.
