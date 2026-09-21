# Source Layout

This is the macro layout authority. Apply [Coding Rules](../rules/CODING.md);
dependencies belong to [System Architecture](ARCHITECTURE.md).

## Current And Target Trees

Retain the App/Core/shared-corpus layout. Standard and PC110 are target profile
subtrees; current XT/PC-AT/Model-40 files are migration inputs, not evidence
that the new targets exist.

```text
src/
  lib/                  shared C and platform services
  common/{session,machine,ui}/
  x86/{xasm32,debug}/
  app/                  main, config/catalog, CLI, composition, recorder
  core/
    core/               reusable CPU/device/memory/bus/time implementation
    machine/            NXVM driver, asset/media and execution adapter
    profile/
      standard/         selected board C, frozen configuration and ROM slots
      pc110/            PC110 board C, frozen configuration and ROM slots
```

Keep shared profile declarations and proven helpers at the profile root.
Do not add a framework or empty future directories. CPU-family implementations
and selection tables stay in generic Core, not copied into board directories.

Profile-specific ROM source and mapping declarations live with the profile.
Protected payloads remain external in owner-managed
`nxvm-assets/profiles/standard/` and `profiles/pc110/`; original manuals
remain in `nxvm-assets/manuals/`. Documentation changes do not move assets.
Session templates stay in `assets/sessions/`; paths are relative to their YAML.

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
Profile tests mirror `test/core/profile/{standard,pc110}`. Keep CPU-family
tests at the Core owner, including models unused by either executable.
`test/integration/` stays separate, using YAML and external assets; unit tests
use code-owned values without external ROM/YAML/media dependencies.

Retire tests only for explicitly retired product behavior. Rehome generic CPU,
chip, transaction, lifecycle and failure regressions before removing board
fixtures. Smaller product matrices must not hide reduced retained coverage.
