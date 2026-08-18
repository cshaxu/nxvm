# T417 S1 Refresh Locality Bridge

M5:T417:S1:REFRESH-LOCALITY:OK

The original D3PE material records refresh as a non-CPU cycle under HLDA and
specifies a refresh-control path. It also states that system DMA and refresh
use standard multiplexed DRAM cycles; it does not state a CPU page-retention
rule, a counter period, or a calibrated D4 phase. The existing Core PC/AT
counter-1 mode-2 refresh owner supplies the generic board mechanism.

T417 binds that output only while the D4 platform is configured. On its active
low pulse, the existing Core locality key becomes invalid. The focused test
constructs a real D4 platform with a 2 KiB/miss+2/hit+0 generic locality
configuration, commits a page-table read, advances the shared counter through
its programmed 18 ticks to the low pulse, then proves that the next same-page
page-table read receives miss +2. Reset clears the key and pending locality
ticks. The same test retains cancellation and DMA-HOLD lifecycle coverage; the
D4 platform and CPU/DMA competition smokes remain green. A fully rebuilt Model-40 private-composition smoke also corrected its stale one-tick assertion to the already configured generic-AT initial-prefetch result of three ticks.

This is a generic-AT policy attached to the existing Core shared-PIT/locality
owner. It does not introduce a scheduler, a second transaction path, a Core/VM
reverse dependency, or a claim that refresh clears physical DRAM page state.
Exact D4 refresh period, physical page retention, DMA/refresh/BWAIT
arbitration, locked/RMW shape, device service timing and Model-L3 acceptance
remain transferred.

Developer artifact: `vm-0-5-0417`, `build/output/nxvm_0_5_0417.exe`,
3,212,215 bytes, SHA-256
`3D509C6680FFEAEFD039744DCF959BC1BED89CEEDB94DA05B30A669DEC1AD4DE`.