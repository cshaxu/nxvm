# T366 S26: 80286 VERR/VERW Timing

Intel's [80286 Appendix-B VERR/VERW page](https://manualsdump.com/en/manuals/intel-80287model-80286model/110730/321)
assigns both protected-mode `0F 00 /4 VERR ew` and `/5 VERW ew` fixed
`14,mem=16` clocks. Selector validation produces ZF and cannot itself fault;
only access to a memory selector can fault.

S26 publishes only unprefixed protected successful forms through the existing
80286 profile-local retirement owner. Real mode, prefixes, access faults,
delivery, bus/device service and physical time remain transferred. The focused
ledger smoke proves register, direct-memory and indexed-memory forms; the
existing semantic smoke emitted `M5:T316:S58:VERR-VERW:OK`.
