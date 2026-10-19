# T3 S11: COP State-Update Family

## Frozen Batch And Source Grade

Baseline is accepted S10 at0a38df8d6. S9's entire state-update batch contains
0205,0905,2a05, all uploaded but not executed in that strict prefix. This S
reconciles all three; it does not silently relabel two implementations as a
complete programmable COP or selected-game qualification.

Fresh read-only research pins MAME commit
c1425a1af00b88aa07477b1b23a30ed5e5f856ff. Exact unit identities:

- src/mame/seibu/seibucop.cpp:
  7E9763D7D1026E82473F135AEE26317DFD8E930C5F1AC257780CD813484A75A9.
- src/mame/seibu/seibucop_cmd.ipp:
  B1E84AEF5BFDEBCA53D8F0AF0B1BE481B8218157BA460C1714853D322AC92EFC.

The [pinned reference](https://github.com/mamedev/mame/tree/c1425a1af00b88aa07477b1b23a30ed5e5f856ff/src/mame/seibu)
is research-only. No reference body, dispatch table, upload words or fingerprint
is copied/transliterated. The following mathematical/register facts define an
original L2 macro contract, not physical COP microinstructions or latency.

| ID | Before disposition | Exact S11 disposition |
| --- | --- | --- |
| 0205 | Rejected; runtime upload recorded | L2 fixed-point position update below, subject to direct bus proof. |
| 0905 | Rejected; runtime upload recorded | L2 velocity update below, subject to direct bus proof. |
| 2a05 | Rejected; runtime upload recorded | Retain explicit rejection. Same-T B07 semantic-reconciliation receiver must establish write width and neighbour preservation before admission. |

The third reference operation reads16-bit values but writes32 bits at two
destinations, including an offset in the middle of a32-bit field. This can
replace a neighbouring word with zero or a carry instead of preserving it.
Neither a silicon/source specification nor an
independent observation presently resolves that width. It is not changed to
the more convenient16-bit interpretation and is not copied as a presumed
hardware32-bit operation. T3 cannot close with this member unresolved.

## Independent L2 Contract Before Code

Let B be COP address register0 and s=(command_address-0x500)/2, with s=0,1,2.
Fields are little-endian. Runtime complete upload/admission and busy/reset
rules remain S10's; 0904 and other unadmitted variants still reject.

- 0205 reads32-bit P at B+4+4s and32-bit V at B+16+4s. N=(P+V) modulo2^32
  replaces P. It then reads16-bit Q at B+30+4s and writes
  Q+((N>>16)-(P>>16)) modulo2^16. The modular delta gives identical16-bit
  results at positive/negative/carry/sign boundaries without C signed overflow.
- 0905 reads32-bit V at B+16+4s and32-bit A at B+40+4s, then replaces V with
  (V+A) modulo2^32. No subtraction variant or extra result/status is invented.

The sole existing post-command Core safe point performs public DMA word cycles.
One local bounded LE memory helper will own checked base/displacement addition
and the two word lanes of a dword, and replace the old copy-only lane loops.
0205 orders P read, V read, N write, Q read, Q write;0905 reads both inputs
before its write. Failure stops there and keeps accepted writes, without
compensation. Address arithmetic must fail before wrap; no host pointer alias,
paused operational read, second clock or mirrored fault state is introduced.
One pending command-slot value is real COP request data, reset with the command.

Independent arithmetic examples:0205 P=0001ffff,V=1,Q=7 gives N=00020000,Q=8;
P=00020000,V=ffffffff,Q=7 gives N=0001ffff,Q=6;
P=ffff8000,V=00010000,Q=ffff gives N=00008000,Q=0.
0905 ffffffff+2 gives1;80000000+80000000 gives0. Test expected constants
must not invoke the production evaluator. Actual guest command writes cover
all slots, source high bits, reset/admission and accepted failure prefixes.

Refined pre-code estimate: existing local production +100--140/-20--40;
existing Board unit +120--200/-0--20. No public API, Shared edit, new test
executable or product artifact is needed. Full final dual-width repository
units, unchanged release identities, actual diff and document checks precede
acceptance. Real-set startup, the remaining COP families and gameplay stay open.

## Implementation And Actual-Diff Review

The original implementation adds one private pending slot, reset with the
existing command state. It introduces no public header/API, queue, clock,
thread or fault mirror. The existing service point/admission remains unique.
Two cohesive local helpers own the bounded word/dword memory transport and
shared update operation. Old copy-only lane loops are removed; all five
admitted macro commands now use the same checked Core DMA lane owner.

Core's public cycle transfers a typed lib_u16 word, not a raw byte-buffer
pointer. Strict compilation caught that distinction during development; the
helper now uses the declared word value and original shifts for dword lanes.
No cast, ABI change or copied reference memory routine bypasses the contract.
All arithmetic uses unsigned modulo behavior; no host signed-overflow result
or guessed physical cost enters the implementation.

The existing Board test adds25 data rows, expanding to41 actual guest command
cases: eight independent arithmetic rows in each of three slots, plus17
failure/refusal rows. These cover both position and increment read lanes,
the auxiliary read, both output word failures, auxiliary write failure,
wide-address rejection/overflow and continued2a05 refusal. Real write-only
COP/GPR boundaries cause failures without a new injection framework or a
second machine fixture. Expected values are independent literal constants,
not calls into the production evaluator.

The refused second word retains the accepted first word; auxiliary failure
retains the already-written position and previous auxiliary value. A nonzero
adjacent-word sentinel proves that normal auxiliary output is16 bits, not an
accidental32-bit overwrite. Existing S10 overlap/read-before-write/upload/
reset/busy/high-address/fault-budget checks remain unchanged and pass.

The source-policy, ownership/naming/Lib-vocabulary and repeated-path sweeps
retain one COP operation owner and one Core fault/timeline route. The current
architecture's stale partial-S49 paragraph is corrected to accepted T2's
qualification; no historical evidence or product UX is rewritten.

Actual git diff --numstat against0a38df8d6: existing COP source +62/-18/net44;
existing Board test +131/-1/net130. This is below the production estimate
because shared transport replaces the previous loops. The test delta owns
the arithmetic/failure matrix in the same executable, not a growing set of
per-command binaries. No Shared source/test/manifest or other App changes.

The three-member ledger after implementation is explicit:0205/0905 have L2
models with direct synthetic bus proof;2a05 is non-eligible pending same-T
write-width/neighbor-preservation evidence. It still faults even after a full
synthetic upload. Original numeric/geometric/collision/DMA/unresolved forms
and selected-set startup remain T3 requirements, not waived by this batch.

## Verification Record

Both focused strict builds and the existing Board unit pass. The final x64
full repository unit run passes687/687 with zero failures/skips in106.12s.
Its LastTest log SHA-256 is
2BF4321D2470EC0FC9FEF17402E48085017E066094B807E0A6AE593C015E234E.
The final x86 full run passes687/687 with zero failures/skips in126.27s.
Its LastTest log SHA-256 is
9AF58FAB7BE48510E443128AFF91700F89BBE8F8342B78FECE6B3F932D9B271B.
Both use the original300-second aggregate budget and four CTest workers;
no timeout or observation delay is interpreted as successful qualification.

Final source/test identities at that frozen build are respectively
6DCC1436F8D79206339E74EC54F7952640E112E1165FD3165949A526E5204884 and
E6E88419C65C6A869264FC6F4A3E7F03ABE7F73EFDAE578DC4E54A624119B2FF.
All ten release identities match S10's committed result record. No MyRaidenII
Product EXE exists yet; local COP changes do not link into those ten products,
so they are correctly retained rather than needlessly refreshed.

Research used a read-only response from the pinned source, with no source,
ROM lane, trace or probe file persisted under build. No ROM/game execution,
external integration suite, desktop/RDP or physical COP timing is claimed.

All ten release hashes,33 protected tracked inputs and six selected external
media-role hashes remain unchanged. Both graphs retain their original default
configuration and caches. Every owned build/test handle is terminal; no
disposable S11 root was created and build contains only .ccache/.ninja.
Documentation governance and git diff --check pass. Actual source/header/test/
doc review, rather than a green structural gate alone, supplies executor review.

## Coordinator Acceptance

Sequential coordinator review accepts pushed P1 b977bfa48 after inspecting
the actual COP/test/architecture/evidence/Current diff against the packet.
The entire three-ID batch has an explicit disposition, not a substituted
two-ID completion claim. The literal arithmetic oracles and real failure
routes prove implemented effects, prefix retention, widths and slots. Existing
copy/reset/admission/fault behavior remains on the same production path.

Final dual-width full units, hashes, unchanged other-product/Shared/artifact
scope and source restrictions match the delivery. The two new private helpers
own real shared memory/update work; the sole extra private slot is request
data, not a lifecycle mirror. No new test binary, framework, public API or
reference implementation is introduced. Document gates and whitespace checks
pass; cleanup needs no destructive action because no temporary root was made.
S11 closes with2a05 and the remaining full B07/B08/startup ledger still in T3.
No external-ROM, physical-time, desktop/RDP or playable-game claim is made.

## Archived S11 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: MyRaidenII M1 T3 S11, after accepted S10; one session performs coordinator then executor roles. |
| Admission And Approval | Owner's standing MyRaidenII implementation approval and delegated same-T S ordering. Only MyRaidenII source/tests/records are admitted; Shared and other products remain unchanged. |
| Objective | Reconcile the complete three-ID state-update family 0205/0905/2a05 from S9's ledger; freeze exact L2 arithmetic, slot, bus-prefix/reset contracts and independent oracles before implementation. Implement only evidenced forms; explicitly retain any underdetermined member with its same-T semantic receiver, never as a no-op. |
| Non-goals | No reference code/table import, game-upload fingerprint, generic COP interpreter, physical latency claim, Shared/API change, other numeric/collision/DMA families or game qualification. |
| Reference Baseline | Clean pushed 0a38df8d6; S9 initial trigger ledger, S10 upload/transfer path and exact-fault publication; pinned reference commit c1425a1af00b88aa07477b1b23a30ed5e5f856ff for factual L2 navigation only. |
| Candidate Proposal | ../proposals/m1-board-cop.md; ../etc/evidence/t3-s1-board-contract.md B07/B08; ../etc/evidence/t3-s9-cop-initialization-protocol.md. |
| Files And ABI Surface | Existing local cop.c and existing Board unit, Current/history/evidence index and one supporting contract/result record; directly stale V30 qualification paragraph in the product architecture. No public interface or other App edits. Pre-code estimate production +100--160/-20--50, tests +120--200/-0--20; documentation approximately +100--160. Refine after contract review and before code. |
| Applicable Rules | Shared Execution complete family dispositions/actual review/P lifecycle; architecture sole COP state and Core timeline/transaction owners; coding existing helpers and Lib vocabulary/no duplicate read-write path; source policy no copied/transliterated reference implementation/table; documentation authority and archived packet boundaries. |
| Verification | Freshly pin reference identities and inspect actual dispatch/word-width semantics; independent modulo arithmetic vectors, all three command slots, carry/negative/wrap/overflow, actual guest writes, reset/busy, every accepted read/write failure prefix. Extend the existing unit, not per-command executables. Both stable graphs run complete units; check source/test/artifact scope, docs, whitespace and cleanup. Optional real-ROM observation is diagnostic only. |
| Expected Markers | Exact admitted operations use current uploaded-trigger admission and existing post-commit service; unsigned modular arithmetic avoids host signed overflow; failures remain actual Core faults. No guessed success for 2a05 or any other unresolved ID. |
| Asset Needs | Owned build/myraidenii-t3-s11 only if probes/downloads/logs are needed, 64MiB/5-minute no-progress/90-minute phase budgets. Research stays outside tracked source and is discarded after identity/result archival. External ROM/media read-only; no runnable MyRaidenII EXE yet. Existing ten releases remain unchanged because this local-only batch cannot affect their links. |
| Reporting Requirements | Report exact family disposition, code ownership and refined estimates before implementation; preserve source/model grade and independent oracle method, actual delta/full-unit results, unverified real-set limits and cleanup; target-scoped complete P then coordinator review. |
| Stop Conditions | Shared/API change, source/license import requirement, copied program/body/table, unsupported silicon precision or unrelated product edits. Underdefined member remains explicit rejection and gets a named T3 receiver; do not silently shrink the family or completion predicate. |
| Exit Criteria | Every member of the frozen three-ID batch has an evidenced accepted model/proof or an explicit non-eligible disposition and receiving requirement; implemented forms have full bus/failure/reset proof, both complete unit suites pass, actual diff/ledger review and cleanup are complete and scoped P commits are pushed. T3 remains open. |
| Original Owner Request | Implement MyRaidenII fully under single-session dual-role governance, grouping coherent work and reusing existing tests instead of proliferating fixtures or executables. |
| Similar-Issue Sweep | Reconcile all three numeric-update IDs and command offsets; use the same bounded LE word/dword transfer owner across existing copy and new updates. Preserve S10 overlap, partial-write, high-address and fault-budget regressions; retain other operation families explicitly unqualified. |
