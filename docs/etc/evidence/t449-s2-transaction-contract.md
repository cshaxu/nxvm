# T449 S2: Copied Transaction Contract

`M5:T449:S2:TRANSACTION-CONTRACT:OK`

## Delivered Boundary

`core_machine_transaction_contract` is the single immutable value selected by
the existing copied Core plan for external-cycle timing, bounded access windows,
DMA wait/BUSRDY, CPU BUSRDY and prefetch reservation. `core_machine_plan`
validates it before machine publication; `core_machine_create_internal` copies
it once into the Core-owned machine. CPU timing, the scheduler and board input
operations now read that one value. Dynamic transaction, HOLD, BUSRDY-level,
DMA-wait counter, prefetch, reset and retirement state remain at their existing
owners.

The six former scattered configuration and machine fields were removed. No
CPU, DMA, port-provider or profile callback was added; the Model-40 value is a
nested initializer in its existing copied configuration, while default PC/AT
and Model-339 retain the zero/default contract through their existing plan-only
materializers. Existing direct Core creation remains a non-product test/VDM
boundary and uses the same typed configuration value rather than a second plan
publisher.

## Validation And Sweep

- `core_machine_transaction_contract_is_valid` validates the existing external
  cycle and access-window forms plus all three boolean gates.
- `core_machine_plan_validate` rejects an invalid contract before
  `core_machine_create_from_plan` allocates or publishes a machine.
- `core-machine-plan-smoke` now makes a non-power-of-two page size and proves
  null-output invalid rejection, emitting `M5:T449:S2:TRANSACTION-CONTRACT:OK`.
- The direct-field sweep
  `rg -n "config\\.(external_cycle_timing|external_access_wait_windows|dma_cycle_wait_quanta|dma_cycle_bus_ready_gate_enabled|cpu_cycle_bus_ready_gate_enabled|cpu_prefetch_reservation_enabled)|->(external_cycle_timing|external_access_wait_windows|dma_cycle_wait_quanta|dma_cycle_bus_ready_gate_enabled|cpu_cycle_bus_ready_gate_enabled|cpu_prefetch_reservation_enabled)" src tests --glob '*.[ch]'`
  has no hits. Remaining names occur only as members of the contract or as
  intentional `transaction_contract.*` consumers.

## Verification

WinLibs UCRT GCC/Ninja rebuilt the runnable target and every focused target.
Direct execution passed the new plan marker and retained plan, external-cycle,
prefetch, arbitration/HOLD, transaction lifecycle, Model-339, Model-40 and
dual-DMA markers. Documentation governance passed for `vm-0-5-0448`, as did
`git diff --check`.

The tracked source/test change is `+145/-119`, net `+26`, from
`git diff --numstat -- src tests`. The new 13-line contract/validator and one
negative test replace six separate configuration and machine selections; no
wrapper or duplicate state remains.

## S3 Prerequisite

S3 may consume only this copied contract and the existing transaction owner to
reconcile CPU/DMA transaction class and lifecycle selection. It must not add a
new static selection field, scheduler or device-local transaction path.
