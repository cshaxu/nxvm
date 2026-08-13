# T317 S3 test-fixture consolidation evidence

## Scope and deterministic inventory

The authoritative input is `PROJECT_T317_STRICT_CPU_SMOKE_INVENTORY` in the
root `CMakeLists.txt`.  It contains 47 unique `target|tests/machine/source`
entries; this record deliberately consumes that list and makes no claim about
other machine tests.  The repeatable inventory command is:

```powershell
$i = Get-Content build/t317-strict-cpu-smoke-inventory.txt
$i | ForEach-Object { ($_ -split '\|')[1] } | ForEach-Object {
  rg -n 'core_machine_(create|bind_execution_provider|freeze_execution_providers|reset|run)|test_core_machine_fixture_(prepare_real_mode_execution|capture_cpu_after_run)' $_
}
```

Pre-change, 41 of the 47 owner sources directly contained the ordinary
create/bind/freeze/reset sequence; 44 consumed the already-private real-mode
preparation helper, and all 47 copied a post-run CPU observation through the
existing private capture helper.

## Centralized operation and callers

`test_core_machine_fixture_create_bind_freeze_reset` is deliberately only the
four-operation short-circuit sequence.  It neither validates nor cleans up a
machine, so its failure order and ownership match the previous caller
expression.  It has no instruction, device, descriptor, interrupt, assertion,
or result-policy input.

Thirty-five callers use the complete helper:

- `core_machine_cli_sti_smoke.c`, `core_machine_cmps_smoke.c`,
  `core_machine_eflags_local_smoke.c`, `core_machine_fs_gs_stack_smoke.c`,
  `core_machine_gpr_push_pop_smoke.c`, `core_machine_lahf_sahf_smoke.c`
- `core_machine_lea_smoke.c`, `core_machine_legacy_sreg_stack_smoke.c`,
  `core_machine_les_lds_s41_smoke.c`, `core_machine_les_lds_smoke.c`,
  `core_machine_lods_smoke.c`, `core_machine_lss_lfs_lgs_smoke.c`
- `core_machine_moffs_smoke.c`, `core_machine_movs_smoke.c`,
  `core_machine_msw_s63_smoke.c`, `core_machine_push_immediate_smoke.c`,
  `core_machine_pusha_popa_smoke.c`, `core_machine_pushf_popf_s47_smoke.c`
- `core_machine_pushf_popf_smoke.c`, `core_machine_scas_smoke.c`,
  `core_machine_sign_extend_smoke.c`, `core_machine_sreg_mov_smoke.c`,
  `core_machine_stos_smoke.c`, `core_machine_xchg_smoke.c`,
  `core_machine_clts_s62_smoke.c`, `core_machine_direct_flags_smoke.c`,
  `core_machine_dttr_s61_smoke.c`, `core_machine_enter_leave_smoke.c`,
  `core_machine_fpu_interface_s65_smoke.c`, `core_machine_gpr_mov_smoke.c`,
  `core_machine_debug_mov_s59_smoke.c`, `core_machine_inc_dec_smoke.c`,
  `core_machine_rotate_smoke.c`, `core_machine_tf_db_s60_smoke.c`

Six callers use `test_core_machine_fixture_bind_freeze_reset`, which keeps the
creation-before-provider-owner, device-installation-before-bind, or
creation-failure ownership local while centralizing the mechanically identical
lifecycle tail:

- `core_machine_bound_s54_smoke.c`, `core_machine_imul_immediate_s56_smoke.c`,
  `core_machine_lar_lsl_s57_smoke.c`, `core_machine_port_io_s55_smoke.c`,
  `core_machine_port_strings_smoke.c`, `core_machine_verr_verw_s58_smoke.c`

The existing `prepare_real_mode_execution` remains the only centralized
real-mode preparation shape, and `capture_cpu_after_run` remains the copied
observation shape.  Bounded runs remain owner-local: each includes a
source-specific instruction image, budget/reason interpretation, diagnostic
classification, or state observation point.  Those inputs cannot be made
implicit in support without creating the prohibited framework or moving
instruction-family semantics.

The other six inventory sources contain no create/bind/freeze/reset lifecycle
shape; their shared machine setup is intentionally outside this S3 operation
boundary.  No governed source retains a direct bind/freeze/reset chain.

## Boundary and focused proof

`rg -n "tests/support" src` is required to produce no hits, proving the
support header remains test-private.  Focused representative consumers are
`core-machine-eflags-local-smoke`, `core-machine-cmps-smoke`, and
`core-machine-legacy-sreg-stack-smoke`; together they cover plain setup,
real-mode continuation, and explicit boolean failure conversion.

The configured `verify-t317-test-type-vocabulary` and
`verify-t317-strict-cpu-smoke-coverage` gates passed (47 owner sources, one
support header, and 47 target-local compile commands).  Documentation
governance and `git diff --check` passed; `rg -n "tests/support" src` had no
matches.  The fresh configured `current-gates-gcc` run passed all 54
static/governance targets and 194/194 CTests.
