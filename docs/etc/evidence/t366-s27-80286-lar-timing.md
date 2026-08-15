# T366 S27: 80286 LAR Timing

Intel's 80286 Instruction Set Appendix B assigns protected-mode
`0F 02 /r LAR r16,r/m16` fixed `14,mem=16` clocks.  The manual's
[clock-table convention](https://tv.manualsonline.com/manuals/mfg/intel/80286.html?p=216)
defines the unlabelled value as the register form and `mem` as the memory
form; it also keeps indexed-address and bus additions outside this admitted
successful-retirement scalar.

S27 publishes only unprefixed protected nonfaulting LAR through the existing
80286 profile-local retirement owner.  Both the ZF-set valid-selector outcome
and the ZF-clear invalid-selector outcome retire at the same register cost;
the focused ledger smoke proves register, direct-memory and indexed-memory
valid forms and preserves the ZF-clear register result.  Real mode, prefixes,
memory faults, delivery, descriptor repair, bus/device service and physical
time remain transferred.  The existing semantic smoke emitted
`M5:T316:S57:LAR-LSL:OK`.
