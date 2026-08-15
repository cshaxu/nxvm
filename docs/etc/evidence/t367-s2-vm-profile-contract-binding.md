# T367 S2: VM Profile-Contract Binding

## Result

Production session CPU selection now crosses the core boundary only through
`vm_profile_default_pc_at_cpu_contract_select`.  The VM profile supplies the
base CPU/FPU, instruction-timing, clock and keyboard timing inputs; the
session chooses a valid VM contract and the existing one materialization path
copies that contract into `core_machine_config`.

The generic default PC/AT session may select the current 8086, 80186, 80286
or 80386 contracts.  The IBM 5170 Model 339 keeps its descriptor-selected
80286 contract: a raw CPU option cannot infer or replace its concrete machine
identity.  Invalid CPU or FPU enum values are rejected by VM contract
selection before core construction.

## Ownership and preservation proof

| Boundary | Before S2 | S2 result |
| --- | --- | --- |
| VM profile | Descriptor held CPU/FPU/timing values. | `vm_profile_default_pc_at_cpu_contract_select` makes the capability/timing bundle explicit and validates it in VM ownership. |
| VM session | The default-PC/AT raw override wrote CPU/FPU directly into `core_machine_config`. | `vm_session_cpu_contract_select` converts the requested values into a VM contract; `vm_session_materialize_profile_core_config` is the single writer of CPU/FPU/timing inputs to core. |
| Core machine | Generic profile resolution, decoder/execution, instruction timing and CPU/DMA transaction lifecycle. | Unchanged.  No core file selects IBM, Compaq, XT or any concrete VM profile; no duplicate executor or timing publisher was introduced. |
| Direct core fixtures | Select generic capabilities for CPU semantics and timing tests. | Unchanged and intentionally outside VM composition. |

The contract copies the pre-existing descriptor timing/clock values without
changing them.  It changes no instruction-retirement value, device service
schedule, bus availability behavior, firmware/media asset, or board topology.
8088 remains a later distinct contract admission and is not represented as an
8086 alias here.

## Verification

- Built and executed `vm-default-pc-at-profile-smoke`,
  `vm-session-initialization-atomicity-smoke`, and
  `vm-session-profile-smoke` successfully.  The profile smoke covers default
  fallback, a valid 8086/8087 contract and invalid CPU/FPU rejection; the
  session profile smoke exercises the current 8086, 80186, 80286 and 80386
  product selections; initialization checks descriptor timing materialization
  and construction atomicity.
- CTest selection passed the registered default-profile and initialization
  atomicity tests (2/2).
- A broad Ninja build was attempted but remains blocked by pre-existing,
  unrelated `tests/platform/vm_request_bridge_smoke.c` references to removed
  `VM_PLATFORM_REQUEST_KEY_PRESS` and `data.key_press` names.  It fails while
  compiling that test before an all-current-smokes result can be produced;
  T367 does not change that file or interface.  This is a gate blocker to be
  repaired by its owning maintenance receiver before a later full-gate claim.

## Transfers and non-claims

S2 completes only the VM ownership migration identified by S1.  It does not
close the queued 80286 retirement timing, PC/AT bus, device service, phase or
machine-L3 audit work, and makes no 5170 L3 claim.  Those receivers retain
their existing Queue order.  The unrelated platform-request build failure is
not repaired here because it is outside this profile-contract scope.
