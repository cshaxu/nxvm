# Td S157 Model-40 D4 Candidate Reconciliation

`M5:Td:S157:D4-CANDIDATE:RETIRED`

The former queued Model-40 D4 refresh-hold deadline candidate is obsolete.
T419 establishes one Core-owned PIT counter-1 to refresh-hold to DMA ordering
route, including reset and cancellation. T507 S14 changes the former
duration-less L1 classification into one Core next-step L2 deadline.

Current code retains the same result: the D4 pending hold contributes
`elapsed_ticks + 1` to the Core deadline selection and consumes one refresh
transaction before DMA. The focused `unit.core-machine-d4-refresh-hold-smoke`
passes against the current build tree.

This decision does not claim physical DeskPro D4 DRAM, RAS/CAS, cache, ISA or
refresh-duration timing. Those remain separately recorded physical-observable
device timing debt. No runtime source, profile configuration, release artifact
or `build/output` YAML changes occur.
