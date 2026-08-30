# T511 S7 XT Keyboard/PPI Phase-Consumer Audit

`M5:T511:S7:XT-KEYBOARD-PPI-PHASE-CONSUMER:NO-HIT`

## Sources

IBM *Personal Computer XT Technical Reference* (April 1983), owner-managed
copy `assets/manuals/controllers/ibm/IBM_5160_Technical_Reference_APR83.pdf`,
defines the separate 8255/PPI ports, PB6 clock hold, PB7 keyboard clear, PA
data, IRQ1, keyboard reset/BAT and serial delivery relations.  Its ranges are
retained as Manual-L3 inputs; Core's sourced macro-axis conversion is L2, not
a claimed electrical clock.  86Box's separate `kbc_xt.c`, `keyboard_xt.c` and
`ppi.c` corroborate the topology and one-byte handoff as Other-L2 only; no
external code was copied.

## Owner/Consumer Matrix

| Boundary | Sole production path | Disposition |
| --- | --- | --- |
| Clock release and reset/BAT | `xt_keyboard.c` owns clock-low accumulation, BAT expiry and the following serial first-edge deadline. | BAT expiry creates a new pending serial phase; it does not consume the first edge. |
| Serial frame to PPI | The XT keyboard owns one FIFO head and releases it to `xt_ppi_keyboard.c` only after the frame completes. | A later head cannot reuse the preceding delivery. |
| PA read and PB7 clear | The PPI exposes the current PA latch; PB7 clear is the one acknowledgement that withdraws IRQ1 and releases the byte. | Repeated PA reads do not create a second consumption rule; a fresh byte needs a fresh PB7 release and serial completion. |
| IRQ1 | The PPI owns the one PIC source and deasserts it with the acknowledged byte. | Successor delivery publishes a fresh IRQ1, never a stale asserted source. |
| Host ingress and reset | VM ingress only queues native bytes through Core; Core reset cancels PPI/keyboard publication in the owning states. | No VM keyboard, PPI or IRQ mirror exists. |

## Executed Proof And Conclusion

The focused Debug cohort passed: `core-machine-xt-ppi-keyboard-smoke`,
`vm-keyboard-host-ingress-smoke` and `vm-xt-5160-268-profile-smoke`.  It covers
PPI PA/PB acknowledgement, IRQ1 withdrawal/republication, BAT-to-first-edge,
serial FIFO handoff, native host ingress, and the distinct XT topology.

No ATA-style stale observation exists.  PB7 is the only PPI acknowledgement;
the existing keyboard/PPI pair already makes every successor traverse a new
serial and IRQ1 publication path, so no code change is warranted.
