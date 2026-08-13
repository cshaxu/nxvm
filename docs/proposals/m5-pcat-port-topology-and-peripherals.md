# M5 PC/AT Port Topology And Selected Peripheral Completion

Reconcile the declared PC/AT profile port ranges, actual core registrations,
IRQ/DMA routes, reset ownership, and firmware-visible topology.  Resolve each
observed discrepancy as a configuration correction or an exact documented
intent.  Admit speaker/PPI, serial, parallel, or game-port owners one at a
time only when a corpus and a hardware contract require them.  Do not create
generic host passthrough or infer device support from unused port numbers.

