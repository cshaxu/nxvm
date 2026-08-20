# M5 T434: Core Timing Contract And Machine Plan

## Task Record

T434 closes the Core timing-plan boundary: all current VM session families
materialize one copied, validated `core_machine_plan`, and Core applies it
atomically before publication. The boundary has one exact declaration and
default L2/NGT disposition for every frozen T433 capability; later timing work
implements sourced L3 rules through it.

## Delivery

- S1: `6e1588f5`, `3b6a8a29`, `f12b4a5c` introduced the immutable Core plan,
  30-ID declaration/seam map, validation and copy proof.
- S2: `7be4dd7d`, `739fd817`, `82b371a2` migrated default PC/AT, IBM 5170
  Model 339 and private Model-40 BYOB composition in one plan-only cutover and
  aligned the current developer artifact.
- S3: this closure record strengthens all-30 regression proof and
  independently reconciles every task predicate.

## Verification And Closure

The [S3 closure audit](../etc/evidence/t434-s3-task-closure-audit.md) records
the complete requirement-to-proof review, static production-route sweep and
eight linked executable smokes. The current developer artifact is
`vm-0-5-0434` / `nxvm_0_5_0434.exe`, SHA-256
`0252F8FDA17BEC2131606F19E3547B46894AC6B56DD37EC3B16BD302494FAFDC`.

T434 closes with no connected consumer, production publication route or
capability disposition deferred. The archived
[proposal companion](M5-T434-core-timing-contract-machine-plan-proposal.md)
retains the original boundary and non-goals.
