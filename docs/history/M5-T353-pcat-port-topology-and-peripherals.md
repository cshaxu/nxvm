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

S2 is accepted at `c64a9c51` and corrected at `6c6a9ef4`.  It replaces the
range-only construction with an exact directional port-leaf and named-route
contract, validates that contract before core-machine publication, and proves
every leaf against the frozen registry plus every selected IRQ/DMA binding.
Its [evidence](../etc/evidence/t353-s2-profile-topology-contract.md) retains
ATA feature semantics as one source and leaves no selected sparse range claim.

S3 is accepted at `149ff080`. Its
[admission decision](../etc/evidence/t353-s3-pcat-peripheral-admission-decision.md)
audits repository-held DOS/Windows probes, media policy, frozen topology, and
the retained PIT/RTC/NMI records. No named corpus and no selected channel-2 or
port-`61h` hardware contract exist, so it makes no peripheral claim or code
change. Port-`61h` PPI/speaker, parity/I/O-channel NMI, and serial/parallel/game
interfaces retain their separate exact TODO receivers and admission conditions.
