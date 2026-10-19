# T3 S17: Integer Angle-Approach Memory Effects

## Complete Numeric Ledger And Dependency Partition

Baseline304881946. The original nine-operation S12 universe remains fixed.
S17 is an implementation-order partition, not a smaller T3 completion goal.
Unlike angle/distance/trigonometric evaluation,6200's guest-memory effect
requires no host math vocabulary or arithmetic-result input. Implement it
through the existing COP owner while the separately reviewed Shared math
proposal remains pending. Do not approximate other rows to evade that review.

| Original ID | Before S17 | S17 disposition and required receiver |
| --- | --- | --- |
| 130e | Rejected | Still rejected; original angle/result/math receiver. |
| 2208 | Rejected | Still rejected; original word-angle/result/math receiver. |
| 338e | Rejected | Still rejected; original angle/byte-output/result/math receiver. |
| 3bb0 | Rejected | Still rejected; original distance/result/math receiver. |
| 42c2 | Rejected | Still rejected; original distance/scale/division/result chain. |
| 4aa0 | Rejected | Still rejected; original inverse division/result chain. |
| 6200 | Rejected, target/step writes refused | Integer circular-memory effect and target/step configuration implemented as L2; common arithmetic-status/result qualification remains with the complete numeric receiver. |
| 8100 | Rejected | Still rejected; original sine/scale/result/math receiver. |
| 8900 | Rejected | Still rejected; original cosine/scale/result/math receiver. |

Every row remains inside T3. In particular6200 is not declared silicon-exact
or fully qualified across the global numeric result surface.5b0h--5b5h
remain explicitly unmodelled and refused, not zero-valued fake results.
The common status-prefix correction from S13 still applies to the later
complete receiver. This batch neither fabricates those status bits nor
drops that obligation.2a05, other unresolved forms, RNG/DMA/sort and genuine
startup also keep their original receivers. S9's uploaded list is not the
entire playable-game universe.

## Research Grade And Independent Model

Fresh read-only [MAME research](https://github.com/mamedev/mame/blob/c1425a1af00b88aa07477b1b23a30ed5e5f856ff/src/mame/seibu/seibucop_cmd.ipp) uses fixed commit
c1425a1af00b88aa07477b1b23a30ed5e5f856ff, seibucop_cmd.ipp SHA-256
B1E84AEF5BFDEBCA53D8F0AF0B1BE481B8218157BA460C1714853D322AC92EFC.
Current selected-form and legacy other-game handlers were distinguished.
The legacy B1/0ch variant is not a second route in this product. Research
supplies L2 operand/ordering facts only; no source body, lookup table,
dispatch/upload table, protected data or reference dependency is imported.
The first local text locator matched both names; correcting the locator
did not change the fixed source or any implementation fact.

Selected facts: use B0, angle byte at B0+34h and flags word at B0; effective
target/step are the low bytes of41ch/41eh. Turn on the256-angle circle by
at most step along the shortest direction, choosing increment on a128-unit
tie. Clear flag4 until the target is reached, then set it; preserve every
other flags bit. Publish flags before the single angle byte. The physical
bus timing, uploaded microcode interpretation and other hosts' word-output
variants remain unqualified.

The local algorithm is independently formulated using forward distance
(target+256-current) modulo256 and its shorter complement. It does not
transliterate the reference's signed-delta branch structure. Values are
bounded unsigned arithmetic; no signed wrap, negative shift, host math,
lookup approximation or new evaluator is required. Literal independent
oracles include10->13 toward20 by3,20->17 toward10, both128 ties incrementing,
255->0,0->255, and the repeated250->254->2->5 sequence.

## One Configuration, Memory And Safe-Point Owner

The existing41ch--41fh descriptor becomes the existing write-only CONFIG
role. Low-byte interpretation is local to the operation; raw register bytes
remain in their sole existing store. No target/step mirror, mutable object
layout, storage extension, new public header, readiness flag or extra queue
is added. Existing pending-command admission prevents configuration writes
while a command is in flight. Reset clears the same store and upload state.

One new private command kind and handler reuse the existing admission,
copied pending command and Core attachment safe point. All three command
ports select B0 for this exact form, not a slot-derived object. The existing
memory helper gains true1-byte Core DMA cycles alongside its unchanged
2/4-byte paths; reads initialize their local bus lane to zero rather than
carrying an unrelated input word. The sole Core transaction owns bus effects.
Displacement and per-lane physical additions retain checked32-bit bounds.

Order is angle read, flags read, flags write, angle write. Input failure
publishes no output. A flags write may accept its low byte before failing;
no angle write follows that failure. A later angle-write failure retains the
already accepted flags word. There is no rollback, retry or side error state;
the existing COP/Board callback-fault path stops the Machine. Other numeric
result reads remain refused, so incomplete work cannot appear as successful
arithmetic output.

## Reused Regression And Failure Stages

All proof extends the existing Board/COP executable and S16's actual-COP/
neutral-Core fixture. No test target, file, framework, production entry or
parallel memory helper is added. Original transfer/update/collision/decimal,
window, input/sound/IRQ and video assertions remain live.

The circular matrix uses literal expected angles/flags, not a second copy of
the production evaluator. Fourteen positive vectors each run on all three
ports, covering directions/wrap, clamp/equality, old flag clearing, zero/max
step, high configuration bits and an operand at the final installed RAM byte.
Neighbour5ah is retained for ordinary byte outputs. The last-RAM-byte vector
must fail an accidental word read or write, independently of neighbour data.

Failure-stage addresses are explicit:

| B0 | Intended failed stage and retained effect |
| --- | --- |
| 3f8h | Angle42ch is write-only; flags remain unchanged. |
| 434h | Angle468h is readable; flags434h are write-only. The seeded flags/address value is a valid upload address and does not alter the uploaded program. |
| 4a2h | Flags are readable GPR1 high; pending configuration prevents their write. Angle remains unchanged. |
| 4bfh | Flags span ordinary RAM4bfh and readable GPR0 low4c0h; the low flags byte is accepted before the pending GPR byte refuses. Initial high flags bytebfh preserves B0's own address. No angle output follows. |
| 499h | Flags RAM write succeeds; angle4cdh is readable GPR6 high but its pending write refuses. Flags prefix persists and the angle/neighbour remain unchanged. |
| ffffffdfh | B0+34h overflows32-bit physical arithmetic before access. |
| 102000h | Operand lies outside installed RAM; no output. |

The4bfh row was identified during final review after starting the first
x64 full suite. Do not replace its running binary. After that run finishes,
add the row, rebuild and rerun the entire affected unit in both widths, then
run the final x86 full suite. Preserve distinct test identities and do not
claim the earlier x64 full log executed that final added row. Unchanged
unaffected686 binaries need not be rebuilt/rerun for a fixture-only literal.

Actual Board guest commands also prove250->254->2->5 across all ports;
word and byte config writes retain low-byte meaning despite high bits.
Busy config writes reject, reset cancels pending work/invalidates upload,
and a new post-reset zero-target/zero-step command sets flag4 at angle0.
This validates both reset values, not merely visible result clearing.

## Scope, Delta And Qualification

Production is one handler/classifier path and one shared memory helper, no
new mutable state, public API, clock, thread, object manager or Shared file.
All ten existing released products do not link this local Core; their EXEs
and protected inputs must remain unchanged. No MyRaidenII executable exists
yet. The local product source/test roots have no manifest to regenerate.

Final actual numstat, source/test/log hashes, complete run results and final
fixture-row qualification are recorded before the complete implementation P.
Use original Ninja/ccache graphs, strict warnings, existing four-worker unit
runner/600-second aggregate and per-test budgets. No task temporary files
or external payload are retained. Desktop/RDP, ROM startup, integration suite,
physical COP timing and playable-game acceptance are not executed or claimed.

## Actual Review And Executor Qualification

Git diff --numstat against304881946: production cop.c adds36/removes5/net31;
existing Board/COP test adds147/removes3/net144. The source is smaller than
estimated because it needs no additional storage or result-state mechanism.
The test removes fewer lines because every original matrix remains intact.
The source/test total is183 additions/8 removals/net175, with one private
algorithm and no new test target. The final21-row angle matrix has14 positive
rows on each port and7 fault rows; the Board additionally executes the three
wrap steps and the post-reset zero-target case. These are cases in the old
unit, not53 additional registered tests.

Actual old transfer/update/collision table text matches304881946 after line
ending normalization. Sole state remains the existing register bytes/pending
command; sole memory transport remains the same checked Core DMA helper.
Every new arithmetic intermediate is bounded unsigned, all old2/4-byte
callers retain their widths, and failed reads do not publish local results.
Read-only result rejection and non-selected legacy forms are not broadened.
No other product, Shared, public API, CMake, manifest or release input changes.

Production SHA-256 before and after the final fixture row:
E5DBD3746F91B6348F46854EB301C9AEE59F94F9C871E349D51265E526C96DFA.
Initial20-row test SHA-256:
266C635963F81220E0AE2AD2FA7ADA01AF0191BFD67EEABCAAF5F0325CFD8DA1;
final21-row test SHA-256:
AA73EA3FE9C989F03E8C62D68DBCC372DEA6E9EC845CE8FACD626DF76BBDE258.

The x64 full suite passes687/687, zero failures/skips,117.73s, before the
one-row-only final fixture addition. Full LastTest.log SHA-256:
78223368F25796DABFA7EA2DCFB8AA3087ABF8735A0F9C58BFFA954A00E47F7A.
The added4bfh row then passes in a complete affected-unit rerun:1/1,0.22s
test/0.29s aggregate. Final focused log SHA-256:
698993E4661F79CA931B26D0FE7AED4659D0AC533A4290F16C479A4405EB7252.
Both cached builds perform exactly two actions for this final addition,
recompiling/linking only that test; production and the unaffected686 test
binaries are unchanged. This is complete current unit-set qualification by
the full baseline plus final affected-unit proof, not a claim that the old
full run included the new row. No running binary was replaced.

Final x86 focused proof passes1/1,0.21s test/0.26s aggregate, before its final
full suite. Final full results, log hash, protected guards and terminal state
are appended after that owned runner completes.

The final x86 full suite passes687/687, zero failures/skips,129.64s using
the final21-row test. LastTest.log SHA-256:
8294E5DEBD461547B93C2A3F26B73CA7F97BE861E6629875FFEADF471CBC13FF.
Both full runner sessions and final focused tests terminate with exit0.
Production and final test identities remain exactly the values above.
Documentation/whitespace checks pass; post-run ten-release,33-asset and6
media-role guards remain unchanged. Only persistent caches exist in build,
with no owned temporary process or disposable directory. No cleanup deletion
or unaffected release rebuild is required.

## Archived S17 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: MyRaidenII M1 T3 S17 after accepted S16 304881946; one-session sequential coordinator/executor. |
| Admission And Approval | Owner's complete implementation approval and delegated same-T ordering. Only MyRaidenII Core/test/records; no inference of Shared math/API approval. |
| Objective | Consume the independent6200 angle-approach memory-effect receiver from B07/S12: qualify target/step configuration, bounded circular motion, flags/byte output, all command ports and each memory-failure prefix on the sole actual COP/Core bus. |
| Non-goals | No Shared/CRT math change, geometry approximation, remaining eight numeric operations, arithmetic result/status publication, generic uploaded microcode, physical timing, DMA/sort/2a05, ROM/startup/gameplay qualification or new production/test entry. Common arithmetic-result qualification remains with the full numeric receiver. |
| Reference Baseline | Clean pushed304881946; S12 nine-operation dependency ledger and S13 status-prefix correction; pinned MAME c1425a1af00b88aa07477b1b23a30ed5e5f856ff seibucop_cmd.ipp hash B1E84AEF5BFDEBCA53D8F0AF0B1BE481B8218157BA460C1714853D322AC92EFC, factual current/legacy distinctions only. |
| Candidate Proposal | ../proposals/m1-board-cop.md B07/B08; ../etc/evidence/t3-s12-cop-geometry-contract.md complete ledger. Partition its dependency-independent6200 memory effect, not a first-failing ROM repair; preserve all nine rows and whole numeric/result receiving gate. |
| Files And ABI Surface | Existing cop.c, existing Board/COP unit, Current/evidence/index and receiving plan. No public headers/new state. Estimate production+40--65/-4--10; existing test+120--180/-5--15; records+170--240. |
| Applicable Rules | Architecture sole COP config/pending command/Core DMA/safe-point owners, flags-before-angle publication; Coding one1/2/4-byte memory helper, checked arithmetic, low-byte effective config, existing fixture reuse; Source factual L2 contract and independently formulated circular-distance algorithm, no copied body/table; Execution dependency-led full finite batch disposition, exact failure stages, full P/review and cache reuse; Documentation explicit arithmetic-status/source/qualification limits. |
| Verification | Literal circular-motion vectors cover both directions/wrap/ties, clamp/equality/zero/max step, preserve/clear/set flag4, high config masking and adjacent-byte preservation on each of three ports. Repeat250->254->2->5 and reset behavior. Target/step word/byte/upper-bit/busy/read denial use actual Board. Isolated actual COP/Core fixture covers angle read, flags read/write including its accepted low-byte prefix, late angle-write failure with flags prefix, overflow/out-of-range and last-RAM-byte operand, retaining every old matrix. Real Board CPU-issued6200 route also executes. Strict cached x64/x86 builds and focused unit, then all687 units sequentially4 workers/600-second aggregate. If final review adds only a fixture row after a full run, retain that full result plus a final whole affected-unit rerun; distinguish the two test hashes/results and unchanged unaffected686 binaries rather than claim the old full run used the new row. Docs/whitespace/frozen hashes/protected-input and ten-release guards. |
| Expected Markers | One private angle kind/handler, target/step join existing config role; bytes1/2/4 use the same checked DMA helper. No result mirror, direct pointer, host math call or second evaluator. Remaining numeric result/status reads stay explicitly unmodelled. |
| Asset Needs | ROM-free literals only; no disposable directory or external payload retained. Reuse .ninja/.ccache; preserve all ten release identities,33 tracked assets and6 media-role hashes. |
| Reporting Requirements | Before code report dependency partition, source/model choices and estimates. Record whole nine-row before/after, exact effect/failure order, source/test actual delta, dual-width evidence, retained non-eligible obligations and complete P/actual coordinator review. |
| Stop Conditions | Shared/public change, unapproved reference-code derivation, altered protected input, additional clock/state mirror, guessed arithmetic status or claim that this memory-effect receiver qualifies all6200 hardware/global results or the nine-member numeric batch. |
| Exit Criteria | The independent circular-memory effect and complete target/step capability have direct actual-bus proof with stable old matrices, boundary/partial failure and both widths; scoped complete P and coordinator acceptance pushed. Full numeric status/results and eight remaining operations keep their original receiver before T3 closure. |
| Original Owner Request | Implement complete playable MyRaidenII in single-session dual-role governance; same-T S sequencing delegated, Shared modifications require explicit review. |
| Similar-Issue Sweep | Verify byte transport does not overread or overwrite neighbours, all displacement/physical limits and old2/4-byte paths remain unchanged; no signed-overflow or high-byte state mirror; all command slots select B0 for this form; accepted flags prefix survives late failure; source legacy other-game B1/0ch variant is not a second selected-board path. |
