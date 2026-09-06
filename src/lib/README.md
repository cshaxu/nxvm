# Shared Library

Each first-level directory is an independent capability. A product composition
root may combine them; a library component may not include another component.

- `base` provides the library's portable scalar, status and atomic vocabulary.
- `host` provides opaque native events and cancellable joined tasks.
- `session` provides opaque lifecycle state and product-run callbacks; it does
  not own a host thread or a machine safe point.
- `storage` owns image bytes and direct-readonly, direct-writable and overlay
  persistence mechanics. Product code owns drive/controller topology.
- `observability` publishes bounded copied outcomes.

`ux` owns copied presentation values and its native Console/Window loops.
Its public headers contain no host SDK type and no product, machine, profile,
or guest pointer. Native code belongs below its host directory and depends only
on the public `ux` contract.
