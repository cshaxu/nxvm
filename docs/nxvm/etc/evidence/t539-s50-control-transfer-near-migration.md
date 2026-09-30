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

## Acceptance evidence to complete this S

- Both widths build and run the CPU near receiver and retained far receiver.
- Complete repository-only units and specialized gates pass on both widths.
- T317/T332 inventories recognize the CPU-local receiver; no #UD category is
  claimed because S50 has no #UD scenario.
- Documentation governance and `git diff --check` pass.  This is test/CMake/
  documentation work only, so no executable input or EXE rebuild is required.
