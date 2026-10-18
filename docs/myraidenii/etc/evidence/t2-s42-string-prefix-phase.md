# T2 S42 Native String Extra-Prefix Phase Accounting

Baseline `453b7f3f4`, after accepted S41. This bounded batch consumes the
supported native string segment/BUSLOCK nominal-prefix mechanism at original
V05/V09/V14/V15. It does not qualify complete V30 time, Machine or gameplay.

## Source And Model Boundary

NEC October1986 V20/V30 user's manual SHA
`FABBFCD19D4A2FCDAAC9F0BC06B00BF0A6A4BEEDAFBD3FF16ACCD075A374EA4C`.
[Original source](https://www.dosdays.co.uk/media/nec/NEC-V20-V30-Users-Manual.pdf).
Ten full pages printed6-4,12-14..20,12-173/176 (physical PDF29,59..65,218,221)
were rendered and visually reviewed. Segment overrides, BUSLOCK and repeat
prefixes have two-clock rows; each aggregate repeat table already includes
its canonical repeat prefix. Section6's restart guarantee retains up to
three prefixes, not arbitrary malformed streams.

For primitive strings, add two clocks per extra segment/BUSLOCK byte with
the existing nominal-source origin. For one canonical repeat, do not charge
its repeat byte twice. Charge extra segment/BUSLOCK terms on first/zero/
restarted execution, not continuation. The manual does not locate combined
prefix fees by repeat phase: this combined policy is explicitly the existing
L2_CONTROL_MODEL, not silicon timing or a proven physical bound. Unprefixed
and ordinary canonical-repeat source grades remain unchanged.

Duplicate repeat bytes and noncanonical repeat forms on noncomparison
strings remain unknown. Full bus/prefetch/wait/entry/dynamic/maximum and
ordinary8080 time retain original receivers. An allocated nominal subtotal
still has source_timing_unallocated true, maximum zero and no Machine
admission. No compatibility CPU or second clock is introduced.

## Sole Implementation And State Owner

The existing V30 selector uses the authoritative prefix extent and decoded
repeat facts. Its bounded byte walk identifies the single repeat byte and
rejects duplicate repeats; it is neither another decoder nor an executor.
The existing finishing block adds only extra-prefix terms, using the
existing phase and original decoded segment/LOCK facts.

Existing repeat identity storage/checking, previously used by selected-SX,
also records V30's exact prefix-plus-opcode bytes. A changed byte, order,
effective segment, opcode or length at the same CS:IP starts setup again;
changing only lookahead does not. The existing fifteen-byte opcode storage
bounds this copy: prefix count plus opcode is no greater than decoded
oplen, itself no greater than fifteen. No persistent field/layout/header
change is made. SX cache-miss accounting stays SX-only; Intel bodies,
instruction handlers and the successful-entry reset owner remain unchanged.

Production changes two existing C files: +24/-8/net+16. No new production
helper, API, enum, field, path, observation, thread or framework.
Five existing test/header paths change +225/-100/net+125; code total
+249/-108/net+141, counted by Git numstat from the baseline, excluding
documentation/artifacts/manifests. Two manifests separately +7/-7/net0.
Positive test growth owns complete prefix/phase matrices and independent
failure assertions, including necessary loop/oracle indentation. Two new
test-local functions own the changed-code restart and comparison refusal
matrices; the existing string oracle remains the one independent row table.

## Actual Execution And Retained Assertions

Block keeps all original1920 contexts and expands to9504 actual scenarios:
six byte/word opcodes, original16 endpoint/DF/flag/wrap contexts, primitive/
zero/one/three iterations, twelve segment/LOCK patterns and every repeat-byte
position. Last override, redundant segment/LOCK, full CPU/RAM, accepted
transfer order and observation absence remain checked. Failure positions
before/after the first success and actual NMI/INT/RETI/same-CS:IP restart
also cover triple prefixes. Seven actual refreshes distinguish byte/order/
segment/opcode/length changes from unchanged code and lookahead changes.

Comparison retains942920 original independent operand/flag/CPU/memory
inputs. An additional1440 locked cases span four opcodes, four canonical
repeat conditions, five effective segments, zero/one/three counts, both
directions/code wrap and termination choices. They reuse every observed
data/fetch/preview refusal position and retain preinstruction CPU rollback,
accepted-prefix transfer and no stale completion/time assertions.

Port strings keep their320 old scenarios and add locked counterparts,
640 total. Original endpoint parity, DF, address/port wrap, transfer phase,
completion/cancel and CPU/RAM assertions remain. Actual repeat zero-count,
NMI/INT/RETI routes also gain triple-prefix counterparts. Scalar and failure
tests retain their original responsibilities.

The conservative common lexeme scanner deliberately rejects LOCK, although
these instructions execute. New tests explicitly assert unavailable,
nonmutating LOCK preview rather than inventing preview support; all original
non-LOCK positive-preview assertions remain. No production preview change
or weakened original assertion is needed.

Two formerly unknown nominal-transfer witnesses, carry-repeat plus segment
comparison, now belong to the actual Comparison seam. The boundary fixture
instead uses still-unallocated overridden packed ADJ4A/ADJ4S. Its24157
ordinary transfer and3410 non-string prefix inputs and live stale-result
guards remain; no negative publication check is removed.

## Development Findings

The first build found a mechanical test edit leaking triple-prefix variables
into the unchanged port failure helper. Restore its original two-byte copy.
The next focused pass had five successes and two failures: new Block/
Comparison tests incorrectly demanded LOCK lexeme preview success. Inspection
of the existing scanner's explicit lock rejection established the actual
contract; fix only those newly added expectations, retain nonmutation and
actual execution checks. Both corrected focused groups pass seven tests.
A first manifest refresh used lowercase digest text, which its existing
uppercase-only validator rejected; regenerate the seven entries in the
canonical format without changing validation. These development failures
are not hidden by weaker guest assertions or larger test budgets.

## Similar-Issue Sweep

Queries inspect `source_repeat_bytes|source_repeat_byte_count` across
`src/x86-core/cpu`, all prefix/phase users in cpu_timing.c/cpu_timing_model.c,
all three original string units and their CPU-local oracle, plus
`lock_prefix` in cpu_instructions.c. Reset clears the existing byte count;
successful entry already retires active repeat identity. No extra repeat
cache or prefix interpretation is added. SX retains its distinct cache-miss
responsibility and all other Intel evaluators remain unchanged. Unsupported
duplicate/noncanonical/long streams, ordinary8080 and physical timing remain
the named original V05/V09/V12/V14/V15/V16 receivers, not silent fallbacks.
All seventeen ledger rows, full proposal and Queue remain intact.

## Qualification

Both original cached Ninja all-target builds and seven-case focused groups
pass. Complete repository units pass678/678 per width, including191 X86 Core
tests, zero failures/skips; full units take352.59/153.72 seconds. Strict498
and Types1389 gates pass; historical warning exceptions remain, not an
exemption-free strict build. [Results](t2-s42-results.json) preserve exact
markers, counts and receiving case identities.

All14 selected receiving integrations plus restored-default identity pass
per width. DeskPro groups take135.67/163.15 seconds. Eight PC0564 EXEs are
rebuilt/deployed; [ten release identities](t2-s42-artifacts.json) pass PE,
hash and absent-debug-section checks. MyNES0044 and INIs remain unchanged.
[Six media originals](t2-s42-media-inputs.json) retain actual pre-integration
hashes/sizes/modes after execution; this is not an entire external archive
audit. Existing firmware source, embedding and protected-input gates remain
unchanged. Both original graphs finish at default, Release and the persistent
ccache launcher retained. Profile switches/restores recompile zero CPU
objects, checked against both Ninja path-separator spellings.

Final owned runner82778 is terminal exit zero; the collector independently
checks frozen source/test/public/handler hashes and actual XML/logs/graphs/
PE/input identities. Ten exact owned page renderings are removed after
review; the pinned manual, logs and reusable cache remain. No fresh complete
external/native, desktop/RDP/audio or game qualification is claimed.

## Admitted Requirement Packet

Shared P1 `45dcb1d26`, NXVM release P2 `0b5807807` and status-only correction
P3 `5a36ea0c9` are pushed. Actual P2 diff review found that a newline-sensitive
documentation splice removed the untouched historical tail of NXVM Current.
P3 restores that exact baseline tail; final receipt-only diff is+11/-11.
No binary/source/test input changes in this correction. Prevention: locate
section boundaries independent of CR/LF, compare preserved suffixes and review
per-file numstat before commit, not just structural document-gate success.

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation, MyRaidenII M1 T2 S42, next unused S after accepted S41. |
| Admission And Approval | Owner standing single-person dual-role automatic implementation and timing-upgrade approval; coordinator admits on 2026-10-18 at clean 453b7f3f4. Targets Shared X86 Core/tests, NXVM eight receiving releases/status and MyRaidenII records. No Lib/Emulator/MyNES or public-contract change. |
| Objective | Complete native string primitive and single-canonical-repeat segment/BUSLOCK prefix nominal accounting across all fourteen string opcodes, using one existing repeat identity/phase owner and real Block/Comparison/Port execution seams. Preserve every old semantic/failure assertion and original full T2 domain. |
| Non-goals | No guest instruction/interrupt/bus/Intel semantics or public API/enum/layout changes. No new persistent field/helper/clock/decoder/runner. Duplicate/noncanonical repeat prefixes, long-stream/restart semantics beyond the documented three retained prefixes, ordinary8080/full physical time/max/Machine/board/game remain original receivers. |
| Reference Baseline | 453b7f3f4; pinned NEC October1986 manual SHA FABBFCD19D4A2FCDAAC9F0BC06B00BF0A6A4BEEDAFBD3FF16ACCD075A374EA4C. Ten full pages printed6-4,12-14..20,12-173/176 (physical29,59..65,218,221) rendered/visually inspected. S36 original source rows and S41 two-clock prefix proof. |
| Candidate Proposal | proposals/m1-v30.md, original V05/V09/V14/V15. Primitive extra segment/LOCK bytes add two clocks each. Canonical repeat aggregate includes its one repeat prefix; extra segment/LOCK terms are charged only on first/zero or restarted phase, never continuation, explicitly L2_CONTROL_MODEL because the manual does not specify combined phase placement. Existing byte-identity storage/checking extends from selected-SX to V30 so changed prefix bytes restart setup; cache-miss behavior stays SX-only. Reuse authoritative prefix extent and decoded repeat facts, no second decoder. |
| Files And ABI Surface | src/x86-core/cpu/cpu_timing.c, cpu_timing_model.c and manifest; existing Block/Comparison/Port and nominal-transfer units, existing CPU-local string oracle header and test manifest. Estimate production +35..65/-12..30; tests initially +120..240/-35..80, revised before remaining fixture expansion to +180..300/-70..140 for triple-prefix fetch refusals and necessary loop/oracle reindentation. No public/private layout/header change, new test target or framework. |
| Applicable Rules | MyRaidenII guide/goal/proposal/ledger/queue/roadmap/architecture/coding/source policy; Shared Execution/Architecture/Coding/Document and Contributing; architecture/coding-governance and read-only PDF skills. Receiving NXVM guide/current/architecture/coding/source policy, X86 Core README. Sole execution/time/publication ownership, Lib Types, explicit source/L2 grades, full packet, target-separated Ps and coordinator committed-diff review. |
| Verification | Retain all1920 original block contexts,942920 comparison inputs, scalar/string port and original full CPU/RAM/flags/transfer/accepted-prefix/provider-refusal assertions. Expand Block prefix orders/segment/LOCK/redundancy, zero/one/multiple/terminal phases and both parities/DF; verify mutated bytes, real NMI/INT/RETI and same-CS:IP restart, failure-before/after first success and no stale timing. Update original nominal-boundary witnesses to genuinely still-unknown forms while retaining newly allocated comparisons at owned seams. Both original Ninja/ccache all-target builds -j8, complete repository units -j4, specialized strict/Types gates,14 selected receiving integrations plus restored identity per width, eight rebuilt PC0564 and ten PE/hash/debug-section/protected-input checks. |
| Expected Markers | All original V30 string/block/comparison/port/entry/refusal markers retained; new V30:PREFIX-STRING:BLOCK:OK with actual matrix count and V30:PREFIX-STRING:RESTART:OK. Both final suites zero failures/skips. Canonical source grades unchanged; combined-repeat prefix phase policy explicit L2; source-time unallocated/max0/Machine closed. |
| Asset Needs | Synthetic units, original cached graphs and pinned manual only. Eight current PC0564 releases, unchanged MyNES0044 and owner INIs/media/firmware/snapshots. Capture six receiving media identities before integration. No ROM import or MyRaidenII EXE; remove ten exact owned page images after review, retain logs/manual/cache. |
| Reporting Requirements | Report design/source-model decision and estimated diff before code; actual per-component added/deleted/net afterward. Record real failures/corrections, sole owner/retained fields, similar-issue sweep and exact dual-width test/release/commit proof without claiming full CPU/game eligibility. |
| Stop Conditions | New public/source/license authority, changed guest/Intel semantics, duplicate repeat state/parser/publication, out-of-bounds byte identity, extra operand observation, false silicon/physical eligibility, protected input mutation or failed qualification prevents acceptance. |
| Exit Criteria | All supported string primitive/single-repeat extra-prefix variants use the existing phase and byte-identity owner; original and added actual execution/interrupt/failure proofs pass. Required complete dual-width qualification, eight receiving releases, evidence/manifest/doc checks and target-separated Ps pushed, followed by coordinator review closing only S42. |
| Original Owner Request | Implement complete playable MyRaidenII, NEC V30 and required facilities in single-person dual-role governed mode. This prefix/phase batch is not the full CPU physical-time/Machine/board/video/sound/game outcome. |
| Similar-Issue Sweep | All repeat-continuation identity users and byte-copy bounds, V30 versus SX cache-miss fields, first/zero/continuation/terminal/reentry/changed-code phase, prefix order and effective segment/LOCK, all source string rows, old unknown-boundary witnesses and failure publication. Preserve original V01--V17, duplicate/noncanonical repeat/long-stream/mode/entry/full bus/time/Machine/board/game receivers. |
