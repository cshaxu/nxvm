# T437 S4 80386DX string and I/O execution

S4 closes its 162-key partition through the existing sole Core retirement
publisher.  It does not publish a partial final 1,411-key result set.

## Observed execution

The 80386DX manifest runner installs a bounded port provider at `0080h`, then
executes each direct `IN`/`OUT`, primitive string and REP recipe from the reset
mapping.  The recipes cover byte, word and operand-size-32 forms; legal source
segment overrides; and the 81 REP first, continuation and zero-count keys.

Continuation is not synthesized: the runner executes a count-two REP once
without recording it, then captures its second retirement and requires the
Core-published continuation phase.  `REPE` CMPS/SCAS inputs are equal and
`REPNE` inputs differ, so a stop condition cannot accidentally replace the
continuation path.

```text
M5:T437:S4:I386-STRING-IO-OBSERVED:162
M5:T437:S4:I386-STRING-IO-COVERAGE:PASS:canonical=162
M5:T437:S4:I386-STRING-IO-INPUTS:PASS
M5:T437:S2:I386-INCOMPLETE-RESULT-REFUSED:PASS
```

All observed retirements are classified, have a non-unallocated source form
and origin, and retain the S2 incomplete-final-result guard.  The focused
8086/80186/80286/80386, prefix, paging, protected-I/O and decoder-ledger
regression set passed.  No public ABI, board timing or FPU contract changed.

## Transfer

S5 owns the remaining ordinary stack, conditional-control, real-mode transfer
and real interrupt keys.  S4 makes no claim for those paths.
