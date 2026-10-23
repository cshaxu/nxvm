# Source, License And Asset Policy

Use the repository-root MIT LICENSE for project-owned source; preserve existing
notices when reusing local project-owned shared components. No second license
or private copy of shared rules is created. Imports/derivations require exact
commit, paths, unit license/notices, owner approval and verification. Root MIT
does not relicense external code.

MAME and other emulators are research references, not source dependencies.
The selected MAME Raiden II driver is marked LGPL-2.1+ with named holders;
M0 has no approval to copy, transliterate, link or redistribute it, its decryption
implementation or tables. Hardware facts and independently reproduced behavior
inform project contracts with explicit source grades. A necessary code import
requires separate per-unit license/architecture review before implementation.

For M1 implementation work, the owner explicitly authorizes inspection of the
published MAME/other-emulator implementations as L2 behavior and layout
references when a primary board-integration source is unavailable. The
implementation remains independently written: it must not copy or transliterate
source, data tables, ROM data, or decryption material. The evidence records the
exact upstream revision/path inspected, the finite facts adopted, and the
resulting L2 limitation.

M1 T4 S47 is the one recorded exception for the selected Raiden II sprite
transform constants. The owner authorized this bounded BSD-3-Clause data
derivation on 2026-10-20 after repeated direction to treat external emulator
implementations as L2 rather than an unavailable physical-source blocker. It
may retain only the audited finite constants and an independently written
transform; it must retain the exact upstream revision, paths, copyright
holders and BSD notice in the evidence and root third-party notice. It does
not authorize any LGPL MAME unit, MAME runtime dependency, driver code,
protected ROM bytes, or a general table-import policy.

Use original NEC/Zilog/Yamaha/OKI manuals and board observations where available.
Reference-derived timing/protection remains L2 rather than a silicon-exact claim.
Keep acquired source/manual payloads outside tracked product source; records
pin revision/edition/hash and describe what was actually inspected.

## Evidence Grades

- **L3** requires an explicit formula, constant, timing value, register rule or
  other directly applicable fact in an inspected primary manual PDF. A broad
  range, feature list or family description is not sufficient.
- **L2** covers a bounded behavior inferred from a secondary publication,
  board observation or independently inspected external emulator. It remains
  a model even when several such references agree.
- **L1** covers a local engineering approximation with no direct supporting
  manual fact or external behavior reference. It must be named as such and
  cannot be promoted by successful execution alone.

Every evidence record identifies the individual adopted fact and its grade.
No grade authorizes source, table, ROM, firmware or generated-output import.

Game/program/sound/graphics/protection ROMs are owner-supplied external inputs
in the nxvm-assets archive. Do not download games, commit raw dumps, embed them
in EXEs, or borrow the PC firmware embedding exception. A filename or possession
is not a redistribution grant. Commit metadata/hashes only after actual audit;
never invent missing contents or bypass protection. Existing ROMs are read-only.

Units use original synthetic ROM-free vectors. Real-set execution belongs only
to test/app-myraidenii/integration with bounded checkpoints and cleanup. Logs do not
dump protected content; raw traces have hard byte/time budgets and are discarded
after retaining neutral observations. Microsoft binaries are not required or
distributed. Sibling repositories are read-only absent explicit import approval.

External research tools must direct generated configuration, NVRAM, snapshots
and logs to an owner-managed location under `nxvm-assets/media-myraidenii/`.
They must not create repository-root configuration directories or become a
tracked/runtime product dependency.
