# M5 T531 S12 Canonical Common and Lib Import

## Scope and provenance

The owner authorized this S on 2026-09-14 to use the committed SoftPC corpus,
excluding later uncommitted source-worktree content.  The sole imported source
is SoftPC commit `662ed4b390b886800c202a66eac754d9aac595b6`:

- `src/lib/` - 94 files;
- `src/common/` - 36 files;
- `test/lib/` - 38 files; and
- `test/common/` - 14 files.

The four trees were exported from that commit and compared byte-for-byte with
their NXVM counterparts.  The comparison is exact.  No SoftPC app, host,
MVDM, assets, firmware, media, product tests, or uncommitted worktree content
was imported.

## NXVM cutover

NXVM consumes the imported public contracts directly:

- Common owns the one lifecycle worker and request queue; `vm/machine` supplies
  only the bounded Core driver callbacks.
- `vm/app` forwards copied Common machine state and frame facts to the one
  Common Session reducer.
- The former VM result-sink path, executor FIFO, and split lifecycle/display
  reporters are removed.  No compatibility shim or parallel Common control,
  UI, or machine owner remains.
- Root CMake builds the canonical Lib and Common trees and aggregates their
  canonical standalone tests with the repository-only unit suite.
- The two stale VM ownership gates now verify this canonical Common-worker
  model rather than deleted pre-cutover files.

## Verification

| Evidence | Result |
| --- | --- |
| Exact four-tree comparison against the named SoftPC commit | pass |
| `lib-verify-manifest`, `common-verify` | pass |
| dependency, VM-owner, VM-lifecycle, product-session and documentation gates | pass |
| Full repository-only unit aggregate | 322 / 322 pass |
| x64 optimized Release | `nxvm_0_5_0531_x64.exe`, PE x86-64, SHA-256 `75C26E5B71EBA7D2A63CA632DC45D69961CFD305F9A6DCDF6D792A699C97AC7D` |
| x86 optimized Release | `nxvm_0_5_0531_x86.exe`, PE i386, SHA-256 `C37BDA18DC7780A4AE52E2047B276AEF7062FFD4836E99F820FFEBA9041BCE43` |

The artifact hashes match the copies in both `build/output/` and
`assets/sessions/`.  T531 remains open; this evidence closes only S12 after
its implementation and governance commits are pushed.

## P2 independent acceptance

The acceptance review inspected the actual P1 diff, repeated the exact
four-tree comparison, and swept production and test sources for the retired
VM result/FIFO/reporter route and App/Product direct Core calls. Both sweeps
were clear. It also reran the listed gates and reviewed the x64/x86 artifact
identity. The canonical corpus remains unmodified by NXVM; the only NXVM
changes are its direct consumers, aggregate registration, obsolete-route gates
and the provenance record above.
