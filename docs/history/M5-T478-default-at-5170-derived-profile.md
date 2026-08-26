# M5 T478 Default-AT 5170-Derived Profile

T478 migrates `default-at` to the same immutable `pc-at-5170` profile route
already used by IBM 5170 and DeskPro. It preserves its actual distinct profile
facts rather than treating it as either an alias of the 5170 root or a generic
80386 machine.

| Subtask | Accepted result |
| --- | --- |
| S1 | Freeze a complete Default-AT versus 5170 field/dynamic-owner ledger and finite parity universe. |

S1 is accepted. Its [ledger](../etc/evidence/t478-s1-default-at-delta-ledger.md)
classifies all fields and limits S2 to one child resolver path using existing
policy representation.

S2 is accepted. Its [resolver evidence](../etc/evidence/t478-s2-default-at-child-resolver.md)
records the one Default-AT child result, its pre-freeze request validation and
the retained S3 direct-session deletion boundary.

S3 is accepted. Its [cutover evidence](../etc/evidence/t478-s3-default-at-session-cutover.md)
records the deleted direct construction path, retained session owners and
257-step current-gate parity.
