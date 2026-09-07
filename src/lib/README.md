# Shared Library

## Publish Invariants

- `src/lib` completely encapsulates host/platform code, symbols and handles.
  A product includes only documented lib contracts; native headers and APIs are
  lib-private implementation detail.
- First-level components are peers. They may depend on `base`, never on one
  another; a product composition root is the only place that combines them.
- `base` is the sole library type facade. Public lib contracts use its `lib_*`
  aliases (`lib_size` for byte counts and offsets), never raw standard-library
  or host SDK types.

Each first-level directory is an independent capability. A product composition
root may combine them; a library component may not include another component.

- `ux` owns copied presentation values and its native Console/Window loops.
  Its public headers contain no host SDK type or product-owned pointer. Native
  code belongs below its host directory and depends only on the public `ux`
  contract. Both palette arrays use platform-neutral `0x00RRGGBB`; native
  adapters convert only at their own host boundary. The opaque presenter owns
  a latest-frame slot, a 32-command FIFO and their sole native wake. Products
  publish frames and request only target, Window title or stop; Console title
  mutation is never part of the contract.
- `host-sync` provides opaque native events, clocks and cancellable joined
  tasks. It contains no guest-time or product execution policy.
- `storage-medium` owns exclusive byte-medium leases. Direct and readonly media retain
  their backing file for offset I/O; overlay retains an immutable base plus only
  changed 4-KiB pages, then discards them. Product code owns storage topology
  and selects safe replacement points.
