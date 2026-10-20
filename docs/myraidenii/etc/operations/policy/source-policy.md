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

Use original NEC/Zilog/Yamaha/OKI manuals and board observations where available.
Reference-derived timing/protection remains L2 rather than a silicon-exact claim.
Keep acquired source/manual payloads outside tracked product source; records
pin revision/edition/hash and describe what was actually inspected.

Game/program/sound/graphics/protection ROMs are owner-supplied external inputs
in the nxvm-assets archive. Do not download games, commit raw dumps, embed them
in EXEs, or borrow the PC firmware embedding exception. A filename or possession
is not a redistribution grant. Commit metadata/hashes only after actual audit;
never invent missing contents or bypass protection. Existing ROMs are read-only.

Units use original synthetic ROM-free vectors. Real-set execution belongs only
to test/myraidenii/integration with bounded checkpoints and cleanup. Logs do not
dump protected content; raw traces have hard byte/time budgets and are discarded
after retaining neutral observations. Microsoft binaries are not required or
distributed. Sibling repositories are read-only absent explicit import approval.

External research tools must direct generated configuration, NVRAM, snapshots
and logs to an owner-managed location under `nxvm-assets/media-myraidenii/`.
They must not create repository-root configuration directories or become a
tracked/runtime product dependency.
