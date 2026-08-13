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
