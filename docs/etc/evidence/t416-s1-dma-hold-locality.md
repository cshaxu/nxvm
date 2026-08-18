# T416 S1 DMA HOLD Locality Bridge

M5:T416:S1:DMA-HOLD-LOCALITY:OK

D3PE schematics show the DeskPro's DMA/HOLD/HLDA/refresh topology. They do not
publish a calibrated D4 DRAM page-retention rule across a DMA handoff. T416
therefore adds one generic-AT policy at the existing Core transaction boundary:
an acknowledged DMA HOLD invalidates the CPU external-memory locality key. The
next committed CPU physical memory cycle cannot receive a same-page hit solely
because it preceded DMA bus ownership.

The policy is Core-owned and applies only after DMA HOLD acknowledgement, not
on request or release. It preserves the existing sole transaction owner and
Model-40 continues only to select the generic 2 KiB/miss+2/hit+0 values. It is
not original DeskPro phase timing.

The focused locality smoke proves request, acknowledgement, release, next CPU
same-page miss and reset. Existing CPU/DMA competition, transaction, paging and
locality regressions pass.

Developer artifact: vm-0-5-0416, build/output/nxvm_0_5_0416.exe, 3,211,636
bytes, SHA-256 347D73EEAAA81BA3F88C6C1AD47C7E2FEAC94E997323B6FA2403C0F7D85BB51F.

Exact D4 DMA/refresh/BWAIT clocks, physical page retention, HOLD waveform,
locked/RMW cycle shape, device service timing and Model-L3 acceptance remain
transferred.