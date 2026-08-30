# T503 S9 KBC And XT Keyboard Firmware-Route Audit

`T503-S9-KBC-XT-FIRMWARE-ROUTE:OK`

## Method

This S re-read the selected primary sources and inspected their rendered pages,
not OCR alone: IBM *Personal Computer AT Technical Reference* (March 1984),
system-board pp. 1-38--1-42; IBM *Personal Computer XT Technical Reference*
(April 1983), system-unit p. 1-5 and keyboard pp. 1-65--1-67; and Intel
*UPI-41A/41AH/42/42AH User's Manual* (October 1993), reset and host-interface
description.  The two IBM manuals define the selected board routes; Intel
establishes the 8042 as a resettable intelligent peripheral, not a substitute
for IBM's board wiring.

Read-only local comparisons covered 86Box's distinct `kbc_xt.c`/`keyboard_xt.c`
and its separate AT keyboard implementation, PCjs `chipset.js`, and Bochs
`iodev/keyboard.cc`.  MAME and QEMU are not present in `O:\repos.external`.
They corroborate topology and common command handling only; no external code
or unsupported behaviour was imported.  In particular, PCjs documents an
immediate-response BIOS accommodation, so it is not timing authority.

## Complete Selected Route Ledger

| Form | Primary fact and level | NXVM sole route and result |
| --- | --- | --- |
| AT 8042 ownership and ports | IBM 5170 p. 1-38 identifies 8042, `60h` output/data and `64h` status/command; OBF clears on `60h` read and IBF describes controller consumption. Manual L3. | `kbc.c` alone owns ports, command/data distinction, FIFO and OBF/IBF. `machine.c` installs it only on non-XT topology. Existing controller and serial-cadence tests cover command/reply and read acknowledgement. |
| AT IRQ, inhibit and output buffer | IBM 5170 pp. 1-39--1-40 defines status, inhibit and output-buffer behaviour. Manual L3. | The current FIFO head is the only output state; it asserts the appropriate existing PIC source and deasserts/reselects on read. No VM or firmware byte buffer exists. |
| AT command byte and keyboard enable | IBM 5170 p. 1-42 defines `ADh`/`AEh`, `C0h`, `D0h`, `D1h` and the following `60h` data write. Manual L3. | `kbc.c` owns pending-write state and command-byte enable/IRQ/translation bits; `machine_scheduler.c` alone advances its queued response/serial/typematic deadlines. 86Box, PCjs and Bochs use the same 60h/64h distinction; their implementation choices do not add a route. |
| AT A20 and CPU reset | IBM 5170 p. 1-42 states output-port bit 0 is System Reset and must not be written low; the rendered 8042 block diagram wires reset and Gate A20. Manual L3 causal route. | `core_machine_kbc_apply_output_port` is the sole D1/Fx receiver: it publishes its one output value, changes RAM A20, and requests CPU reset when reset is low. No profile or VM side path owns either signal. PCjs and Bochs independently implement this same D1 ownership. |
| XT topology | IBM 5160 p. 1-5 assigns keyboard scan codes to IRQ1 and describes a serial keyboard interface; its p. 1-67 circuit shows PPI/port-B to keyboard clock/data and IRQ1. Manual L3. | `CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI` installs only `xt_ppi_keyboard` at `60h`--`63h`; `64h` is unsupported and no KBC/A20/reset/AUX state is created. 86Box keeps a distinct XT device, corroborating the separation. |
| XT PPI clear and IRQ1 | IBM 5160 p. 1-67 supplies the PPI/flip-flop wiring; the XT BIOS contract recorded in T484/T496 reads port A and clears the latched byte through PPI-B. Manual L3 causal order. | `xt_ppi_keyboard` exclusively owns the PPI latch and one IRQ1 source. Reading `60h` observes; PB7 clear releases/deasserts and calls the keyboard's one continuation callback. This is not a second FIFO or polling path. |
| XT reset, BAT and serial delays | IBM 5155/5160 March 1986 pp. 4-25--4-26 and 4-32--4-33: clock-low reset >=12.5 ms, BAT result after 300--500 ms, RTS/CTS and 60--120 us first edge / 25--50 us active phase. Manual L3 input facts; a selected deterministic range endpoint remains macro L2 integration, not physical pacing. | `xt_keyboard` owns the keyboard FIFO, reset/BAT and serial state and reports its earliest event to the sole Core scheduler. The repair consumes BAT time before serial time: an exact BAT-deadline advance now leaves the first 60-us edge pending instead of consuming it immediately. The regression proves that state and eventual `AA`/IRQ1 path. |
| XT NMI separation | IBM 5160 p. 1-5 reserves NMI for memory parity; keyboard remains IRQ1. Manual L3 route separation. | `xt_ppi_keyboard` owns parity/I/O-check latch masking and invokes the existing Core NMI request only for those inputs; keyboard delivery cannot produce NMI. Existing smoke covers both sides. |
| HLT/deadline and host ingress | The manuals specify the device events, not host wall-clock scheduling. Manual L3 device inputs / L2 macro conversion as above. | `machine_scheduler.c` asks only the active keyboard owner for its next event; Core advances all device state. `core_machine_keyboard_receive_native_bytes` is the sole typed ingress. VM has no guest-visible keyboard state or deadline producer. |

## Repair, Sweep And Boundary

Before this S, `core_machine_xt_keyboard_advance` completed BAT but reused the
same elapsed ticks to consume the first serial edge.  A deadline advance ending
exactly at BAT completion could therefore publish the next frame too early.
The owner-local loop now consumes each elapsed portion once, then starts the
next phase with its untouched delay.  No new state, clock, callback class,
profile exception, or firmware workaround was added.

The audit also swept: KBC self-test/disable/enable, command reply, translated
input, AUX absence, D1/Fx reset, A20, IRQ1/IRQ12; XT `60h`--`63h`, PB6/PB7,
PPI mode/latch, parity NMI, serial FIFO overflow, reset/BAT, PIC IRQ1, HLT
deadline and reset across XT and AT topologies.  The only demonstrable route
gap was the BAT-to-first-edge consumption fixed here.  Exact wall-clock pacing
and physical keyboard electrical modelling remain outside this controller
route and retain their existing time-axis receivers; they are not hidden as a
firmware or VM fallback.

## Proof And Simplicity Accounting

Focused KBC/XT owner tests pass 6/6.  The repository-only unit suite passes
312/312.  The changed tracked production/test paths are `+38/-28` lines
(net +10): the retained `xt_keyboard` owner replaces two independent phase
updates with one remaining-tick loop, and the single regression observes the
formerly invisible deadline.  The sole production state owners remain `kbc`,
`xt_ppi_keyboard`, and `xt_keyboard` according to topology.
