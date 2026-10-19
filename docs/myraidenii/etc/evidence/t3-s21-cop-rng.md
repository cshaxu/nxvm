# T3 S21: Complete COP RNG Register Receiver

## Boundary And Model

Baseline6d7fad708 accepts the [S20 residual contract](t3-s20-residual-contract.md).
Its fixed research revision and hashes remain the source-grade basis; no new
third-party payload, ROM, table or implementation is imported. The reference
only reports RNG verification for68k games. This V30 implementation is an
explicit nominal L2 model, not measured physical randomness or cycle accuracy.

The existing maximum word42c and its byte merges remain the sole configuration
owner. Four read-only aliases5a0/5a2/5a4/5a6, including odd-address byte views,
sample the existing Core timeline once per provider read and return
`timeline.now modulo (maximum + 1)`. Addition is widened before evaluation:
ffff means65536, never division by zero. No seed, PRNG, mirror clock, cached
result, second path, interface or persistent state is added. The actual
execution-owner MMIO callback uses the existing runtime-safe timeline getter,
not the paused-only elapsed-time/debug observation interface. Inspection is
non-advancing, not synchronization with an independently running thread.

Maximum reads remain refused by the selected map. Result writes remain
refused; a pending command still blocks configuration changes. Reset clears
the maximum through the existing register store. Remaining450/6fc unmodelled
groups and unknown commands retain explicit refusal. Initial macro coverage
remains18/22: RNG is not an additional uploaded macro or all of B07/B08.

## Existing-Test Reuse And Independent Proof

Only the existing Board unit executable changes. Its existing Video IRQ loop
already advances the real Core owner to independently asserted ticks245760,
534528 and823296. Fifteen literal RNG word vectors cover maximum65535,256,
250,0,1 at those three edges. A small assertion helper checks eight byte
addresses, four word aliases and three odd-address cross-word reads, normal
and byte inspection, plus unchanged Core time across the entire observation.
No duplicate clock driver, private Core time advance or second Video fixture
is introduced. The existing IRQ delivery/reset assertions remain in place.

A separate case in that same executable runs two guest NOPs, then actual
`MOV AX,[5a4]` and `MOV [3000],AX` while Core is RUNNING. With maximum65535,
the saved word must equal the low16 of the independently captured pre-load
timeline. This detects use of a paused-only API in the MMIO path. It also
checks merged maximum bytes, write-only configuration, all result byte/word
write refusals, pending-command configuration refusal and reset at nonzero
guest time. No algorithm is called from the test oracle. Existing command,
numeric/collision/transfer and failure-stage tables are unchanged; only the
now-modelled RNG range leaves the obsolete unmodelled-range assertion.

Actual production delta: +18/-1, net+17 in core/cop.c. Actual test delta:
+121/-4, net+117 in unit/core/board_bus_smoke.c. One new private assertion
helper and one fixture case are necessary; no production helper, state or
public interface is added. Initial estimate was source+20--40 and test+80--140.
No Shared source/test/build/manifest change or affected existing release link
exists. Ten published EXEs are therefore retained, not timestamp-rebuilt.

## Archived Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | MyRaidenII M1 T3 S21; sequential coordinator/executor |
| Admission And Approval | Owner delegates ordinary same-T ordering; S20 confirms existing Core timeline, no Shared request |
| Objective | Complete four RNG aliases and byte views using the existing maximum and one guest timeline |
| Non-goals | No DMA/sort, Video latch, residual macro, Shared edit, new clock/state or playable-game claim |
| Reference Baseline | Clean pushed6d7fad708 and S20's fixed research identities |
| Candidate Proposal | proposals/m1-board-cop.md; B07 RNG receiver, not all remaining B07/B08 |
| Files And ABI Surface | Local cop.c and existing board_bus_smoke.c plus product records; no public ABI; estimated source+20--40/test+80--140 |
| Applicable Rules | Product guide/source policy/Architecture/Coding; shared Execution/Architecture/Coding/Document/Contributing; architecture/coding governance skills |
| Verification | Strict cached x64/x86 build, actual Board unit, full units, protected hashes, documentation/diff review |
| Expected Markers | Complete suites pass; independent edge vectors, aliases/bytes, actual guest MMIO, reset/read-only constraints |
| Asset Needs | No import/frontend/new affected EXE; preserve ten release identities and protected inputs |
| Reporting Requirements | Actual delta, exact qualification/identities, explicit nominal L2 and unverified boundaries |
| Stop Conditions | Shared/API need, unknown ownership, protected-input mutation or contract contradiction |
| Exit Criteria | Complete RNG family/proof; coordinator actual-change review, full committed/pushed P, clean worktree |
| Original Owner Request | Continue admitted MyRaidenII implementation with cohesive batches and existing-test reuse |
| Similar-Issue Sweep | Four aliases, byte lanes, widened divisor, non-advance, live execution-owner read and unchanged unresolved refusals |

Executor confirmed this packet in Current before source changes. Architecture
and coding governance constrain this to the existing register/bus/time owners.

## Qualification

Strict cached x64/x86 build-unit-tests builds pass. Earlier focused dual-width
runs passed before the final strengthened nonzero-time reset assertion;
the final whole affected unit passes inside both complete repository suites.
An additional x64 focused run also passes with that assertion. Final full
qualification is687/687 per width, zero failures/skips: x64 387.69s, x86
171.12s. Both use the existing aggregate,4 workers and600-second budget,
sequential architectures; neither aggregate is restarted. A read-only
process check identified a sibling SoftPC qualification; it was untouched.
The additional brief x64 focused invocation overlapped the active aggregate
after checking the strengthened assertion; it is not counted as an additional
full run or a replacement for the final full-suite pass.

Final native CTest LastTest.log SHA-256:

| Width | SHA-256 |
| --- | --- |
| x64 | CCAFA79D925C6347DFBD539E73C7CB6B91062D7413404BB5884C5920F2654F4F |
| x86 | F6274295B85C3541F8C3A9CE9344791E2138B09875098BB77CBF3308D4F1D8E2 |

Subsequent builds report no work to do in both stable Ninja/ccache trees.
Documentation governance and diff whitespace checks pass. Ten release
identities,33 tracked asset guards and six protected media-role hashes are
unchanged before/after qualification. All owned executions are terminal;
build contains only persistent .ccache/.ninja roots. No new temporary fixture
or task output requires cleanup. No manifest owns either local changed file.

Frozen source SHA-256:
4BB0F3F3115D2BB5FFED04FE39AA104361DC925145EEBA2346B144452913887E.
Frozen test SHA-256:
F0A639D221049AB89F4CD59E43461A7BFACBD928FA16D0A304F369519BB6EB1A.

No rendering, sound, real-set startup, external integration, desktop/RDP or
playable-game validation is claimed. DMA banking/transfer/sort, Video latches,
2a05 and unresolved macro evidence remain mandatory T3 receivers; T3/M1 stay
open after this bounded local receiver.

## Coordinator Acceptance

Sequential coordinator reviewed the actual six-file P1 bede72c2a, including
all source/test changes and the complete records, rather than accepting the
executor summary alone. Delivery is+298/-6 overall; source/test deltas remain
exactly those above. The runtime-safe clock boundary, widened divisor,
existing busy/reset owner and every old literal failure table are retained.
No new Shared edge, public ABI, persistent state, second time path, platform
dependency or unrelated formatting/import is present. Documentation and
whitespace checks pass; the frozen qualification is unchanged by this
records-only acceptance. S21 closes, not T3/M1 or the playable-game goal.
