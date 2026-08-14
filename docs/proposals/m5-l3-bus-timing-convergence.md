# M5 Physical L3 Bus-Timing Convergence

## Objective

Converge the selected PC/AT machine from deterministic device-event ordering to
an evidence-backed L3 transaction model: CPU memory and I/O availability,
DMA bus ownership, controller-service competition, and interrupt
acknowledgement visibility must each have one owner, one timing/publication
boundary, reset semantics, and focused proof. The work consumes T346's
equal-tick order and T347--T353's explicit device/topology contracts.

It must distinguish three things that are often conflated: deterministic order
already present in the core; selected, architecturally observable transaction
latency and arbitration; and unsupported cycle/pin/analog fidelity. It may
only introduce a wait/arbitration rule from a primary hardware contract or a
reproducible selected corpus requirement.

## Completion Standard

Closure requires a source/manual-to-core-to-proof ledger for every selected
CPU memory/I/O transaction and device requester. Each row is either:

1. proven to remain immediate under the selected PC/AT contract;
2. reconciled to one bounded wait/arbitration mechanism with deterministic
   reset/cancellation and no partial transaction publication; or
3. transferred to one explicit later receiver with an admission condition.

The result is selected L3 bus timing, not a false cycle-exact PC claim. It
does not make unselected peripheral timing, prefetch/bus-pipeline, analog,
pin-level, or host-clock behavior a default contract.

## Proposed Subtask Sequence

| Subtask | Bounded outcome |
| --- | --- |
| S1 | Build the complete selected PC/AT transaction and timing ledger: CPU fetch/data/memory/I/O, port dispatch, A20/ROM, DMA request/grant/EOP, PIC INTA/spurious boundary, PIT/RTC/KBC/VADP/FDC/ATA service readiness, and every existing timeline callback. Classify immediate, deterministic-event, physically unspecified, or a primary-contract/corpus-backed timing requirement; allocate no implementation by convenience. |
| S2 | Reconcile the common selected transaction availability/commit mechanism only if S1 proves a repeated construction defect or an authoritative wait/arbitration requirement. It must cover validation, enqueue/deny, visibility, cancellation, reset, tracing, CPU/DMA interaction, and all writer/reader consumers rather than add per-device sleep counters. |
| S3 | Reconcile the selected DMA/CPU ownership and controller-service competition admitted by S1: 8237 request/grant/EOP, memory/I/O transaction visibility, FDC/ATA readiness, and 8259A INTA acknowledgement boundary. Each timing rule needs a device/manual or corpus proof and deterministic multi-request ordering. |
| S4 | Close selected L3 timing through reset, cancellation, trace, firmware/DOS consumer proof, and a transaction ledger audit. Transfer unselected devices, exact wait-state tables lacking a contract, prefetch/pipeline, pin/analog, and host-time behavior precisely. |

S1 may not implement a timing symptom. S2 may not become a generic clock/bus
framework unless the ledger establishes one common owner and all selected
callers fit its validation-to-publication contract. S3 may not reopen device
command semantics already closed by T347--T353 unless an actual timing defect
reproduces at the shared owner.

## Rules And Mechanism Discipline

Core owns guest time, timeline execution, port and memory dispatch, DMA/PIC
state, and transaction visibility. The VM profile only declares selected
topology and timing capabilities; it never supplies host time or a second bus
truth. A timing rule has explicit units, start/end/commit conditions, pending
state, cancellation/reset behavior, trace observability, and overflow/error
handling. Equivalent CPU and DMA variants share the same availability and
commit owner; distinct paths require an Intel/IBM layout, bus-cycle, or device
semantic difference.

Every repair sweeps transaction producers, dispatch consumers, timing
callbacks, lifecycle/reset/finalize paths, IRQ/DMA interactions, trace
records, and focused tests. A failed or cancelled transaction leaves no
partial memory, port, register, IRQ, DMA, or host publication unless the
named hardware contract explicitly requires an earlier committed phase.

## Non-goals And Transfers

Do not add a universal cycle counter, host wall-clock source, device-thread
sleep, generic port-bus facade, arbitrary wait-state table, serial/parallel/
game/PPI/speaker timing, PCI/ISA PnP, cache/prefetch/pipeline simulation,
pin-level 8237/8259A/ATA/FDC waveforms, analog display/audio, or Windows
success claim. Physical INTA waveform, DRAM refresh electrical contention,
and controller delays without a selected contract remain transfers. Windows
readiness stays a later map consumer, not a timing oracle.
