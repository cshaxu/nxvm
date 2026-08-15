# T366 S22: 80286 LES/LDS Timing

Intel's [80286/80287 Programmer's Reference Manual, Appendix B](https://bitsavers.org/components/intel/80286/210498-005_80286_and_80287_Programmers_Reference_Manual_1987.pdf)
lists both `LES r16,m16` and `LDS r16,m16` at seven clocks.  Unlike the
separate `MOV Sreg,r/m16` rows, these rows state no real/protected split and no
effective-address or odd-word addition.  S22 therefore publishes exactly seven
ticks only after a successful unprefixed memory-form `C4` or `C5` retirement.

The private 80286 classifier remains below the string, dynamic, primary, and
control/stack source owners, so it cannot duplicate their values.  Its
`flagMem` guard retains register-direct `#UD`, prefixes, rejected encodings,
selector and operand faults, delivery, bus/device service and physical time at
their existing receivers.  Existing LES/LDS semantic smokes retain the
real/protected selector, fault atomicity and prefix/encoding proof.

The focused timing-ledger smoke proves both opcodes in real and protected mode,
using direct even and indexed odd pointer locations.  Every successful case
loads the expected offset/selector and publishes seven ticks through the one
elapsed-time provider.  The protected bootstrap is outside the measurement
window; the measurement counters are reset before each later case.

The focused target emitted `M5:T357:S6:80286-INSTRUCTION-TIMING-LEDGER:OK`.
The configured `run-current-smokes` gate, documentation governance and `git
diff --check` passed before P1.

Coordinator review of P1 `1d3391bb` confirms that the sole 80286 profile-local
branch remains after the higher source owners and before the existing
unallocated receiver. The `flagMem` guard and the retained LES/LDS semantic
smokes keep non-memory forms, prefixes and every failed load outside the new
successful-retirement clock row.
