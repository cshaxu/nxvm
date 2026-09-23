# T536 S1 Product-Namespace Layout

## Owner Layout Contract

The owner supplied the physical rename and approved its repository-wide
completion.  The sole live layout is:

```
src/{lib,common,x86,app-nxvm}
test/{lib,common,x86,app-nxvm}
assets/binary-nxvm/<profile>/NXVM.ini
```

`lib`, `common`, and `x86` stay peer shared corpora.  `app-nxvm` is the sole
NXVM product namespace.  A later product may add only peer roots such as
`app-mynes`, `test/app-mynes`, and `assets/binary-mynes`; no compatibility
root or product-specific shared path was retained.

The external BYOB archive was renamed by the owner as well.  CMake now selects
firmware from `../nxvm-assets/profiles-nxvm` and test/runtime media from
`../nxvm-assets/media-nxvm`.  Profile INIs retain relative references to that
external archive.

## Repairs

- Root CMake, profile binding generation, deployment and all live verification
  scripts now resolve the product roots.
- NXVM-only tests are under `test/app-nxvm/unit` and
  `test/app-nxvm/integration`; the independent shared suites remain under
  `test/lib`, `test/common`, and `test/x86`.
- Active architecture, coding, execution and asset documentation names the
  new roots.  Historical records and evidence retain their original paths.
- The old T345 residual inventory was corrected after the move: the relocated
  Model 40 composition is a retained-strict `vm-profile` source, so it is not
  a deferred `vm-machine` residual.

## Verification

- Fresh CMake configure succeeds with the renamed external archive roots.
- `ctest --test-dir build/mingw-gcc-x64 -L unit -j 4 --output-on-failure`:
  **336/336 passed** in 26.78 seconds.
- `ctest --test-dir build/mingw-gcc-x64 -L integration -j 4 --output-on-failure`:
  **20/20 skipped, 0 failed**.  All require owner-provided external runtime
  assets unavailable in this workspace; no result is claimed as behavioral
  integration success.
- Direct T345 deferred-ownership verification passes: 210 rows and 35 exact
  residual production entries.
- Documentation governance passes for `vm-0-5-0535`.
- A live-tree sweep excluding historical/research/evidence archives finds no
  former `src/app`, `src/core`, `test/app`, `test/core`, `test/integration`,
  `assets/sessions`, `assets/binary`, or old `nxvm-assets/profiles`/`media`
  reference.

## Review

The Git change is a rename-dominant migration: source implementation behavior,
public C ABI, shared corpus identity and profile semantics are unchanged.  No
EXE, ROM, CMOS seed, disk image, or other external binary is staged.
