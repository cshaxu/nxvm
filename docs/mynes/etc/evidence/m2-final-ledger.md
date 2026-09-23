# M2 Final Acceptance Ledger

Historical M2 closure receiver map, retained as recorded. Subsequent source
inspection found architecture/boundary differences; the owner requires their
correction in M3. See the [remediation ledger](../m3-remediation.md). This older
map and its executable counts do not prove those corrections are complete.

| Row | Direct current receiver |
| --- | --- |
| BUILD | C11 x64/x86 component builds, complete executable regression and documentation governance check. |
| ROM | `cartridge_contract_smoke`, `media_failure_contract_smoke`, `lifecycle_smoke`. |
| MAP | `bus_smoke`, `addressing_smoke`, `indirect_wrap_smoke`, `store_addressing_smoke`. |
| CPU-ALL | `decode_ledger_smoke`, `opcode_profile`, `cycle_ledger_smoke`. |
| CPU-ADDR | `addressing_smoke`, `branch_smoke`, `indirect_wrap_smoke`, `stack_control_smoke`. |
| CPU-ALU | `alu_exhaustive_smoke`, `alu_smoke`, `shift_semantics_smoke`, `logic_compare_smoke`. |
| CPU-CTRL | `control_smoke`, `reset_smoke`, `interrupt_smoke`, `stack_control_smoke`. |
| RUN | `machine_smoke`, `run_step_equivalence_smoke`, `lifecycle_smoke`. |
| DBG | `debug_workflow_smoke`, `app_debug_commands_smoke`. |
| APP | `command_smoke`, `composition_smoke`, `app_media_commands_smoke`. |
| LIFE | `lifecycle_smoke`, `app_media_commands_smoke`. |
| HOST | `media_failure_contract_smoke`, Common `composition`, `common_machine`, `machine_wait`, `session_monitor`. |
| PROMPT | Common `session_monitor`, `session_queue_failure`, `session_frame`. |

At the recorded M2 closure, the complete regression passed 79 executables on each architecture.
Product artifacts: x64 PE `5B3C03C9C44FAB94035C4341364FBE6128FD35968A79617FD70778AB7CC48A41`;
x86 PE `517BDEDEFCCAB04B827790C86499FEB5FEA99AA060B2B0FF3351292EC149B727`.
