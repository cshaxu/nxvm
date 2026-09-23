# T389 S2: Stop-And-Requeue Audit

`M5:T389:S2:STOP-AND-REQUEUE:OK`

## Decision

The owner approved stopping T389 after S1 proved that the candidate's assumed
precondition is false. T388 makes physical publication safe by rejecting an
unallocated success, but it does not establish a complete physical 80386
retirement axis. The current Model-40 composition remains deterministic with
six neutral `{ 1, 1, 0 }` clock domains. Proceeding would require an invented
16 MHz conversion, DCLK grant or wait state.

T389 is therefore **stopped, not board-timing complete**. Its original proposal
is retained beside task history as the record of the invalid dependency order;
no accepted evidence, source behavior or physical/nonphysical conclusion is
rewritten.

## Requeue Result

The Queue replacement is ordered as follows:

1. shared 80386 physical-retirement qualification, which owns Intel-primary
   form/context coverage and existing physical-contract eligibility;
2. a fresh DeskPro Model 40 board-level timing candidate, which consumes only
   that accepted qualification; then
3. the existing DeskPro physical-device/firmware timing and Model-L3 audit
   candidates.

The qualification candidate is intentionally neither a generic CPU-completeness
program nor a board-delay task. It must make the selected Model-40 approved
corpus exact-or-nonphysical before profile physical selection; unselected,
range-only, x87, delivery and source-underdetermined forms retain their
existing transfers. The later board candidate owns only the physical board
conversion, DMA/PIT/wait/order consequences after that acceptance.

## Similar-Issue Sweep

Reviewed Queue candidates; T359/T360/T363 timing ledger closures; T388 S2--S9;
T389 S1; the retained `m5-instruction-timed-execution` proposal; and the
DeskPro board/device proposals. None is an active candidate with the required
80386 physical-eligibility scope. T388 is a completed safety contract, T359--
T363 are source-accounting ledgers, and the retained instruction-timed proposal
is historical context. The new candidate is singular and owns the gap.

## Verification

This is documentation-only. It changes no source, build, test, artifact,
firmware, media, profile or runtime contract. `git diff --check` and
documentation governance are required before acceptance.