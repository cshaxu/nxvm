# T437 S1 80386DX Canonical-Key Partition

The executable [partition verifier](../../../tools/Verify-80386TaskPartition.ps1)
expands the retained 80386DX manifest's 450 base forms and 961 legal contexts.
It assigns every canonical key to one later implementation S; a context inherits
its base form's owner. This is a coverage/proof partition, not a timing result
or an amendment to the Intel source ledger.

| owner | base-form boundary | canonical-key boundary |
| --- | --- | --- |
| S3 | ordinary compatible/80386 arithmetic, data movement, multiply/divide, shifts, bit forms, flags and XLAT | every corresponding base and inherited size, segment, LOCK and multiplier key |
| S4 | direct and string I/O, primitive strings and REP strings | every phase, override and size key for those forms |
| S5 | ordinary stack, conditional control, real-mode direct/indirect transfer and real interrupt paths | every outcome, next-component and inherited context key for those forms |
| S6 | protected, privilege, gate, task and VM86 control-transfer/interrupt paths | every direct/indirect, real/protected/VM86, privilege/gate/task and next-component key assigned to those paths |
| S7 | protected segment, descriptor, system and special-register forms | every legal register/memory and inherited context key for those forms |

S8 owns no unimplemented form. It consumes the already partitioned 1,411-key
result set for complete result publication, regression, obsolete-route cleanup
and the task-level audit.

Baseline run:

```text
M5:T437:S1:I386-PARTITION-COMPLETE:PASS:canonical=1411
M5:T437:S1:I386-PARTITION-NO-OVERLAP:PASS:base=450
```
