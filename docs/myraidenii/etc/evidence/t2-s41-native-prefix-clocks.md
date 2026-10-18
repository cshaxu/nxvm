# T2 S41 Native Non-String Prefix Nominal Clocks

Baseline `05b4e7822`, after accepted S40. This batch consumes all already
allocated native non-string base families with segment-override/BUSLOCK
prefixes at original V05/V14/V15, not complete CPU or gameplay eligibility.

## Sources And Admission

Original NEC October1986 V20/V30 manual SHA
`FABBFCD19D4A2FCDAAC9F0BC06B00BF0A6A4BEEDAFBD3FF16ACCD075A374EA4C`.
[Source](https://www.dosdays.co.uk/media/nec/NEC-V20-V30-Users-Manual.pdf).
Full printed12-173/176 (physical PDF218/221, zero-based217/220) are rendered
and visually inspected: BUSLOCK and each segment override have two clocks
and no transfers. Section12's prefetched/zero-wait assumptions remain.

Pre-admission investigation also reviews NEC section8 and the already pinned
8080AF material. Compatibility does not establish V30 emulation clocks.
MAME's live research-only
[8080 implementation](https://github.com/mamedev/mame/blob/master/src/devices/cpu/nec/nec80inst.hxx)
observed2026-10-18 explicitly calls its clocks unknown and uses placeholders.
That unpinned observation is not a timing authority or implementation input;
no source/table/code is copied or derived from it. Discrete8080 counts and
placeholder1 are not silently promoted to V30 timing. Ordinary8080 remains
its original explicit source/model receiver; there is no overall blocker.

## One Owner And Complete Bounded Batch

The sole private V30 selector reuses its existing prefix-count owner and
points base opcode/ModRM/suffix/immediate accesses after those bytes.
Branch outcome receives the actual opcode index. Already decoded segment
routing facts identify an override: an override selects one segment for
both default-segment routes. Existing lock-prefix recognition is reused
and renamed from an unnecessarily Intel-specific private name; its body
and Intel call semantics are unchanged. No second prefix decoder, helper,
state, API/enum/layout, operand observation or timing publication is added.

For a successfully selected non-string base row, add two clocks per retired
segment/BUSLOCK byte once, then publish its prefix inputs. The original
source/L2 grade survives; an unknown/prohibited base does not acquire a
standalone prefix cost. All already allocated transfer/ALU/stack/branch/
shift/flag/decimal/NEC/port/software-entry families use this one finish.
Repeat-decorated non-string forms remain unknown. Canonical string setup/
continuation, extra-prefix string rows, long-stream/restart semantics,
ordinary8080, full bus/prefetch/waits/entry/max/Machine stay original receivers.
This is a nominal subtotal, not exact elapsed time or physical bus lock.

## Actual Execution Proof And Necessary Changes

Fourteen existing family execution units preserve their complete independent
CPU/defined-flag/memory/alias/segment/wrap/host-failure assertions and update
only independent prefix clock/key/input expectations. Successful DIV and DE
entry stay distinct. Source and existing L2 models retain their grades.
Original19 ALU boundary inputs remain: newly allocated cases gain explicit
expected rows; prohibited/undefined/radix/DE inputs keep zero/unknown results.
No instruction execution handler or Intel evaluator changes.

The existing nominal-transfer unit adds one cohesive cross-family matrix:
eleven independently transcribed base rows, every1/2/3-byte sequence of four
segment overrides and BUSLOCK, and normal/wrapped code starts:3410 actual
executions. It checks base/immediate/ModRM/suffix/PREPARE offsets, repeated
LOCK without false segment metadata, override order, source versus L2,
unallocated/max guards and no extra observations. All observed fetch/data
refusal positions for homogeneous triple segment/LOCK chains reject successful
publication and restore CPU state. Three original formerly-excluded inputs
(LOCK XCH, overridden CWD and INS) move into this complete matrix rather than
being deleted; original transfer counter becomes24157 plus3410 distinct
matrix inputs. The live success/unknown/failure publication sequence now uses
a genuinely still-unknown repeat-decorated NOP instead of the now-allocated CWD.

Production+28/-18/net+10; tests+187/-86/net+101, total+215/-104/net+111.
Manifests separately+15/-15/net0. Within admission estimates. One renamed
existing production recognizer, no new production helper/state/API.
The one added test function owns a real cross-family matrix, not a framework.

## Development Findings And Corrections

The first14-unit x64 pass fails four units. A mechanical opcode-normalization
change incorrectly uses the normalized opcode while validating the raw
canonical string-repeat prefix; restore that original raw prefix access.
The control oracle reads the prefix rather than opcode for its key; use the
actual encoded opcode. ALU boundary and live-publication tests retain stale
unknown assumptions; keep all old contexts, allocate supported rows and
use a genuinely unknown sequence for stale-publication rejection.

The first correction leaves one ALU test failure because rejected instructions
have zeroed keys, unlike successfully completed-but-unallocated forms.
Preserve that real distinction and strengthen each assertion. A subsequent
failure finds one existing signed-divide witness actually enters DE: retain
its original unknown combined-entry row, never assign successful-DIV clocks.
No failure is hidden or fixed by weakening guest assertions or budgets.
The targeted four-unit rerun passes except that then-corrected ALU boundary;
its final targeted rerun passes. Final frozen qualification is below.

## Similar-Issue Sweep

Queries cover every fixed byte-index access in the V30 selector and every
native timing oracle: `rg -n 'opcodes|prefix|branch_taken' src/x86-core/cpu/cpu_timing.c`,
`rg -n 'prefix.*0u|!.*prefix|nominal|unallocated' test/x86-core/cpu --glob 'cpu_v30*smoke.c'`.
Normalized base operands use the sole prefix count; raw repeat validation/
extension remains raw, mode escapes remain their own existing encoding.
The existing lock recognizer body is unchanged and reused; decoded segment
routing avoids a second byte walk and distinguishes repeated LOCK-only chains.
Every allocation still occurs only after real successful retirement; no
pending failure can inherit an earlier result. Existing pure semantic tests
without nominal assertions retain their complete owned behavior rather than
duplicating fourteen new clock tables.

## Qualification And Input Protection

Final frozen14/14 focused groups, all-target builds and full678/678 repository
units pass both widths, zero failures/skips; full units take318.67/139.40
seconds. Each width also passes498 retained strict rows and1389 Types inputs;
historical warning exceptions remain, not exemption-free compilation.
[Results](t2-s41-results.json) retain complete unit counts, actual markers and
receiving case identities. All14 selected PC integrations plus restored-default
identity pass per width; DeskPro groups take121.43/169.98 seconds. Both original
graphs finish at default, retaining Release and the persistent ccache launcher.
Zero CPU objects recompile during switching/restoration; the collector checks
both slash spellings in Ninja logs, including rechecking S40's earlier zero
count. Eight PC0564 releases are rebuilt/deployed and all
[ten artifact identities](t2-s41-artifacts.json) pass PE/hash/debug-section
checks. MyNES0044 is unchanged because it has no CPU dependency.

Six selected external media identities were captured before any receiving
integration in [media inputs](t2-s41-media-inputs.json); the collector confirms
their unchanged hashes/sizes/modes afterward. INIs and protected firmware/
source owners remain unchanged. Final owned runner52887 is terminal exit zero.
Final builds/dynamic tests have no failures; the development failures above
remain explicit, not overwritten by these passes.
No full external archive or fresh complete external/native, desktop/RDP,
physical audio/controller, physical-time/Machine/board/gameplay qualification
is inferred.

Shared P1 `a3ed1c043` contains17 source/test/manifest files (+230/-119,
net111 including manifests). NXVM receiving P2 `c4f1ecc5a` publishes eight
current PC artifacts and receiving status. Coordinator actual committed-diff
acceptance remains before S41 closure; T2/M1 and full original scope stay open.

## Complete Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation, MyRaidenII M1 T2 S41, next unused subtask after accepted S40. |
| Admission And Approval | Owner standing automatic single-person coordinator/executor implementation and timing-upgrade approval. Admitted on 2026-10-18 at clean 05b4e7822. Targets Shared X86 Core/tests, NXVM eight receiving artifacts/status and MyRaidenII evidence; no Lib/Emulator/MyNES change. |
| Objective | Complete native non-string segment-override/BUSLOCK nominal accounting for every already allocated base family through the sole V30 selector; preserve base source/L2 grades, successful guest behavior and full original T2 domain. |
| Non-goals | No new instruction/decoder/entry/Intel semantics, public API/enum/layout, persistent state, operand observation or physical-time eligibility. Canonical string-repeat setup/continuation and extra-prefix string costs remain their distinct original phase receiver, not guessed here. Ordinary 8080 timing remains unknown, not discrete-8080/MAME placeholder timing. |
| Reference Baseline | 05b4e7822. Original NEC October1986 manual SHA FABBFCD19D4A2FCDAAC9F0BC06B00BF0A6A4BEEDAFBD3FF16ACCD075A374EA4C, full printed12-173/176 (physical PDF218/221) rendered and inspected; each prefix explicitly two clocks. Existing S30--S40 real execution proofs. |
| Candidate Proposal | proposals/m1-v30.md, original V05/V14/V15. Reuse existing prefix-count/decoded facts, point private nominal byte access at actual opcode, reject repeat-decorated non-string forms, add two clocks per segment/LOCK prefix once after successful base selection; preserve source or L2 origin and publish segment/LOCK inputs. Do not create a second parser or fallback clock row. |
| Files And ABI Surface | src/x86-core/cpu/cpu_timing.c and manifest; existing native timing execution tests and test manifest. Estimate production +25..45/-15..30; tests +100..200/-50..100 across existing family owners, plus a bounded multi-prefix cross-family matrix in the existing nominal-transfer unit. No new target, production helper or interface. |
| Applicable Rules | MyRaidenII guide/proposal/ledger/queue/goal/roadmap/architecture/coding/source policy, Shared Execution/Architecture/Coding/Document, Contributing; architecture/coding-governance and read-only PDF skills. Receiving NXVM guide/current/architecture/coding/source policy. Sole decode/time/publication owner, Lib Types, original style, source grades, target-separated Ps and coordinator actual committed-diff acceptance. |
| Verification | Retain every existing native family CPU/defined-flag/memory/alias/wrap/host-failure assertion; update independent prefix-cost/key/input expectations, include segment/LOCK order, repeated override, original immediate/ModRM offsets, source versus L2, refused fetch/data and live publication. Both original Ninja/ccache all-target builds -j8 and complete repository units -j4; specialized strict/Types gates; 14 selected PC receiving integrations plus restored-default identity per width; rebuild eight PC0564, verify all ten PE/hash/debug sections and protected INI/media/MyNES inputs. |
| Expected Markers | All existing V30 semantic/nominal markers retained; add V30:NOMINAL-PREFIX:OK with actual case count. Zero final failures/skips. Existing total-time-unallocated, max0 and Machine rejection remain; unknown base rows do not gain clocks solely from a prefix. |
| Asset Needs | Synthetic units and existing original cached graphs/manual only. Eight current PC0564 releases; MyNES0044 unchanged. Preserve owner INIs/media/firmware/snapshots. No new MyRaidenII EXE, ROM or third-party implementation import; delete two owned page renderings after review, retain logs/manual/cache. |
| Reporting Requirements | Before code report scope/estimates; after code report actual production/test added/deleted/net, retained owners, full similar-issue sweep, source/model grades, build/test/artifact results and residual domain. Record real failures without weakening tests/budgets. |
| Stop Conditions | Public/source/license expansion, changed instruction/Intel behavior, duplicate prefix parser/time path, new state/read, fabricated 8080/physical eligibility, protected input change or qualification failure prevents acceptance. |
| Exit Criteria | All already allocated non-string families consume segment/LOCK prefixes at one owner with independent actual-execution proofs and unchanged guest/failure behavior; dual-width full qualification, receiving releases, evidence and target-separated commits pushed, then coordinator committed-diff acceptance closes only S41. |
| Original Owner Request | Implement complete playable MyRaidenII, NEC V30 and required facilities in single-person dual-role governed mode. Prefix nominal accounting is not complete CPU physical timing/Machine/board/video/sound/game qualification. |
| Similar-Issue Sweep | All fixed byte-index accesses within the V30 selector; branch outcome opcode index, ModRM/group/NEC suffix/immediates/PREPARE/direct port/address; all native family timing oracles, repeated/ignored prefixes and unsupported bases, actual failure publication, 8080 isolation and Intel/source/public guards. All original V01--V17 and remaining phase/source/model receivers retained. |
