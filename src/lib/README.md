# Shared Library

Each first-level directory is an independent capability. A product composition
root may combine them; a library component may not include another component.

- `ux` owns copied presentation values and its native Console/Window loops.
  Its public headers contain no host SDK type or product-owned pointer. Native
  code belongs below its host directory and depends only on the public `ux`
  contract. Both palette arrays use platform-neutral `0x00RRGGBB`; native
  adapters convert only at their own host boundary. A product supplies a
  copied initial Window title and may later request a title only while a
  Window surface is active; Console title mutation is never part of the
  contract.
- `host-sync` provides opaque native events, clocks and cancellable joined
  tasks. It contains no guest-time or product execution policy.
- `storage-medium` owns exclusive byte-medium leases. Direct and readonly media retain
  their backing file for offset I/O; overlay retains an immutable base plus only
  changed 4-KiB pages, then discards them. Product code owns storage topology
  and selects safe replacement points.
