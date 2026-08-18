# T419 S2 D4 DMA Grant Wait

M5:T419:S2:D4-DMA-GRANT-WAIT:OK

Original D3PE DMA-byte-cycle text says HAKDMA follows HAK by one DCLK and that
all DMA cycles receive one DCLK-length wait; DCLK is 4 MHz. Core has no physical
nanosecond clock relation, so this implementation is an original-source-backed
discrete bridge: `dma_cycle_wait_quanta` delays a pending DMA grant by one
*configured DMA scheduling quantum*. It does not assert that an NXVM elapsed
tick is 250 ns.

Core remains the sole transaction owner. A zero selector preserves the existing
route. Model-40 private and BYOB composition select one quantum; reset clears
the pending delay. The retained Core competition smoke passes unchanged, while
the Model-40 DMA topology smoke proves the selected value and emits the T419
marker. No D4 pin waveform, BUSRDY stretch, refresh priority or BWAIT model is
claimed.

Artifact: `vm-0-5-0419`, `build/output/nxvm_0_5_0419.exe`, 3,213,818 bytes,
SHA-256 `CE739C2B99E41F473A6F31EDB91DE3D8BD67DBFA56363285DD6EA1CAFC857B67`.