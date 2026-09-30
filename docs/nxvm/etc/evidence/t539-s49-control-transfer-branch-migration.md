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

## Acceptance evidence

P1 `19ea7597e` changes eight NXVM paths: 366 additions and 398 removals
(net -32).  `git show --check` and the final worktree `git diff --check` pass.

- x64 and x86 each build and run `cpu-control-transfer-branch-smoke` and the
  retained `core-machine-control-transfer-smoke`.
- Complete repository-only unit suites pass 417/417 on x64 (62.93 seconds)
  and x86 (57.54 seconds).
- Both specialized-gate aggregates pass.  T332 recognizes 36 correct CPU
  fixture owners; T344 recognizes 105 retained direct-constructor inputs and
  the renamed retained source.  Neither count is reduced by a hidden path.
- Documentation governance passes.  This is test/CMake/documentation work
  only, so no product executable input changes and no EXE rebuild is required.
