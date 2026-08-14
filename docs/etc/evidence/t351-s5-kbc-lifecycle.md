# T351 S5: Combined 8042, Keyboard, And AUX Lifecycle Closure

## Scope

This final task-level check composes the accepted controller, keyboard, and
selected compatible AUX state machines.  It does not add a second FIFO or
expand the selected three-byte AUX protocol.

## Combined Owner Proof

`core_machine_kbc_mixed_fifo_lifecycle` in
`tests/machine/core_machine_kbc_controller_smoke.c` establishes one local
controller/PIC/port fixture and proves the following ordered sequence:

1. AUX enable acknowledgement is read from the one FIFO through IRQ12.
2. A keyboard scan byte, a complete AUX report, and a controller command
   response are queued in that order.  The keyboard head publishes IRQ1; the
   AUX head then publishes IRQ12; the two remaining AUX bytes retain AUX-head
   ownership; the controller response is finally read without a KBC IRQ
   source.
3. A delayed controller response remains behind a queued keyboard scan byte,
   even after its delay expires.
4. Reset clears queued AUX output and deasserts both KBC-owned IRQ sources;
   a new keyboard byte can subsequently publish IRQ1, and finalize releases
   both KBC-owned sources.

The assertion deliberately observes `t_kbc` source assertion state rather
than treating a previously latched PIC edge request as a KBC source.  PIC
delivery itself remains proved by the accepted S2 and S4 focused vectors.

## Composition And Transfers

The retained `core-machine-input-display-s5-smoke` owns the T346 equal-tick
machine ordering proof; retained input/session and DOS mouse tests own copied
ingress and guest-consumer proof.  S2--S4 retain their command, typematic,
response-history, delayed-report, clipping, and full-FIFO atomicity evidence.

The selected boundary is now closed.  Wheel identification, scaling,
remote/read-data, resend/error protocol breadth, and host capture remain the
existing `TODO(Medium) Advanced 8042 AUX protocol` receiver.  IBM PC/AT port
61/PPI and parity/I/O-channel NMI remain separate platform-signal transfers.

## Verification

The combined controller smoke, retained KBC/AUX/keyboard/mouse/timeline
focused owners, documentation governance, current-artifact verifier, and the
full current gate are required at T351 closure.  The T351 developer artifact
identity and SHA-256 are recorded in `CURRENT.md` and the T351 history record.
