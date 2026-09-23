# T344 S4: Historical Fixture-Shape Inventory

## Scope And Method

The inventory is intentionally based on every tracked `tests/machine/*.c`
source that directly calls `core_machine_create`, rather than a target-name or
directory convention. The T332 fixed 47-owner set is already governed by its
own lifecycle verifier; this inventory closes the 53 remaining direct
constructors and distinguishes a common execution-provider tail from the
owner-local construction semantics that must remain visible to a test.

`cmake/verify_t344_historical_fixture_shapes.cmake` is the fixed mechanical
inventory. It rejects an unclassified direct constructor, duplicate inventory
entry, or a migrated source that restores direct bind/freeze calls. It reports
53 direct constructors: 22 converged private-support tails and 31 retained
shapes.

## Converged Tails

These owners retain their existing configuration, state zeroing, provider,
failure cleanup, and all later test setup. Only their already identical
execution-provider bind, freeze, and reset tail now calls
`test_core_machine_fixture_bind_freeze_reset` in
`tests/support/core_machine_cpu_fixture.h`.

| Sources | Equivalence proof |
| --- | --- |
| `core_machine_80286_protected_mode_smoke.c`; `core_machine_80386_paging_smoke.c`; `core_machine_call_gate_privilege_entry_smoke.c`; `core_machine_call_gate_smoke.c`; `core_machine_descriptor_system_smoke.c` | Each creates one owner machine, binds exactly its local execution provider, freezes execution providers, then resets before any descriptor/image execution. The retained cleanup stays outside the helper. |
| `core_machine_bit_scan_smoke.c`; `core_machine_bit_test_smoke.c`; `core_machine_double_shift_smoke.c`; `core_machine_imul2_smoke.c`; `core_machine_movx_smoke.c` | Device registration remains before the helper. The helper owns only the identical execution-provider tail after that registration succeeds. |
| `core_machine_fpu_8087_smoke.c`; `core_machine_idt_privilege_entry_smoke.c`; `core_machine_operand_address_smoke.c`; `core_machine_protected_privilege_smoke.c`; `core_machine_segment_selector_smoke.c`; `core_machine_setcc_smoke.c` | The local provider, reset callback, and descriptor setup remain owner-local; their common frozen execution transition is identical. |
| `core_machine_task_switch_smoke.c`; `core_machine_tss_iomap_port_smoke.c`; `core_machine_vm86_delivery_smoke.c`; `core_machine_vm86_iret_smoke.c`; `cpu_profile_gate_smoke.c`; `fpu_escape_smoke.c` | Task/TSS, I/O map, VM86, profile, and FPU test state is still written locally after reset. The helper changes no state outside bind/freeze/reset. |

## Retained Shapes

| Sources | Semantic difference that prohibits extraction | Future admission condition |
| --- | --- | --- |
| `core_machine_bound_s54_smoke.c`; `core_machine_imul_immediate_s56_smoke.c`; `core_machine_lar_lsl_s57_smoke.c`; `core_machine_port_io_s55_smoke.c`; `core_machine_port_strings_smoke.c`; `core_machine_verr_verw_s58_smoke.c` | T332 already established that each creates instruction-specific GDT/IDT or port-provider state before it delegates its later lifecycle tail. It has no duplicate direct bind/freeze tail. | Admit a helper only with a fixed pre-bind configuration transaction and proof that descriptor/provider timing remains unchanged. |
| `core_machine_controller_authority_smoke.c`; `core_machine_display_authority_smoke.c`; `core_machine_dma_binding_token_smoke.c`; `core_machine_dma_rtc_authority_smoke.c`; `core_machine_executor_run_smoke.c`; `core_machine_fdc_media_change_port_smoke.c`; `core_machine_fdc_smoke.c`; `core_machine_fdc_topology_port_smoke.c`; `core_machine_hdc_smoke.c`; `core_machine_port_assembly_smoke.c`; `core_machine_port_ownership_smoke.c` | These tests observe device installation, ownership, token, media, port, or executor lifecycle itself. Hiding construction steps would create a second test contract and obscure the boundary under test. | Admit a narrow per-subsystem fixture only after its ownership/failure sequence is independently specified and all callers of that subsystem shape are swept. |
| `core_machine_control_transfer_smoke.c`; `core_machine_instruction_timing_smoke.c`; `core_machine_legacy_lock_s1_smoke.c`; `core_machine_protected_16_gate_s3_smoke.c`; `core_machine_protected_data_access_s2_smoke.c`; `core_machine_protected_far_s1_smoke.c`; `core_machine_real_mode_386_address_smoke.c`; `core_machine_real_mode_386_rep_cmps_smoke.c`; `core_machine_real_mode_corpus_smoke.c`; `core_machine_real_mode_tick_smoke.c`; `core_mantle_shape_smoke.c`; `cpu_fault_diagnostic_smoke.c`; `cpu_fpu_profile_smoke.c`; `cpu_int_ivt_smoke.c` | Their direct lifecycle is coupled to instruction timing, prefix/fault observation, explicitly staged protected state, raw reset/image setup, or a construction/failure assertion. `core_machine_control_transfer_smoke.c` additionally owns a reset-vector route with no execution provider. A generic helper would either erase the observation point or broaden its responsibility beyond a setup tail. | Admit only after a future task defines one identical validation-to-commit/failure-observation contract for the named family and proves every owner preserves the same observation boundary. |

## Result

The private helper is now the sole owner of the proved-equivalent historical
execution-provider tail. No public fixture API, production behavior, or
generic test framework was introduced. The retained 31 sources are not a
blanket exemption: each is mechanically named above and has a bounded
admission condition.
