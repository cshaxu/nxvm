# M5 T477 DeskPro 386 5170-Derived Runnable Profile

T477 makes the existing Model-40 composition an explicit immutable child of
`pc-at-5170` without changing its selected device behavior or claiming a new
physical/L3 result.

| Subtask | Accepted result |
| --- | --- |
| S1 | [Delta ledger](../etc/evidence/t477-s1-deskpro-delta-ledger.md) freezes every inherited, child and dynamic field. |
| S2 | [Resolver evidence](../etc/evidence/t477-s2-deskpro-child-resolver.md) proves copied child Core/policy data. |
| S3 | [Cutover evidence](../etc/evidence/t477-s3-deskpro-session-cutover.md) removes direct session Core configuration. |
| S4 | [Closure audit](../etc/evidence/t477-s4-closure-audit.md) records full parity, 294-target gate and Release 0477. |

The retained dynamic session owners are BYOB ROM bytes, D4 backing and media;
Core remains sole mutable machine/device owner. Artifact
`nxvm_0_5_0477.exe` has SHA-256
`29AF6BCB351A5EE0F1D6C992EDC4BCA71C5BF6897E88B4BE848907547A9E9F01`.
