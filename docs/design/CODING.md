# Source Layout

This is the macro layout authority. Apply [Coding Rules](../rules/CODING.md);
dependencies belong to [System Architecture](ARCHITECTURE.md).

## Current And Target Trees

The App/Core/shared-corpus layout is current. `devices`, `machine` and
`profiles` are live owner roots; `pc110` remains a future Profile only when its
separate evidence task admits real source files.

```text
src/
  lib/                  shared C and platform services
  common/{session,machine,ui}/
  x86/{xasm32,debug}/
  app/                  main, INI configuration, CLI and composition
  core/
    devices/            reusable CPU/device/memory/bus/time and execution
    machine/            NXVM driver, asset/media and execution adapter
    profiles/
      xt/               IBM 5160 board composition and firmware slots
      at/               IBM 5170 board composition and firmware slots
      model40/          retained DeskPro 386 composition and firmware slots
      default_profile/  retained default PC/AT composition and firmware slots
      pc110/            PC110 board C, frozen configuration and ROM slots
```

Keep shared profile declarations and proven helpers at the profiles root. The
former singular `core/profile` root is retired; do not restore a compatibility
directory. Preserve existing machine identities and
variants rather than renaming them into a replacement Standard profile.
Do not add a framework or empty future directories. CPU-family implementations
and selection tables stay in generic Core, not copied into board directories.

Profile-specific ROM source, mapping declarations and an asset manifest live
with the profile. Protected payloads remain external in the user-supplied,
owner-managed `nxvm-assets/profiles/<machine>/` tree; original manuals remain
in `nxvm-assets/manuals/`. CMake receives the untracked absolute
`NXVM_PROFILE_ASSETS_ROOT` for a selected build and creates an ignored generated
configuration header/source containing that local root. It neither copies ROM
bytes into the source/build output nor permits an unconfigured generic root.
Documentation changes do not move assets. Each generated product EXE and its
common NXVM.ini live only in the ignored
`assets/binary/<profile>/` directory; relative runtime-media paths resolve from
that file. It has no firmware/CMOS/font asset path keys. NXVM.ini is the sole
product runtime configuration route; repository-only tests do not load it.
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
Profile tests mirror `test/core/profiles/{xt,at,model40,default_profile,pc110}`
when implemented; migrate the current singular test owner with its source.
Move generic Core tests to `test/core/devices` with their source owner, preserving
CPU-family tests including unused models. `test/integration/` stays separate;
at cutover it uses the real INI path and external assets. Unit tests use code-
owned values without external ROM/INI/YAML/media dependencies.

Preserve every implemented board's tests and boot scenarios. Rehome CPU, chip,
transaction, lifecycle and failure regressions with their owner. Factoring
fixtures or removing duplicate machinery must not reduce behavior coverage.
