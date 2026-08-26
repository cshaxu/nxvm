# M5 T475 VM Profile Resolver Kernel

T475 establishes the VM-only profile resolver that follows T474's
machine-neutral Core time boundary and precedes every PC/AT profile connection.
It does not migrate a runnable profile.

## Accepted Work

| Subtask | Accepted result |
| --- | --- |
| S1 | The [construction ledger](../etc/evidence/t475-s1-profile-resolver-ledger.md) freezes the three current profile and Core-plan construction routes, excludes mutable state, and assigns each retained direct route to its unique downstream migration. |
| S2 | The [resolver proof](../etc/evidence/t475-s2-profile-resolver.md) establishes a one-parent typed declaration mechanism, copied resolved values and provenance, contract/window/route/policy validation, one existing Core-plan handoff and focused acceptance/rejection coverage. |
| S3 | The [closure audit](../etc/evidence/t475-s3-integration-closure-audit.md) records the 294-target gate and stripped Release 0475 artifact. |

## Retained Boundary

VM owns profile identity, inheritance, provenance and session policy. Core
receives only copied neutral values and remains the sole owner of mutable
machine, device, transaction and guest-time state. The resolver has no current
runnable profile consumer; its first consumers remain the queue-ordered IBM
5170 root, DeskPro 386 child and default-AT child migrations. Each deletes its
one legacy direct route only after its own parity proof.

The artifact is `build/output/nxvm_0_5_0475.exe`, banner `0.5.0475`,
1,198,682 bytes, SHA-256
`26E00D29D41EF4A69E234D31A6FD54D886D879B8DD2402F45C59FA2E55EE1281`.
