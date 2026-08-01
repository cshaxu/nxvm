# Contract Design

## Authority

This is the sole forward interface-design authority. It complements
`module-layout.md`: that document owns source boundaries, while this document
owns the public C contracts crossing those boundaries. Requirements describe
observable behavior; planning documents do not define interfaces.

## Foundation

`src/type.h` is the common system type header. It defines `BOOL`, `SIZE_T`,
fixed-width aliases, `STATUS`, and the stable `STATUS_*` result constants.
`src/type.c` exists only if a non-inline type helper is necessary.

`src/version.h` and `src/version.c` are the only version and build-identity
source. They provide product banner identity and build-time information. No
module contract contains an ABI version, timestamp, compatibility probe, or
module-local status type.

Public symbols use source-path ownership: `core_machine_*`,
`core_platform_*`, `core_product_*`, `vm_machine_*`, `vm_platform_*`,
`vm_product_*`, `vm_profile_*`, and VDM counterparts. Root composition uses
`vm_composition_*` and `vdm_composition_*`.

## Common Rules

- Public state is opaque; configuration, commands, snapshots, and callback
  payloads are plain C data structures.
- `*_create` transfers a created handle to its caller; `*_destroy` releases
  only resources owned by that module and accepts `NULL`.
- Inputs are borrowed unless an API explicitly registers or retains them.
  Outputs are copied into caller-provided storage or copied callback payloads.
- `STATUS` distinguishes invalid arguments, invalid state, unsupported
  capability, no memory, busy, and fault. It never carries product policy.
- Callbacks state their thread, synchronization, ownership, and teardown rule.
  They do not re-enter mutable operations on their originating object.

## Contract Sequence

The following sections are completed in order before a migration changes the
corresponding runtime path:

1. `core/machine`: lifecycle, execution, memory/port/interrupt, provider, and
   read-only state contracts.
2. `core/platform`: host capability providers plus platform frame/event
   contracts, with no machine type dependency.
3. `core/product`: generic command, debug, trace, and registry target
   contracts, with no machine or platform type dependency.
4. Root composition: VM/VDM adapters for machine snapshots, platform events,
   product targets, callback binding, and teardown.
5. Profile override: ROM assets, declarative metadata, and limited firmware
   callbacks against public core contracts.

No implementation detail or old M2/M4 contract becomes current merely because
it remains in `history/`.
