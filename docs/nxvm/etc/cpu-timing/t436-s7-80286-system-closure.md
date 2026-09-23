# T436 S7 80286 Segment, Descriptor And System Closure

S7 owns the final 36 Appendix-B bases and 92 canonical 80286 records: segment
register moves and stack loads, `LEA`, `LDS`/`LES`, `LAR`/`LSL`, `VERR`/`VERW`,
descriptor-table, machine-status, LDT and task-register forms.  It includes
their legal register/memory, protected-mode, effective-address, segment-override
and odd-word contexts.

The retained runner had real recipes for 85 of the records, but omitted seven
legal intersections: `MOV Sreg` store/load and `LDS`/`LES`, each with the
base-index-displacement effective address plus an odd word.  The real-mode
fixtures write their word or far pointer at `0x1001`; the protected fixtures
write the selector or far pointer at linear `0x4001` after the normal
protected bootstrap.  These are real retirements, not copied observations.
Their Appendix-B ticks are respectively 6/8/12 in real mode and 22/26 in
protected mode.

The runner now makes S7 the disjoint complement of the accepted S3--S6
partition and checks all 92 records.  Every record is a classified successful
retirement: primary timing origin except stack `POP Sreg`, which correctly
uses Control Stack; effective-address, segment-override and odd-word keys
publish their corresponding formula inputs.  The complete runner captures all
771 legal canonical records: S3/S4/S5/S6/S7 = 303/129/54/193/92.

Final result publication remains S8-owned.  Although S7 proves the complete
set, the writer requires explicit final-closure authorization and refuses the
S7 call; no `t436-s8-80286-timing-results.json` file is created.  S8 will
perform the all-key publication, cross-profile regression and final audit.

```text
M5:T436:S2:I286-RESULT-PRODUCER:PASS:observed=771:canonical=771
M5:T436:S2:I286-INCOMPLETE-RESULT-REFUSED:PASS
M5:T436:S7:I286-FINAL-RESULT-DEFERRED:PASS
M5:T436:S7:I286-SYSTEM-OBSERVED:92:canonical=92
M5:T436:S7:I286-SYSTEM-COVERAGE:PASS:canonical=92
M5:T436:S7:I286-SEGMENT-DESCRIPTOR-PATHS:PASS
```

Focused `core-machine-80286-instruction-timing-ledger-smoke` and
`core-machine-protected-data-access-s2-smoke` pass.  The manifest, partition
and decoder-ledger verifiers also pass.  The similar-issue sweep is the full
S7 ownership predicate and its runner recipes; it found exactly the seven
missing odd-word EA intersections above and no production fallback or
out-of-scope hit.
