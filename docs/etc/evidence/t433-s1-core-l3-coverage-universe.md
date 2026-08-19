# T433 S1: Core L3 Coverage Universe

`M5:T433:S1:CORE-L3-UNIVERSE:OK`

## Decision

This is the finite coverage universe for T433.  Its unit is a current public
Core capability family: a family is one externally selectable, observable, or
composition-facing mechanism with one Core owner and lifecycle boundary.  It is
not a C function, a test fixture, a VM-only descriptor, nor an implicit claim
that the family is L3.  A later S may split a family only when evidence exposes
a distinct owner, lifecycle, or public contract; that S must update this record
and account for every affected disposition.

The S1 audit inspected every header below `src/core/`, the `core-machine`
public include boundary in `CMakeLists.txt`, and the retained evidence families
listed in the T433 proposal.  Internal headers with no exported capability
remain evidence of an owner, not additional universe rows.  VM profile policy
is deliberately excluded: it is a consumer of these Core capabilities.

## Master-ledger schema and completion predicate

Every row in the later master ledger must contain these fields:

| Field | Required content |
| --- | --- |
| Capability ID | The stable neutral ID below; no model/profile name. |
| Owner and caller boundary | Owning Core module/header and the public caller or composition boundary. |
| Historical evidence | Exact retained ledger/audit or a precise `none` finding. |
| Contract | Selected L3 contract, accepted L2 fallback, not-applicable reason, or unsupported reason. |
| Lifecycle | Begin/wait/commit or publication, acknowledgement/deassertion where relevant, cancellation and cold-reset behavior. |
| Regression owner | Existing named smoke/corpus owner, or the next candidate that must create one. |
| Conformance disposition | Exactly one: conformant; discrepancy with earliest next-Core receiver; accepted L2 fallback; not applicable; or unsupported. |

T433 can close only when every frozen ID has exactly one such disposition and
an evidence/code owner, and every discrepancy has the earliest receiver among
Core timing-plan, CPU-program, transaction/arbitration, controller/device, or
integration-export work (or a named TODO).  A boot or generic default has no
conformance value by itself.

## Frozen capability universe

| ID | Current Core owner and public boundary | Retained corpus family | Reconciliation batch |
| --- | --- | --- | --- |
| `CPU-EXEC` | `machine/cpu_interface.h`, `cpu.h`, `cpu_instructions.h`; execution provider and machine callers | T357/T359/T363/T401/T420 CPU ledgers | S2 CPU execution |
| `CPU-EXCEPT` | `cpu_interface.h`, `cpu_instructions.h`; fault/interrupt delivery through machine execution | T357/T401 | S2 CPU execution |
| `CPU-PREFETCH` | `cpu_instructions.h`, `machine_interface.h`; CPU/external-cycle callers | T354/T369 and retained shared-Core DeskPro evidence | S2 CPU execution |
| `CPU-RETIRE` | `retirement_observation_interface.h`, `machine_interface.h`; execution/observation callers | T346/T354/T357/T401 | S2 CPU execution |
| `CPU-FPU` | `fpu_interface.h`, `fpu.h`; escape and profile callers | T373/T401 | S2 CPU execution |
| `TIME-CLOCK` | `clock.h`, `timeline.h`, `machine_interface.h`; machine clock-plan callers | T346/T354/T369/T377 | S3 transaction and time |
| `TIME-LIFECYCLE` | `lifecycle_interface.h`, `execution_provider.h`, `machine_interface.h`; start/stop/reset callers | T346/T354/T377 | S3 transaction and time |
| `TXN-MEMORY` | `memory_interface.h`, `transaction.h`, `machine_interface.h`; CPU, DMA and stopped-state callers | T354/T369 and T401 | S3 transaction and time |
| `TXN-PORT` | `port_interface.h`, `port.h`, `transaction.h`; CPU, device and stopped-state callers | T354/T369 | S3 transaction and time |
| `TXN-ARBITRATION` | `transaction.h`, `dma.h`, `machine_interface.h`; CPU/DMA/refresh/HOLD callers | T354/T369 and retained shared-Core DeskPro evidence | S3 transaction and time |
| `MEM-RAM-A20-PARITY` | `memory.h`, `memory_interface.h`, `machine.h`; CPU/DMA/firmware callers | T346/T354/T373/T377 | S3 transaction and time |
| `MEM-ROM-FIRMWARE` | `rom_mapping_interface.h`, `firmware_interface.h`, `entry_plan_interface.h`; firmware/provider callers | T346/T373/T377 | S6 integration/export |
| `MACHINE-CONFIG` | `machine_interface.h`, `machine.h`, `controller_interface.h`; VM composition callers | T346/T373/T377/T404 | S6 integration/export |
| `CTRL-PIC` | `pic.h`, `controller_interface.h`; IRQ source and CPU acknowledgement callers | T346/T354/T370/T377 | S4 controller/device |
| `CTRL-DMA` | `dma.h`, `controller_interface.h`; FDC/device request and memory callers | T346/T354/T370/T377 | S4 controller/device |
| `CTRL-PIT` | `pit.h`, `clock.h`; timer/IRQ/speaker callers | T346/T354/T370/T377 | S4 controller/device |
| `CTRL-RTC-CMOS` | `rtc.h`, `clock.h`, `controller_interface.h`; calendar/IRQ callers | T346/T354/T370/T377 | S4 controller/device |
| `CTRL-KBC-NMI` | `kbc.h`, `controller_interface.h`; host-input, IRQ and NMI callers | T346/T370/T377/T404 plus T432 | S4 controller/device |
| `CTRL-FDC` | `fdc.h`, `fdc_observation_interface.h`, `controller_interface.h`; DMA/media/IRQ callers | T346/T354/T370/T377/T404 | S4 controller/device |
| `CTRL-HDC` | `hdc.h`, `controller_interface.h`; media/IRQ callers | T346/T354/T370/T377/T404 | S4 controller/device |
| `MEDIA-BACKING` | `media_interface.h`; controller and composition backing-resource callers | T346/T370/T373/T404 | S4 controller/device |
| `DISPLAY-VADP` | `vadp.h`, `display_interface.h`; display-port/memory and machine callers | T346/T370/T373/T377/T404 | S4 controller/device |
| `DISPLAY-PRESENT` | `presentation_interface.h`, `display_frame.h`; copied-frame consumer callers | T346/T370/T404 | S6 integration/export |
| `INPUT-HOST` | `platform/input_interface.h`, `kbc.h`; copied host-input to Core KBC boundary | T346/T370/T404 plus T432 | S6 integration/export |
| `TRACE-DEBUG` | `trace_interface.h`, `debug_interface.h`, `debug_access.h`; debugger/observation callers | T346/T354/T401/T404 | S6 integration/export |
| `PLATFORM-MAILBOX` | `platform/presentation_mailbox_interface.h`; presentation consumers | T346/T370/T404 | S6 integration/export |
| `PLATFORM-RESOURCE` | `platform/backing_resource_interface.h`; media/provider consumers | T346/T370/T404 | S6 integration/export |
| `PLATFORM-WAIT` | `platform/wait_interface.h`, `utils/wait*.h`, `platform/sleep.h`; platform adaptation callers | T346/T373/T404 | S6 integration/export |
| `SESSION-COMMAND` | `product/session/{session_interface,session_provider,command_interface}.h`; session/product consumers | T404 | S6 integration/export |
| `PRODUCT-DEBUG` | `product/debug/*.h`, `product/utils.h`; product tooling consumers | T404 | S6 integration/export |

## Corpus allocation and S boundary

S2 consumes CPU forms, exception/retirement, prefetch, and FPU rows from the
T357/T359/T363/T401/T420 corpus.  S3 consumes timeline, clock, memory, port,
arbitration and reset rows from T346/T354/T369/T377.  S4 consumes controller,
media and display state-machine rows from T346/T370/T373/T377/T404.  S6
consumes the remaining public Core integration and platform/product boundaries
and creates the conformance report.  S5 is reserved for whole-ledger
cross-batch reconciliation and only exists if S2--S4 identify a finite class
that no assigned batch can truthfully dispose of.

The input corpus is frozen as the proposal names it: T346, T354, T357,
T359/T363, T369, T370, T373, T377, T401, T404 and T420, plus later retained
DeskPro evidence only where it changed the shared Core owner.  These sources
are evidence inputs, not competing current architecture authorities.

## S1 verification

- `rg --files src/core | rg "\\.h$"` enumerated 53 headers, accounted for as
  follows: `utils/` (2: `wait*.h`); `platform/` (6: backing resource, display
  frame, input, presentation mailbox, sleep and wait); `product/` (10: banner,
  utilities, three session contracts, three debug contracts and two xasm
  contracts); and `machine/` (35: CPU/execution/FPU/retirement, machine/time/
  lifecycle, memory/port/firmware/ROM/entry, controller configuration, device,
  display/presentation, trace and debug contracts).  The table groups these
  headers by capability family rather than treating each header as a separate
  L3 assertion.
- `CMakeLists.txt` declares `core-machine` with `src` as its public include
  root; the audit also swept `core/platform` and `core/product` headers so that
  public integration paths cannot disappear behind a machine-only filter.
- The table has 30 unique neutral IDs, one owner/boundary, one corpus family,
  and one later batch per ID.  S1 makes no conformance disposition and records
  no retained-ledger contradiction.

This completes only the coverage-freezing S.  It neither accepts an L3 rule nor
changes a public or runtime capability.