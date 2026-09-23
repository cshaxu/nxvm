# T366 S29: 80286 SMSW Timing

Intel's [80286/80287 Programmer's Reference Manual](https://www.dosdays.co.uk/media/intel/210498-005_80286_and_80287_Programmers_Reference_Manual_1987.pdf)
Appendix B assigns `0F 01 /4 SMSW r/m16` fixed register `2` and memory `3`
clocks in both real and protected modes.  The manual's clock-table convention
defines the unlabelled value as the register form and `mem` as the memory form;
addressing and bus additions remain outside this admitted retirement scalar.

S29 publishes only unprefixed nonfaulting SMSW through the existing 80286
profile-local retirement owner.  The focused ledger smoke proves real and
protected register, direct-memory and indexed-memory forms; the existing
semantic smoke emitted `M5:T316:S63:MSW:OK`.  Prefixes, operand faults,
delivery, descriptor/system-register loads, bus/device service and physical
time remain transferred.
