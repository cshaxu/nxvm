# M5 PC/AT Port Topology And Selected Peripheral Completion Proposal

## Objective

Reconcile the default PC/AT profile's declared port ranges, actual core
registrations, IRQ/DMA routes, reset/finalize ownership, and firmware-visible
topology before the L3 bus-timing package. The result is one auditable
machine-port graph: every declared range has one composition receiver and
every registered device range has a profile/topology reason, a lifecycle
owner, and focused proof or an exact transfer.

This package does not equate a numerically unused port with a missing device.
It distinguishes selected declared PC/AT devices already present in the
profile, internal device ports requiring a declaration or bounded correction,
and intentionally absent peripherals requiring a corpus and hardware contract
before admission.

## Dependency And Completion Standard

T353 consumes T346's deterministic due-event ordering, T347--T351 selected
controller closures, and T352's completed video state graph. It precedes L3
bus-timing convergence: arbitration cannot become physically more precise
while selected port and route topology is ambiguous.

Completion requires a PC/AT source/manual-to-profile-to-core-to-proof ledger
for every selected port family. Every row is reconciled to one owner and
lifecycle boundary, repaired as a topology defect, or transferred to one
Queue/TODO receiver with an admission condition. It does not claim that all
optional PC/AT cards are implemented.

## Proposed Subtask Sequence

| Subtask | Bounded outcome |
| --- | --- |
| S1 | Build the complete selected PC/AT port/IRQ/DMA/reset ledger from the profile descriptor, composition wiring, actual registration table, controller evidence, and IBM PC/AT references; classify each declared and registered row. |
| S2 | Reconcile declared-profile versus actual registration/IRQ/DMA/reset topology for selected PIC, PIT, DMA, KBC, VADP, CMOS, FDC, and ATA/HDC paths; repair a proven topology or transaction defect only after the full caller/consumer sweep. |
| S3 | Reconcile selected system-control/PPI/speaker or other highest-ROI missing peripheral only if S1 establishes a corpus-backed hardware contract; otherwise record an exact no-owner transfer without generic host passthrough. |
| S4 | Compose the selected port graph through reset, timeline, firmware-visible paths, and artifact closure; transfer serial, parallel, game, advanced system-control, and unselected card breadth precisely. |

S1 may consolidate S2/S3 only if it demonstrates one shared owner, validation,
publication, and lifecycle boundary. It must not add a broad port-bus wrapper
or manufacture an emulator-wide peripheral framework.

## Rules And Mechanism Discipline

Core owns port dispatch, controller state, guest time, and IRQ/DMA requests.
The default PC/AT profile declares composition topology; VM composition binds
only those declared capabilities. A device's command state, pending event,
IRQ/DMA source, reset, and finalization each retain one owner. Equivalent
variants share validation and publication; a separate path requires a real
hardware, lifetime, or profile difference.

Platform adapters may report bounded input/output through composition but may
not create host passthrough, select implicit guest state, or mutate a device
outside its owner. A repair must audit writers, readers, registration-failure,
reset/finalize, IRQ/DMA consumers, and focused owners.

## Non-goals And Transfers

Do not add generic host I/O passthrough, serial, parallel, game-port, MIDI,
network, SCSI, PS/2 expansion, PCI, ISA PnP, VGA/VBE, or audio implementation
merely because a historical machine may expose those addresses. Speaker/PPI
or system-control work needs a named DOS/Windows corpus and documented
8254/port contract. Physical bus wait states, INTA waveforms, DMA/refresh
contention, and controller service durations belong to the later L3 bus-timing
candidate. Windows execution remains the readiness-map consumer.
