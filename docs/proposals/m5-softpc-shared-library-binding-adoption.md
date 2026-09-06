# M5 SoftPC Shared-Library Binding Adoption

## Goal

Adopt NXVM's manifest-selected `src/lib` corpus in SoftPC unchanged, adding
only a SoftPC-owned binding for host presentation, lifecycle, storage and
observability mechanics.

## Boundary

The source list and normalized hashes in NXVM's `src/lib/MANIFEST.sha256` are
the import authority. The adopting task copies every listed file byte-for-byte
and must fail its admission check if a local copy differs. It may not edit a
selected library source to fit SoftPC.

SoftPC retains its machine, CPU, devices, firmware, profiles, guest media,
rendering-state ownership and product policy. The binding converts its copied
frame/input values and registers its product actions; it does not expose a
machine pointer, guest pointer or native handle through `lib`.

## Slices

1. **S1 - Read-only binding inventory.** Map every SoftPC host/UI, lifecycle,
   image and outcome route to one library contract or one distinct
   machine-owned retained route. Report a gap before changing any source.
2. **S2 - Byte-identical corpus and product binding.** Copy the manifest
   corpus unchanged; add only SoftPC-boundary adapters and replace any complete
   generic predecessor. Preserve the existing SoftPC machine baseline.
3. **S3 - Consumer and route closure.** Run the neutral library conformance
   suite plus SoftPC product-local regressions; prove no duplicate generic
   presenter, lifecycle, storage or host-sync path remains.

## Acceptance

- Every selected library file hash matches NXVM's manifest.
- SoftPC modifies no selected `lib` source and does not require NXVM headers,
  binaries or runtime linkage.
- SoftPC has one product binding per adopted mechanism; all remaining routes
  have a distinct machine-owned reason.
- No guest execution, controller, firmware or profile behavior changes merely
  to accommodate the library.
