# T351 S2 8042 Controller Mechanics

## Contract and repaired mechanism

The IBM PC/AT Technical Reference states that reset and controller self test
inhibit the keyboard interface, then place `55h` in the output buffer on a
successful self test. The selected controller implementation previously
reported `55h` and set SYS, but retained the keyboard interface enabled. This
left a scan-code and IRQ1 publication path open after `AAh`, contradicting the
controller lifecycle described by IBM.

`core_machine_kbc_write_command` now makes the self-test transition atomically
under the existing controller owner: it disables the keyboard interface, sets
the command-byte disable-keyboard bit and SYS, refreshes the current FIFO-head
IRQ source, then schedules controller-origin `55h`. `AEh` remains the explicit
recovery transition. No new FIFO, public interface, device protocol, or
timeline owner was added.

Primary sources:

- [IBM PC/AT Technical Reference, March 1984](https://ftp3.us.freebsd.org/pub/misc/bitsavers/pdf/ibm/pc/at/1502494_PC_AT_Technical_Reference_Mar84.pdf), keyboard-controller initialization and `AAh` self-test behavior.
- [IBM PC/AT Technical Reference, September 1985](https://www.bitsavers.org/pdf/ibm/pc/at/6139362_PC_AT_Technical_Reference_Sep85.pdf), initialization sequence: self test inhibits the keyboard interface before successful `55h` completion.
- [Intel UPI-41/42 User's Manual](https://www.ceibo.com/eng/datasheets/Intel-8041-Manual.pdf), 8042 family status and host-interface context.

## Selected controller matrix

| Route | Owner and observable result | Proof/disposition |
| --- | --- | --- |
| `20h` / `60h` | one command-byte state gates keyboard/AUX eligibility | retained controller smoke; S2 preserves direct selected command-byte behavior. |
| `AAh` then `AEh` | self test sets SYS, sets the keyboard-disable command bit, deasserts IRQ1 eligibility, queues controller `55h`; `AEh` clears the bit and restores accepted keyboard FIFO/IRQ1 publication | strengthened controller smoke. |
| `ABh` / `A9h` | selected keyboard/AUX interface test replies are controller-origin and do not create keyboard/AUX IRQ ownership | retained controller/AUX smokes. |
| `ADh` / `AEh`, `A7h` / `A8h` | command byte and enabled state are updated before FIFO-head source refresh; disabled head deasserts its eligible source | controller and AUX-port smokes. |
| `D0h` / `D1h` | one output-port owner maps A20 and active-low reset through machine callbacks | retained controller smoke and machine reset coverage. |
| `D4h` | only chooses the pending AUX-device write path; it does not create a second FIFO or an IRQ source | retained AUX-port and guest AUX smokes; S4 owns device semantics. |
| FIFO and delay | 64-byte FIFO with per-byte origin; controller replies wait KBC-owned behind full typeahead and are published only when capacity permits | retained controller smoke; no guessed IBF processing delay introduced. |
| reset/finalize | reset clears KBC state, reapplies output-port defaults, and deasserts sources; finalize deasserts both routes | retained lifecycle/timeline tests; S5 will audit combined cancellation. |

Status parity/timeout semantics and controller commands outside the selected
IBM-supported set remain unimplemented rather than guessed. Keyboard command
semantics remain S3; selected AUX device semantics remain S4.

## Similar-issue sweep

The repair class is a controller state transition that changes interface
eligibility without refreshing command-byte and FIFO-head IRQ state. The sweep
reviewed every `kbc.c` controller-command case, command-byte writer,
`core_machine_kbc_refresh_current_irq` caller, FIFO enqueue/dequeue helper,
output-port callback, reset/finalize route, KBC timeline invocation, and the
controller/AUX focused tests. `ADh`/`AEh`, `A7h`/`A8h`, command-byte writes,
reset, dequeue, and finalize already use the same state-and-refresh ownership;
`AAh` was the sole omitted selected controller transition and is repaired.

## Verification

- `core-machine-kbc-controller-smoke` passes with the strengthened `AAh` /
  `AEh` and IRQ1 eligibility proof.
- The retained focused KBC, AUX, host-ingress, mouse, and T346 timeline set
  passes 9/9.
- The full current-gate run is required before S2 acceptance.
