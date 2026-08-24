# T447 S9 VDM forwarding-layer closure

## Inventory and disposition

The pre-change VDM composition session had exactly two source files and two
callers. `session.c` owned only a wrapper allocation and null checks, then
forwarded create, reset, destroy, keyboard, text, snapshot, and port-read
operations to `vdm_machine_dos_minimal_*`. The callers were the minimal-session
smoke and the presentation adapter. CMake compiled that wrapper into
`vdm-composition`; no `nxvdm` executable target existed.

S9 deletes `vdm/composition/session.c` and `session.h`. The minimal-session
smoke now belongs to `tests/machine`, links `vdm-machine`, and exercises the
owner directly. The presentation adapter still belongs to `vdm-composition`:
it owns a bounded native-input queue and timestamped copied text capture, so it
has a real adapter responsibility. It takes the opaque machine-owned handle
directly. No caller, alias, compatibility route, or extra public operation
remains.

`vdm_machine_dos_minimal` remains the sole owner of its Core-machine handle,
snapshot, create rollback, reset, and destruction. The VDM skeleton stays
library/smoke-only; S9 adds neither mantle/DOS behavior nor an application
runner target.

## Regression guard and verification

`verify-t447-vdm-forwarding-closure` rejects the retired source/header, any
remaining `vdm_session` facade use in VDM source or focused tests, CMake
compilation of the retired source, and an `nxvdm` executable target. It emits
`M5 T447 S9 VDM forwarding closure: OK`.

- Focused configuration, both VDM smoke builds, the guard, and the two CTest
  cases pass.
- The current artifact remains `vm-0-5-0447`, SHA-256
  `CAA87452E18E90C5E72114D8C8B6C04FD830920BB45DA160CA6562787AA3B920`.
- Excluding documentation, the implementation diff removes 148 lines and adds
  83 lines, for a net reduction of 65 lines. The only new logic is
  the narrow recurrence guard; production behavior is unchanged.
