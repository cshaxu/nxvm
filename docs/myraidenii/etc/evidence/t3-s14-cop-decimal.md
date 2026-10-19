# T3 S14: Complete Decimal Register Receiver

## Before-Code Contract And Source Boundary

Baseline is accepted S13 d398e0ff5. This consumes B07's complete decimal
register family, not any of the nine pending geometry macros. The latter
still need the separately requested four-name Types authorization. No Shared
code, interface, manifest, App or artifact change is admitted here.

Read-only fixed MAME research commit:
c1425a1af00b88aa07477b1b23a30ed5e5f856ff.
The board map and COP facts have the existing independently checked hashes:
raiden2.cpp FDDE57F6E9024657F2A9830864523305139562229750B87C967E597324CDB26A;
seibucop.cpp 7E9763D7D1026E82473F135AEE26317DFD8E930C5F1AC257780CD813484A75A9;
seibucop.h 69857872F1E68D807442FA6018562A29390ACE4E0D66ECF5D86ABE080829E962.
These are research-only references; no body, table, firmware or uploaded
microprogram is imported/transliterated. The original implementation uses
ordinary positional decimal arithmetic with independent literal oracles.

| Register | Explicit L2 behavior |
| --- | --- |
| 420h--423h | Write-only unsigned32 input, little-endian; each accepted byte/word merge publishes a new converted result. Low and high halves both trigger. |
| 424h--425h | Write-only16-bit mode. Changing mode alone does not reconvert the last result. Exact mode3 uses ASCII zero padding; all other values use spaces. This mode interpretation is reference guesswork, not measured silicon. |
| 590h--598h | Nine least-significant decimal digits, least-significant first. At least one ASCII zero for input0; discard any tenth/higher digit, without modulo-reformatting zero into a single character. |
| 599h | Constant zero sentinel after an input-triggered conversion. |

Construction/reset clears input, mode and completed result bytes to zero;
it does not fabricate a conversion. This is explicit L2 reset policy. All
result reads are side-effect-free, whether operational or observational.
Result writes reject. Existing busy admission also rejects input/mode writes
while a macro is pending; no new ready bit, queue or completion phase is added.

Use the existing byte store as the only input/mode/result owner. Completed
digit bytes are real command-time state, not a needless mirror: mode changes
do not alter them until another input write. Extend the store only through
599h; no separate digits array or lifetime object is needed. One bounded
private converter emits significant digits then fills the remaining width.
It does not call a host formatting service or add a numerical framework.

S13's RESULT reader maps addresses>=588h back by8. With a second RESULT
family that predicate must become exactly588h/589h; otherwise decimal reads
would alias unrelated bytes. Tighten the existing owner, not a new reader.

## Independent Proof And Failure-Stage Reconciliation

Literal vectors include0,1,65535,65536,123456789,999999999,1000000000,
305419896 and4294967295. Expected nine-character strings, plus a zero sentinel,
are derived by independent base10 formatting/reversal, never by a production
converter. Mode0/1/2/3/0103/ffff distinguish the full mode word, not only its
low byte. Test delayed mode effect and later input-triggered publication.

The existing Board unit supplies the real public memory route. Test low/high
halves, every byte lane, a mixed unaligned input word, byte/word/odd output
reads, write-only input/mode policy, read-only output, collision/result
independence, pending rejection, and reset. No public COP getter or test-only
production seam is added. Earlier transfer/update/IRQ/video cases remain.

New420h--425h decode makes three old position-failure addresses and one old
box-word failure address fail too early. Preserve their exact intended stages
with explicit real route addresses: position flags/axis0/axis1/axis2 bases
426h/42ch/46ch/468h; hit-box word0/word1/word2 pointers428h/426h/42eh.
Each earlier input is ordinary RAM, and the selected final read reaches a
write-only COP register. Do not merely retain an error return and falsely
claim unchanged failure-stage coverage. Earlier state-update vectors do not
read the newly decoded decimal locations on their accepted prefixes.

Estimate: production+45--75/-5--15; existing test+100--150/-5--10;
records+100--170. Both stable Ninja graphs build the focused unit, then run
the complete687-name unit inventory sequentially, four workers,600-second
aggregate containment, unchanged per-test budgets. Ten EXEs and protected
inputs must retain their hashes; no real-ROM/gameplay qualification is claimed.

## Implementation And Actual-Change Review

One private bounded converter consumes the existing32-bit register value,
emits significant digits and fills the remainder. The same stored result
bytes serve every operational/debug read. Input writes call it once after
the complete accepted byte merge; mode writes and all reads do not call it.
It adds no object, public interface, pending state or independent lifecycle.
The existing store grows18 bytes through599h; no separate mirrored digit
array or lazy-reader fallback is introduced. Collision alias decoding is
narrowed at its one existing RESULT owner before adding the second family.

The13 literal numeric/mode vectors stay in the existing Board executable.
One small private assertion helper inspects real provider output as every
byte and overlapping aligned/odd word, using operational and observational
public routes. Separate sequential cases prove mode-word/mode-byte delay,
all four input byte lanes and an unaligned middle word. An actual V30 guest
writes mode/input, reads the result and stores it into ordinary RAM before
HALT; the test checks both the completed digits and that copied word.
The same fixture proves all result words and the sentinel reject writes,
input/mode reads reject through both APIs, pending writes preserve the old
completed output, reset returns zero bytes and the next input uses reset mode.
Collision results remain untouched. No production converter supplies oracles.

All earlier transfer/update and four-command collision tests remain, including
all17 collision failures. Only explicit physical failure locations change
where the new decimal decode would otherwise cause an accidental earlier fault.
The source/caller sweep confirms all preceding read words are ordinary RAM
and the named failing word is a real write-only COP route. The old update
case addresses and assertions require no change. No test binary/framework,
reference program table, Core-private pointer or other product fixture is added.

Actual tracked delta against d398e0ff5 by git diff --numstat:
production32 added/4 removed/net28; test143 added/5 removed/net138.
Production is smaller than the45--75-line estimate because its existing byte
merge/result/reset owners supply the required behavior. Test additions fit
the100--150 estimate; they prove a new complete register family and preserve
the earlier fault-stage contracts rather than retesting CPU arithmetic.
No Shared, build, public header, manifest, other App or artifact path changes.
Local MyRaidenII source/tests have no standalone manifest to regenerate.

The read-only research responses are freshly hashed to the three fixed unit
identities above. No download, ROM lane, trace or probe is persisted. Frozen
source SHA-256:
FED4991EA370C453E9D0DD90ADC2D62ECB6E2BC310D7F82D7A6E59B552125492;
test SHA-256:
B1AF1E127095D609B014C70CA5B3A990BDA4082595087EF6C45E149C4FBE8749.

Self-review also catches two COP pointer expressions which added a physical
address to the array pointer before subtracting the register base. Compute the
integer displacement first in both the old GPR read and new decimal output;
this avoids an out-of-bounds intermediate pointer without changing intended
bytes, state or interface. The whole local pointer-offset sweep finds no
second occurrence. An earlier full x64 run passed687/687 in116.26s before this
review correction; it is not used as final frozen-source qualification.

## Final Qualification And Boundaries

The final frozen-source complete unit runs pass687/687 each, no failures/skips:
x64 109.49s, log SHA-256
0565040FB628B770A29708A13F493E6CEF4BB42CD67B92F14824334373B57558;
x86 129.18s, log SHA-256
63E2EB67BA189EB62AFDFA0B3DA4E9CD4F3CC69B23E503EC2A7F169138911BD3.
Both use the original cached Ninja graphs and strict flags. No unit name,
assertion or per-test timeout changes. No new executable, framework, public
API, state object or duplicate production path is introduced.

All ten release identities match S10's exact result record;33 protected
tracked inputs and six external media roles match S49's input record. No
MyRaidenII Product EXE exists yet, and this local Core cannot affect those
other products' links. Keep their unchanged releases rather than refreshing
unaffected binaries. Documentation governance and git diff --check pass.
No disposable task root or background owner remains; build contains only
.ccache/.ninja, with no cleanup deletion required.

This accepts one complete decimal register family inside B07/B09. The nine
previously implemented uploaded macros are unchanged, not relabelled as more
of the22-ID initialization list. Geometry/math authorization,2a05, unresolved
macros, remaining registers/DMA and actual selected-set startup remain T3
requirements. No real-ROM run, external integration suite, physical timing,
desktop/RDP, rendered frame, audio or playable-game qualification is claimed.

## Coordinator Acceptance

Actual pushed P1 c28a162d5 is reviewed across all five source/test/record
files against the full packet, not accepted from a summary alone. The whole
decimal register family has one input/mode/result owner and one accepted-write
publication path; reads cannot recompute or alias collision state. Exact mode,
padding, digit width, reset and pending behavior have direct independent
proof, including the actual guest path. Earlier failure addresses preserve
their intended read stages under the new decoder, not just an error return.
Both pointer offsets are computed before pointer addition. No new public,
platform, lifecycle, state-mirror, framework or imported-body dependency exists.

The final strict frozen-source dual-width suites, hashes, scope and cleanup
match the packet. Earlier pre-review tests are explicitly not the final source
qualification. The complete original B07/B08/startup goal is unchanged; this
family is not another implemented uploaded macro or a playable-game claim.
Coordinator accepts S14 and closes it after the scoped pushed delivery.

## Archived S14 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: MyRaidenII M1 T3 S14 after accepted S13 d398e0ff5; one session performs coordinator/executor roles sequentially. |
| Admission And Approval | Owner's standing MyRaidenII implementation approval and delegated ordinary same-T ordering. Only MyRaidenII local source/test/records; Shared math authorization remains separately requested, not inferred. |
| Objective | Consume the entire B07 integer-to-decimal register family: input420h--423h, mode424h--425h and ten read-only result bytes590h--599h, with exact accepted-write, digit/padding, byte-order, reset and publication behavior under an explicit L2 contract. |
| Non-goals | No Shared/math/API change, geometry, PRNG, unresolved macro guess, source/table import, new COP executor, display renderer, physical timing or game qualification. Other T3 obligations remain required. |
| Reference Baseline | Clean pushed d398e0ff5; accepted S13 range/result/command owners and existing Board unit. Fixed MAME c1425a1af00b88aa07477b1b23a30ed5e5f856ff board map and decimal helper facts, research only. |
| Candidate Proposal | ../proposals/m1-board-cop.md and ../etc/evidence/t3-s1-board-contract.md B07/B09; S13 result-register owner and original S11/S13 failure-prefix regressions. |
| Files And ABI Surface | Existing local cop.c and Board unit plus Current/evidence/index. No public header or manifest changes. Estimate production+45--75/-5--15; tests+100--150/-5--10; records+100--170. Reconcile existing injected addresses which become real write-only decimal registers without dropping the intended failure-stage coverage. |
| Applicable Rules | Execution complete family/actual-diff/P lifecycle and bounded testing; Architecture one input/mode/result store and accepted-write publication, no lazy-read second path; Coding private bounded conversion, Lib vocabulary and existing fixture reuse; source policy mathematical facts only, no reference body/table; Documentation explicit L2/source/verification limits and packet archival. |
| Verification | Independently formatted literal vectors for zero, low/high halves, mixed/maximum32-bit values, nine-digit truncation, exact mode3 versus other16-bit modes, delayed mode effect, byte/unaligned writes, operational/observational result bytes/words, read-only results, pending rejection and reset. Preserve exact stage coverage of all earlier COP failure vectors at their newly accurate physical decode. Strict focused builds and complete687-name unit sets on both stable graphs, four workers,600-second aggregate containment. Scope, docs/whitespace, release/input hashes and cleanup checks. |
| Expected Markers | Accepted input writes merge full bytes then publish bounded decimal output once. Mode writes alone do not recompute; reads never recompute. Nine least-significant digits appear least-significant first plus a zero sentinel; at least one ASCII zero, space padding except exact mode3. Reset clears the real result latches to zero. Collision aliases map only588h/589h, not new decimal results. |
| Asset Needs | ROM-free vectors and read-only pinned research responses. Owned build/myraidenii-t3-s14 only if needed,32MiB/5-minute no-progress/60-minute phase budget. No game ROM or EXE needed; preserve ten current releases and protected inputs. |
| Reporting Requirements | Confirm whole-family source/contract and refined estimate before code; report actual source/test delta and same-class failure/alias review. Preserve exact dual-width results and hashes; complete implementation P push, actual coordinator review and governance P. |
| Stop Conditions | Shared/public capability, protected input modification, licensed source/body/table import, per-game condition, invented physical precision or material scope expansion. Unmodelled PRNG/macro/DMA behavior stays explicit, not guessed. |
| Exit Criteria | Full decimal register family has original bounded L2 implementation and independent actual-bus proof; previous operation/failure/reset regressions and both complete unit suites pass. Actual-diff/packet review, release/input scope, cleanup and pushed delivery precede S14 acceptance; T3 remains open. |
| Original Owner Request | Implement complete MyRaidenII under single-session dual-role governance, group coherent work efficiently, reuse existing tests and obtain explicit Shared review. |
| Similar-Issue Sweep | All input lanes, modes, output lanes and reset/publication phases share the same owner. Check all RESULT reads for the old overly broad collision-alias predicate and all old failure vectors for incidental early faults after new register decode. Do not merely preserve a failure return while losing its intended stage. Local review also repairs both COP byte-store pointer expressions to compute integer displacement before pointer addition, without new state or API. |
