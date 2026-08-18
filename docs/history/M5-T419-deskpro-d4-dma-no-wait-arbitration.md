# M5 T419: DeskPro D4 DMA No-Wait Arbitration

T419 consumes the original-source-backed high-level DMA receiver from the DeskPro
physical-cycle proposal. The D3PE description explicitly says system DMA and
refresh use standard multiplexed RAM cycles with no wait states, and its D4-RCTL
material defines DMA entry after HLDA. The existing Core already has one CPU/DMA
transaction owner and one DMA clock route. T419 qualifies that route under actual
D4 composition rather than adding a board scheduler or a VM-side transaction path.

P1 added a marker without configuring D4 and was insufficient evidence. P2 corrects
that evidence gap: the competition regression uses the 80386 profile plus D4
platform topology, then proves ordered CPU retirement, HOLD request/acknowledge,
DMA transaction begin/commit, HOLD release and DMA advance, while the one-instruction
run stays at its existing three ticks. This is not a calibrated D4 DMA waveform or
an assertion of BWAIT/refresh arbitration.