# Source Layout

This is the macro layout authority. Apply [Coding Rules](../rules/CODING.md);
dependencies belong to [System Architecture](ARCHITECTURE.md).

## Current And Target Trees

Retain the App/Core/shared-corpus layout. The tree below is the migration
target, not a claim that Devices, INI or four build targets already exist.

```text
src/
  lib/                  shared C and platform services
  common/{session,machine,ui}/
  x86/{xasm32,debug}/
  app/                  main, INI configuration, CLI, composition, recorder
  core/
    devices/            reusable CPU/device/memory/bus/time and execution
    machine/            NXVM driver, asset/media and execution adapter
    profile/
      xt/               IBM 5160 board composition and firmware slots
      at/               IBM 5170 board composition and firmware slots
      standard/         selected board C, frozen configuration and ROM slots
      pc110/            PC110 board C, frozen configuration and ROM slots
```

Keep shared profile declarations and proven helpers at the profile root.
Do not add a framework or empty future directories. CPU-family implementations
and selection tables stay in generic Core, not copied into board directories.

Profile-specific ROM source and mapping declarations live with the profile.
Protected payloads remain external in owner-managed
`nxvm-assets/profiles/` per machine; original manuals
remain in `nxvm-assets/manuals/`. Documentation changes do not move assets.
The common NXVM.ini lives beside the executable; relative paths resolve from
that file. Existing session YAML remains the runtime baseline until cutover.
Do not rename/move external assets merely to match target source directory names.

## Files And Names

Headers stay beside implementation; only `*_interface.h` is public across
components. Keep cohesive files flat until a real subsystem needs a directory.
Stable symbol prefixes need not change because a directory moved.
`src/lib/types` owns shared C vocabulary; legacy root aliases need a separate
caller migration, not another facade.

One build-selected profile entry supplies the existing factory. Prefer direct
construction and small immutable descriptions over string dispatch, recursive
inheritance or duplicated build source lists. Define the concrete interface
from actual construction requirements during implementation.

## Source Organization

Repository-only tests mirror owners under `test/{app,core,lib,common,x86}`.
Profile tests mirror `test/core/profile/{xt,at,standard,pc110}` when implemented.
Move generic Core tests to `test/core/devices` with their source owner, preserving
CPU-family tests including unused models. `test/integration/` stays separate;
at cutover it uses the real INI path and external assets. Unit tests use code-
owned values without external ROM/INI/YAML/media dependencies.

Retire tests only for explicitly retired product behavior. Rehome generic CPU,
chip, transaction, lifecycle and failure regressions before removing board
fixtures. Smaller product matrices must not hide reduced retained coverage.
