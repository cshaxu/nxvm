# T419 S1 D4 DMA No-Wait Arbitration

M5:T419:S1:D4-DMA-NO-WAIT:OK

The original DeskPro D3PE processor description says that system DMA and refresh
perform standard multiplexed RAM cycles with no wait states. Its D4-RCTL PAL
material also records DMA read/write transitions after HLDA. T419 does not
translate those signal phases into a fabricated duration. It qualifies the
already-owned Core DMA route in an actual D4 platform composition.

The focused competition smoke now creates the 80386 profile, enables the
auxiliary PIT required by D4 composition, configures D4 port-B topology, and
verifies the D4 observation before execution. It then proves the sole existing
order: CPU memory-read begin/commit and retirement; DMA HOLD request and
acknowledge; DMA memory-write begin/commit; HOLD release; DMA advance; then
peripheral advancement. The one-instruction run remains three ticks. No
additional CPU-locality surcharge, Core/VM reverse dependency, second scheduler
or second CPU/DMA transaction path is present.

Developer artifact: `vm-0-5-0419`, `build/output/nxvm_0_5_0419.exe`, 3,212,215
bytes, SHA-256 `E49081DCCAB999D756FFAE1F3855309DDAB6ECF720FE134B2321043F098A4ABE`.

Similar-issue sweep: `rg -n -C 3 "core_machine_dma_advance_transaction|dma_clock|clock_domain" src/core/machine` finds one Core DMA clock and one transaction advance route. The 80286/80386 HOLD branch invokes it only after acknowledged HOLD; the non-HOLD branch is for other CPU profiles. D4 composition supplies topology only. Exact D4 DMA/refresh/BWAIT arbitration, four-CLK32 memory state waveforms, row retention, bus-master cascade timing and Model-L3 acceptance remain transferred to the same physical-cycle proposal.