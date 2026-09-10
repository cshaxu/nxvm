# M5 Canonical SoftPC Library Adoption And NXVM Cutover

## Purpose

Replace NXVM's current `src/lib` corpus with one exact, manifest-verified
canonical corpus from the owner-controlled SoftPC repository, then make NXVM
use that corpus through one product binding.  This is a replacement, not a
merge, wrapper layer, or compatibility mode.

The source candidate is SoftPC `src/lib` at the exact commit and
`MANIFEST.sha256` revision frozen by S1.  S1 must confirm that the candidate
remains product-neutral and importable under the source policy before any copy.
Only the canonical library corpus may be copied; SoftPC `src/app`, MVDM, ROM,
media, binaries, tests and product configuration are excluded.

## Ownership

- The shared library owns generic host/platform mechanics only: logical
  Console, Console broker, native Window/Console loops, copied input/frame
  values, mailboxes, tasks/clocks and byte-medium storage.
- NXVM owns session lifecycle, multi-session Console ownership policy, guest
  input mapping, hotkey meaning, product titles, display target policy, Core
  interaction and every machine/profile decision.
- Core remains unaware of host windows, consoles, source identities, native
  handles and host input.
- The NXVM product coordinator is the sole composition root.  It requests
  library actions through public neutral APIs and never calls a platform SDK.

## Required Result

- `src/lib` is byte-identical to the frozen SoftPC corpus and passes its exact
  manifest verifier in NXVM.
- NXVM retains one presentation/input/storage/host path.  Its current
  unified presenter, duplicate native Window/Console/input/mouse code and
  parallel lib contracts are removed in the same cutover.
- A process has one host Console broker.  NXVM product policy decides which
  session, if any, owns raw Console input; Window sessions remain independent.
- New raw Console ownership intentionally begins with a clean host-input
  boundary.  This is the owner-approved explicit input-discard capability;
  it is not an implicit debugger or guest input flush.
- Window initial title is supplied by NXVM at construction; later title,
  mouse and target controls use the canonical library contract.
- Input source identities are mapped only to NXVM presenter/session
  generations for retirement filtering.  They never enter Core.

## Subtask Plan

1. **S1 — Freeze and admission audit.** Record the exact SoftPC commit,
   manifest hash, path inventory, licence/provenance disposition and public
   ABI comparison.  Resolve every current-NXVM to canonical-lib mapping,
   including the approved raw-Console input-discard capability.  Stop for any
   product vocabulary, unreviewed source notice, API leak or semantic gap.
2. **S2 — Exact corpus replacement.** Copy only the frozen `src/lib` tree
   byte-for-byte, replace NXVM lib CMake/manifest verification with the
   canonical build entry, and prove no local lib file diverges.  Do not yet
   retain a second NXVM adapter path.
3. **S3 — NXVM single-path binding.** Replace NXVM's old presenter/platform
   paths with one product coordinator using the canonical Window, Console and
   broker contracts.  Remove obsolete source, tests and build declarations;
   map source retirement into NXVM-owned session generations and preserve
   multi-session Console ownership without native handles in product code.
4. **S4 — Product and closure proof.** Run focused native Console/Window,
   ownership, input, title, mouse and source-retirement proofs; full
   repository-only unit; lib manifest/build/CTest; static boundary sweeps;
   owner-managed integration; and fresh stripped x64/x86 artifacts.  Review
   the actual diff for duplicate paths before closure.

## Non-goals

- No SoftPC application, MVDM, firmware, ROM, media, binary, monitor command,
  machine logic or test fixture import.
- No Core, controller, CPU, timing, profile, YAML or asset behavior change
  except an unavoidable removal of host-UX duplicate wiring.
- No parallel compatibility implementation, one-product branch or lib fork.
- No claim of a Linux presenter runtime beyond the canonical corpus's stated
  support level.

## Acceptance

- The frozen corpus is provenance-recorded and byte-identical in both
  repositories.
- Public headers contain no platform SDK type or product/machine vocabulary.
- Product code contains no direct Window/Console/platform calls where the
  canonical library owns that capability.
- Every replaced NXVM route has one retained owner or is deleted; no wrapper
  survives merely for compatibility.
- All S and task-level verification passes on freshly configured build trees.
