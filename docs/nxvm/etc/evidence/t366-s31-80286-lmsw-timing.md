# T366 S31: 80286 LMSW Timing

Intel's [80286/80287 Programmer's Reference Manual](https://www.dosdays.co.uk/media/intel/210498-005_80286_and_80287_Programmers_Reference_Manual_1987.pdf)
Appendix B assigns `0F 01 /6 LMSW r/m16` fixed register `3` and memory `6`
clocks in its documented real-address and protected-CPL0 forms.  The manual's
`mem` distinction is the r/m register-versus-memory form; addressing and bus
additions remain outside this admitted retirement scalar.

S31 publishes only unprefixed nonfaulting LMSW through the existing 80286
profile-local retirement owner.  The focused ledger smoke proves real and
protected-CPL0 register/direct/indexed forms; the existing semantic smoke
emitted `M5:T316:S63:MSW:OK`.  Prefixes, protected nonzero-CPL fault, operand
fault, delivery, descriptor/table work, bus/device service and physical time
remain transferred.
