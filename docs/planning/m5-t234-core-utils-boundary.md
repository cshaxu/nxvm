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
6. Remove target-only legacy links after source audit; a target names only its
   actual lowest-level dependency, never a convenient aggregate library.

T234 closes only when no VM/VDM peer imports `core/product` outside its matching
`*/product` module, no target conceals a forbidden peer edge, and the retained
BIOS-image path has one documented compliant owner.

## Completion Evidence

Source commit: `263bf0d` (`M5 T234 S2 P1`). `core/utils` now owns the injected
wait scope; `core/product` retains assembler/disassembler tooling only. VM
machine recording receives disassembly through a composition-owned callback,
and default-profile BIOS writes composition-preassembled code bytes without a
product import.

`current-gates-gcc` passed with MinGW-w64 GCC 16.1.0: the T234 boundary gate,
existing governance gates, and 67/67 current smoke tests passed. The current
artifact is `build/output/nxvm_0_5_0234.exe`, SHA-256
`1A94A5D3D9E09FD2302CE71C3DC51434E9AA9915C2F2879BB42589AAE29591E9`.

### S4 Closure

Removed the residual `vm-platform-requests -> core-machine` and
`vm-product -> core-machine` target-only links. The former now names
`type-facade`, its actual lowest dependency; the latter retains only
`core-product-utils`. The previously reported `vm-machine -> vm-profile` edge
was already removed in S2. GCC 16.1.0 `current-gates-gcc` passed again with
67/67 current smoke tests. Session-layout encapsulation remains a separate P2
design item.

### S5 Design

The approved opaque-session migration is recorded in
[the S5 session-layout design](m5-t234-s5-session-layout-design.md). It is a
future implementation subtask: no runtime layout or test route changes in S5.

### S6 Implementation

S6 implements that design. `session.h` now exposes opaque `vm_session`, with
the complete layout in composition-private `session_private.h`. The test-only
fixture provides named borrows and legacy storage helpers without re-exporting
the aggregate layout. `verify-session-layout-boundary` rejects private-header
escapes and direct session-field reads; MinGW GCC passed all 67 current smokes.
