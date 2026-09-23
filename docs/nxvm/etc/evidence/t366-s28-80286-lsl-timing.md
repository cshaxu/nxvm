# T366 S28: 80286 LSL Timing

Intel's 80286 Instruction Set Appendix B assigns protected-mode
`0F 03 /r LSL r16,r/m16` fixed `14,mem=16` clocks.  The manual's
[clock-table convention](https://tv.manualsonline.com/manuals/mfg/intel/80286.html?p=216)
defines the unlabelled value as the register form and `mem` as the memory
form.  Unlike later 80386 LSL, the 80286 row has no descriptor page-granularity
timing distinction.

S28 publishes only unprefixed protected nonfaulting LSL through the existing
80286 profile-local retirement owner.  Both valid-selector ZF-set and
invalid-selector ZF-clear outcomes retire at the register cost; the focused
ledger smoke proves register, direct-memory and indexed-memory valid forms and
preserves the ZF-clear register result.  Real mode, prefixes, memory faults,
delivery, descriptor repair, bus/device service and physical time remain
transferred.  The existing semantic smoke emitted `M5:T316:S57:LAR-LSL:OK`.
