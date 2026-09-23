# M5 T354 S1: Physical L3 Transaction And Timing Ledger

## Decision

T346 already supplies deterministic guest-time progression: CPU retirement
advances one core-owned timeline, and its three recurring callbacks run in
sequence order at every due tick. T347 through T353 close selected controller
service and topology ownership on that foundation. This is a useful L3 event
order, but it is not yet one physical transaction model: CPU instruction
execution and DMA still access the physical-memory and port owners directly,
with no common availability, ownership, commit, cancellation, or trace
boundary.

T354 therefore admits no arbitrary wait counter. It allocates the repeated
CPU/DMA construction to S2, the admitted requester/service competition to S3,
and final reset/trace/consumer reconciliation to S4. Electrical timing and
unselected hardware remain explicit transfers.

## Current Timing Owner Map

| Selected family | Current owner and transaction/commit boundary | Time or order now | Lifecycle and proof | S1 disposition |
| --- | --- | --- | --- | --- |
| CPU retirement and instruction cost | `machine.c:core_machine_instruction_cost` derives a base cost plus limited prefix, branch, moffs-memory, and ordinary-I/O surcharges; `core_machine_advance_scheduler` advances the timeline only after retirement. CPU instruction helpers call physical RAM and port owners directly. | Synthetic elapsed ticks after each retired instruction; no independently owned memory or port transaction. | Cold reset clears elapsed ticks and re-arms the timeline. Existing CPU/timeline smokes prove retirement order, not a bus claim. | Retain cost as deterministic execution cadence. S2 must establish how selected CPU memory/I/O availability relates to it without double charging or inventing a cycle table. |
| CPU fetch, data memory, A20 and immutable ROM | `cpu_instructions.c:_m_read/_m_write` calls `core_machine_memory_read_physical/write_physical`; mapping, A20 and immutable ROM remain memory-owner semantics. | Immediate data publication at helper call; no bus owner, duration, or contention. | Checked physical route and immutable mapping are retained; reset resets memory state. External stopped/paused API traces separately. | S2 common transaction candidate for CPU data/fetch only if selected contract requires availability. Preserve mapping validation before visibility. Exact DRAM wait states transfer. |
| CPU ordinary and string I/O | `cpu_instructions.c:_p_input/_p_output` calls the frozen `t_port` registry directly; controller port callbacks own device-local state changes. | Immediate callback and publication within instruction execution; instruction cost has only ordinary IN/OUT surcharge and does not cover every port/string path. | Provider failures use instruction fault paths; registry is reset but topology is frozen through session construction. | S2 common transaction candidate; retain port-provider atomicity and device-local commit. S3 consumes only selected DMA/controller competition. |
| Stopped/paused external memory and port APIs | `memory_interface.c` and `port_interface.c` validate lifecycle, execute direct physical/port operations, then record API traces. `core_machine_bus_*` is a configuration/debug API, not an executing bus. | Intentionally immediate outside CPU execution. | Lifecycle gate rejects running access; port write restores the prior dword on provider failure. | Immediate retained proof. S2 must not route debugger/configuration APIs through guest-time arbitration. |
| 8237A DMA ordinary and M2M transfers | `dma.c` preflights checked memory routes, then reads/writes physical RAM directly; `dma_complete_transfer` owns terminal publication. `machine.c:core_machine_arbitration_tick` grants advances before PIT/PIC. | One selected DMA advance per converted arbitration tick; no CPU ownership or transaction reservation. | Reset clears transient transfer state and retains bindings; T348 proves preflight/no-publication, M2M and EOP lifecycle. | S3 owns DMA versus CPU/service ownership and deterministic competing-request order. S2 supplies any shared availability/commit primitive proven necessary. Physical DREQ/DACK waveform transfers. |
| PIC request, refresh and CPU acknowledgement | Device owners assert sources; `core_machine_pic_refresh` follows DMA/PIT at arbitration tick; CPU delivery scans/peeks/acknowledges in the executor. | A readiness-produced source becomes visible to PIC on the next arbitration tick. No modeled INTA bus transaction. | PIC/device reset and source lifecycle are proved by T349--T351. | S3 owns the selected acknowledgement visibility boundary and competing source proof. Physical INTA/spurious waveform remains a TODO transfer. |
| PIT and RTC clocks | Clock domains convert one core tick to device ticks. Arbitration advances PIT before PIC; readiness advances RTC after FDC/HDC, so its IRQ8 is visible next arbitration tick. | Explicit converted clock ticks and equal-due ordering. | Cold reset resets every clock domain; T350 proves retained timer/calendar and IRQ lifecycle. | Existing bounded timing retained. S3 must only consume them when proving competition; no oscillator-phase or exact wait claim. |
| FDC and ATA service | `core_machine_readiness_tick` calls FDC advance/refresh then ATA advance/refresh. Controller-local pending states own command/data service publication. | One service opportunity per readiness due tick after arbitration; readiness products become eligible next arbitration tick. | Reset/media/control/finalize cancel controller-local pending work; T347 has focused lifecycle and DOS consumer evidence. | S3 owns interaction with DMA/CPU availability only. Do not reopen command semantics or invent seek/rotation durations. |
| KBC/AUX and VADP | `core_machine_peripheral_tick` advances converted KBC then VADP after readiness. KBC/VADP own FIFO/raster state and copied host boundaries. | Explicit later equal-due callback order. | Cold reset resets both; T351/T352 prove lifecycle and copied boundaries. | Immediate/deterministic-event retained. S4 must ensure final trace/reset ledger covers them; no host cadence or analog timing. |
| Firmware, VM session and profile composition | VM session reset reaches `core_machine_cold_reset`, which resets all selected owners, timeline and clock domains, schedules the three callbacks at tick 1, then invokes execution-provider and firmware reset. Profile composition validates frozen selected routes before publication. | No host time source; reset starts timeline at zero with three deterministic callbacks. | T353 S4 proves reset vector, route topology and rearm. | S4 must prove any new transaction state cancels before provider/firmware reset and cannot leak across session reset. |
| Trace and consumers | Machine records retirement, API memory/port, DMA/PIT/PIC, FDC/HDC/RTC, KBC/VADP and reset events. There is no bus owner/transaction identifier or availability/commit trace. | Event order is observable; transaction ownership is not. | Copied display/input and firmware consumers are retained. | S4 must make admitted S2/S3 transaction state diagnosable without exposing mutable internal state. |

## Mechanism And Caller Sweep

The common repeated construction is narrow and real: CPU instruction helpers
and DMA each call checked physical memory directly; CPU port helpers call the
port registry directly; controller service is only subsequently observed by
the three timeline callbacks. The existing `core_machine_bus_*` name must not
be mistaken for an execution-time arbiter, because it is legal only while the
machine is stopped or paused and owns no state.

The only production scheduling sites are the three `machine.c` recurring
callbacks. `timeline.c` provides deterministic `(due_tick, sequence)` ordering,
bounded capacity, cancellation, and reset, but neither CPU nor DMA creates a
per-transaction event. Thus S2 must either prove selected immediate access is
sufficient or introduce one shared, bounded availability-to-commit owner for
the exact selected CPU/DMA paths. It may not place independent delays in CPU,
DMA, FDC, ATA, or port providers.

## Allocation And Completion Logic

| Receiver | Required bounded result | Cannot claim |
| --- | --- | --- |
| S2 | A complete source/manual/corpus decision for CPU memory and I/O transaction availability. If a common mechanism is required, it owns validation, request admission, deterministic owner selection, commit/publication, failure cancellation, reset, and trace for every selected CPU/DMA consumer. If immediate behavior is retained, provide direct proof that no selected contract needs an unavailable state. | A generic bus facade, universal cycle counter, guessed wait values, port-provider rewrite, or debugger API timing. |
| S3 | One selected CPU/DMA ownership and controller-service competition contract. Sweep 8237A ordinary/M2M request/grant/EOP, FDC/ATA DRQ/service, and PIC acknowledgement visibility against S2's boundary, with deterministic multi-request ordering and no partial transaction publication. | Physical DREQ/DACK/EOP/INTA waveforms, arbitrary controller duration, or reopening accepted T347 command semantics. |
| S4 | Reset/cancel/finalize/trace and firmware/DOS consumer closure of every admitted transaction state; verify cold and session reset, event order, no leaked reservation, and a truthful selected-L3 closure transfer table. | Windows installation success, host-time behavior, analog/pin fidelity, or unselected peripheral timing. |
| TODO transfer | Exact DRAM refresh/electrical contention and wait tables, prefetch/pipeline/bus phase, physical INTA waveform, serial/parallel/game/PPI/speaker timing, and any device duration lacking a selected contract. | A blanket claim that these are implemented by deterministic tick ordering. |

## S1 Exit Reconciliation

Every selected source has one current owner and one receiver. The only S1
implementation finding is the repeated direct execution-path construction,
which is deliberately assigned to S2 rather than repaired in isolated CPU or
DMA helpers. Existing timeline/device ordering is retained as evidence, not
promoted to cycle-exact behavior. This is the finite basis for the rest of
T354 and prevents a per-device wait-counter campaign.
