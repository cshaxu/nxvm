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
