# M5 YAML Asset And Read-Only CMOS Seed Contract

## Purpose

Make every YAML-declared media image explicit about its host backing and write
disposition, while making `firmware.cmos` one simple immutable cold-start seed.
Removable and fixed media choose `readonly`, `direct`, or `overlay`; CMOS has
no mode, is never written back, and is copied into Core-owned writable NVRAM
for the life of one session.  This is also the one integration-test asset path:
tests load the same session YAML as the product rather than copying media,
injecting a replacement, or building a parallel session configuration.

## Contract

Each YAML media entry keeps its path and adds a `mode`; CMOS keeps only its
path:

```yaml
firmware:
  bios:
    path: ../nxvm-assets/profiles/example/firmware/system.rom
    mode: readonly
  cmos: ../nxvm-assets/profiles/example/cmos/default.cmos
media:
  removable:
    - path: ../nxvm-assets/media/msdos-5-360k.img
      mode: overlay
  fixed:
    - path: ../nxvm-assets/media/user-disk.img
      mode: direct
```

- `readonly` uses the selected source as read-only backing.  Guest writes are
  rejected by the owning device, so the source is never changed.
- `direct` uses the selected source as the writable backing.  Guest writes are
  persisted to that exact file; it is an explicit user request, never a
  fallback.
- `overlay` reads the selected source into session-owned memory.  Guest writes
  affect only that in-memory copy and are discarded when the session closes.

ROM remains Core-immutable regardless of backing mode: its valid YAML
disposition is `readonly`.  CMOS is always a 64-byte read-only seed: session
composition copies only `0Eh`--`3Fh` into Core, which owns the live RTC and
NVRAM state; BIOS Setup changes are session-local and disappear at cold start.
Disk media may use all modes where their device contract permits writes.  No
profile, CLI, test runner, or firmware path may add a second copy/write policy.

## Scope And Design

VM YAML parsing produces one frozen asset request.  Media has path, role and
load mode; CMOS has just its read-only seed path.  Session composition creates
exactly one backing object or NVRAM copy for the owning Core device:

`YAML request -> frozen VM image request -> one device backing -> Core owner`

The VM owns host-file access, overlay allocation and profile-specific semantic
translation from declared topology to the seed's board bytes.  Core owns ROM
mapping, live CMOS state and controller/media state; it receives no host path
and performs no file I/O.  An overlay is not a second device state or an
additional media path: it is merely the one selected backing for that device.

### CMOS Schema

YAML and VM composition use one machine-neutral semantic schema; neither may
write a raw CMOS offset.  A field omitted from YAML retains its seed value.
Profile composition is the only translator from an explicit field into its
machine's completed NVRAM image.

| Semantic field | Type | Meaning |
| --- | --- | --- |
| `memory_kib` | optional unsigned total | Installed physical RAM.  It is the sole user-facing memory input; the profile derives conventional and BIOS-visible extended-memory fields from its actual map. |
| `display_adapter` | optional enum | Installed MDA/CGA/EGA/VGA-class adapter, used for the board's equipment representation; it is distinct from the option-ROM asset path. |
| `fpu` | optional enum | Installed coprocessor topology and its documented board-visible representation. |
| `floppy_drives[]` | ordered capability array | Installed mechanical drives by controller slot and supported physical format, not mounted floppy images. |
| `fixed_disks[]` | ordered controller-device array | Installed fixed-disk personality, BIOS type and only source-qualified geometry; it is not a guest partition or DOS drive letter. |
| `boot_preference` | optional ordered device-kind array | Encoded only where the selected profile's BIOS/CMOS fields are documented; otherwise the seed/BIOS retains its own rule. |
| `vendor_settings` | profile-specific typed substructure | Only ROM/manual-proven vendor NVRAM settings, such as Model-40-specific bytes; never a free offset/value map. |

Equipment and checksum bytes are derived outputs, never independent YAML
inputs.  IBM 5160 consumes the common hardware schema but has the explicit
`no_cmos` disposition.  IBM 5170 encodes the standard AT fields and checksum;
Model 40 reuses the common fields and adds only its evidence-backed Compaq
settings.  For example, Model 40's installed 2 MiB may map to 640 KiB basic
and 1 MiB standard extended memory in CMOS while its remaining board-mapped
RAM is not falsely reported as ordinary AT extended memory.

Existing media YAML that omits `mode` must be rejected with a clear diagnostic;
`firmware.cmos` must not accept one.  Every CMOS-capable profile must name one
external seed, while the 5160 explicitly names `null`.  The task will migrate
repository session templates explicitly rather than retain a compatibility
default.  Normal integration tests use their declared external assets through
`readonly` or `overlay`; they never modify those source assets.
An integration test may use `direct` only when persistence is its asserted
behavior and its YAML names a purpose-built writable test image.  The runner
does not create, copy, replace, or transform that image.

## Subtask Plan

1. **S1 - request grammar and CMOS collision ledger.** Audit all image loading
   routes and YAML templates; define the media mode table plus every
   profile/YAML topology field that maps to CMOS.  Record the one 5160
   no-CMOS disposition and direct diagnostics for missing or contradictory
   seeds.
2. **S2 - one composition/backing implementation.** Implement the frozen
   media backing factory and the sole immutable CMOS-seed copy/composer.
   Retire direct/overlay CMOS branches and profile-side post-load defaults
   while preserving Core's host-path-free boundary.
3. **S3 - template and product migration.** Add explicit modes to every
   supported media entry, use only `firmware.cmos: <path>` (or `null`), and
   migrate product/session callers to the one contract.  Confirm a cold start
   restores the immutable seed, BIOS Setup writes remain local, `readonly`
   never changes source bytes, `direct` persists changes, and `overlay`
   discards them.
4. **S4 - direct-YAML integration migration and closure audit.** Make every
   integration runner open its declared session YAML through the product
   catalog/provider route.  Delete runner-local media copying, transformation
   and parallel configuration.  Cover each legal role/mode pair,
   invalid/omitted modes, all profile templates and the integration asset path;
   then run complete unit and external integration gates and conduct the
   required similar-issue/code-path audit.

## Non-goals

- Do not change user-managed `build/output` YAML.
- Do not add media snapshot formats, copy-on-write block layers, implicit
  temporary files, test-runner media copies/transforms, or profile-specific
  write semantics.
- Do not import, modify, or commit protected external firmware or guest media.
- Do not expose host paths or file handles to Core.

## Exit Criteria

- Every YAML media entry has an explicit legal mode and one diagnostic path for
  an omitted or incompatible mode; every CMOS-capable YAML has exactly one
  read-only 64-byte seed and no CMOS mode.
- `readonly`, `direct`, and `overlay` have the stated, verified persistence
  behavior for every supported writable media role; ROM and CMOS seed stay
  immutable.
- All profiles and integration session paths consume the same frozen request
  and backing factory, with no hidden fallback, runner-local media hack or
  parallel policy.
- Full unit and external integration suites pass without modifying external
  source assets, except where a persistence test deliberately declares its
  own dedicated `direct` image in YAML.
