# M5 Canonical SoftPC Lib Refresh And Common Adaptation

## Purpose

Replace NXVM's current `src/lib` tree with the exact shared-library corpus from
the owner-controlled SoftPC commit `3cabea6a6a0ce931a5480f3549599ebb74f3681d`
(`shared-t55-s13-p1`).  Adapt `src/common` to that public ABI first, then make
only the necessary NXVM product-side adaptations.  This is a whole-corpus
replacement, never a merge, local lib patch, compatibility facade or a
second host-services route.

## Ownership And Source Boundary

- SoftPC `src/lib` is owner-provided shared source.  The import records its
  commit, manifest revision, source/destination paths and verification in the
  S evidence and provenance index.
- `src/lib` owns only its documented neutral `types`, `console`, `host`,
  `storage`, `ui-base`, `ui-console` and `ui-window` capabilities.
- `src/common` consumes only Lib public `*_interface.h` contracts.  It remains
  free of platform, Core, VM, profile, firmware and guest state.
- Any required non-Common change must be a direct consequence of a changed
  public Lib contract.  The evidence names the old/new contract, destination
  owner and why Common cannot own it.

## Convergence Ledger

The finite import universe is the 93 tracked files under SoftPC `src/lib` at
the frozen commit, plus every NXVM tracked source, test and CMake consumer of
the former Lib public contracts.  Each member is either copied byte-identically,
migrated to the new public contract, deleted as the retired route, or marked
non-applicable with an evidence-backed reason.  Completion requires no local
Lib divergence, no external private-Lib include, and one retained production
path per affected capability.

## Subtask Plan

1. **S1 - exact import and consumer cutover.**  Freeze and record provenance;
   replace `src/lib` byte-for-byte; adapt Common; sweep every NXVM consumer and
   delete displaced paths.  Build x64/x86, run the full repository-only unit
   suite, applicable Lib/architecture gates and the external integration suite;
   build stripped `0530` artifacts, commit and push for owner testing.  Record
   every necessary non-Common adaptation separately.
2. **S2 - owner-test response and closure.**  Resolve any owner-observed
   regression through the same single paths, then independently review the
   actual diff, repeat required verification and close only after owner accepts
   the artifacts.

## Non-goals

- No SoftPC app, MVDM, firmware, media, executable, test or product-policy
  source enters NXVM.
- No Core/controller/profile/guest semantic change, except a strictly required
  adaptation to a neutral Lib contract.
- No Linux runtime promise beyond the imported Lib's declared support.

## Exit Criteria

- NXVM `src/lib` is byte-identical to SoftPC `3cabea6` and its manifest passes.
- Common has one public-Lib integration path; non-Common changes are fully
  enumerated and justified in evidence.
- No NXVM production code calls native host APIs where the canonical Lib owns
  that operation, and no duplicate/compatibility Lib route remains.
- Required x64/x86 builds, complete unit suite, applicable gates and integration
  suite have current recorded results; stripped `0530` x64/x86 artifacts are
  available for owner testing.
