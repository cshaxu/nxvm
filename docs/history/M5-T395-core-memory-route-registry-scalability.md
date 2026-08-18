# M5 T395: Core Memory-Device Route Registry Scalability

## Task Record

T395 removes the bounded fixed memory-device-provider registration limitation
before further multi-route machine composition. It is Core-owned infrastructure
and does not select any DeskPro timing, firmware or physical clock behavior.

## S1 Implementation Record

S1 replaces the private fixed provider array with a 12-to-64 bounded growable
registry, preserving registration order, overlay/decline behavior, freeze
rejection and rollback. The [capacity and caller audit](../etc/evidence/t395-s1-memory-route-registry-audit.md)
records every production-path hit and focused proof. No Core/VM ABI or machine
mapping semantics changed.

S1 P3 adds a direct deterministic allocation-failure injection at the private route-registry growth boundary: a failed thirteenth registration retains the original twelve providers and capacity, and the retry grows successfully. No Core/VM interface changed.

S1 P4 builds the required current developer artifact `vm-0-5-0395` / `build/output/nxvm_0_5_0395.exe` from the complete source graph, SHA-256 `5DBD4B50762BAD5E393CD8EFA8A0CB7E06FD9A1304A44F8993CC64915A99D4A6` (3,180,342 bytes).
