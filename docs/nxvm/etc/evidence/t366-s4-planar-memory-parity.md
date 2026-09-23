# T366 S4: Model 339 Planar Memory and Parity Binding

The selected Model 339 512 KB planar RAM is now an admissible initial core
configuration. The generic high BIOS alias is installed only when backing RAM
covers `F0000h`; a low-memory profile must instead supply its high reset ROM
through its explicit firmware provider.

The sole shared `t_ram` physical read/write path owns optional parity backing.
When the selected planar controller enables it for 512 KB, writes establish
stored parity and reads compare it after producing the data. A mismatch invokes
the S3 board-parity producer and therefore retains its `61h` latch and `70h`
mask contract. CPU, DMA, firmware and display paths all use this same physical
route; no parallel RAM or test-only public corruption API exists.

`core-machine-planar-parity-nmi-s3-smoke` now emits
`M5:T366:S4:PLANAR-MEMORY-PARITY:OK`. It proves 512 KB creation, good-parity
write, owner-local stored-parity mismatch/read/NMI latch and publication, and reset. A parity-
bound machine rejects RAM reconfiguration. An ordinary machine also rejects a
post-freeze shrink below its retained high BIOS alias; that is a mapping
invariant, not a 512 KB admission failure.

The complete current smoke gate, documentation governance and diff check pass.
No I/O-channel check, expansion memory, fixed disk, speaker/timer, bus wait,
DMA arbitration or physical/cycle timing is selected by this S.
