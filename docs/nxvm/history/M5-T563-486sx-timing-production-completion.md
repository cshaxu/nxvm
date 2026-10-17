# M5 T563 Remaining 486SX Timing And Production Qualification

Owner admits the queued CPU successor after T562 closure, baseline 606ee84f4.
The [proposal](../proposals/m5-486sx-timing-production-completion.md) retains
all original C01--C17 obligations and source/model restrictions. S1 audits
current ownership and remaining cohorts; it does not open C17 or qualify CPU
timing by reusing a successful older-family boot.

S1 audited the current source/test owners and recorded ten residual receiver
gaps in [evidence](../etc/evidence/t563-s1-rebase.md). The proposal now plans
S2--S8 with explicit cohort exits. Fresh builds and all 631 repository units
pass on each width using the persistent Ninja/ccache graphs. No source, test,
public API, manifest, App configuration or executable input changed.

Coordinator reviewed pushed `895c1e360` against the S1 packet: five NXVM
documentation files, +185/-33, with no executable changes. Original C01--C17
and source/model limitations remain referenced, all residual cohorts have
receivers and fresh unit results match their logs. S1 is closed; T563 stays
open for S2--S8. This acceptance is an audit delivery, not CPU completion.

S2 accepted Shared `4fb3d5238` and NXVM `b331fe40d`, with coordinator
acceptance `68dd1b9e0`; its nominal/form evidence remains linked from Current.
S3 now accepts Shared `6f4a03a89` and NXVM `1006281a9` after actual-commit
review. [Memory/translation evidence](../etc/evidence/t563-s3-memory-translation.md)
maps S1 R3/R4 to completed access allocation and real-execution proof. Both
631-unit inventories, 16 selected PC integrations per width, published CLI
identities, specialized gates and eight 0563 PC artifacts pass. Lib, Emulator,
MyNES and owner inputs are unchanged. S3 is closed, not complete C16/C17;
fetch/bus/decode/REP, events/system, final selector/preview, production admission
and whole qualification retain their S4--S8 receivers.

S4 accepts Shared `f48b7984c` and NXVM `f5b4b1eee` after coordinator
actual-commit review of the frozen F1--R3 cohort. The [S4 evidence](../etc/evidence/t563-s4-fetch-bus-repeat.md)
records owner-approved wait feedback/Core-clock bus advancement, one CPU phase
publisher, four-slot L2 timing, transaction-deduplicated invalidation
observations and actual REP/fetch/failure/lifetime proof. Both widths pass
632 repository units, 16 selected integrations, final published CLI checks and
specialized gates. Eight 0563 artifacts are refreshed. S5 system/event totals,
S6 sole selected runtime/preview and C17/final qualification remain mandatory;
this delivery is not a complete 486SX or PC110 claim.
