# T366 S23: 80286 Segment-Register Stack Timing

Intel's [80286/80287 Programmer's Reference Manual, Appendix B](https://bitsavers.org/components/intel/80286/210498-005_80286_and_80287_Programmers_Reference_Manual_1987.pdf)
lists `PUSH ES/CS/SS/DS` at three clocks and `POP ES/SS/DS` at five clocks.
The selected rows carry no effective-address, odd-word, real/protected or
selector-state addition. S23 therefore publishes those fixed costs only after
successful unprefixed legacy segment-stack retirement.

The 80286 profile-local classifier remains below every higher source owner and
above only the visible unallocated receiver. Existing instruction handlers and
the legacy segment-stack smoke retain stack/selector validation, fault
atomicity and the POP-SS interrupt shadow. Prefixes, failed selector or stack
accesses, delivery, bus/device service and physical time remain transferred.

The timing-ledger smoke proves all four pushes and all three pops in real and
protected mode. It checks each pushed stack image or loaded selector, stack
pointer result and elapsed-provider publication. Protected bootstrap time is
outside every measured case.

The focused timing and legacy semantic targets emitted
`M5:T357:S6:80286-INSTRUCTION-TIMING-LEDGER:OK` and
`M5:T316:S46:LEGACY-SREG-STACK:OK`. The configured `run-current-smokes` gate,
documentation governance and `git diff --check` passed before P1.
