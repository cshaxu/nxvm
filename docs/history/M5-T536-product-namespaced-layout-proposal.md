# Product-Namespace Repository Layout

## Goal

Make the repository ready to host multiple products without turning product
code into shared code.  The shared first-level components remain `lib`,
`common`, and `x86`; NXVM owns every remaining implementation below
`src/app-nxvm/`, its tests below `test/app-nxvm/`, and its generated local
artifacts below `assets/binary-nxvm/`.  A future MyNES product can enter as
the peer roots `app-mynes`, `test/app-mynes`, and `assets/binary-mynes`.

## Scope

- Consume the owner's existing physical moves as one Git rename-only layout
  migration; do not redesign device, machine, profile, shared-component, or
  runtime behavior.
- Repair all live NXVM build, verifier, script, test, documentation, and
  deployment references to the new roots.
- Preserve the existing four NXVM profile artifact directories as children of
  `assets/binary-nxvm/`; do not add a MyNES directory or empty product stub.
- Preserve the independent Lib/Common/x86 suites.  NXVM-only unit and
  integration tests live beneath `test/app-nxvm/`.

## Invariants

- `lib`, `common`, and `x86` remain the only shared first-level source and
  test components; their public contracts and source identity do not change.
- `app-nxvm` owns all NXVM-specific device, machine, profile, product and
  application composition code.  Moving it does not make those mechanisms
  common or create a second implementation path.
- Only the selected product writes under its ignored `assets/binary-<product>`
  root.  Existing NXVM.ini files remain adjacent to their product artifacts.
- Current documents name the new layout. Historical records retain their
  original paths as historical facts.

## Completion Standard

Every live source/build/test/tool/deployment reference resolves through the
new NXVM product roots; no live old root remains outside explicitly historical
documentation. CMake configures, the complete repository-only unit suite and
the retained external integration suite pass, documentation governance passes,
and the committed rename graph has no unintended generated binary or external
asset addition.
