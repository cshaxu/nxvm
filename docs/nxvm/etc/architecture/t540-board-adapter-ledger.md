# T540 S2 Board-Adapter Ledger

This ledger is the finite S2 design universe.  It records a destination only
after inspecting state ownership, all consumers, reset/finalize ordering and
dependencies.  `Candidate` is deliberately not a permission to move source.

| Adapter family | Mutable state owner today | Current board consumers | Initial disposition | Proof still required before source move |
| --- | --- | --- | --- | --- |
| Pending inventory | Pending S2 inspection | XT, 5170, Model 40, Default | Pending | Complete source/caller/test review |

The final ledger must not classify a file by filename or chip name alone.
Independent chip state remains in `x86/devices`; this ledger concerns only the
remaining board attachment and construction mechanisms.
