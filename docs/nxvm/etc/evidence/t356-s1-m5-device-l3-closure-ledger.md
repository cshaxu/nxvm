# T356 S1: M5 device and L3 closure ledger

## Method and classification

This is a source-to-owner-to-proof audit, not an emulator feature matrix.  It
inspected every file under `src/core/machine/`, VM machine/composition route,
CMake current/media partition, T346--T355 histories and indexed evidence, and
every hardware/timing TODO.  The static queries were:

```text
rg --files src/core/machine src/vm/machine src/vm/composition tests/core tests/machine
rg "core_machine_(timeline|transaction|pic|dma|pit|rtc|kbc|fdc|hdc|vadp)_..." src/core/machine
rg "PROJECT_CURRENT_(SMOKE|FDD|HDD|MEDIA)|PROJECT_T344_CURRENT" CMakeLists.txt
ctest --test-dir build/mingw-gcc-x64 -N -L current-gate
rg "TODO\((High|Medium|Low)\)" docs/states/TODO.md
```

`Accepted` means a bounded source owner, lifecycle and focused/current-gate
proof exist. `Deferred` means the open boundary is named below with an
admission condition; it is not implicitly implemented. `Unproved` would mean
an owner has no evidence. `Contradictory` would mean source/current authority
conflict. No unproved or contradictory selected owner remains after T355's
FDC/ATA baseline correction.

## Current owner ledger

| Surface | Owner and visible route | Evidence classification | Physical/L3 boundary and receiver |
| --- | --- | --- | --- |
| CPU executor, checked memory and CPU ports | `cpu.c`, `memory.c`, `port.c`; machine binds the executor and transaction state. | Accepted for 8086--80386DX architecture closure and selected atomic CPU memory/port transactions. | Instruction costs, bus cycles, prefetch and wait states are deferred to the timing receivers below. |
| Coprocessor boundary | `fpu.c` and the optional FPU provider route. | Explicitly excluded from the owner request; only the retained no-FPU/optional-8087 boundary is covered. | Broad 80287/80387 or x87 execution remains the existing x87 TODO and cannot be inferred from this device/L3 audit. |
| Transaction, timeline, trace and reset | `transaction.c`, `timeline.c`, `machine.c`; reset cancels transaction then resets/re-arms three due-tick callbacks. | Accepted selected L3 lifecycle: begin/commit/cancel, `(due_tick, sequence)`, reset trace continuity. | One transaction cannot represent physical bus occupancy, contention or durations; receiver: bus-timed PC/AT / cycle-exact TODO. |
| Dual 8237A DMA | `dma.c`, frozen FDC binding in `machine.c`. | Accepted for page/word layout, request/cascade/EOP, M2M, validation-before-publication and selected transaction crossing. | DREQ/DACK/EOP/HOLD/HLDA waveform and broad external-device DMA remain deferred to bus-timed PC/AT. |
| Dual 8259A PIC | `pic.c`, device source bindings, machine arbitration refresh. | Accepted for command/priority/cascade/OCW3/source lifecycle and logical CPU delivery. | Physical INTA/spurious waveform is deferred to bus-timed PC/AT. |
| 8254 PIT | `pit.c`, machine arbitration tick and IRQ0 route. | Accepted retained counter/output/channel-0 IRQ lifecycle. | Per-instruction cycle costs and channel-2/PPI/speaker corpus are deferred. |
| MC146818/CMOS | `rtc.c`, readiness tick and IRQ8 route. | Accepted calendar/event/CMOS/IRQ8 lifecycle. | Parity/I/O-channel NMI source is absent by design; receiver: PC/AT NMI TODO. |
| FDC and FDD | `fdc.c`, `vm/machine/fdd.c`, DMA binding, readiness tick. | Accepted selected pending command/data service, reset/media cancellation and FDC-DMA/DOS-read consumers. | Broad command/error/latency compatibility remains deferred; no hidden controller-duration claim. |
| ATA/HDC and HDD | `hdc.c`, `vm/machine/hdd.c`, readiness tick. | Accepted selected PIO pending service, `nIEN`/IRQ14, reset/media cancellation and DOS/HDD/INT13 consumers. | Extended IDE, sparse/backed media, LBA48 and generic wait/error timing are deferred. |
| 8042, keyboard and AUX | `kbc.c`, profile ingress and copied host input boundary. | Accepted selected controller FIFO, keyboard, compatible three-byte AUX, IRQ1/IRQ12, delayed/reset lifecycle. | Advanced AUX and host capture remain deferred; no DOS mouse API is inferred. |
| Digital video | `vadp.c`, display/presentation interfaces and copied frame consumers. | Accepted selected CGA text/graphics and two direct EGA planar modes, reset and deterministic KBC-after readiness progression. | Remaining CGA/EGA breadth, VGA/VBE, composite and renderer timing are deferred. |
| Port topology/profile/firmware | `port.c`, `machine.c`, VM profile firmware/session composition. | Accepted default PC/AT selected directional leaves, ROM mapping, frozen topology and reset proof. | PPI/speaker, serial, parallel, game and other unselected interfaces have no fabricated owner. |
| Host/provider and product composition | VM session/media/display/input composition and copied interfaces. | Accepted one bootable NXVM product/session route and host-boundary regression consumers. | Mantle/DOS/NXVDM and non-Win32 runtime are separate product admissions. |

## Deterministic L3 versus physical timing

`machine.c` schedules three repeating same-due callbacks: arbitration performs
`DMA -> PIT -> PIC`; readiness performs `FDC -> HDC -> RTC`; peripheral work
performs `KBC -> VADP`.  The timeline orders by due tick then insertion
sequence.  T354 proves CPU commit/retire precedes these selected callbacks and
that reset cancels active selected transactions.  Current-gate discovery lists
234 tests, including dedicated timeline, transaction, competition, lifecycle,
controller and DOS consumer coverage.

This is L3 **event-and-bus ordering**, not full physical L3.  `clock.c` only
converts elapsed executor ticks through configured ratios; it has no
instruction-form cost table, wait-state owner, bus reservation, prefetch queue,
pin phase, DRAM refresh or electrical contention model.  The source and T354
evidence agree on that transfer, so it is deferred rather than contradictory.

## Finite receiver ledger

| Priority | Receiver | Exact admitted prerequisite |
| --- | --- | --- |
| High | Bus-timed PC/AT operation | Primary hardware/corpus contract for memory/I/O wait states, CPU/DMA ownership and named device latency; must extend the transaction owner rather than add a parallel scheduler. |
| High | Cycle-exact profile | A selected profile requires clock phases, prefetch/bus behavior and device microstates; cannot be inferred from current L3 order. |
| Medium | Instruction-timed execution | Profile-specific instruction/prefix/branch/memory/I/O cost model with a validation-to-publication contract. |
| Medium | PC/AT NMI source ownership | Named parity or I/O-channel source with latch/clear/mask/reset/timeline and real/protected/VM86 delivery proof. |
| Medium | PPI/speaker `61h` | Corpus-selected 8253 channel-2/gate/speaker and copied host-audio contract. |
| Medium | Advanced AUX and digital video breadth | Separate protocol/mode matrices: AUX wheel/remote semantics; complete digital CGA, broader EGA/VGA/VBE. |
| High / Medium | HDD backing and unselected storage breadth | Opaque sparse/paged/WASM backing, LBA48 and selected error/timing contract; no synchronous host-cache mutation. |
| Low | Serial, parallel and game interfaces | One named corpus plus controller/port/IRQ/DMA/reset/timeline owner per interface. |
| Product | Mantle/DOS/NXVDM and Windows checkpoints | Existing M6 Queue program and separately admitted BYOB corpus checkpoints; T355 cannot turn a stable INT13 read into compatibility. |

All entries map to existing `TODO.md` debt except the ordered receiver
proposals that S2 must create for the three timing layers.  The audit found no
justification to erase those physical boundaries, no basis to run more Windows
probes, and no evidence that an unselected device should be synthesized.
