# T539 S77 CPU Arithmetic Receiver Evidence

## Result

S77 replaces ten NXVM-only CPU fixtures with one Shared CPU receiver group.
The executable CPU implementation remains the S76 Shared corpus; this S moves
test ownership only.

## Receiver Map

The sole CPU-only test owner is `test/x86/devices/cpu/`:

- `cpu_inc_dec_{first_group,second_group,final_group}_smoke.c`;
- `cpu_legacy_alu_s2_smoke.c`, `cpu_rotate_smoke.c`, and
  `cpu_direct_flags_smoke.c`;
- `cpu_lahf_sahf_smoke.c`, `cpu_pushf_popf_smoke.c`,
  `cpu_xchg_smoke.c`, and `cpu_gpr_mov_smoke.c`; and
- `support/cpu_instruction_fixture.h`.

The fixture implementation exists once, in that Shared directory.  The NXVM
header with the historic name is now only a forwarding include.  It has live
callers outside S77; S78--S80 move their bounded receiver groups and S81
deletes the retired App path.  It creates neither fixture state nor a second
test recipe.

The following NXVM tests remain board receivers because they intentionally use
real Core memory, fault, interrupt, PIC, or wiring behavior:

- `core_machine_inc_dec_{first_group,second_group,final_group}_board_smoke.c`;
- `core_machine_legacy_alu_s2_smoke.c` and `core_machine_rotate_smoke.c`;
- `core_machine_direct_flags_board_smoke.c`,
  `core_machine_lahf_sahf_board_smoke.c`, and
  `core_machine_pushf_popf_board_smoke.c`; and
- `core_machine_xchg_smoke.c` and `core_machine_gpr_mov_smoke.c`.

## Verification

- Shared focused arithmetic receivers passed on x64 and x86.
- Complete repository-only `ctest -L unit -j 4` passed 427/427 on x64 and
  427/427 on x86 after the final CMake graph removed the duplicate App
  registrations.
- `ctest -N -L unit` reports 427 tests for both build trees.
- The CPU/PIC authority check, Shared corpus verifier, Shared test manifest,
  and documentation governance gate pass.
- `git diff --check` passes for both target commits.

No runtime CPU input changed: this is a Shared-test registration and NXVM-test
path migration.  The eight S76 `0.5.0539` product artifacts remain current;
no executable rebuild was manufactured for this test-only change.

## Commits

- `3ffb7c9fc` — Shared M5 T539 S77 P1, canonical test corpus.
- `669bd8fac` — NXVM M5 T539 S77 P2, retired App registrations and sources.
