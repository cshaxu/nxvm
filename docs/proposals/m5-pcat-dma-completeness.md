# M5 PC/AT 8237A DMA Completeness

Audit and complete the PC/AT dual-8237A contract after T347 has established
real storage DRQ service states.  Reconcile primary/secondary controller port
and page topology, word-address semantics, cascade, request/mask/terminal
count, mode boundaries, reset, and deterministic bus ownership.  Every
adopted mode requires a primary manual contract and focused transfer/fault
proof.  Do not add host-device shortcuts, a second DMA owner, or generic
cycle-exact timing; unsupported modes transfer once to TODO or a later queue
candidate.

