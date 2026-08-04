# Source Layout

The only source roots are `core`, `vm`, and `vdm`.

- `core/{machine,platform,product}` contains behavior shared by VM and VDM.
  A concrete host implementation belongs under `core/platform/<host>` only when
  it is reusable by both products without product policy.
- `vm/{machine,platform,product,profile}` contains bootable whole-machine
  behavior, retained VM Console/debugger behavior, and machine profiles.
- `vdm/{machine,platform,product,profile}` contains DOS application-runner
  behavior and DOS execution profiles.

Public headers live beside their implementations. The M1 NXVM import has been
provenance-recorded and fully migrated; no baseline or adapter source root is
part of the product build.
