# T366 S30: 80286 SLDT/STR Timing

Intel's [80286/80287 Programmer's Reference Manual](https://www.dosdays.co.uk/media/intel/210498-005_80286_and_80287_Programmers_Reference_Manual_1987.pdf)
Appendix B assigns protected-mode `0F 00 /0 SLDT r/m16` and `/1 STR r/m16`
fixed register `2` and memory `3` clocks.  The manual's `mem` distinction is
the r/m register-versus-memory form; addressing and bus additions remain
outside this admitted retirement scalar.

S30 publishes only unprefixed protected nonfaulting SLDT/STR through the
existing 80286 profile-local retirement owner.  The focused ledger smoke proves
register, direct-memory and indexed-memory selector stores; the existing
semantic smoke emitted `M5:T316:S61:DTTR:OK`.  Real-mode `#UD`, prefixes,
operand faults, delivery, LLDT/LTR, bus/device service and physical time remain
transferred.
