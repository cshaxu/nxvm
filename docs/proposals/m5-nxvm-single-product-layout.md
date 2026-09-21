# M5 NXVM Single-Product Layout

## Goal

Make NXVM a single virtual-machine product with four top-level source
components: shared `lib`, `common`, and `x86`, plus NXVM-owned `app` and
`core`. Retire the non-runnable VDM product line rather than retaining an
empty or forwarding skeleton.

The approved source relocation is:

```text
src/vdm              -> deleted
src/vm/app           -> src/app
src/core/machine     -> src/core/core
src/vm/machine       -> src/core/machine
src/vm/profile       -> src/core/profile
```

`src/vm/main.c` and its App-only immutable session-request value become the
App entry point and `src/app/request_interface.h`. Every retained
repository-only test mirrors the final source owner: `test/app`,
`test/core/core`, `test/core/machine`, or `test/core/profile`.
`test/integration` remains a separate external-asset boundary.

## Boundaries

- `lib`, `common`, and `x86` remain byte-identical shared components; this
  task does not alter their API, implementation, manifests, or tests.
- `app` owns NXVM executable composition, CLI/configuration and product UX.
- `core/core` retains the existing neutral guest-emulation implementation.
- `core/machine` owns the NXVM machine adapter, Core composition and media
  provider binding; `core/profile` owns NXVM profile selection and immutable
  board/firmware declarations.
- No empty `vm`, `vdm`, or test mirror shell survives. Existing public symbol
  names remain unless a path-specific name prevents an unambiguous owner.

## Planned Batches

1. **S1: complete relocation ledger.** Inventory every tracked production,
   test, CMake, tool and documentation path below `src/vm`, `src/vdm`,
   `test/vm`, and `test/vdm`. Fix the complete destination and deletion
   disposition for residual root files and tests before moving code.
2. **S2: source and build relocation.** Use Git moves for the approved source
   mapping and all S1-resolved residual owners; update direct includes, CMake,
   static gates and architecture/source-layout authorities. Delete VDM only
   with all of its build/test consumers.
3. **S3: mirrored-test and closure sweep.** Move or retire every affected
   repository-only test to its final owner mirror, retain integration as its
   own root, prove no former path survives, and run the complete unit suite,
   integration suite, dual Release builds and documentation governance.

## Completion Standard

The source and test trees contain no `vm` or `vdm` root. All production and
test paths, includes, CMake entries and static gates use the new owner map.
There is one NXVM entry/composition route, no forwarding compatibility tree,
and no behavior change. Closure records every deleted or relocated path and
the remaining external integration status truthfully.

## S1 Frozen Disposition

- `src/vm/main.c` and `src/vm/request_interface.h` move with the App to
  `src/app/`. The request is parsed and consumed only by App catalog,
  configuration and composition; it is not a Core machine configuration API.
- The former generic Core source and every repository-only Core test move from
  `core/machine` to `core/core`, including existing flat `test/core/*`,
  `test/core/machine/*`, and their Core-only fixtures. Public `core_machine_*`
  symbols do not change merely because their owning directory does.
- The NXVM adapter and its media provider move from `vm/machine` to
  `core/machine`; adapter tests and their unit-ROM fixtures move to
  `test/core/machine/` and its local `support/` subtree.
- Profile declarations and their repository-only tests move from `vm/profile`
  to `core/profile` and `test/core/profile`. Their in-process ROM/CMOS byte
  fixtures remain unit fixtures; no external asset enters a repository-only
  test.
- `src/vdm` and `test/vdm` have no retained consumer. Their two libraries,
  two smoke targets, unit registrations, compile options, production-target
  list and VDM-only static gate are deleted in the same source/build batch.
- `test/integration` is not moved or reclassified. Its external-ROM/media
  boundary remains intact even where it reuses a Core fixture.
- CMake target names and public C symbols remain stable in this layout-only
  task. Direct source paths, include paths, source-layout authorities and
  static-gate file paths change together; no compatibility include or empty
  `vm`/`vdm` tree survives.
