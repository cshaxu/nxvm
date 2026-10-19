# T3 S13: Complete Collision Receiver

## Frozen Four-Command L2 Contract

Baseline70b98a367; selected batch is exactly a100/a900/b100/b900 from S9.
All four initially reject. This is an original local macro model, not a
microinstruction interpreter, copied reference implementation or physical
COP timing claim. Fixed research commit is
c1425a1af00b88aa07477b1b23a30ed5e5f856ff. Source SHA-256 identities:
seibucop.cpp7E9763D7D1026E82473F135AEE26317DFD8E930C5F1AC257780CD813484A75A9,
seibucop.h69857872F1E68D807442FA6018562A29390ACE4E0D66ECF5D86ABE080829E962,
seibucop_cmd.ippB1E84AEF5BFDEBCA53D8F0AF0B1BE481B8218157BA460C1714853D322AC92EFC,
raiden2.cppFDDE57F6E9024657F2A9830864523305139562229750B87C967E597324CDB26A.
Fresh read-only responses confirm the declarations; none are persisted/imported.

| Command | Complete selected-form behavior |
| --- | --- |
| a100 | Consume object0 flags word at GPR0+2, then latch three signed16 positions at GPR0+6,+10,+14. This exact form does not flip intervals even when flag bits are set. |
| a900 | Same position capture for object1 from GPR1. |
| b100 | Read pointer word at GPR2, combine it with high-base word436h; capture three offset/extent byte pairs and update object0 bounds, then collision results. |
| b900 | Same hit-box update for object1 from GPR3. |

Each offset is signed8 and each extent unsigned8. Each lower/upper endpoint
is a signed16 word: lower=wrap16(position+offset), upper=wrap16(lower+extent).
The result's bit i is set when the intervals fail the strict endpoint predicate
upper0>lower1 AND lower0<upper1. Exact touching therefore separates; a zero-size
point strictly inside a nonempty interval satisfies the stated predicate.
No silent sorting, clamping or mathematical-empty-interval reinterpretation
changes the word-level L2 model. Differences position0-position1 publish as
three modulo16 words. Both b forms select three axes;2-axis/flipping alternate
IDs remain unsupported, not silently redirected to these four forms.

Position latches and evaluated interval latches have distinct command-time
meaning. A later position load does not move previously evaluated bounds
until its b command; preserve both without storing redundant offset/size copies.
All candidate input words are read before replacing that command's local latch
or completed result. Failure stops with old completed state retained; it
reaches S10's existing Core fault path, not a shadow ready/error flag.

COP owns read-only580h status and582h/584h/586h differences.588h is the same
three-bit L2 status view, not a separately mutable mirrored field; unknown
extra hardware bits are not invented. Reads, including unaligned byte reads,
are non-mutating, writes reject, reset clears slot/bounds/result state and
invalidates uploaded commands. Only the existing register store is enlarged
to the actual new result capacity. Private explicit range roles and one
command-kind classifier align admission/service without a generic framework.
All bus work uses existing Core DMA word cycles at the existing safe point;
three offset/extent pairs are L2 word transactions on the selected little-endian
V30 route, not a claim of recovered physical byte-cycle timing.

## Before-Code Proof Plan

Independent interval cases cover overlap, each separated axis/all axes, exact
touching, negative offsets, unsigned255 extent, zero-size interior/boundary,
negative coordinates and signed16 endpoint wrap. Literal expected masks and
modulo16 differences do not invoke a production evaluator. Both command orders
and all three command ports must produce the same selected-form result.

Actual guest command sequences cover high hit-base addressing, separate
position/bounds latching, read-only result alias and byte order, every position
flags/axis and hit-pointer/data-word read failure, retained old result, reset,
busy/incomplete-upload and rejection of alternate forms. Reuse the existing
Board fixture and factor only its repeated guest-command assembly if useful;
no per-opcode test executable or general fault-injection framework.

Production estimate +140--220/-20--40; existing tests +170--250/-10--30.
Positive code owns the actual device latches/results and independent proof.
Full final dual-width unit suites and unchanged ten-release/input identities
are required. No Shared math alias or geometry implementation is admitted.

## Explicit Correction To The S12 Candidate Contract

Fresh inspection of the command-dispatch prelude, not just its handlers,
finds that admitted command dispatch clears arithmetic status bit8000 before
per-operation work. Consequently S12's claim that successful2208 preserves
that prior error bit is wrong as a complete command statement: the handler
preserves other bits, but the common entry has already cleared bit15.
Every one of the nine future numeric rows must apply this common prefix;
operand failure may retain that already-accepted clearing. The geometry
receiver must own it once at service dispatch, not patch2208 alone.

No arithmetic status register exists in current production, so S13 does not
add fictional geometry state just to clear it. This correction supersedes
the affected S12 candidate statements and failure-order wording. The pending
four-alias/six-file Shared review request is unchanged and remains unapproved.

## Implementation And Similar-Issue Review

The whole four-ID batch now has one command classification for admission and
service, one existing Core DMA word-transport owner and the original Core
safe point. No alternate interpreter, clock, queue, API or reference program
fingerprint is introduced. The old transfer/update branch now shares the same
pending-command retirement rather than returning through a second clear site.

Private position/bounds latches contain36 bytes: two objects, three signed-word
positions and two three-axis interval endpoints. These represent different
command-time facts, not a mirrored live memory view. Offset/size data are not
retained after interval capture. The existing register store grows130 bytes
through587h;588h/589h read the authoritative580h/581h bytes without storage.
Explicit range roles replace the old command boolean and duplicated GPR range
condition, so result read/write policy has the same register-map owner.

Five new private helpers own command classification, defined signed-word
conversion, atomic position capture, LE result encoding and atomic box/result
capture. All have real reused preparation or publication responsibilities;
there are no forwarding-only helpers. Signed arithmetic stays in representable
32-bit intermediates; conversion first narrows modulo16 then subtracts65536
before casting to signed16. Touching, point-interior and wrapped intervals use
the frozen literal contract rather than a host geometry library.

The existing Board test gains two bounded local helpers, not a new executable
or framework: actual guest command emission and public result-byte inspection.
Fourteen literal interval vectors cover overlap, individual/all separation,
touching, negative coordinates/offsets, unsigned255 extent, zero-size interior
and boundary, and signed endpoint wrap. Alternating load orders and all three
ports are exercised. The high-base case uses14000h/14010h rather than the low
4000h/4010h addresses. All flags are set to prove these selected forms do not
acquire alternate-form flipping semantics.

Seventeen failure rows first prime a valid completed result, then fail each
slot's flags/position read, pointer/data-word read and one high-address input.
They use actual write-only/unmapped bus routes, not a mocked COP implementation;
all five completed result words remain intact and the Core reports a fault.
Byte, odd-word, read-only alias, separate position/bounds latching, result reset,
latch reset and rejected alternate-ID checks are in that same executable.
Existing busy/incomplete-upload tests remain shared admission proof; old
transfer/update/route/IRQ/video cases and their assertions are unchanged.

Tracked source/test line count by git diff --numstat against70b98a367:
production159 added/34 removed/net125; test191 added/1 removed/net190.
Production and test additions fit the recorded estimates. Tests remove only
one line rather than the estimated10--30: existing single-command/HALT
regressions remain intact, while the new bounded multi-command sequences stop
before HALT so subsequent latch operations can run. Positive code implements
new hardware effects and independent proof; no old collision implementation is kept.
No Shared, other App, build, public header, manifest or artifact path changes.
MyRaidenII-local source/tests have no separate manifest to regenerate.

## Verification Containment Revision

Both strict focused builds and Board runs pass. The first complete x64
aggregate attempt was stopped by its existing300-second overall deadline
after503/687 completions; it is not a complete-suite pass. No per-test failure
or runtime root cause is inferred from that cutoff. Coordinator authorizes
the existing runner's600-second aggregate parameter for this S's final runs,
retaining four workers, every original test and all per-test assertions/timeouts.
This is a run-budget clarification inside the90-minute phase, not a production,
test, build-configuration or Shared policy modification. The final runs must
still report687/687 with no failure/skip before acceptance.

## Final Frozen-Source Qualification

The final x64 full unit run passes687/687, zero failures/skips, in146.13s.
Its LastTest log SHA-256 is
E3AD4750B293A285EE2CEBF6D7768C52821C3B419C31B5AC79CD628721FAFBA1.
The earlier incomplete300-second attempt is explicitly excluded from this pass.
The original x86 graph's build-unit-tests target reports no work to do after
the strict focused build; all intermediate products are reused, not copied
to a task-numbered tree.

The final x86 full unit run passes687/687, zero failures/skips, in148.06s.
Its LastTest log SHA-256 is
B808AF2AFBD1FDFEF015DC767B340E1ADA4AC9DF081A368F0CACCB6CF5232AE0.
Both runs include the existing Shared, PC, MyNES, MyRaidenII and repository
unit labels. All actual owned test/build handles are terminal. No temporary
S13 root, research download, copied media or probe was created; build contains
only .ccache/.ninja. No cleanup deletion is needed. Documentation governance
and git diff --check pass. These structural gates supplement, not replace,
the actual source/test/packet review.

Frozen production SHA-256:
C00B8D2453E7294315DEECAAD6BF4941729F0BE785D1DA3654F1FE4FAEF5F0B8.
Frozen Board test SHA-256:
AD897972D38592578956C781D77CACDEAAC35B0BB9431566E4E0B115EF51900E.
All ten released EXE hashes still match S10's exact result record;33 tracked
protected inputs and six selected external media roles match the S49 input
record. MyRaidenII has no Product EXE yet and its local Core is not linked into
those ten products, so no unrelated artifact is rebuilt or relabelled.

No real ROM run, complete external integration suite, physical COP timing,
desktop/RDP, rendering, sound or playable game is qualified by this S. The
four modeled IDs advance S9's initial22-ID ledger from five implemented forms
to nine;2a05, the nine numeric forms, three unresolved IDs and further DMA/
result/startup requirements retain their mandatory T3 receivers. The22-ID
initialization list is still not the full game's command universe.

## Archived S13 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: MyRaidenII M1 T3 S13 after accepted S12; one session performs coordinator/executor roles sequentially. |
| Admission And Approval | Owner's standing implementation approval and delegated same-T S ordering. MyRaidenII-local collision implementation and direct S12 contract correction only; pending Shared math proposal is not approved or bypassed. |
| Objective | Consume the whole four-ID collision batch a100/a900/b100/b900 from S9: actual object/box capture, three-axis signed-word L2 interval comparison, copied read-only results and complete failure/reset/admission proof. Correct S12's missed common command-entry error-bit clearing before any geometric implementation. |
| Non-goals | No Shared/API/math change, geometry implementation, reference body/table import, game fingerprint/bypass, generic collision framework, hardware-exact latency or gameplay qualification. Other B07/B08/DMA/startup obligations stay open. |
| Reference Baseline | Clean pushed70b98a367; actual S11 COP memory/safe-point/run-fault path, S9 four uploaded IDs, fixed MAME c1425a1af00b88aa07477b1b23a30ed5e5f856ff as L2 facts only. |
| Candidate Proposal | ../proposals/m1-board-cop.md; ../etc/evidence/t3-s1-board-contract.md B07/B08; S9 initial trigger ledger and S12 numeric receiver for the specific contract correction. |
| Files And ABI Surface | Existing local cop.c and existing Board unit; Current/history/evidence index, one collision record and explicit appended correction to S12 record. No public header/layout change. Production estimate +140--220/-20--40; existing test +170--250/-10--30, refine before material expansion. |
| Applicable Rules | Execution entire finite-batch disposition and actual review/P lifecycle; Architecture one position/bounds/result owner, Core bus/time/fault owners and no mirrored alias state; Coding private cohesive helpers, Lib vocabulary and existing test reuse; source policy no implementation/table copy; documentation explicit correction, not silent historical rewrite. |
| Verification | Fresh source hashes/width/byte-order/three-axis/strict-edge facts; independent boxes for overlap/separation/touching/zero extents/negative offsets/signed wrap and both load orders. Actual guest command sequences test position and bounds latch separation, high hit-base pointers, result aliases/bytes/read-only behavior, input failure prefixes, reset/busy/unknown forms. Both original stable graphs run complete units. Source/test/artifact scope, docs/whitespace and cleanup proof. |
| Expected Markers | Four exact macros share one admission/classification, one existing service point and public DMA path. Results580/582/584/586/588 are owned and side-effect-free;580/588 have one L2 authoritative value. Input read failure preserves prior completed device result; no copied memory pointer, extra queue/clock or fake success. |
| Asset Needs | Synthetic ROM-free vectors and read-only fixed-source responses. Owned build/myraidenii-t3-s13 only if logs/probes are needed,64MiB/5-minute no-progress/90-minute phase budgets. No MyRaidenII EXE exists yet; ten unrelated release identities and protected inputs remain unchanged. |
| Reporting Requirements | Freeze exact source/model/width/order rules and final estimate before code; report actual source/test delta, reused/removed paths and state, complete dual-width results, input/release hashes, qualification limits and cleanup. Complete implementation P then actual coordinator review and governance P. |
| Stop Conditions | Unapproved Shared/public API change, source/license import, protected output, per-game condition, invented physical precision or unrelated App edit. Discovered numeric prelude correction is recorded, not implemented as geometry state in this collision batch. |
| Exit Criteria | Whole four-command collision/result path has direct independent actual-bus proof with complete failure/reset/latch cases and explicit L2 limits; old transfer/update regressions and full dual-width units pass. Actual diff/class ledger review, artifact/input scope, clean temporary lifecycle and pushed P delivery precede acceptance. T3 remains open. |
| Original Owner Request | Complete MyRaidenII under single-session dual-role governance; efficiently group coherent work and reuse existing tests while public Shared edits require explicit review. |
| Similar-Issue Sweep | All four macros, both object slots, three axes, flags read/non-flipping selected forms, hit-pointer high word, signed16 interval publication and result aliases share ownership. Reconcile the common command prelude across all nine geometric rows; do not infer handler-local status behavior in isolation. |
