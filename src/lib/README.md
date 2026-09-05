# Shared Library

Each first-level directory is an independent capability. A product composition
root may combine them; a library component may not include another component.

`ux` owns copied presentation values and its native Console/Window loops.
Its public headers contain no host SDK type and no product, machine, profile,
or guest pointer. Native code belongs below its host directory and depends only
on the public `ux` contract.
