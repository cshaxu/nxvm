# T436 S1 80286 Canonical-Key Partition

The executable [partition verifier](../../../tools/Verify-80286TaskPartition.ps1)
expands the retained 80286 manifest's 286 base forms, 459 legal single-axis
contexts and 62 combined contexts. It assigns each canonical key to exactly
one later implementation S; context keys inherit their base form's owner.
This is a coverage/proof partition, not a timing result and not an amendment
to the Intel source ledger.

| owner | base-form boundary | canonical-key boundary |
| --- | --- | --- |
| S3 | non-control, non-string forms, including arithmetic, r/m, EA, odd-word and legal LOCK contexts | every corresponding base and inherited context key |
| S4 | primitive strings, port strings and REP forms | every phase, override and word odd-address key for those forms |
| S5 | conditional branches, `JCXZ`, LOOP forms and `INTO` | every outcome and inherited context key for those forms |
| S6 | call, return, jump and interrupt transfer paths | every direct/indirect, real/protected, privilege/gate/task and next-byte key for those forms |
| S7 | segment, descriptor and protected system forms | every legal register/memory form and inherited context key for those forms |

S8 owns no unimplemented form. It consumes the already partitioned 807-key
result set for full closure, regression, cleanup and the task-level audit.

Baseline run:

```text
M5:T436:S1:I286-PARTITION-COMPLETE:PASS:canonical=807
M5:T436:S1:I286-PARTITION-NO-OVERLAP:PASS:base=286
```
