# M5 T234: Core Utility Boundary

## Purpose

T234 closes the P1 dependency leak in which VM peer modules import
`core/product` utilities. It establishes `core/utils` as the only shared,
strictly neutral utility and callback layer. This is an architectural closure
task; it does not change guest-device, BIOS, DOS, or user-visible behavior.

## Contract

- `core/utils` depends only on `type-facade` and exports `core_utils_*` APIs.
- It may contain small stateless helpers and opaque callback scopes needed by
  otherwise independent owners, including the injected wait contract.
- It may not include or expose machine state, host-native handles, product
  commands, debug UX, assembler/disassembler behavior, registries, profiles,
  or composition policy.
- `core/machine`, `core/platform`, and `core/product` remain independent peers;
  `vm/{machine,platform,product,profile}` and VDM equivalents remain peers.
  Composition alone integrates product peers.

## Sequence And Exit

1. Record the permitted source and CMake target DAG; remove target-only peer
   edges, including `vm-machine -> vm-profile`, unless a documented public
   contract proves one necessary.
2. Move the wait callback/scope from `core/product` to `core/utils`; composition
   continues to own its implementation and lifetime.
3. Remove unused `core/product/utils.h` imports and the disabled machine debug
   assembler diagnostic. Do not replace them with a generic convenience API.
4. Design and approve the retained default-profile BIOS image migration before
   removing its active assembler dependency. Prefer profile-owned byte images
   or descriptors; a temporary adapter, if unavoidable, belongs in composition.
5. Add a mechanical source and target-DAG gate that rejects peer imports and
   links forbidden by Module Layout. Run the applicable GCC and smoke matrix.

T234 closes only when no VM/VDM peer imports `core/product` outside its matching
`*/product` module, no target conceals a forbidden peer edge, and the retained
BIOS-image path has one documented compliant owner.
