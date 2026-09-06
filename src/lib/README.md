# Shared Library

Each first-level directory is an independent capability. A product composition
root may combine them; a library component may not include another component.

- `base` provides the library's portable scalar, status and atomic vocabulary.
- `host` provides opaque native events and cancellable joined tasks.
- `session` provides opaque start, stop, reset and generic pause/resume state.
  Product code owns execution callbacks, host threads, safe points and every
  product-specific pause or stepping policy.
- `storage` owns exclusive byte-medium leases. Direct and readonly media retain
  their backing file for offset I/O; overlay retains an immutable base plus only
  changed 4-KiB pages, then discards them. Product code owns storage topology
  and selects safe replacement points.
- `observability` publishes bounded copied outcomes.

`ux` owns copied presentation values and its native Console/Window loops.
Its public headers contain no host SDK type or product-owned pointer. Native
code belongs below its host directory and depends only on the public `ux`
contract.
