# T3 S12: Complete Geometric/Numeric Receiver

## Frozen Scope And Source Boundary

Baseline is accepted S11 e44e522d2. This is research/design, not a production
or gameplay delivery. The entire S9 geometric batch is exactly these nine
uploaded IDs:130e,2208,338e,3bb0,42c2,4aa0,6200,8100,8900. Upload does not
prove execution, and these are not the entire playable game's command universe.
All nine still reject in the actual production COP. S10/S11's five accepted
transfer/update macros and all other B07/B08 obligations remain unchanged.

Fresh read-only responses use MAME commit
c1425a1af00b88aa07477b1b23a30ed5e5f856ff. Exact SHA-256 identities:

| Research unit | SHA-256 |
| --- | --- |
| raiden2.cpp | FDDE57F6E9024657F2A9830864523305139562229750B87C967E597324CDB26A |
| seibucop.cpp | 7E9763D7D1026E82473F135AEE26317DFD8E930C5F1AC257780CD813484A75A9 |
| seibucop.h | 69857872F1E68D807442FA6018562A29390ACE4E0D66ECF5D86ABE080829E962 |
| seibucop_cmd.ipp | B1E84AEF5BFDEBCA53D8F0AF0B1BE481B8218157BA460C1714853D322AC92EFC |

The [pinned units](https://github.com/mamedev/mame/tree/c1425a1af00b88aa07477b1b23a30ed5e5f856ff/src/mame/seibu)
are LGPL-2.1+ research references, not an import approval. Current and legacy
selectors/handlers were compared; no body, dispatch/upload table, numerical
implementation or game data is copied. Only the factual candidate contracts
below and independently computed mathematical oracles are retained. Source
TODOs and undefined host arithmetic are not physical-hardware specifications.

## Complete Nine-Member Contract Ledger

Let B0/B1 be the existing address registers, D the distance result, S the
two-bit scale and A the angle result. Every row is a proposed explicit L2
macro, not an implemented operation or bit-exact silicon claim.

| ID | Operands and observable candidate result | Required qualification / retained uncertainty |
| --- | --- | --- |
| 130e | Differences of32-bit coordinates at B0/B1+4,+8; angle uses atan(dx/dy) in256 units/turn, with half-turn correction when dy<0. Status is7, or8007 and angle0 when dy=0. No optional guest write for this ID. | Reference delegates to338e while warning that the upload/mask distinction is unresolved. Explicit L2 alias only; qualify quadrants, zero denominator and the32-bit difference rule, not generic uploaded microcode. |
| 2208 | Word operands at B0+12h,+16h; angle from their ratio, no optional guest write for this ID. dy=0 sets status bit8000 and angle0; ordinary completion does not normalize status. | Reference reads zero-extended words despite a negative-denominator branch. Signed16 interpretation is not established; the receiver must explicitly select/document its L2 word interpretation, not silently sign-extend. |
| 338e | Same coordinate/angle candidate as130e, with one optional angle byte written at B0+34h. | Reference explicitly questions byte versus word. Selected V30 L2 byte behavior must preserve the adjacent byte and retain the source-grade limitation; no claim that all COP/host variants use it. |
| 3bb0 | Integer-part coordinate differences; D=floor(sqrt(dx^2+dy^2)); writes a word at B0+3ah for this exact ID. | Current reference re-reads coordinates; a comment says internal latching is expected. Receiver must name live-read L2 versus hardware latch uncertainty. Wide intermediates avoid signed-square overflow; old legacy axis/storage paths are not another implementation. |
| 42c2 | Divisor word at B0+36h. Nonzero: status7 and word floor(D*2^(5-S)/divisor) at B0+38h. Zero: preserve status bits, set8000 and write0. | D is the existing distance-result owner, not recomputed by division. Test complete command chains, all scales, truncation, output width and zero behavior. Other status-bit meanings remain unqualified. |
| 4aa0 | Divisor word at B0+38h; word quotient at B0+36h, status7. Candidate reference L2 replaces divisor0 with1. | Zero policy differs from42c2 and must remain an explicit per-operation contract, not a generic divider accidentally sharing42c2's fault policy. |
| 6200 | Current angle byte at B0+34h, flags word at B0; low-byte target/step configuration. Move on the256-angle circle by at most step; set flag4 when target reached, preserve other flags. Write flags word before angle byte. | Exact half-turn tie uses the ascending direction. Test wrap, step0, already-at-target, target/step high-byte masking and failure after the flags prefix. No trigonometric host function is required. |
| 8100 | Angle low byte and amplitude low byte at B0+34h,+36h. Candidate sine magnitude is amplitude*2048, truncation before scale, dword output at B0+10h. | Reference doubles only raw anglec0. Treat this unexplained axial asymmetry as an explicit candidate L2 quirk, not a derived identity or copied lookup table. Negative output scales by safe multiplication, not undefined signed left shift. |
| 8900 | Same inputs/scale, cosine dword output at B0+14h. | Reference doubles only raw angle80. Preserve the distinction from8100 and separately qualify all cardinal/near-cardinal/negative cases. |

No row is dropped because it needs another row or standard math.130e/2208/
338e, distance and division share result state but not identical status rules.
S11's2a05 write-width ambiguity and the other collision/unresolved/DMA families
remain their original T3 receivers, not falsely closed by this numeric ledger.

Only named results change.3bb0 updates D while preserving status/angle;
division updates status/output without replacing D;6200 and8100/8900 do not
mirror object-memory angles/velocities into the arithmetic result words.
2208's ordinary path preserves prior status, including a prior error bit.
These are distinct state meanings, not caches to merge for visual symmetry.

## Sole State, Register And Bus Owners

The actual current COP has no result route at5b0h--5b5h and no target/step
route at41ch--41fh. Admitting geometry without adding those owners would let
ordinary RAM impersonate missing device state. Add these routes before any
new command is considered qualified:

- write-only target and step words at41ch/41eh;
- scale word444h, interpreted through only its low two bits;
- read-only status/distance/angle words at5b0h/5b2h/5b4h.

Prefer extending the existing small register store through5b5h, with explicit
range roles, rather than storing both result fields and mirrored register
bytes. The extra176 storage bytes are not another frame/cache/state machine.
One owner validates and encodes result words; read-only operational and debug
reads have identical non-mutating effects, and result writes must reject.
Target/step use low-byte effective values, without another cached policy field.
The source initializes results/configuration to zero at construction; a local
reset-to-zero rule must be declared as L2 reset policy, not claimed as measured
hardware reset merely from a C++ constructor.

Use S11's sole checked memory helper, extending its existing Core DMA operation
to1/2/4-byte operands where needed. No raw memory pointer, paused operational
read, second executor/clock, separate output queue or readiness flag is needed.
One copied pending command uses the existing safe point after CPU write commit.
Arithmetic results and memory writes have an explicit accepted-prefix order:
operand failure publishes no newly calculated result; later output failure
retains any already-published result/word/byte.6200 preserves its flags prefix
if the subsequent angle write fails. No rollback or fabricated completion.

The later implementation must freeze the proposed modulo32 coordinate
difference and signed interpretation explicitly, use wide square intermediates
and bounded floating-to-integer conversions, and test their edge semantics.
Reference signed overflow/right-shift/negative-left-shift behavior is not a
portable C specification and will not be copied into production.

## Independent Oracle And Coverage Plan

No production evaluator or imported table will generate expected values.
Independent .NET mathematical samples were evaluated read-only in this S:

| Mathematical sample (before any operation-specific quirk) | Result |
| --- | --- |
| angle64, amplitude1: sine*2048 / cosine*2048 | 2048 /0 |
| angle192, amplitude255: sine*522240 | -522240 |
| angle128, amplitude255: cosine*522240 | -522240 |
| angle32, amplitude255: truncated sine/cosine | 369279 /369279 |
| integer distance3,4 | 5 |
| integer distance32768,32768 | 46340, not signed32 overflow |
| D=5, S=3, divisor3 | 6 |

The two explicit axial candidate quirks therefore require separate expected
-1044480 before scaling, rather than pretending to be mathematical sine/cosine.
Maximum scaled magnitude is8355840, inside signed32. Exact scalar constants,
cardinals and Pythagorean expectations are independent of the future CRT calls.
Add branch/quadrant and near-boundary rows without materializing a huge duplicate
production table.6200 has independent sequence250->254->2->5 for target5,
step4; both half-turn ties choose increment, and step0 reaches only equality.

Extend the existing Board unit once for this cohesive numeric/result receiver:
all three command ports, little-endian byte/word lanes, read-only result
rejection, reset/busy/upload admission, configuration high-bit masking, the
angle->distance->division chain and its distinct status/zero policies, every
accepted memory-failure prefix, neighbour preservation and wide addresses.
Do not make one executable or fixture per opcode or duplicate Core CPU tests.
Retain all transfer/state-update/fault-budget regressions. Dual-width complete
units are required after actual code, not after this source-unchanged research.

## Minimal Public Vocabulary Proposal: Awaiting Owner Review

Actual Lib Types has no standard sin/cos/atan/sqrt vocabulary. C double is
deliberately allowed as a language scalar; lib_f32/lib_f64 aliases are explicitly
absent by design. Do not add them, call raw CRT from App, import private FPU
helpers, add a parallel numerical library or hand-approximate geometry merely
to avoid the public-interface review.

Proposed optional new header src/lib/types/math.h contains only ISO C math.h
and these four one-to-one aliases (no runtime implementation or state):

```c
#define lib_c_sin  sin
#define lib_c_cos  cos
#define lib_c_atan atan
#define lib_c_sqrt sqrt
```

This adds four public names, no changed existing function/type/enum/ABI. Keep
the header opt-in, not included by types_interface.h. Types README documents
unchanged CRT/domain/link behavior. Extend existing types_contract_smoke.c with
volatile function-pointer calls at exact0/1/4 inputs; no new test binary. The
existing Lib test target links m on POSIX where required; the later MyRaidenII
consumer separately supplies its own math link, not a global Types dependency.
The existing layout gate already accepts common math.h and derives raw tokens
from aliases; do not change/weaken that gate or the root registration helper.

Expected Shared change: new declaration header approximately11--15 lines,
Types README approximately4 lines, existing contract test approximately10--15,
test/lib/CMakeLists.txt approximately3 for POSIX linking, and two manifests.
Six Shared files total, approximately+30--40/-3--5 including manifest rows.
Shared edits require the owner's explicit approval; none are made here.
After approval, verify changed manifests, Lib layout/boundary/negative checks,
real runtime aliases and complete repository units on both existing graphs.
Existing products do not include this new optional header; retain their ten
release hashes unless a real product input/link changes.

## Implementation Receiver And Qualification Limits

The next admitted receiver should consume the whole numeric/result ledger,
including the required public vocabulary approval, rather than implement one
first-failing ROM command. Estimated local production +220--320/-20--50,
existing Board tests +180--280/-10--30, no public App/Shared execution API,
thread, frame format or product artifact. Refine estimates before code after
resolving stated L2 choices.2a05 and unresolved/collision/DMA families remain
separately named, mandatory T3 work; no raw-ROM fallback is restored.

No production, test, CMake, Shared corpus or artifact changes are made by S12.
The unchanged exact S11 source retains its687/687 qualification on both widths;
this is not a new unit or external/game run. Source/model and actual-change
review, document checks, release identities and cleanup complete this research
delivery; they do not qualify geometry, startup or playable MyRaidenII.

## Executor Verification And Scope Review

The actual existing COP/source and Board/test SHA-256 remain respectively
6DCC1436F8D79206339E74EC54F7952640E112E1165FD3165949A526E5204884 and
E6E88419C65C6A869264FC6F4A3E7F03ABE7F73EFDAE578DC4E54A624119B2FF,
identical to S11's dual-width687-name qualification. Git's actual tracked diff
contains only this product's records, no source, test, build, manifest or
Shared edit. No new tests/builds/EXEs are needed for this research-only change.
All ten release identities,33 tracked guards and six selected media-role
hashes are checked unchanged. Documentation governance and whitespace checks
pass; these structural checks do not qualify future numerical execution.

Read-only research and independent numeric samples created no task file,
reference checkout, probe, background worker or ROM copy. Build still contains
only persistent .ccache/.ninja; no cleanup deletion or cache purge is required.
Source restrictions and all nine operands/results/status/failure dependencies
were reviewed against actual local owners and the fixed reference units.
The Types proposal awaits explicit owner review; it is not inferred from the
general product implementation approval and has not changed Shared.

## Archived S12 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: MyRaidenII M1 T3 S12, after accepted S11; single-session sequential coordinator/executor roles. |
| Admission And Approval | Owner's standing implementation/research approval and delegated ordinary same-T S ordering. This S admits only MyRaidenII research/design records; no Shared source or public API change. |
| Objective | Reconcile the complete nine-ID geometric/numeric batch from S9 (130e/2208/338e/3bb0/42c2/4aa0/6200/8100/8900), its shared result/configuration dependencies, source-grade ambiguities, arithmetic/failure contract and independent oracle plan. Produce one cohesive implementation receiver and exact prerequisite request rather than following successive first faults. |
| Non-goals | No production/test change, copied/transliterated reference body/table, game data/patch, generic COP interpreter, new math library, physical latency claim, invented width/signedness or final startup/gameplay qualification. |
| Reference Baseline | Clean pushed e44e522d2; S9 uploaded-ID ledger, accepted S10/S11 memory/safe-point/fault owners; pinned MAME c1425a1af00b88aa07477b1b23a30ed5e5f856ff as research-only L2 source; actual current Lib Types public vocabulary. |
| Candidate Proposal | ../proposals/m1-board-cop.md; ../etc/evidence/t3-s1-board-contract.md B07/B08; ../etc/evidence/t3-s9-cop-initialization-protocol.md. |
| Files And ABI Surface | Current/history/evidence index and one nine-member contract/review record only. Source/test/artifact delta zero. Estimated documentation +140--200/-0--10, refined if evidence demands it. Proposed later public Types declarations are described, not implemented. |
| Applicable Rules | Execution full finite-batch dispositions, role/P lifecycle and actual-change review; Architecture one operation/state/time owner and no peer/private import; Coding use existing Lib vocabulary rather than duplicate math wrappers; source policy no reference implementation/table import; Document records support rather than replace design/current authority. |
| Verification | Re-read pinned operation/dispatch/register declarations and fresh SHA-256; inspect result/config maps and current bus/memory helpers; independently derive bounded arithmetic/cardinal/quadrant/Pythagorean/zero-divisor/scale/wrap/byte-neighbour oracles and failure-order matrix. Confirm no source/test or ten-release identity change, docs/whitespace and build-root hygiene. Unchanged S11 unit evidence is retained, not relabelled a new suite run. |
| Expected Markers | Every one of nine IDs has operands/results/width/grade/dependencies and a proof/decision receiver. Actual missing standard math vocabulary is explicit; no direct CRT call, App-local clone, private FPU dependency or unapproved Shared edit. |
| Asset Needs | Read-only pinned reference responses; no ROM required. Owned build/myraidenii-t3-s12 only if a probe is necessary, 8MiB/5-minute no-progress/45-minute phase budgets; archive necessary neutral results/hashes before cleanup. Ten artifacts and all user inputs stay unchanged. |
| Reporting Requirements | Report concrete architectural/numeric gaps and exact later source/test/API diff proposal; distinguish known facts from L2 choices and unsupported forms; report actual document delta, validation and unexecuted qualification; complete P push then actual coordinator review. |
| Stop Conditions | Source/API/Shared modification, licensed implementation/table import, protected content output or unsupported precision claim. Record the narrow approval/evidence prerequisite and continue safe independent contract reconciliation. |
| Exit Criteria | Complete nine-member ledger and common result/bus/arith proof matrix are reviewed with exact subsequent implementation boundaries/estimates and any public-interface approval request. Actual diff/source/artifact checks and cleanup pass; scoped research P and coordinator acceptance are pushed. No implemented-command or T3 completion claim. |
| Original Owner Request | Implement complete MyRaidenII under single-session dual-role governance, efficient coherent batches, existing test reuse and explicit Shared review before edits. |
| Similar-Issue Sweep | Check all nine current/legacy reference variants, endian/width/signedness, angle/distance latches/status/config fields and memory-failure publication. Retain source inconsistencies as named receiving obligations; do not mistake a shared spelling or a passing sample for complete behavior. |
