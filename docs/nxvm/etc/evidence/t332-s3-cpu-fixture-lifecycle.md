# T332 S3: CPU Smoke Fixture Lifecycle Convergence

## Fixed Inventory

S3 consumes the authoritative 47-entry
`PROJECT_T317_STRICT_CPU_SMOKE_INVENTORY`, rather than scanning a directory to
create a moving requirement. All 47 owner translation units now reach
`tests/support/core_machine_cpu_fixture.h` for their common execution-provider
bind/freeze/reset lifecycle. The mechanical verifier reads the generated
inventory and rejects a missing shared operation or a reintroduced direct
`core_machine_bind_execution_provider` or
`core_machine_freeze_execution_providers` call.

Six T316 owner translation units are wrappers that include one of five retained
historical owner sources. S3 migrates those five inherited sources as well:
`core_machine_arpl_smoke.c`, `core_machine_cli_sti_smoke.c`,
`core_machine_protected_return_atomicity_smoke.c`,
`core_machine_protected_iret_smoke.c`, and
`core_machine_interrupt_entry_smoke.c`. This makes the wrapper graph and the
direct targets use the same lifecycle owner.

Six owner fixtures still call `core_machine_create` directly before the shared
bind/freeze/reset helper: BOUND, immediate IMUL, LAR/LSL, ordinary port I/O,
port strings, and VERR/VERW. Each installs instruction-specific GDT/IDT or
port-provider state before binding execution; only the later common lifecycle
uses the support helper. They are retained local setup, not a second bind or
freeze path.

## Boundary And Transfer

The all-tracked-source query also finds older CPU/device/protected-mode smokes
outside the fixed T316 owner inventory. They have not been mechanically proven
to share the same setup contract and are not silently exempted: the remaining
audit is transferred to the bounded TODO admission below. S3 deliberately does
not make a generic fixture framework or rewrite those unrelated historical
families.

## Verification

`verify-t332-cpu-fixture-lifecycle` performs the positive 47-owner inventory
check and an internal negative self-check for direct bind/freeze detection.
The affected ARPL, CLI/STI, HLT, software-INT, protected IRET, and outer-IRET
smokes are rebuilt and rerun through the full current CTest selection. Existing
smoke markers remain unchanged.
