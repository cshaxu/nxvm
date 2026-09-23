# T433 S2: Core CPU Capability Reconciliation

`M5:T433:S2:CPU-LEDGER:OK`

## Batch decision

This record disposes exactly the five S1 CPU capability families. It reconciles
current code and tests with the retained T357/T359/T363/T401/T420 corpus; it
does not repeat those audits or elevate their bounded claims to physical/L4.

| ID | Owner / caller boundary | Contract and lifecycle | Regression owner | Disposition |
| --- | --- | --- | --- | --- |
| `CPU-EXEC` | `cpu_interface.h`, `cpu_instructions.h` and `machine.c:core_machine_run`; VM binds an execution provider through `machine_interface.h`. | One successful refresh reaches `core_machine_instruction_cost`; the private classifier chooses one retained source family or sets explicit unallocated state. Faulted/rejected forms publish no successful-retirement time; cold reset clears CPU state. | `core-machine-instruction-timing-ledger-smoke`, `verify-t359-instruction-timing-inventory`, T401 S77 matrix. | Conformant for the frozen selected form/context contract. Unallocated/range forms retain their explicit nonphysical transfer, not a silent default. |
| `CPU-EXCEPT` | `cpu_instructions.h` fault delivery consumed by `machine.c:core_machine_run`; PIC/NMI route is a later controller row. | A synchronous instruction fault commits its frame/vector, returns before elapsed/device-time publication, and begins the handler on the next round; cold reset clears execution state. | `core-machine-exception-delivery-s2-smoke`, `core-machine-interrupt-entry-smoke`, T401 S78. | Explicit accepted L2 fallback: semantic delivery is audited, but exception/IRQ/NMI cycle costs remain an unaccepted CPU timing receiver. |
| `CPU-PREFETCH` | `cpu_instructions.h` reservation and `machine_interface.h` policy, selected by Core configuration. | Reservation is optional, invalidated on completed round/reset/HOLD/refresh paths, and may not turn completed adjacency into overlap; reset clears pending/overlap state. | `core-machine-prefetch-locality-smoke`, retained T411/T419 evidence. | Explicit accepted L2 fallback: the present deterministic lifecycle has no asynchronous physical producer, cache or phase contract. |
| `CPU-RETIRE` | `retirement_observation_interface.h`, `machine.c` sole elapsed-time publisher; copied observer is installed while stopped/paused. | Observation is captured before publication; physical mode rejects unallocated or unqualified success before elapsed/timeline visibility. Reset clears observation and eligibility state. | `core-machine-instruction-timing-ledger-smoke`, `core-machine-retirement-observation-s3-smoke`, T401 S78. | Conformant for deterministic/nonphysical retirement observation and guarded physical rejection; physical retirement qualification remains a named external receiver. |
| `CPU-FPU` | `fpu_interface.h`/`fpu.h` called through ESC/FWAIT execution in `cpu_instructions.c`; profile/state is exposed by `machine_interface.h`. | Core resets the embedded FPU in cold reset; ESC/FWAIT profile gating and copied state are retained. | `core-machine-fpu-interface-s65-smoke`, `core-machine-fpu-8087-smoke`, T401 S78. | Explicit accepted L2 fallback: current bounded interface/selected operations exist, while numerical x87 breadth and timing remain `Broaden present x87` TODO. |

## Source and conformance sweep

T359/T363 establish that the 256 primary and 256 `0F` dispatch inventories have
one successful-retirement classifier or an explicit receiver. T401 S78 confirms
the current selected semantic/source-timing matrix and the sole publication
boundary. T420 is a duplicate-audit withdrawal, so it supplies no competing
CPU conclusion. Current `machine.c` retains this ownership: cost selection and
publication occur after successful refresh; fault delivery returns before
retirement publication; `core_machine_cold_reset` resets CPU, FPU, retirement
observation, external-cycle and prefetch state.

No retained-ledger contradiction was found. The three fallback rows are not
implementation discrepancies: their missing physical/timing facts are
explicitly transferred to the later Core CPU-program, transaction/arbitration,
or controller/device candidates and the named x87 TODO. No runtime code,
public ABI or developer artifact changed in S2.