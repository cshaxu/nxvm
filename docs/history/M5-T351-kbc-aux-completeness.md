# M5 T351: PC/AT 8042 And AUX Completeness

## Task Record

T351 consumes T346's deterministic KBC timeline owner and T350's completed
platform-signal graph. It reconciles the selected PC/AT Intel 8042 controller,
keyboard, and AUX protocol state machines before digital-video, port-topology,
and bus-timing candidates proceed.

## Active Subtasks

| Subtask | Purpose |
| --- | --- |
| S1 | Build the complete 8042, keyboard, and AUX manual-to-source-to-proof ledger and allocate every retained or transferred gap to S2--S5. |
| S2 | Reconcile controller command/status/FIFO/output-port/IRQ lifecycle. |
| S3 | Reconcile selected keyboard protocol and IRQ1 lifecycle. |
| S4 | Reconcile selected AUX protocol and IRQ12 lifecycle or record exact transfers. |
| S5 | Audit the combined deterministic controller state graph and close the package. |

## Accepted Progress

S1 is accepted at `a2e7fc47`: it establishes the single 8042-owned
guest-visible FIFO and its keyboard/AUX IRQ lifecycle, distinguishes the IBM
PC/AT controller from the selected PS/2-compatible AUX extension, and assigns
each retained command, response, timing, profile, reset/finalize, and host
boundary row to S2--S5 or the existing advanced-AUX TODO. Its
[ledger](../etc/evidence/t351-s1-kbc-aux-ledger.md) preserves primary-source
provenance without importing any external implementation.

S2 is accepted at `386aba69`: it repairs the selected IBM controller self-test
transition so `AAh` inhibits the keyboard interface, refreshes stale FIFO-head
IRQ eligibility, and returns controller-origin `55h`; `AEh` restores eligible
keyboard FIFO and IRQ1 publication. Its
[controller evidence](../etc/evidence/t351-s2-kbc-controller.md) retains one
FIFO/output-port/reset owner and leaves keyboard/AUX device protocol to S3/S4.

S3 is accepted at `a9d8bd01`: it gives the selected AT keyboard private
response history at the one FIFO enqueue boundary, so controller/AUX output
cannot corrupt `FEh` RESEND. It also reconciles selected default, enable/disable,
typematic, and NOP command state without a second queue or host boundary. Its
[keyboard evidence](../etc/evidence/t351-s3-keyboard-device.md) leaves all AUX
device behavior to S4.

S4 is accepted at `da411b9b`: it proves and reconciles the selected compatible
three-byte AUX command/report path through its one FIFO/IRQ12 owner, including
delay, reset/finalize, copied guest ingress, and full-FIFO atomicity. Its
[AUX evidence](../etc/evidence/t351-s4-aux-device.md) transfers wheel, scaling,
remote/read-data, resend/error, and capture behavior to the bounded advanced
AUX TODO rather than calling them PC/AT behavior.
