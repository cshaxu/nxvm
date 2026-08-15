# T376 S2: Raw-IMG Sidecar Lifecycle

`M5:T376:S2:RAW-IMG-SIDECAR-LIFECYCLE:OK`

## Delivered Contract

`vm_machine_fdd` owns an in-memory mark byte for every logical sector and
publishes it only through the existing frozen media-provider address-mark
callbacks. Raw images without a sidecar mount with all marks set to ordinary
data. A sidecar is optional and has no sector payload.

The canonical UTF-8 JSON form is a fixed, deterministic object with `version`
1, `raw_size`, FNV-1a `raw_checksum`, `[cylinders,heads,sectors,bytes]`
geometry and a logical-sector-order `deleted` bit string. The parser accepts
only that complete form. Truncation, extra content, duplicate/reordered
fields, invalid bits, geometry mismatch and raw checksum mismatch reject the
mount before either candidate bytes or marks are published.

FDD eject serializes the raw bytes and sidecar before calling the VM media
pair-save helper. It stages both files, moves old files to distinct backups,
publishes both replacements, and restores old names when a replacement fails.
An old backup collision fails before guest-visible FDD removal. Raw `.img`
files remain sector-payload files usable without reading the adjacent JSON.

## Focused Proof

`vm-media-provider-smoke` now proves:

- a raw-only FDD publishes address-mark capability and defaults to ordinary
  data marks;
- setting a deleted mark, ejecting and reinserting preserves both that mark and
  the raw sector byte;
- changing raw data while retaining the sidecar makes a fresh mount fail and
  leaves its candidate FDD absent; and
- a reserved sidecar backup name makes paired save fail while the existing FDD
  remains mounted.

The focused GCC build and run emit:

```text
M5:T376:S2:RAW-IMG-SIDECAR-LIFECYCLE:OK
```

## Boundary And Transfer

This S does not add 8272A command decoding. Read Deleted Data, Write Deleted
Data, normal-read Control Mark and Scan remain the next controller-owned T376
receivers. They must use these provider callbacks, the single existing DMA2 and
IRQ6 path, and T375's accepted byte/seek cadence; they may not acquire a host
file or test-only route.
