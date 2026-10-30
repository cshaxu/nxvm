# T582 S26 — asynchronous entry access scope upper bound

The 486 refresh path saves, clears and restores entry access observations
around post-retirement asynchronous processing, including when no interrupt
is ultimately accepted. The scope preserves the interrupted instruction's
access and bus observations while an interrupt, exception, SMM entry or
special cycle accounts for its own work. Earlier event-system evidence
`t563-s5-event-system.md` documents why those distinctions matter.

A task-only experiment removed the outer save/clear/restore while retaining
the existing accepted-transfer assignment. This intentionally breaks the
general observation contract and is not a production candidate. Its purpose
was to estimate the best case before attempting a more complex lazy scope.
At the 10,000,000-instruction PC110 private-media startup checkpoint, ordered
baseline/candidate/candidate/baseline elapsed times were
18,987/19,025/19,310/19,419 ms on x64 and
23,254/22,824/22,833/23,581 ms on x86. All runs reported 79,964,294 guest
ticks and PC `0003dbc5`. Equality of that checkpoint is not full-state or
interrupt correctness. The comparisons show no reliable large improvement;
the small x86 difference is not proof of a representative gameplay gain.

No lazy-entry scheme is admitted on this evidence: it would need to isolate
each accepted interrupt, debug trap, SMM entry and special cycle without
duplicating or reordering the existing checks. No production code, interface,
test, media or executable changed. PC110 usability remains unresolved.
