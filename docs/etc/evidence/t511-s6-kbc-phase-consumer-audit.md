# T511 S6 KBC Phase-Consumer Audit

`M5:T511:S6:KBC-PHASE-CONSUMER:NO-HIT`

## Sources

Intel *UPI-41A/41AH/42/42AH User's Manual*, order 231318-006 (October 1993),
owner-managed copy
`assets/manuals/controllers/intel/231318-006_UPI-41A_42AH_Users_Manual_Oct1993.pdf`,
defines the host input/output-buffer separation and OBF clear-on-read.
IBM AT material fixes the 60h/64h, IRQ1, A20 and reset routes.  86Box and
Bochs corroborate separate queue/IRQ ownership as Other-L2 references only;
no external code was copied.

## Owner/Consumer Matrix

| Boundary | Sole production path | Disposition |
| --- | --- | --- |
| Command response | `kbc.c` keeps delayed response bytes owner-local until its deadline and FIFO capacity allow publication. | A response is not visible before its own deadline. |
| Keyboard serial bytes | The KBC serial queue promotes one byte at a time; delivery delay belongs to that queue. | Each promoted FIFO head is a fresh OBF/IRQ event. |
| 60h read/IRQ acknowledgement | Dequeue removes only the current head, deasserts that origin's IRQ, refreshes the new head, then resumes serial drain. | The next byte cannot reuse the prior IRQ acknowledgement. |
| Typematic | The KBC owns initial/repeat countdown and publishes each repeated native byte through the same serial/FIFO path. | No consumer-side synthetic repeats or stale OBF reuse. |
| IRQ1/IRQ12 origin | The FIFO head origin selects one bound PIC source; command-byte changes refresh it from the same FIFO state. | No profile or VM mirror owns IRQ state. |
| Output port | KBC's output-port operation alone updates A20/reset via the existing Core callback. | It is synchronous state application, not a multi-phase ready protocol. |

## Executed Proof And Conclusion

The focused Debug cohort passed: `core-machine-kbc-controller-smoke`,
`core-machine-kbc-serial-cadence-smoke`, `core-machine-kbc-aux-port-smoke`,
`vm-kbc-aux-guest-smoke`, `vm-keyboard-host-ingress-smoke` and
`vm-keyboard-set1-mapper-smoke`.  It covers delayed command output, repeated
serial promotion, 60h acknowledgement/fresh IRQ, typematic, keyboard/AUX
origin selection and host ingress.

No ATA-style stale observation exists.  The only production path already
withdraws the current IRQ before promoting a successor, so no code change is
warranted.
