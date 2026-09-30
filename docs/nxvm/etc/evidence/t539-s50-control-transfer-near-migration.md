# M5 T539 S50 Control-Transfer Near Migration

## Scope and ownership

`cpu_control_transfer_near_smoke.c` links only `x86-cpu` and owns the former
near CALL/RET families: direct 16/32-bit CALL and RET forms, RET-immediate
stack adjustment, register-indirect CALL, and target-limit rollback.  It also
owns the two `FF /4` indirect-JMP cases and their rollback boundary because
they shared the one original function with the `FF /2` CALL cases.  This is a
single CPU instruction family, not a new shared fixture or Core interface.

The CPU fixture retires an explicit sequence for programs that contain
`MOV -> CALL -> RET -> MOV`; unlike the old Core runner, it intentionally
retires one instruction per refresh.  Stack address and code-size inputs are
now explicit test inputs rather than side effects of hidden Core setup.

The retained Core source contains only S51 far-transfer families after this
migration.  It has no near CALL/RET helper, invocation or success marker.

## Acceptance evidence

P1 `64c261cf7` changes six NXVM paths: 203 additions and 185 removals
(net +18).  `git show --check` and the final worktree `git diff --check` pass.

- x64 and x86 build and run the CPU near receiver and retained far receiver.
- Complete repository-only unit suites pass 418/418 on x64 (61.95 seconds)
  and x86 (56.22 seconds).
- Both specialized-gate aggregates pass.  T317 compiles the new receiver with
  strict warnings; T332 recognizes 37 lifecycle owners; T344 retains its
  105 direct-constructor rows.  S50 makes no #UD claim because it has none.
- Documentation governance passes.  This is test/CMake/documentation work
  only, so no executable input changes and no EXE rebuild is required.
