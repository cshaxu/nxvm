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
receiving map and dual-width verification. Actual-commit review accepts pushed
P1 `442088410`; no part of S31-S45 is claimed here.

### S31: 2 matching files

- `core_machine_imul_immediate_s56_smoke.c`
- `core_machine_rotate_smoke.c`

S31 implementation assigns all 335 original contexts to two CPU-only
receivers (324) and the two board receivers (eleven). Neither original `.c`
source reads private CPU state now. The S30 count of 75 direct-private `.c`
consumers plus one shared fixture header falls to 73 `.c` files plus that
header, or 74 pending consumers assigned to S32-S42. [S31 evidence](t539-s31-imul-group2-migration.md)
records the receiving map and verification; actual-commit review accepts
pushed P1 `38bc5b10c`. S32-S45 remain unaccepted.

### S32: 2 matching files

- `core_machine_legacy_alu_s2_smoke.c`
- `core_machine_legacy_lock_s1_smoke.c`

S32 assigns all 775 original contexts to CPU-only receivers (767) or public
board receivers (eight). Neither original source reads private CPU state now.
The S31 count of 73 direct-private `.c` consumers plus one fixture header
falls to 71 `.c` files plus that header, or 72 pending consumers assigned to
S33-S42. [S32 evidence](t539-s32-legacy-alu-lock-migration.md) records the
receiving map and verification; actual-commit review accepts pushed P1
`9f785a551`. S33-S45 remain unaccepted.

### S33-S35: 1 original matching file, retired after three case groups

- `core_machine_inc_dec_smoke.c` (retired by S35)

S33 owns INC/DEC through DIV/IDIV; S34 owns TEST rm/reg through SBB; S35
owns OR through XLAT and the Group-1 matrix. S33 and S34 left the source
live; S35 assigns its last cases and deletes it. The S32 count of 71
direct-private `.c` consumers plus one fixture header falls to 70 `.c`
files plus that header, or 71 pending consumers assigned to S36-S45.

### S36: 4 FLAGS matching files

- `core_machine_direct_flags_smoke.c`
- `core_machine_lahf_sahf_smoke.c`
- `core_machine_pushf_popf_s47_smoke.c`
- `core_machine_pushf_popf_smoke.c`

S36 removes all four private-state test sources. CPU-only replacements are
`cpu_direct_flags_smoke.c`, `cpu_lahf_sahf_smoke.c` and
`cpu_pushf_popf_smoke.c`; real guest protected-mode and PIC IRQ receivers are
the corresponding three `core_machine_*_board_smoke.c` files. The included
PUSHF/POPF S21 source is retired with its S47 includer, not left as a second
test path. The S35 count of 70 direct-private `.c` files plus one fixture
header falls to 66 `.c` files plus that header, or 67 pending consumers
assigned to S37-S45. The negative verifier's deliberate rejected-code strings
are not a private-state consumer. [S36 evidence](t539-s36-flags-migration.md)
records the receiving map and verification.

### S37: 2 string-transfer matching files

- `core_machine_lods_smoke.c` (retired by S37)
- `core_machine_movs_smoke.c` (retired by S37)

The 119 original MOVS/LODS execution contexts retain CPU instruction or
public-board receivers. CPU-local synthetic segment-cache faults complement
the original guest-loaded descriptor and PIC cases; they are not a second
production path. The S36 count of 66 direct-private `.c` tests plus one
fixture header falls to 64 `.c` tests plus that header, or 65 pending
consumers assigned to S38-S45. [S37 evidence](t539-s37-string-transfer-migration.md)
records the complete case-family map and verification.

### S38: 3 string-scan/compare matching files

- `core_machine_cmps_smoke.c` (retired by S38)
- `core_machine_scas_smoke.c` (retired by S38)
- `core_machine_stos_smoke.c` (retired by S38)

The original 203 STOS/SCAS/CMPS contexts have CPU or public-board
receivers, with complementary protected-fault assertions. The pending
direct-private `.c` test count falls from 64 to 61, plus the shared fixture
header. [S38 evidence](t539-s38-string-scan-compare-migration.md) records
the receiving map and verification.

### S39: 3 port-I/O matching files

- `core_machine_port_io_s55_smoke.c` (retired by S39)
- `core_machine_port_ownership_smoke.c` (renamed to its board owner by S39)
- `core_machine_port_strings_smoke.c` (retired by S39)

The 170 original scalar and sixty original string-port contexts retain CPU
or public-board receivers; port ownership stays board-local. The pending
direct-private `.c` test count falls from 61 to 58, plus the shared fixture
header. [S39 evidence](t539-s39-port-io-migration.md) records the complete
receiving map and verification.

### S40: ARPL base and includer, 2 matching files

- `core_machine_arpl_s53_smoke.c` (retired by S40)
- `core_machine_arpl_smoke.c` (retired by S40)

The original base and S53 cases have CPU-only or public-board receivers, and
the direct source includer is removed. The pending direct-private `.c` count
falls from 58 to 56, plus the common fixture header. [S40 evidence](t539-s40-arpl-migration.md)
records the receiving map and dual-width verification.

### S41: BOUND, 1 matching file

- `core_machine_bound_s54_smoke.c`

### S42: Table-register instructions, 3 matching files

- `core_machine_dttr_s61_smoke.c` (retired by S42)
- `core_machine_lgdt_lidt_smoke.c` (retired by S42)
- `core_machine_sgdt_sidt_smoke.c` (retired by S42)

S42 moves instruction-local forms, attributes, invalid encodings and local
rollback to the three CPU receivers. It moves actual guest table construction,
memory source/store boundaries, privilege delivery, descriptor consumption and
PIC context to `machine_table_register_board_smoke.c`. The pending direct-
private `.c` count falls from 55 to 52, plus the shared fixture header. Its
receiving map and final verification are recorded in
[S42 evidence](t539-s42-table-register-migration.md).

### S43: Descriptor system, one mixed matching file portion

- `core_machine_descriptor_system_smoke.c`

S43 is accepted: its descriptor/table/cache cases moved to
`cpu_descriptor_system_smoke.c`. Its `SMSW/LMSW/CLTS/MOV CR` control-state
block remains in the original source as S45's sole input; S43 did not delete
or duplicate it merely to retire a filename.

### S44: Descriptor queries, accepted

The two matching mixed sources are retired. Their instruction-local cases now
belong to `cpu_lar_lsl_smoke.c` and `cpu_verr_verw_smoke.c`; the retained 80386
timing runner owns the LSL page-granularity rows.

### S45: Control state, accepted

- `core_machine_clts_s62_smoke.c`
- `core_machine_msw_s63_smoke.c`
- `dt_test_msw_and_control_registers()` in
  `core_machine_descriptor_system_smoke.c` (the sole retained S43 block)

The two complete mixed sources and the retained block are retired.  The
CPU-local receiver owns instruction semantics; the public Core/PIC receiver
owns machine-observable IRQ, interrupt-frame and board-option cases.  The
complete receiving map and dual-width verification are recorded in
[S45 evidence](t539-s45-control-state-migration.md).

### S46: Debug state, 2 matching files

- `core_machine_debug_mov_s59_smoke.c`
- `core_machine_tf_db_s60_smoke.c`

### S47: 5 matching files

- `core_machine_protected_16_call_gate_s7_smoke.c`
- `core_machine_protected_16_external_s4_smoke.c`
- `core_machine_protected_16_gate_s3_smoke.c`
- `core_machine_protected_16_outer_iret_s6_smoke.c`
- `core_machine_protected_16_outer_s5_smoke.c`

### S48: call-gate privilege entry, 1 matching file

- `core_machine_call_gate_privilege_entry_smoke.c` (retire into one public board receiver)

### S49--S51: control transfer, 1 matching file

- `core_machine_control_transfer_smoke.c` (S49 branch/loop, S50 near call/return,
  S51 far transfer; S49 retires its branch/loop portion into the CPU receiver and
  renames the retained S50/S51 input to `core_machine_control_transfer_near_far_smoke.c`)

### S50: IDT/privilege entry pair, 2 matching files

- `core_machine_idt_privilege_entry_smoke.c`
- `core_machine_protected_privilege_smoke.c`

### S53: accepted protected far/data pair

- Retired `core_machine_protected_data_access_s2_smoke.c` into the CPU-local
  `cpu_protected_data_access_smoke.c` plus its independent PIC board receiver.
- Retired `core_machine_protected_far_s1_smoke.c` into the CPU-local
  `cpu_protected_far_smoke.c` plus its independent PIC board receiver.

### S54: accepted outer return pair, 2 matching files

- `cpu_outer_return_smoke.c` (CPU-local return and exception routes)
- `machine_outer_iret_pic_board_smoke.c` (PIC IRR→ISR delivery route)

### S55: accepted 16-bit/task-gate task-switch half

- The 16-bit/task-gate contexts of `core_machine_task_switch_smoke.c` are
  retired into the CPU-only `cpu_task_switch16_smoke.c` and public
  PIC-board `machine_task_switch16_pic_board_smoke.c` receivers.

### S56: accepted 80386 task-JMP decode, 1 matching file

- The four functional `66h`/`67h` task-JMP rows are retired into CPU-only
  `cpu_task_switch32_decode_smoke.c`, sharing the S55 CPU fixture rather than
  duplicating a TSS image. The original construction recipes remain only for
  the S65 80386 timing-runner includer; its original functional calls are gone.

### S57: accepted TSS32 state/fault contexts, 1 matching file

- Direct TSS32 baseline, operand/address forms, descriptor rejection, LDT
  load/validation and state-image fault rows are retired into CPU-only
  `cpu_task_switch32_state_smoke.c`. Its fixture executes the real CPU
  `LGDT`/`LTR` bootstrap, so the cached task register is not synthesized by
  the test.

### S58-S62: residual task-switch half, 1 matching file

- `core_machine_task_switch_smoke.c`

### S63: TSS I/O-map port, 1 matching file

- `machine_tss_iomap_port_authorization_smoke.c`

### S64-S67: 10 matching files

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

### S68: common timing baseline, 8 matching files

- `machine_instruction_timing_ledger_smoke.c`
- `machine_instruction_timing_smoke.c`
- `machine_legacy_timing_normalization_s2_smoke.c`
- `machine_t359_s2_timing_smoke.c`
- `machine_t359_s3_timing_smoke.c`
- `machine_t359_s4_timing_smoke.c`
- `machine_t359_s5_timing_smoke.c`
- `machine_t359_s6_timing_smoke.c`

### S69: 8086 timing corpus, 2 matching files

- `core_machine_8086_instruction_timing_ledger_smoke.c`
- `core_machine_8086_timing_manifest_runner.c`

### S70: 80186 timing corpus, 2 matching files

- `core_machine_80186_instruction_timing_ledger_smoke.c`
- `core_machine_80186_timing_manifest_runner.c`

### S71: 80286 ledger and protected I/O, 2 matching files

- `core_machine_80286_instruction_timing_ledger_smoke.c`
- `core_machine_80386_protected_io_timing_smoke.c`

### S72: 80286 manifest and call-gate includer, 2 matching files

- `core_machine_80286_timing_manifest_runner.c`
- `core_machine_call_gate_smoke.c` (80286 timing-runner includer)

### S73: 80386 timing corpus, 1 matching file

- `core_machine_80386_timing_manifest_runner.c`

### S74: remaining-consumer sweep, 4 matching files

- `core_machine_fpu_interface_s65_smoke.c`
- `cpu_profile_gate_smoke.c`
- `fpu_escape_smoke.c`
- `support/core_machine_cpu_fixture.h`

## Include Dependency Closure

The matching-file inventory is not the entire compile dependency graph. S47
must also keep the 80286/80386 timing runners building when their included
16-bit gate/task/call fixtures change; S72 owns the 80286 runner's direct
`core_machine_call_gate_smoke.c` includer. S52 includes outer-IRET's
atomicity-fixture consumer. S64 includes
`core_machine_interrupt_return_composition_s4_smoke.c`; S65 includes the
protected-IRET includer; S67 includes `core_machine_vm86_lgdt_lidt_s5_smoke.c`
and hardware delivery, which inherit private setup through included sources.
Do not migrate one included source's return shape while leaving its includers
with the old shape.

The CPU-owner bus/context and EFLAGS tests, migrated board/machine tests and
integration probes remain required regressions even though they no longer
match this private-state search. Their earlier S18 mappings remain in evidence.
No test target or original case was removed for baseline recovery.
