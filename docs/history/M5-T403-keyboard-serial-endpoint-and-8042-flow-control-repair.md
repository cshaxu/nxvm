# M5 T403: Keyboard Serial Endpoint And 8042 Flow-Control Repair

## Task Record

T403 accepts the queue's first candidate under the owner-authorized DeskPro
hardware-gap program. It repairs a shared Core keyboard/8042 flow-control
mechanism whose one CPU-visible FIFO can discard a complete native break while
congested, leaving typematic active. The repair is shared Core behavior used by
the DeskPro composition; it is functional input correctness, not a physical
DeskPro timing or L3 result.
## S1 Acceptance

P1 `d3d68564` implements the admitted shared-Core repair. Actual review confirms
that native keyboard bytes, their break/typematic state and endpoint capacity
remain KBC-private; existing CPU output, 60h/64h, IRQ, response and AUX owners
remain singular. The focused KBC/AUX/mapper/DOS regressions and full 285/285
current gate pass; no physical DeskPro timing or L3 claim is made.
