# M5 T476 PC/AT 5170 Root Profile Normalization

T476 is the first ordered consumer of T474's Core time boundary and T475's VM
resolver. It turns the existing runnable IBM 5170 Model 339 composition into
one copied `pc-at-5170` session root without changing selected machine
behavior. DeskPro 386 and default-PC/AT remain ordered later migrations.

The original approved [proposal](M5-T476-pc-at-5170-root-profile-normalization-proposal.md)
is retained beside this closure record.

## Accepted Work

| Subtask | Accepted result |
| --- | --- |
| S1 | The [root ledger](../etc/evidence/t476-s1-ibm5170-root-ledger.md) freezes selected configuration, L3/L2 provenance, direct construction and finite parity. |
| S2 | The [resolver evidence](../etc/evidence/t476-s2-ibm5170-root-resolver.md) establishes copied root data and one validated Core materialization. |
| S3 | The [cutover evidence](../etc/evidence/t476-s3-ibm5170-root-cutover.md) moves IBM sessions to their owned root and deletes the second session materialization path. |
| S4 | The [closure audit](../etc/evidence/t476-s4-closure-audit.md) records the 294-target gate, retirement of a duplicate exception aggregate and stripped Release 0476. |

## Retained Boundary

VM owns immutable profile/root identity and session policy. Core receives only
copied neutral configuration and remains the sole mutable machine, device and
guest-time owner. The retained static Model-339 descriptor is declaration
input to the root builder only; no session reaches it directly.

The artifact is `build/output/nxvm_0_5_0476.exe`, banner `0.5.0476`,
1,206,544 bytes and SHA-256
`F3A5004E6C8932AAB2462B6000B2B8F0BA1F94ABE3CB30E2A3EE68A6487F052A`.
