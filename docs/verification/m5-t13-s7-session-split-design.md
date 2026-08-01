# M5 T13 S7 Session-Split Design

The current `core/product/runtime/session.*` is not generic lifecycle code: it
hard-codes both product profile IDs and includes both `vm/product/full_pc.h`
and `vdm/machine/dos_minimal.h`. S8 replaces it as follows.

- `core/product/runtime/profile_descriptor.h` retains only a product-neutral
  descriptor shape and generic device-capability bits. It names no VM or VDM
  profile and selects none.
- `vm/profile/full_pc_profile.*` owns the immutable `nxvm.full_pc` descriptor.
  `vm/product/full_pc_session.*` is moved from the current core session source
  and owns FDD/HDD configuration, reset, destroy, and descriptor access.
- `vdm/profile/dos_minimal_profile.*` owns the immutable
  `ntvdm64.dos_minimal` descriptor. `vdm/product/minimal_session.*` owns the
  no-media DOS-minimal create/reset/destroy path.
- Existing `runtime/session_smoke.c` splits into focused VM and VDM session
  smokes. Existing profile consumers obtain their descriptor from the owning
  product rather than a core static table.

The replacement preserves the same profile names, device bits, reset behavior,
and local fixture inputs. It affects internal M3/M5 test APIs only; the retained
NXVM Console startup path does not call this API. S8 may use `git mv` for the
current session pair, must remove the core pair after callers migrate, and must
not introduce a core callback that dispatches by product/profile ID.
