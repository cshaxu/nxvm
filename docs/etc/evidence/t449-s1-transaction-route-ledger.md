# T449 S1: CPU-To-Board Transaction Route Ledger

`M5:T449:S1:TRANSACTION-ROUTE-LEDGER:OK`

## Frozen Scope And Method

This ledger consumes exactly `CPU-PREFETCH`, `TXN-MEMORY`, `TXN-PORT`,
`TXN-ARBITRATION`, and `MEM-RAM-A20-PARITY`. It rechecks the T354/T369 route
ledgers against the current source tree; it does not infer a board wait value
or change runtime behavior. The sweep covered the production occurrences of
the transaction lifecycle, CPU physical/port helpers, DMA transfers, scheduler
callbacks, external-cycle/BUSRDY/prefetch state, A20/parity state, copied plan
construction, and VM materializers. Stopped/paused memory, port and debug
operations are deliberately excluded: they are immediate bounded APIs outside
guest execution.

## Actual Route Ledger

| Capability | Current owner and sole production route | Default behavior and evidence tier | Direct-path residue / S2 receiver |
| --- | --- | --- | --- |
| `CPU-PREFETCH` | `cpu.c` owns CPU prefetch/reservation state; `machine_scheduler.c` releases a reservation only when the shared transaction and DMA/refresh state are idle. | Reservation is selected by copied config, reset-safe, and explicitly has no asynchronous producer or physical overlap claim. This is L2. | `core_machine_config.cpu_prefetch_reservation_enabled` and the external-cycle observer are raw configuration/mechanism details. S2 must select a registered plan contract, not add a CPU-side producer or profile callback. |
| `TXN-MEMORY` | `cpu_instructions.c` begins/commits/cancels CPU checked physical reads and writes; `dma.c` does the same for ordinary and memory-to-memory DMA; `transaction.c` is the one mutable transaction-state owner. | Validation remains memory-owned; all executing CPU/DMA accesses are synchronous commit-or-cancel. T354 focused transaction and competition smokes remain the regression owners. L2 where a board availability value is absent. | `machine.c` receives external-cycle observations and derives retirement waits after committed CPU accesses. It is a parallel timing selection path, not a second memory commit path. S2 must fold its selection into the plan contract while retaining one transaction owner. |
| `TXN-PORT` | `_p_input` and `_p_output` own CPU transaction wrapping; `port.c` owns provider dispatch and its atomic restoration on failure; `transaction.c` publishes begin/commit/cancel. | Executing port access is synchronous. Stopped `port_interface` operations remain immediate. Electrical/ISA timing is L2 unless a selected contract exists. | The seven fixed external-access wait-window slots are raw configuration data consumed through the external-cycle observer. S2 must make their capability/rule selection declarative and reject an incomplete entry rather than duplicate port-provider logic. |
| `TXN-ARBITRATION` | `machine_scheduler.c:core_machine_arbitration_tick` is the only recurring arbitration publisher; it advances refresh, DMA, PIT and PIC in its fixed order. `transaction.c` owns HOLD request/acknowledge/release; DMA owns request selection and transfer; D4 refresh uses the same HOLD and transaction lifecycle. | CPU instructions finish their synchronous transaction before retirement. 80286/80386 DMA uses the existing logical HOLD lifecycle; all other profiles retain the same deterministic DMA route. Physical waveforms and unspecified priority duration remain L2. | DMA wait/BUSRDY gates and D4-specific refresh values are copied raw config/state. S2 must register the generic availability/arbitration choice; later S5 consumes it without creating another scheduler or device-local arbitration path. |
| `MEM-RAM-A20-PARITY` | `memory.c` owns checked RAM, address mapping, A20 policy and parity storage/detection; `machine_plan.c` owns plan-time device/parity materialization; `machine` owns parity/NMI integration and reset clears latches. | Mapping and parity effects are functional Core behavior. Controller detection latency, latch visibility timing and board-specific classes are L2 until selected evidence exists. | VM Model-40 materialization supplies topology/data to the copied plan, while the default PC/AT route materializes its own topology. S2 must not merge these profiles or put board names in Core; it must expose only neutral transaction-class selection. |

## Ownership Decision

The current tree already has one transaction state (`transaction.c`), one
arbitration callback (`machine_scheduler.c`), one virtual-time publisher
(`core_machine_publish_elapsed_ticks`), and one CPU retirement publisher. No
second CPU/DMA memory-or-port commit route was found. The FPU command
transaction is a separate x87 control capability and is not a member of this
five-row ledger; controller command/DRQ/IRQ phases likewise remain the next
candidate's scope.

The finite S1 finding is narrower: `core_machine_config` still transports
external-cycle timing, access-wait windows, DMA wait/BUSRDY gates and prefetch
selection as raw fields. `core_machine_plan_create` copies those fields, but
the existing 30-capability declarations do not yet make their registered rule,
L2 exception, lifecycle and rejection semantics the sole selected transaction
contract. The CPU external-cycle observer and retirement-wait state consume
those raw values after a CPU transaction commits. They must be consolidated at
the plan/transaction boundary, never copied into CPU, DMA, port providers or
VM profile callbacks.

## S2 Admission Boundary

S2 must inventory each selected transaction/arbitration declaration and make
the immutable Core plan validate and own it. The plan must distinguish
transaction class, availability/BUSRDY policy, arbitration eligibility and
explicit L2 fallback, reject unregistered or incomplete selection before
machine publication, and preserve the present default configurations. It may
reuse the current copied plan and transaction owner, but may not introduce a
bus facade, profile-named Core branch, second scheduler, mutable cross-module
pointer, or per-device timing wrapper.

The T449 S2 proof must cover the existing default PC/AT, Model-339 and
Model-40 plan materializers, rejection of an invalid selection, reset of the
copied availability state, and no change to stopped/paused APIs. S3--S6 may
then migrate the shared mechanism one owner at a time.
