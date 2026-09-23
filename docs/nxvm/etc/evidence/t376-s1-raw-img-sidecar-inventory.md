# T376 S1: Raw-IMG Sidecar Owner And Lifecycle Inventory

`M5:T376:S1:RAW-IMG-SIDECAR-INVENTORY:OK`

## Current Owner Graph

| Route | Current owner and state | S1 disposition |
| --- | --- | --- |
| Guest media contract | `core/machine/media_interface.*` owns immutable registry dispatch, geometry validation, address-mark capability and flush dispatch. | Reuse this ABI unchanged; FDC must observe marks only through it. |
| Raw floppy resident state | `vm/machine/fdd.*` owns 1.44-MB raw bytes, geometry, insert/remove/reset, generation and the VM FDD media provider. It currently omits `ADDRESS_MARKS` and returns no mark callbacks. | This is the sole future owner for in-memory mark state, raw-only defaults and mount/eject transitions. |
| Host persistence | `vm/machine/media_save.*` owns a single-file temporary-write/replace helper. `vm_machine_fdd_remove_for` invokes it for raw bytes only. | Add one VM-owned paired-media operation; do not let FDC, tests or console write sidecar files. |
| Product ingress | `vm_session_insert_fdd` validates/copies the image path and calls FDD insertion; the console adapter delegates to the session. | A same-basename sidecar is discovered solely as part of FDD mount; no new host-facing or test-only input API is needed. |
| Controller behavior | `core/machine/fdc.c` accepts only ordinary `READ DATA` (`06h`) and `WRITE DATA` (`05h`) transfer dispatch. The opcode mask/length switch has no Read/Write Deleted Data or Scan family route. | Later controller subtasks bind supported address-mark operations to the existing DMA/IRQ/reset/timing transfer owner. |

## S1 Contract

An ordinary raw `.img` remains the complete payload artifact. On mount, absence
of `<img>.json` initializes every logical sector as ordinary-data marked. If a
sidecar exists, `vm_machine_fdd` validates it before publishing either bytes or
marks: schema version, raw-image identity/size, frozen geometry and each
logical sector position must be exact, non-duplicated and in range. The
sidecar carries no payload, CHRN inference, flux, gaps, CRC or timing data.

Only FDD state mutation may change an address mark through the existing core
provider callback. That mutation participates in the FDD media generation and
the same flush/eject persistence boundary as raw bytes. A read-only FDD rejects
mark writes. Failed parse, allocation, raw read, sidecar read or paired save
leaves the currently mounted guest-visible media unchanged. Ordinary raw-only
mount/save remains supported without creating a sidecar.

The paired save must stage both replacement candidates before replacing either
published path, retain recoverable old files on a replacement failure, and
return failure rather than exposing a partially committed guest media pair.
This is a VM persistence contract, not a core media-registry extension.

## Implementation Receivers

1. **S2 -- FDD sidecar schema and paired persistence.** Add bounded parser,
   canonical serializer, in-memory marks, mount validation and failure-atomic
   raw-plus-sidecar save/remove lifecycle at the VM FDD/media-save owners.
   Focused raw-only, valid-sidecar, absent-sidecar, malformed/stale/duplicate,
   read-only and injected persistence-failure regressions prove the contract.
2. **S3 -- Deleted-Data controller path.** Add the documented 8272A Read
   Deleted Data, Write Deleted Data and normal-read Control-Mark results using
   the existing media capability, DMA2/IRQ6/reset owner and T375 128-tick byte
   cadence. It adds no FDC filesystem route or timing scalar.
3. **S4 -- Scan controller path and task closure.** Add Scan Equal,
   Low-or-Equal and High-or-Equal through the same transfer/cancellation
   machinery, then replay raw-only and sidecar media against T375 timing and
   perform the T376 transfer audit.

## Sweep And Verification

The sweep used:

```text
rg -n "address.mark|address_mark|deleted|Deleted|sidecar|media.*save|raw.*img|fdd\.img|floppy" src tests CMakeLists.txt
rg -n "vm_machine_fdd_(insert_for|remove_for|replace_bytes|media_provider)|vm_machine_media_save_atomically" src tests CMakeLists.txt
rg -n -C 4 "DELETED|Deleted|SCAN|Scan|READ_DATA|WRITE_DATA|address_mark|media_(get|set)_address_mark|media_flush" src/core/machine/fdc.c src/core/machine/fdc.h tests/machine tests/core
```

Production hits resolve to the owner graph above. Test fixtures with synthetic
marks prove the frozen core ABI but do not create a production sidecar path.
The VM FDD smoke currently proves raw-only non-support for marks and the
single-file atomic-save helper; S2 must replace that assertion only with a
production FDD provider behavior proof. T375's FDC cadence remains unchanged.
