# T366 S24: 80286 BOUND Timing

Intel's [80286/80287 Programmer's Reference Manual, Appendix B](https://bitsavers.org/components/intel/80286/210498-005_80286_and_80287_Programmers_Reference_Manual_1987.pdf)
lists successful `62 /r BOUND rw,md` at thirteen clocks. The row has no
effective-address, odd-word, real/protected-mode or selector-state addition.
S24 therefore publishes that fixed cost only after successful, unprefixed
memory-form 80286 `BOUND r16,m16&m16` retirement.

The profile-local classifier remains below every higher source owner and above
only the visible unallocated receiver. The existing handler continues to own
the 80186 profile gate, memory-only ModR/M requirement, signed lower/upper
comparison and `#BR` production. Register-form `#UD`, out-of-range `#BR`,
operand/segment faults, prefixes, delivery, bus/device service and physical
time retain their existing receivers.

The focused ledger smoke proves in-range direct and indexed forms in real and
protected mode. It checks the fixed tick publication plus unchanged index and
FLAGS state; protected-bootstrap time is outside each measured case. The
existing BOUND semantic smoke and real/protected `#UD` delivery smokes retain
the bounds, access-fault and register-form boundaries.

The focused targets emitted `M5:T357:S6:80286-INSTRUCTION-TIMING-LEDGER:OK`,
`M5:T316:S54:BOUND:OK`, `M5:T337:S1:REAL-UD-DELIVERY:OK` and
`M5:T326:S1:PROTECTED-UD-DELIVERY:OK`. The configured 228-target
`run-current-smokes` gate, documentation governance and `git diff --check`
passed before P1.
