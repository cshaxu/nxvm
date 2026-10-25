# M5 T580 S41 — MyPC110 Turbo execution policy

## Change

`x86_nxvm_product_machine_binding` now owns one validated initial runner-speed
selection.  MyPC110 selects the existing Turbo value during construction;
zero-valued existing bindings remain Standard.

Turbo increases the existing runner quantum and removes host pacing only.  It
does not alter Core retirement, guest time, firmware, port 61, device routes,
or scheduler ownership.

## Verification

- Stable Ninja x64 and x86 unit build graphs completed successfully.
- `ctest -R "^(x86-nxvm|mypc110)\\."` passed 116/116 on x64.
- The same selection passed 116/116 on x86.

## Boundary

This validates construction and policy propagation, not a completed BIOS POST.
The selected external BIOS's receiver contract remains source-gated; no
artifact or boot qualification is claimed by this S.
