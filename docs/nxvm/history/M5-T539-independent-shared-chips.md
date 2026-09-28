# M5 T539: Independent Shared Chips

## Scope And Baseline

Owner admitted the first [migration proposal](../proposals/m5-shared-chip-extraction.md)
after Td S174 (996a19a17), requesting research/design before source changes.
T538 is closed; T539 remains open. Current's packet is the execution authority.

## S1: Boundary Research And Design

Original request: determine file/directory structure and dependencies, the
independence gaps of every current chip, required diffs and architecture
decisions that must precede implementation.

Allowed changes are NXVM documentation only. Audit all tracked files under
src/app-nxvm/devices and their actual production/test/build consumers. Retain
source anchors, a finite migration ledger and proposed responsibility map.
No production migration, shared API change, firmware import or binary rebuild.

Research delivery: [design review](../etc/architecture/t539-independent-chip-design.md)
and [81-file ledger](../etc/evidence/t539-chip-migration-ledger.md). All 81 tracked
files occur exactly once in the ledger. Inspection identifies direct peer state,
fixed PC port/board wiring, timing catalog/build coupling and mixed chip/board
tests; it does not claim a fresh instruction-semantic completeness audit.

Proposed interfaces do not become runtime authority by being written in a
research record. Source changes require the next admitted S. Decisions for owner
review: non-Intel adapter scope, one-instance PIC/DMA contracts, KBC/PPI subset
identity, CPU firmware hook/FDC unready ownership and explicit timing units.

P1 delivers admission and research documents only. Source, tests, assets, INIs,
manifests and receiving executables remain unchanged. Validation: exact 81/81
inventory comparison, diff whitespace check, NXVM documentation governance gate
and all local Markdown links in the nine delivery documents passed. No runtime tests or builds
are required or claimed for this design-only S.
