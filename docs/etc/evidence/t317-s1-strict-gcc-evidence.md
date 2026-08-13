# T317 S1 Strict CPU Smoke Evidence

`PROJECT_T317_STRICT_CPU_SMOKE_INVENTORY` in the root CMake file is the
authoritative 47-entry source-to-executable mapping. Configuration rejects a
missing target, a source not directly owned by that target, a duplicate target,
or a duplicate source. `verify-t317-strict-cpu-smoke-coverage` builds those 47
executables and queries Ninja's actual command database; it requires all four
of `-Wall`, `-Wextra`, `-Wpedantic`, and `-Werror` on the direct command for
each mapped source. This is target-local evidence only: it neither adds flags
to nor makes a strictness claim for `core-machine` or its other dependencies.

## Warning-Remediation Inventory

Every owner-smoke edit below is limited to an actual GCC strict diagnostic.
Configuration edits replace positional `core_machine_config` initializers with
the same three explicit values; all omitted members retain their former C
aggregate zero value. No test assertion, fixture action, instruction input, or
runtime marker changes.

| Files | Original diagnostic category | No-behavior-change remediation |
| --- | --- | --- |
| `core_machine_cli_sti_smoke.c`, `core_machine_clts_s62_smoke.c`, `core_machine_direct_flags_smoke.c`, `core_machine_eflags_local_smoke.c`, `core_machine_enter_leave_smoke.c`, `core_machine_fpu_interface_s65_smoke.c`, `core_machine_gpr_mov_smoke.c`, `core_machine_gpr_push_pop_smoke.c`, `core_machine_imul_immediate_s56_smoke.c`, `core_machine_lahf_sahf_smoke.c`, `core_machine_lea_smoke.c`, `core_machine_legacy_sreg_stack_smoke.c`, `core_machine_les_lds_s41_smoke.c`, `core_machine_les_lds_smoke.c`, `core_machine_lods_smoke.c`, `core_machine_lss_lfs_lgs_smoke.c`, `core_machine_moffs_smoke.c`, `core_machine_movs_smoke.c`, `core_machine_msw_s63_smoke.c`, `core_machine_port_io_s55_smoke.c`, `core_machine_port_strings_smoke.c`, `core_machine_prefix_attributes_s64_smoke.c`, `core_machine_push_immediate_smoke.c`, `core_machine_pusha_popa_smoke.c`, `core_machine_pushf_popf_s47_smoke.c`, `core_machine_pushf_popf_smoke.c`, `core_machine_rotate_smoke.c`, `core_machine_scas_smoke.c`, `core_machine_sign_extend_smoke.c`, `core_machine_sreg_mov_smoke.c`, `core_machine_stos_smoke.c`, `core_machine_xchg_smoke.c` | `missing-field-initializers`, then `missing-braces` | Complete designated configuration initializer preserves the three former values and the zero initialization of every other field. |
| `core_machine_arpl_s53_smoke.c`, `core_machine_les_lds_s41_smoke.c`, `core_machine_prefix_attributes_s64_smoke.c`, `core_machine_inc_dec_smoke.c`, `core_machine_lods_smoke.c`, `core_machine_enter_leave_smoke.c`, `core_machine_gpr_push_pop_smoke.c`, `core_machine_gpr_mov_smoke.c`, `core_machine_lss_lfs_lgs_smoke.c` | `unused-variable`, `unused-function` | Removed declarations and helpers with no callers or reads; no assertion or input uses them. |
| `core_machine_fs_gs_stack_smoke.c`, `core_machine_scas_smoke.c`, `core_machine_cmps_smoke.c` | `sign-compare` | Rewrote equivalent unsigned stack/index arithmetic and narrowed an already-bounded byte-count subtraction. |
| `core_machine_cmps_smoke.c`, `core_machine_interrupt_entry_smoke.c` | `parentheses` | Added grouping only, preserving the evaluated boolean/arithmetic expression. |
| `core_machine_tf_db_s60_smoke.c` | `unused-but-set-variable` | Retained the real-mode pre-run snapshot where it is asserted and removed only the unrelated unused snapshot. |

## Retained FPU Smoke Contract Migration

`tests/machine/fpu_escape_smoke.c` now installs a real-mode IVT vector-7
handler (`INC AX; HLT`) for the two already-delivered CPU-side `#NM` cases:
ESC with `CR0.EM`, and WAIT with `CR0.TS|CR0.MP`.  Each case proves that
execution transfers to vector 7 without a terminal diagnostic, and proves the
complete real-mode frame at the selected `SS:SP`: restart IP is the synchronous
faulting instruction IP (`0`), followed by the captured pre-fault CS and
FLAGS, with the 16-bit three-word SP decrement preserving ESP's high half.
The handler then executes through its halt while advancing AX.  The pre-existing
no-FPU consume cases and the optional-8087 success case remain intact.  This
is a retained-test expectation migration only; it makes no FPU/provider or
CPU delivery change.  `docs/states/TODO.md` records that any future FPU-execution
admission must rerun these no-FPU, vector-7, optional-8087, and provider
boundary probes.

## Verification Record

- Fresh `mingw-gcc-x64` configuration completed.
- `verify-t317-strict-cpu-smoke-coverage` built all 47 inventory targets and
  passed its actual-Ninja-command audit.
- `current.core-machine-fpu-escape-smoke` passed after the retained vector-7
  contract migration.
- Documentation governance and `git diff --check` passed.
- The full `current-gates-gcc` gate passed all 194 CTests.
