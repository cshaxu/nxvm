# T436 S2 80286 Result Producer

The 80286 manifest runner now retains the first real retirement observation
for each selected canonical metadata record. The captured record carries the
same tick, formula-input, form-ID, timing-origin and unallocated-disposition
fields consumed by the 80286 result verifier.

The runner contains the final result-document writer, but the writer refuses
to open its output unless all 807 canonical 80286 records have been observed.
This prevents an S3--S7 partial recipe set from producing a document that
looks like a valid closure artifact. Repeated observations of the same key do
not overwrite the first captured record; protected bootstrap activity has no
active canonical metadata index and is not captured as a target result.

The S2 baseline run observes 606 real records and deliberately refuses final
emission. The remaining 201 observations remain assigned to S3--S7 by the S1
partition; no missing observation becomes conforming through this mechanism.

```text
M5:T436:S2:I286-RESULT-PRODUCER:PASS:observed=606:canonical=807
M5:T436:S2:I286-INCOMPLETE-RESULT-REFUSED:PASS
```
