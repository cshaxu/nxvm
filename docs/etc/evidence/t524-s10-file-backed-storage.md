# T524 S10 File-Backed Storage

## Decision

`lib/storage/medium.h` is the sole neutral byte-medium API. `open()` selects
one mode once; every later read, write, fill, flush and discard uses the same
opaque lease.

| Mode | Base | Write result | Removal |
| --- | --- | --- | --- |
| Direct | retained read/write file handle | offset write-through | closes file |
| Readonly | retained read-only file handle | rejected | closes file |
| Overlay | retained read-only file handle | allocates/updates only affected 4-KiB dirty pages | closes base and discards pages |

The overlay has no commit operation and never materializes the whole source
image. Anonymous zero-base overlays remain a repository-only construction aid
for blank media and unit fixtures; mounted image files always use `open()`.

## Owner Sweep

- FDD retains geometry and deleted-data marks, but all byte access and file
  ownership use the medium lease.
- HDD retains its logical capacity and geometry, but all byte access and file
  ownership use the same medium lease.
- The obsolete RAM image, sidecar persistence, commit API and integration
  commit toggle are removed. No FDD/HDD source retains native file I/O or a
  full-file loader.

## Focused Proof

- `storage_medium_smoke`: Direct writes through; readonly rejects writes;
  untouched overlay pages observe the retained base on demand; a dirty page
  masks later base changes.
- `vm_media_direct_readonly_smoke`: FDD/HDD readonly rejection and Direct
  write-through.
- `vm_media_provider_smoke`: FDD/HDD overlay writes remain absent from the
  original host files after eject.

The active task retains full unit/integration and portable-library gates as
required before S10 delivery and owner review.
