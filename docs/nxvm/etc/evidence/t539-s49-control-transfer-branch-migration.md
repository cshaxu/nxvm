# M5 T539 S49 Control-Transfer Branch Migration

## Scope and ownership

The former mixed `core_machine_control_transfer_smoke.c` is split at the
instruction-family boundary.  The new
`cpu_control_transfer_branch_smoke.c` links only `x86-cpu` and owns the
instruction-local cases: short and near conditional branches, direct
short/near jumps, LOOP/LOOPE/LOOPNE, JCXZ/JECXZ, code and address-size forms,
the 8086/80186/80286/80386 real-mode matrix, 80286 near-Jcc #UD, and target
limit rollback.  It uses the CPU-owned instruction fixture and its fault
diagnostic callback; it neither constructs a Core machine nor accesses a
machine-private CPU or memory field.

The retained `core_machine_control_transfer_near_far_smoke.c` contains only
the S50 near-call/return and S51 far-transfer families.  It remains the one
historical direct-private construction input for those pending packages; S49
does not preserve a second branch/loop path there.

## Acceptance evidence to complete this S

- Both host widths build and run `cpu-control-transfer-branch-smoke` and the
  retained `core-machine-control-transfer-smoke`.
- The complete repository-only unit suite and specialized gates pass on both
  widths.
- The T344 retained-constructor inventory recognizes the renamed retained
  source; no count is reduced merely by hiding a direct constructor.
- Documentation governance and `git diff --check` pass.  This is test/CMake/
  documentation work only, so no product executable input changes and no EXE
  rebuild is required.
