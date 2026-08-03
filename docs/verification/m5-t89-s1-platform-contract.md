# M5 T89 S1: VDM Audit And Core Platform Contract

## Result

`vdm/machine/dos_minimal.c` has no mutable static state: the machine, text
snapshot, and keyboard queue belong to each DOS-minimal session. Its current
single-threaded scope needs no new synchronization code.

`core/platform/host_surface_interface.*` defines an opaque native-surface
context and an atomic, caller-owned lease. The contract is product-neutral and
does not include machine types, renderer storage, or policy.

## Verification

- `ntvdm64-core-platform-host-surface-smoke`:
  `M5:T89:S1:HOST-SURFACE-LEASE:OK`.
- `ntvdm64-vdm-minimal-session-smoke`: `M5:T13:S8:VDM-SESSION:OK`.
- `nxvm-dos-minimal-profile-smoke`: `M3:T3:S2:DOS-MINIMAL-PROFILE:OK`.
