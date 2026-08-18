# T418 S2 Instruction-Boundary Locality Closure Audit

M5:T418:S2:INSTRUCTION-BOUNDARY-LOCALITY-CLOSURE:OK

Fresh review of P1 `69d46e9b` confirms one Core-owned conservative generic-AT
boundary: each new CPU instruction round invalidates the prior locality key.
The direct locality contract retains same-round cycles, cancellation, DMA HOLD,
refresh and reset proof; the runnable regression proves a later instruction
cannot inherit the earlier prefetch key. The serialized 288-test current gate,
documentation governance and diff hygiene pass.

Original D3PE evidence states only that an inserted CPU idle ends PAGE HIT.
Core cannot observe the physical overlap/idle phase, so T418 does not claim that
an instruction boundary is an original D4 idle state. Exact pipelined overlap,
D4 row retention, BWAIT/DMA/refresh arbitration and Model-L3 remain transferred.