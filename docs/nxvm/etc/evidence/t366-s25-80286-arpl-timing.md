# T366 S25: 80286 ARPL Timing

Intel's [80286/80287 Programmer's Reference Manual, Appendix B](https://bitsavers.org/components/intel/80286/210498-005_80286_and_80287_Programmers_Reference_Manual_1987.pdf), corroborated for this stepping-sensitive form by the [B-2/B-3 errata](https://docs.pcjs.org/manuals/intel/80286/80286_B2_B3_Errata-1984-11-21.pdf), gives protected-mode `ARPL r/m16,r16` `10,mem=11` clocks.

S25 publishes only those successful unprefixed 80286 rows through the existing
profile-local retirement classifier. The original ARPL handler retains the
profile/mode gate, RPL adjustment and ZF semantics. Real-mode rejection,
prefixes, null-selector erratum, access faults, delivery, bus/device service
and physical time remain transferred.

The focused timing smoke covers a register adjustment, direct-memory adjustment
and indexed-memory no-adjust result after an isolated protected bootstrap. It
checks fixed ticks, selector image and ZF outcome. The existing ARPL semantic
smoke retains metadata, rejected profile and prefixed-memory coverage.

The focused targets emitted `M5:T357:S6:80286-INSTRUCTION-TIMING-LEDGER:OK`,
`M5:T263:S2:ARPL:OK` and `M5:T263:S3:ARPL:CORPUS:OK`.
