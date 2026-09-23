# Governance Bootstrap Evidence

## Scope And Review

Owner approval: conversation on 2026-09-19. Scope is governance migration,
Git initialization, MIT, and Neko's Nes Emulator identity in directory mynes.
The owner explicitly deferred emulator implementation until design/planning.

The same session performs executor work and then coordinator actual-change
review; this is not an independent reviewer or a runtime compatibility audit.
The source NXVM worktree is read-only and was clean at the pinned revision.

## Migration Inventory

- Root instruction/submission pointers and documentation entry are adapted.
- Four rules retain the MTSP lifecycle, single authority, source discipline,
  review, evidence, similar-issue sweep, trace and artifact gates.
- Five design documents contain MyNes identity, owner-selected layers, bounded
  first increment and next design gate; no x86/product implementation is copied.
- Current held the sole active bootstrap packet during execution; closure removes it. Queue names only the
  unnumbered next design proposal. TODO contains no manufactured project debt.
- Documentation tooling retains topology, links, packet, identifier and
  governance checks, with explicit pre-implementation handling and self-tests.
- Root MIT notice is preserved; provenance maps imported units and changes.

## Filesystem And Assets

git init succeeded before relocation. Windows held the original directory
open, so the repository contents and existing assets were moved into mynes.
The old empty directory may remain until the application releases it.
Local assets and ROM extensions are ignored; no ROM content is used as evidence.

## Verification

The documentation gate passed with the initial active M0 Td S1 packet, and its
self-test suite passed. The final idle closure is checked again before commit.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File tools/Verify-DocumentationGovernance.ps1 -RepositoryRoot .
powershell -NoProfile -ExecutionPolicy Bypass -File tools/Verify-DocumentationGovernance.ps1 -SelfTest
git diff --cached --check
git check-ignore assets/drmario.nes assets/smario2.nes
```

Self-tests exercise principal-document schemas, relative links, supporting
indexes, queue/proposal consistency, debt, packet fields, numeric/governance
allocation, continuation and status retention. MyNes additions verify unborn
repositories, initial governance admission, skipped identifiers, idle/active
conflicts, product-build rejection during bootstrap, and an empty candidate queue.

Actual-change review compares the copied rules and checker against the pinned
NXVM files, reviews each new identity/design/state/support file, and scans the
entire tracked-candidate surface for stale product, artifact and status terms.
Retained NXVM references identify provenance only. Checker fixtures use
synthetic historical identifiers and never allocate MyNes product tasks.

| Owner outcome | Reviewed proof |
| --- | --- |
| Initialize Git and use mynes | Independent Git root; repository metadata and original local assets relocated. |
| MIT and full project name | Root LICENSE, README and the sole current baseline. |
| Migrate NXVM governance first | Reading set, four rules, five design authorities, Current/Queue/TODO, proposal/history/support topology, checker and import map. |
| MTSP before implementation | Bootstrap is standalone Td; next design is an unnumbered candidate; no src, product CMake or executable. |
| First increment bounded to layers/ROM/CPU/tests | Roadmap and next-design proposal preserve this scope without claiming implementation. |
| Local commits without remote, push once configured | Execution Change Discipline applies the owner-approved condition to every P and its closure/report semantics. |
| Preserve existing local ROMs | assets exists and both existing files are ignored; no ROM is staged. |

No product unit/runtime test or playable-game result is claimed. No third-party
runtime corpus or Lib-specific source CI was imported. Self-test repositories
are temporary and removed through a validated cleanup path. Source NXVM remains
unchanged. There is no product code-size delta; the checker is governance tooling.

## Delivery

The owner approved a permanent remote-aware rule: local commits without a remote; immediate push for every commit once any remote is configured. No remote is configured; this delivery is local only, identified by Git subject `M0 Td S1 P1: bootstrap MyNes governance from NXVM`. No numeric
implementation task, emulator source, build scaffold or runnable artifact is admitted.


Coordinator acceptance: same-session actual-change review accepted the bounded
bootstrap on 2026-09-19 after the gate and self-tests. Closure is carried in
the standalone Td delivery commit. The next design proposal is queued, not
admitted; it must produce reviewed contracts and the immediate implementation
breakdown before product work begins.
