# M5 T353: PC/AT Port Topology And Selected Peripheral Completion

## Task Record

T353 follows the deterministic selected-device L3 foundation through T352.  It
reconciles the default PC/AT profile's selected port topology, composition,
routes, and lifecycle before the later physical bus-timing package.  It does
not claim all historical PC/AT peripherals or Windows readiness.

## Accepted Progress

S1 is accepted at `a95f6bd3`.  Its
[ledger](../etc/evidence/t353-s1-pcat-port-topology-ledger.md) maps the
selected manual-to-profile-to-composition-to-core-to-proof surface and assigns
the range-only topology construction, including dual-DMA sparse leaves, port
`92h`, sparse controller windows, and ATA's separate descriptor, to S2.  It
retains speaker/PPI and optional interface breadth as existing exact debt
receivers.
