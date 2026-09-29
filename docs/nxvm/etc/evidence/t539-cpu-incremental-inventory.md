# T539 CPU Incremental Baseline Inventory

Current receiving numbers follow the S21 intake subdivision in the linked
work plan; S18-S20 historical evidence retains its original planned numbers.
No accepted S is renumbered.

## Baseline Recovery

The 2026-09-29 S18 replan preserves the original CPU architecture but defers the
board's opaque allocation cutover to S38. The existing board-owned CPU and
decoder remain the sole storage, bound through the existing execution-context
initializer. Bus, timing and copied-observation work remains in place. No
private-pointer accessor, state mirror or second execution path was added.

A native Git binary diff of all tracked pending work was saved before recovery
as `build/t539-s3/s18-before-incremental-baseline.patch` (1,221,515 bytes,
SHA-256 `D87DEB564901D04CCB024C14F64C921FAE9BC91269842FC00A2CA7C72A1187B8`).
This is an ignored local recovery artifact, not an accepted source revision;
keep it with the retained build trees until its deferred edits are reconciled.
Untracked CPU bus/fixture/evidence files remain in the worktree, unchanged by
that backup. The snapshot is not a claim that the pending corpus was buildable.

Deferred changes and exact receivers:

- S38: machine CPU allocation/destruction and pointer-member call syntax.
  CPU-owned create/destroy implementation and its independent lifecycle test
  remain; the board still uses its original embedded allocation lifetime.
- S35: the incomplete `core_machine_interrupt_entry_smoke.c` migration. Restore
  its HEAD form and existing CMake delivered-fault definition together, so its
  software-INT/hardware-delivery includers retain their original API and cases.
  Its public-snapshot migration is preserved in the recovery patch and earlier
  S18 evidence; CPU-local NMI/rollback regressions remain in the active corpus.
- Other pending source/test work stays active, with eventual receiving packages
  named by the [work plan](../architecture/t539-cpu-work-packages.md). Its mere
  presence does not accept S19-S40. Later briefs review retained work rather
  than replaying or rewriting the same implementation.

Full x64 build recovered after those two changes. Complete unit verification,
cross-width verification, artifacts and actual-diff acceptance are recorded in
[S18 evidence](t539-s18-cpu-extraction.md), not inferred from this inventory.

## Remaining Private Test Consumers

Search: `rg -l 'executor_cpu\.|executor_cpu_instructions|support/core_machine_cpu_fixture.h' test/app-nxvm/unit`.
The recovery snapshot has 100 matching files, all under
`test/app-nxvm/unit/core/devices/`. Names below are relative to that directory.
These are migration inputs, not 100 failing tests or a complete count of cases.
Each receiving S must retain all original cases/loops and map any moved case
between chip and board tests; a filename search alone cannot prove coverage.

S18 actual-diff review additionally assigns S32 a BOUND operand-form check:
the CPU-local fixture observed emulator-error for 32-bit `62 C0`, versus UD
for the original 16-bit case. Reproduce against the pre-migration baseline and
audit related memory-only operand forms before deciding the repair. This is
an unresolved observation, not a timing downgrade or an accepted CPU result.

### S21: 2 matching files

- `core_machine_lea_smoke.c`
- `core_machine_movx_smoke.c`

S21 implementation removes both files' private CPU dependencies. Original
chip cases receive CPU-only targets `cpu_lea_smoke.c` and `cpu_movx_smoke.c`;
board IRQ/provider/fault assertions remain in the original files. See
[case mapping and proof](t539-s21-lea-movx-migration.md). Current owns acceptance.

### S22: 2 matching files

- `core_machine_gpr_mov_smoke.c`
- `core_machine_moffs_smoke.c`

S22 implementation removes these two files' private CPU dependencies. Their
269 chip cases move to `cpu_gpr_mov_smoke.c` and `cpu_moffs_smoke.c`; eight
board fault/IRQ cases remain. The original faulted-state physical-memory
assertions are board-owned, not CPU access. See [S22 evidence](t539-s22-mov-moffs-migration.md).
Current owns acceptance; 96 original direct consumers remain with S23-S37.

### S23: 1 matching files

- `core_machine_xchg_smoke.c`

S23 implementation moves 96 instruction contexts to cpu_xchg_smoke.c; five
board fault/IRQ contexts remain without private CPU access. The remaining
original inventory is 95 files, assigned to S24-S37. The broad search also
matches cpu_bus_boundary_negative.cmake's intentional rejected-code strings;
that verifier is not an unassigned CPU consumer. See
[S23 evidence](t539-s23-xchg-migration.md). Current owns acceptance.

### S24: 3 matching files

- `core_machine_gpr_push_pop_smoke.c`
- `core_machine_push_immediate_smoke.c`
- `core_machine_pusha_popa_smoke.c`

S24 removes all three board-private CPU dependencies. The 198 original
contexts have 190 CPU and 16 board executions: eight protected-fault contexts
retain complementary private-cache and real-board assertions. No case is
removed. See [S24 evidence](t539-s24-gpr-stack-migration.md). Current owns
acceptance; 92 original direct consumers remain assigned to S25-S37, excluding
the negative verifier's deliberate rejected-code strings.

### S25: 1 matching files

- `core_machine_enter_leave_smoke.c`

S25 removes this private board dependency. The 53 original contexts receive
51 CPU and four board executions, with two protected faults retaining both
private-cache and board observations. Original nesting/width/rejection tables
remain. See [S25 evidence](t539-s25-enter-leave-migration.md). Current owns
acceptance; 91 original private consumers remain assigned to S26-S37.

### S26: 2 matching files

- `core_machine_legacy_sreg_stack_smoke.c`
- `core_machine_fs_gs_stack_smoke.c`

S26 implementation removes both board-private CPU dependencies. All 164
original contexts remain in two CPU-only receivers; 18 complementary board
executions retain PIC and machine-fault proof. The two FS/GS board receivers
use valid public descriptor setup while the original inconsistent cache
preconditions remain CPU-local. See [S26 evidence](t539-s26-segment-stack-migration.md).
Current owns acceptance; 89 original private consumers remain assigned S27-S37.

### S27: 3 matching files

- `core_machine_les_lds_s41_smoke.c`
- `core_machine_les_lds_smoke.c`
- `core_machine_lss_lfs_lgs_smoke.c`

S27 implementation removes these three board-private CPU dependencies. All
117 original contexts retain receivers: 110 CPU executions and 20 board
executions, with thirteen complementary fault receivers. Real PIC/SS-shadow
checks stay board-owned. See [S27 evidence](t539-s27-far-pointer-migration.md).
Current owns acceptance; 86 original private consumers remain assigned S28-S37.

### S28: 2 matching files

- `core_machine_segment_selector_smoke.c`
- `core_machine_sreg_mov_smoke.c`

S28 removes both board-private CPU dependencies. The 244 original execution
contexts and three metadata queries retain 241 CPU and 25 board executions:
22 fault contexts have complementary receivers, and the three SREG IRQ cases
remain board-only. The inverted 286 rejection helper is corrected in both
receivers. See [S28 evidence](t539-s28-segment-migration.md). Current owns
acceptance; 84 original private consumers remain assigned S29-S37.

### S29: 2 matching files

- `core_machine_operand_address_smoke.c`
- `core_machine_prefix_attributes_s64_smoke.c`

S29 removes both board-private CPU dependencies. The operand suite's 28
original contexts retain 26 CPU and four board executions, with two fault
contexts checked at both boundaries. The prefix suite's eleven CPU-owned
groups and one real PIC/IRQ group keep their original program matrices. See
[S29 evidence](t539-s29-operand-prefix-migration.md). Current owns acceptance;
82 original private consumers remain assigned S30-S42. S30's original
eleven-file, 7,000-plus-line assignment is subdivided before implementation;
the single `inc_dec` source is shared across staged S33-S35, but remains one
inventory file until its last private access is removed.

### S30: 6 matching files

- `core_machine_bit_scan_smoke.c`
- `core_machine_bit_test_smoke.c`
- `core_machine_double_shift_smoke.c`
- `core_machine_imul2_smoke.c`
- `core_machine_setcc_smoke.c`
- `core_machine_sign_extend_smoke.c`

S30 implementation assigns all 549 contexts to six CPU-only receivers (539)
and six surviving board receivers (ten). These six `.c` sources no longer read
private CPU state. The S29 HEAD search counted 81 direct-private `.c` consumers
plus one shared fixture header; S30 removes six `.c` consumers, leaving 75
`.c` files plus that header, or 76 pending consumers assigned to S31-S42.
[S30 evidence](t539-s30-bit-condition-extension-migration.md) records the
receiving map and dual-width verification. Actual-commit review still owns
acceptance; no part of S31-S45 is claimed here.

### S31: 2 matching files

- `core_machine_imul_immediate_s56_smoke.c`
- `core_machine_rotate_smoke.c`

### S32: 2 matching files

- `core_machine_legacy_alu_s2_smoke.c`
- `core_machine_legacy_lock_s1_smoke.c`

### S33-S35: 1 matching file, three non-overlapping case groups

- `core_machine_inc_dec_smoke.c`

S33 owns INC/DEC through DIV/IDIV; S34 owns TEST rm/reg through SBB; S35
owns OR through XLAT and the Group-1 matrix. The source remains a live
consumer until S35, so do not decrement the matching-file inventory at S33
or S34 merely because some cases move.

### S36: 12 matching files

- `core_machine_cmps_smoke.c`
- `core_machine_direct_flags_smoke.c`
- `core_machine_lahf_sahf_smoke.c`
- `core_machine_lods_smoke.c`
- `core_machine_movs_smoke.c`
- `core_machine_port_io_s55_smoke.c`
- `core_machine_port_ownership_smoke.c`
- `core_machine_port_strings_smoke.c`
- `core_machine_pushf_popf_s47_smoke.c`
- `core_machine_pushf_popf_smoke.c`
- `core_machine_scas_smoke.c`
- `core_machine_stos_smoke.c`

### S37: 13 matching files

- `core_machine_arpl_s53_smoke.c`
- `core_machine_arpl_smoke.c`
- `core_machine_bound_s54_smoke.c`
- `core_machine_clts_s62_smoke.c`
- `core_machine_debug_mov_s59_smoke.c`
- `core_machine_descriptor_system_smoke.c`
- `core_machine_dttr_s61_smoke.c`
- `core_machine_lar_lsl_s57_smoke.c`
- `core_machine_lgdt_lidt_smoke.c`
- `core_machine_msw_s63_smoke.c`
- `core_machine_sgdt_sidt_smoke.c`
- `core_machine_tf_db_s60_smoke.c`
- `core_machine_verr_verw_s58_smoke.c`

### S38: 5 matching files

- `core_machine_protected_16_call_gate_s7_smoke.c`
- `core_machine_protected_16_external_s4_smoke.c`
- `core_machine_protected_16_gate_s3_smoke.c`
- `core_machine_protected_16_outer_iret_s6_smoke.c`
- `core_machine_protected_16_outer_s5_smoke.c`

### S39: 11 matching files

- `core_machine_call_gate_privilege_entry_smoke.c`
- `core_machine_call_gate_smoke.c`
- `core_machine_control_transfer_smoke.c`
- `core_machine_idt_privilege_entry_smoke.c`
- `core_machine_iret_outer_s52_smoke.c`
- `core_machine_protected_data_access_s2_smoke.c`
- `core_machine_protected_far_s1_smoke.c`
- `core_machine_protected_privilege_smoke.c`
- `core_machine_protected_return_atomicity_smoke.c`
- `core_machine_task_switch_smoke.c`
- `core_machine_tss_iomap_port_smoke.c`

### S40: 10 matching files

- `core_machine_cli_sti_s48_smoke.c`
- `core_machine_cli_sti_smoke.c`
- `core_machine_hardware_delivery_s3_smoke.c`
- `core_machine_hlt_s49_smoke.c`
- `core_machine_interrupt_entry_smoke.c`
- `core_machine_iret_s51_smoke.c`
- `core_machine_protected_iret_smoke.c`
- `core_machine_software_int_s50_smoke.c`
- `core_machine_vm86_delivery_smoke.c`
- `core_machine_vm86_iret_smoke.c`

### S41: 16 matching files

- `core_machine_80186_instruction_timing_ledger_smoke.c`
- `core_machine_80186_timing_manifest_runner.c`
- `core_machine_80286_instruction_timing_ledger_smoke.c`
- `core_machine_80286_timing_manifest_runner.c`
- `core_machine_80386_protected_io_timing_smoke.c`
- `core_machine_80386_timing_manifest_runner.c`
- `core_machine_8086_instruction_timing_ledger_smoke.c`
- `core_machine_8086_timing_manifest_runner.c`
- `core_machine_instruction_timing_ledger_smoke.c`
- `core_machine_instruction_timing_smoke.c`
- `core_machine_legacy_timing_normalization_s2_smoke.c`
- `core_machine_t359_s2_timing_smoke.c`
- `core_machine_t359_s3_timing_smoke.c`
- `core_machine_t359_s4_timing_smoke.c`
- `core_machine_t359_s5_timing_smoke.c`
- `core_machine_t359_s6_timing_smoke.c`

### S42: 4 matching files

- `core_machine_fpu_interface_s65_smoke.c`
- `cpu_profile_gate_smoke.c`
- `fpu_escape_smoke.c`
- `support/core_machine_cpu_fixture.h`

## Include Dependency Closure

The matching-file inventory is not the entire compile dependency graph. S38
must also keep the 80286/80386 timing runners building when their included
16-bit gate/task/call fixtures change; S41 owns their subsequent timing-corpus
migration. S39 includes outer-IRET's atomicity-fixture consumer. S40 additionally
includes `core_machine_interrupt_return_composition_s4_smoke.c` and
`core_machine_vm86_lgdt_lidt_s5_smoke.c`, which inherit private setup through
included source rather than a direct search hit. Protected IRET is in S40
with its CLI/STI-dependent includer. Do not migrate one included source's return
shape while leaving its includers with the old shape.

The CPU-owner bus/context and EFLAGS tests, migrated board/machine tests and
integration probes remain required regressions even though they no longer
match this private-state search. Their earlier S18 mappings remain in evidence.
No test target or original case was removed for baseline recovery.
