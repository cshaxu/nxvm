# T524 S6: Shared Storage Lease Contract

## Disposition

`lib/storage` remains a byte-image and persistence capability, not a media or
device subsystem.  It depends only on `lib/base` and exposes three byte-image
modes: direct-readonly, direct-writable, and private writable overlay.

Every `lib_storage_image` is an exclusive lease.  A caller must select its own
safe replacement point; `lib_storage_image_replace()` atomically transfers a
replacement lease (including no lease) into the caller slot, returns the
retired lease through an initially-null output, and neither destroys nor
mutates either image.  Empty-to-empty is a successful no-op; a nonempty
self-alias, output alias, or occupied retired output is rejected and leaves
the caller slot unchanged.  `commit()` similarly leaves the lease and its
bytes unchanged on failure.  Destruction is an explicit caller operation after
the successful transfer.

This supplies the missing generic transaction without moving topology,
geometry, scheduling, or persistence policy into the library.

## Product Binding And Sweep

`vm_machine_fdd_commit_candidate()` and
`vm_machine_hdd_commit_candidate()` are the only existing live candidate-image
swap points.  Both now call the one lease replacement API, then update their
already-owned derived byte view and retire the old lease.  Creation and final
destruction remain local ownership transitions, not replacement transactions.
The source sweep found no other candidate-to-live image assignment or image
replacement path.

The neutral public consumers are `storage_image_smoke` (all three modes,
successful replacement, no-lease replacement, empty no-op, alias rejection,
and commit failure preservation) and `storage_file_smoke`; the standalone neutral
consumer conformance build also compiles the public storage interface without
product headers.

## Verification

- Focused `storage-image`, `vm-media-provider`, `vm-hdc-port`, and
  `vm-fdc-port` checks pass.
- Full repository unit: 311/311 pass in 26.07 seconds.
- `cmake -DLIBRARY_ROOT:PATH=src/lib -P src/lib/verify_manifest.cmake` passes.
- Standalone `src/lib` MinGW Makefiles configure/build and CTest pass.
- Strict Linux C11 syntax for storage common/Linux implementations and their
  repository-only tests passes.
- Storage-root direct-include and forbidden-vocabulary sweeps pass after the
  retained README wording was made neutral; whole-corpus closure remains S8.
- Documentation governance and `git diff --check` pass.

## Simplicity Result

The retained production path is one neutral image replacement transaction.
FDD and HDD no longer each spell their own pointer swap.  The implementation
adds 27 production lines and removes 5; the test adds 19 lines.  No wrapper,
parallel image state, or product-specific storage branch remains.
