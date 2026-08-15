# T375 S1: Model-339 Timing Inventory And Repair Allocation

## Decision

The frozen Model-339 has one deterministic event graph, but its descriptor
still carries generic scheduling ratios.  Those ratios are not evidence of
IBM 5170 board timing merely because they cause repeatable callbacks.  This
inventory separates the two facts, preserves every accepted functional owner,
and selects only the timing repairs supported by a primary source and the
existing project clock domain.

The project clock advances from successful 80286 retirement and is not host
time.  T368's Intel source rows are CPU contributions under their documented
no-bus-delay assumptions.  Therefore a component rate can be expressed as a
nominal ratio to the selected 8 MHz CPU clock only when the source establishes
both rates; it cannot silently supply RAM/ROM/ISA READY, prefetch, controller
service, or physical waveform time.

`M5:T375:S1:MODEL339-TIMING-INVENTORY:OK`

## Selected Route And Clock Matrix

| Boundary | Current owner and actual behavior | Primary fact / current evidence | S1 disposition |
| --- | --- | --- | --- |
| Successful 80286 retirement and timeline | `machine.c` is the sole publisher: it charges an accepted source instruction cost, increments `elapsed_ticks`, then advances the timeline. | T368 accepts Intel 210498-005 successful-retirement rows only. T371 already excludes prefetch, bus occupancy and READY from that publisher. | Retain as the nominal CPU-clock domain only; no new instruction scalar or wait value. |
| CPU memory, ROM, CGA, ISA and port availability | One shared transaction owner provides begin/commit/cancel and copied trace, but no duration field or wait state. | IBM 6280099 establishes board topology; Intel 80286 hardware material establishes READY/HOLD protocol, but neither selects a Type-3 per-route project duration. | Reference-exhausted, explicitly zero-added-wait current behavior. A later S may add only source- or probe-constrained availability; S1 allocates no generic table. |
| DMA/HOLD and 8237A service | `dma.c` selects and transfers; the arbitration callback performs the logical 80286 hold lifecycle and one transfer per delivered DMA tick. The Model-339 ratio is currently `1/1`. | Intel 8237A specifies state-machine clocks, HRQ/HLDA and optional Ready waits, but not the Type-3 board's DMA input rate, grant latency, DACK/AEN phase or FDC-paced service. | The `1/1` ratio is deterministic scheduling, not a Model-339 scalar. Retain logical ordering; S2 must expose a bounded service/availability contract before altering DMA pace. |
| PIT/IRQ0 and PIC visibility | The arbitration callback converts the descriptor's PIT ratio (`1/4`), advances the 8254, then refreshes PIC. | The PC/AT board supplies the 8254 timer route. The documented PC timer input is approximately 1.193182 MHz; 8254 and IBM material establish counter/IRQ behavior, not PIC propagation or INTA waveform. | Existing `1/4` is not the nominal 8 MHz-to-PIT relation. S2 is eligible to replace it with a source-labelled rational clock conversion, while retaining PIC/INTA latency as separately bounded. |
| RTC/CMOS/IRQ8 | The readiness callback converts the current RTC ratio (`1/1`) and `rtc.c` treats `rtc_ticks_per_second=50000` as one second. | IBM PC/AT Technical Reference 6280099, System Board 1-57, says the initialized divider selects a **32.768-kHz time base**. The selected MC146818 route and IRQ8 are already functionally accepted. | Concrete repair selected for S2: use the primary-selected 32,768 Hz oscillator with a rational conversion from the nominal 8 MHz CPU domain and set the RTC second divisor consistently. IRQ propagation phase is still not a physical waveform claim. |
| 8042 / keyboard / IRQ1 | The peripheral callback advances the KBC at ratio `1/1`; all Model-339 command-response and typematic durations are zero, so no delayed response or repeat duration is enabled. | IBM documents the 8042/keyboard protocol, not a selected controller-to-project clock conversion. T370 S4 already records this exact zero-duration boundary. | Retain logical native-input/FIFO/IRQ1 behavior and zero-duration configuration. No scalar or artificial host-input latency; later timing work needs an admissible controller/keyboard measurement contract. |
| FDC, field-upgrade drive, DMA2 and IRQ6 | The readiness callback advances one pending FDC state then refreshes media. Command/result, DRQ and IRQ are functional; no FDC clock or drive rotational duration is represented. | uPD765A supplies controller-local protocol/timing classes, but IBM's no-factory 1.44 MB field upgrade does not choose a TEAC/controller/board conversion. Raw IMG explicitly excludes Deleted/Control-Mark/Scan. | Keep the accepted raw-IMG functional boundary. The one-readiness-step behavior is ordering-only, not drive service evidence. Any nonzero command/rotation/DRQ pace needs a separately source-labelled FDC service model in this T. |
| CGA raster and frame visibility | The peripheral callback converts a `1/1` VADP clock and advances synthetic text timing `{48, 8, 8}`; snapshots are copied after core ownership. | Model-339 selects digital CGA. IBM CGA/board documentation establishes ports and topology, but the existing values lack a Model-339 dot/retrace/contention source mapping. | Existing raster values are deterministic presentation state, not a physical CGA claim. S2 must classify/admit a source-backed digital-CGA clock/cadence contract or retain the visible reference-exhausted boundary. |
| Planar parity/NMI and reset | `machine.c` owns logical latch/mask/delivery ordering and cold reset clears controllers, timeline and clock domains before scheduling three tick-one callbacks. | IBM establishes the parity/NMI topology and reset-visible behavior; no selected propagation or settling scalar was found. | Preserve deterministic reset/cancellation order. Do not add a reset delay or NMI waveform without an admissible source/probe. |
| Cross-device same-tick order | Timeline order is `DMA -> PIT -> PIC -> FDC -> HDC -> RTC -> KBC -> VADP`; HDC is absent in Model-339 but the shared callback remains. | T346/T369/T371 focused proofs establish logical order, reset replacement and copied trace. | Retain order as a model contract. It does not convert same-tick ordering into sub-cycle electrical phase or make excluded ATA/HDC a Model-339 device. |

## Source And Reference Boundary

Primary authority for the selected machine is the [IBM PC/AT Technical
Reference, March 1986, 6280099](https://www.minuszerodegrees.net/manuals/IBM/IBM_5170_Technical_Reference_MAR86.pdf).
Its RTC section selects the 32.768-kHz base explicitly.  Intel's
[8237A data sheet](https://www.pcjs.org/documents/datasheets/intel/INTEL_8237A_DMA.pdf)
is used only for component state/clock requirements: it states that service
uses clocked states and may insert Ready waits, not a Model-339 board scalar.
The relevant 80286, 8254, 8259A, uPD765A, MC146818 and UPI-41/42 material
remains component authority in the same sense.

No source, ROM, media, binary, trace, or derived implementation was imported.
86Box `ibmat`, MAME `ibm5170a`, and PCjs 5170 remain permitted only as
revision-pinned, same-profile behavioral cross-checks after the primary-source
disposition; none is a board-fact source or a substitute for a missing rate.

## Mechanism Finding And S2 Allocation

The repeated mechanism issue is not a device-local typo: the descriptor's
clock-plan entries are consumed by one shared `core_machine_clock_domain`, so
an unsupported ratio can make a generic scheduling cadence look physical for
multiple devices.  The shared owner is the Model-339 profile-to-core clock
contract; consumers are arbitration (DMA/PIT/PIC), readiness (FDC/RTC), and
peripheral (KBC/VADP) callbacks.  S2 must repair the source-backed RTC and
PIT entries through that one contract, prove their reset phase and IRQ-visible
effects, and leave every unsupported device rate visibly bounded rather than
manufacturing a parallel device clock or test-only route.

The broader timing task remains open after S1.  In particular, it must decide
whether the current retirement-only advancement can represent device progress
while the CPU is halted or externally unavailable; no current evidence lets a
retirement batch be called physical elapsed board time.  This is an in-scope
phase-model question, not a reason to change an unrelated functional device.

## Verification

The audit inspected the Model-339 descriptor/contract copy, core clock-domain
conversion, all three timeline callbacks, CPU retirement publisher,
transaction/HOLD owner, FDC/RTC/KBC/VADP advance paths, reset scheduling, and
the retained T346, T368--T374 evidence.  Documentation governance and
`git diff --check` are required for acceptance.  S1 changes no executable or
test target, so it does not make a full-build or physical-timing pass claim.
