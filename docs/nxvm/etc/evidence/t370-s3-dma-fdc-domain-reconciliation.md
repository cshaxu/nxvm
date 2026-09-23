# T370 S3: Dual-8237A And FDC Domain Reconciliation

## Decision

The selected Model-339 FDC route has one logical request and publication path:
`fdc.c` owns command/result phase, DRQ assertion/deassertion and IRQ6 source;
the opaque bound request reaches DMA channel 2; `dma.c` alone selects, validates
and commits the transfer; T369's arbitration owner provides one logical
80286 HOLD lifecycle; and `pic.c` owns IRQ6 visibility and logical
acknowledgement. Reset and finalization release both DRQ and IRQ6 before local
FDC state is cleared.

Intel 8237A and IBM PC/AT documents establish request, cascade, terminal/EOP
and dual-controller topology. NEC uPD765A material establishes command/result
protocol and SRT/HLT/HUT controller timers. Neither source maps those timer
units, DRQ pacing or drive mechanics onto the project's completion-derived
elapsed ticks. The FDC's current transition from `PENDING_COMMAND` or
`PENDING_COMPLETE` on a subsequent readiness callback is therefore a
deterministic logical event, not a calibrated uPD765 duration. No S3 timing
value or runtime change is admitted.

## Owner and lifecycle matrix

| Boundary | Existing owner and proof | S3 disposition |
| --- | --- | --- |
| FDC command acceptance/result phase | `fdc.c` owns ports `3F0h--3F7h`, command assembly, main status, result bytes and terminal phase. `core-machine-fdc-smoke` and retained VM FDC corpus smokes exercise these routes. | Retain protocol state; command-to-execution delay is not allocated. |
| DRQ to DMA2 | FDC invokes its opaque assert/deassert binding; `machine.c` binds the selected FDC route to primary DMA channel 2; `dma.c` alone holds request/mask/service state and commits transfers through the shared transaction owner. | Retain logical request publication and T369 HOLD ordering. Physical DREQ/DACK/AEN and per-byte pace remain phase work. |
| DMA terminal/EOP | `dma_complete_transfer` owns terminal publication, auto-init/mask/TC and provider terminal callback; binding-token validation prevents another channel from terminating the active path. `core-machine-dma-channel-smoke` proves the complete S2--S4 DMA matrix. | Retain transaction semantics. A terminal condition is not proof of FDC sector, rotational or service time. |
| Completion/IRQ6 acknowledgement | FDC result completion raises one bound IRQ6 source; Sense Interrupt Status deasserts that source after returning status. PIC selection/EOI remains its own owner. | Retain logical source lifecycle. No physical IRQ propagation or INTA spacing is admitted. |
| Cancellation/reset/finalize | DOR reset, drive-not-ready cancellation, FDC reset and finalization deassert DRQ and IRQ6; DMA reset clears transients while retaining valid frozen binding ownership. | Existing cancellation prevents stale publication. No timed reset/recalibrate interval is claimed. |
| Trace and deterministic order | Readiness runs FDC advance/refresh before later arbitration; arbitration owns DMA then PIT/PIC. Competition and FDC-DMA boundary smokes prove the copied visible logical order. | Trace sequence proves order only, not bus/controller duration. |

## Controller-domain result

T366 S8 already records the decisive source limit: uPD765 SRT, HLT and HUT are
controller-clock quantities, while the current readiness callback is scheduled
from CPU-retirement time. S1/S2 additionally establish that no existing core
ratio is source-labelled as the uPD765 clock or an 8 MHz-to-controller
conversion. Thus a correct S3 result is a **non-admission**, not a nominal
conversion or reference-emulator scalar.

86Box, MAME and PCjs are not comparable timing authorities here: none is a
primary timing source for the locked Model-339 plus compatible field-upgrade
drive, and the prerequisite project-domain conversion is absent. They remain
future behavioral cross-checks only after a primary controller/board contract
admits a bounded comparison.

## Sweep and S4 receiver

The audit sweeps all DMA bind/request/assert/deassert/terminate/advance/reset/
finalize paths, FDC command/result/DRQ/IRQ/reset/finalize routes, the shared
transaction trace adapter, Model-339 FDC composition and the DMA/FDC/
competition smokes. The existing routes retain one owner each; no separate
service scheduler, raw DMA exposure or unclassified reset route exists.

S4 receives 8042/keyboard FIFO/IRQ command-response reconciliation. The
remaining FDC controller-clock, DRQ pacing, motor/seek/recalibrate, rotational
and TEAC mechanical service values transfer to selected-profile phase
refinement after T370. **This S does not claim 5170 Model-339 L3.**
