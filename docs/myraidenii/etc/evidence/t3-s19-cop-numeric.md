# T3 S19: Complete Nine-ID Numeric And Result Receiver

## Scope, Provenance And Before/After Ledger

Baseline e7de691e0 accepts S18's explicitly approved four optional Types math
names. The owner delegates ordinary same-T planning; S19 receives the whole
S12 nine-ID batch and S13 command-entry correction. It changes no Shared file
or public API. All implementations are original explicit L2 macros, not
recovered microcode, physical latency or silicon-exact math. The remaining
2a05, RNG, sorting/DMA/unresolved and real-set startup obligations retain
their original T3 receivers. No new first-failing ROM instruction selects
this batch; no ROM run is performed in S19.

The pinned MAME c1425a1af00b88aa07477b1b23a30ed5e5f856ff cmd.ipp is freshly
read in memory; SHA-256 remains
B1E84AEF5BFDEBCA53D8F0AF0B1BE481B8218157BA460C1714853D322AC92EFC.
The [S12 facts/source ledger](t3-s12-cop-geometry-contract.md) and explicit
[S13 correction](t3-s13-cop-collision.md#explicit-correction-to-the-s12-candidate-contract)
are consumed, not reference bodies/tables or game data. Current, not legacy
other-game axes/offsets, determines the declared L2 model. S12's missing4aa0
numerator detail is reconciled: both divisions use D*2^(5-S). Its quoted
176-byte future store increase was arithmetic error; the actual S17 store
ends599h and grows28 bytes to5b5h, without another result mirror.

| Batch member | Before S19 | Complete S19 disposition and proof |
| --- | --- | --- |
| 130e | Explicitly rejected | L2 modulo32 coordinate differences, signed interpretation and bounded atan angle; all quadrants, negative axis, zero denominator and wrapping operands, every command port. No object angle write. |
| 2208 | Explicitly rejected | L2 unsigned word operands at12h/16h, existing status preservation after common error clearing; high-bit words, zero and nonzero denominator, reset-zero status and no object angle write. |
| 338e | Explicitly rejected | Same coordinate preparation and angle as130e; one byte at34h, adjacent byte preserved. Extreme signed32 inputs and late failed output retain the published angle/status. Byte width remains reference-uncertain L2. |
| 3bb0 | Explicitly rejected | Live-read L2 signed integer parts of modulo32 differences; wide squared norm, floor square root to D and word output3ah. Negative fractions, signed extremes, zero and changed-after-angle coordinates; failed output retains new D. |
| 42c2 | Explicitly rejected | Existing D scaled by low two configuration bits, unsigned divisor36h; quotient38h modulo16, status7. Zero sets bit8000 and attempts output0 without changing D/A. All scales, truncation, large divisor, word narrowing and late failure. |
| 4aa0 | Explicitly rejected | Same bounded numerator but divisor38h and output36h; zero is explicitly replaced by1, unlike42c2. Status7, existing D/A retained; input and late output refusal proved. |
| 6200 | Memory effect accepted by S17, global numeric contract pending | Existing circular handler unchanged; common dispatch now clears error bit15 while retaining D/A, including a real Board chain after divide-by-zero. All21 old literal cases remain identical. |
| 8100 | Explicitly rejected | L2 sine of low angle byte and low amplitude byte, amplitude*2048, final double rounding then truncation before integer scaling, dword10h. Explicit raw-c0 axial doubling, cardinals/neighbours/negative/max scale and partial output. |
| 8900 | Explicitly rejected | Same shared trigonometric preparation and scaling, cosine and dword14h; explicit raw-80 doubling, neighbouring angles/negative/cardinals and partial output. No duplicate sine/cosine dispatcher. |

All nine now have direct ROM-free L2 implementation proof. This does not turn
the nine uploaded IDs into the entire game's COP universe or close T3/M1.

## Single State And Failure Owners

One existing private register store owns status5b0h, distance5b2h and angle5b4h.
The one existing result route admits ordinary and observational reads, rejects
byte/word writes and resets to zero with the other COP state. No extra ready,
error, arithmetic latch or mirrored output field is introduced. Target/step
and scale remain the existing configuration, with only effective low bits used.
The store's additional unused register gaps do not make unmodelled routes RAM.

Every admitted service clears status bit8000 once, before its actual handler.
Rejected/unuploaded commands do not clear it. Merely enqueueing a command does
not execute this prefix. Reset cancels pending work and invalidates upload.
All result reads remain side-effect-free. Existing transfer, update, collision
and approach handlers receive the same prefix, rather than a numeric-only
approximation of the corrected source contract.

Coordinate preparation has one checked path: for each axis read B1 then B0,
subtract unsigned32 and interpret explicitly. Distance uses signed high words,
not implementation-defined negative shifts; squaring uses signed64 with sum
at most2^31. Angle atan input is finite, output before correction is bounded
to a quarter turn. Distance is at most46340; D*32 fits unsigned32. Trig input
is bounded by a byte, its doubled magnitude at most1044480 and scaled result
at most8355840. Negative scaling is multiplication, not undefined left shift.
All narrowing reflects the stated modulo word/byte output, not overflow UB.

No newly computed result is published until its inputs have been read. Common
entry error clearing is an already accepted prefix even if an operand fails.
Angle/status or distance publication precedes optional object writes; a later
write failure retains them. Divide status publication likewise precedes its
word write. Trig changes only object memory, not arithmetic D/A. All writes
use the existing Core DMA helper, so an accepted first byte/word survives a
later provider refusal. There is no rollback, alternate memory accessor,
numerical worker, hardware-state mirror or second clock.

S17's integer circular algorithm and its one transport helper are retained;
new numeric preparation uses that same helper. Two added private functions
own coordinate preparation and the cohesive numeric family. They are not
public forwarding wrappers. Optional math is included only by this actual
consumer; its static target links m privately on UNIX, not globally or through
Types. Windows uses existing CRT linkage. Linux is not executed in this S.

## Test Reuse, Findings And Same-Issue Sweep

The existing Board executable gains one39-row literal numeric table, each
run at all three command ports (117 cases), one26-row operand failure table,
one8-row output failure table and one actual Board chain. There is no new
test executable, framework, production entry or assertion generated by the
production evaluator. Literals use quadrant/cardinal identities, Pythagorean
and integer arithmetic, plus S12's independent mathematical samples.

Wrapping coordinate tests use a nonzero negative base for both objects, so
ordinary subtraction and signed-overflow assumptions cannot impersonate the
chosen modulo32 rule. Distance changes coordinates after prior angle work,
rejecting an accidental stale-coordinate latch. Amplitude/angle/scale high
bits, full negative maximum output, neighbour preservation and near-quirk
angles distinguish each selected behavior without a copied lookup table.

The26 input rows cover each coordinate dword's low/high word for both objects
and both angle/distance preparation, each word-angle/trig/division operand,
displacement overflow and out-of-range memory. All start with prior8007/5/32;
failure retains D/A and leaves7 from the common service prefix. The8 output
rows prove new angle or D remains published, a word split at5afh accepts its
low byte but cannot modify read-only status, and a dword split at4beh accepts
its low word but preserves the pending-protected GPR high word. Further rows
separate42c2 zero-error publication from4aa0 and wholly refused trig output.

The real Board chain proves error clearing by2208,6200 and5205, rejected-command
non-clearing, large-D quotient narrowing, guest CPU MOVs reading all three
result words into RAM, every result byte/word write refusal, pending-command
nonpublication/busy configuration, reset cancellation and reset-zero2208
status preservation. This is real V30/Core/COP/provider wiring, not callbacks
called by a simulated mock. The old13 transfer,25 update,14 collision and21
angle-approach literal matrices compare byte-for-byte equal to e7de691e0.

Two early tests exposed fixture mistakes rather than production defects:
4aa0's output neighbour is its input divisor, not the original sentinel;
and Core's existing public operational read rejects FAULTED, while its
observational inspect remains valid. The final result helper explicitly
checks that rejection, uses inspect after fault, and compares normal/observe
results in eligible states. No Shared lifecycle rule was relaxed.

Dual-width execution then finds a real arithmetic issue: x87 retains excess
precision in atan(1)*128/pi, yielding31 at direct integer conversion versus32
on x64. Final local volatile double values explicitly commit the angle and
trig calculation before integer truncation. This is one owner-local numerical
evaluation policy, not epsilon, axis/game special handling, an extra hardware
state or a change to Lib math. Both widths now pass the same independent
literal oracles; universal bit-exact host math/silicon precision is not claimed.

Similar-issue review covers all four floating operations, bounded inputs and
integer conversions, every dispatch branch and result publication, retained
unmodelled routes and changed old refusal assertions. No raw CRT call, FPU
private dependency, new API/ABI, signed negative shift or other production
consumer of this product's math exists. The independent Shared wide-query
classification debt from S15 remains in TODO, not fixed or worked around here.

## Delta And Qualification

Before-code estimate: production+150--220/-5--15, existing test+220--320/-5--15.
Actual tracked production/build delta is+136/-3/net133 (cop.c+133/-3 and
its CMake+3). Existing test delta is+333/-2/net331; combined code+469/-5/net464.
Records/artifacts are excluded from that git diff --numstat count. Production
is smaller than estimated because existing storage/transport suffice. Test
growth exceeds its initial upper estimate by13 additions to retain actual
guest result reads and explicit failure-state diagnostics, not a framework,
new test target or weaker coverage. Positive code implements the complete
missing batch and its direct proof; no redundant owner/path is retained.

Frozen COP SHA-256:
3A9AA4F1D2E33474C779C0CA53E21D435B7E7EB5DC09F5A75CF69517BF4D5FB0.
Consumer CMake SHA-256:
C597D792366FC95A58C426FA2FCAA4F38222132E24DD47A41D269F95708141A4.
Existing Board test SHA-256:
AC169849A1FBEF858A88FB07DFE6D6500E8017184883791F28810B1BBC4BE0BD.

Focused final strict builds and the whole existing Board unit pass on x64
(0.26s) and x86 (0.25s). Initial failing local fixture runs and the genuine
x87 discrepancy are not qualification. Full suites run sequentially on the
same cached graphs, four workers with600-second aggregate budgets. Results
and final hashes are recorded only after both owned runners terminate.

Final complete units pass687/687 per width with zero failures/skips: x64
116.99s and x86 130.43s. Both aggregate runners terminate exit0; frozen code
and tests are the exact identities above. LastTest.log SHA-256:

- x64: 680741AA2E4B8ACD3A33A16AFC6A52377E7B00126758B5346E6393D232D34997.
- x86: 897FA8F632BDC448F6766E12C0DD09BA160615D4862187B50A5649409F3CF6C6.

Documentation governance and git diff --check pass. Subsequent stable builds
on both architectures report no work to do. A final actual-path diff review
finds no Shared, peer App or artifact edit. Ten release,33 asset and six
external media-role hash guards match the accepted records before delivery.
Only the two persistent caches remain in build; owned runner sessions are
terminal. No cleanup deletion or unchanged-product rebuild is required.

No MyRaidenII frontend EXE exists. No existing PC/MyNES production/link input
changes; ten current release hashes,33 tracked assets and six external media
roles must remain unchanged. No external integration, ROM startup, desktop,
RDP, physical timing, rendering, sound or playable-game proof is inferred.
No disposable task directory or downloaded payload is retained; only existing
.ninja/.ccache graphs are used. T3 and M1 remain open.

## Archived S19 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: MyRaidenII M1 T3 S19 after accepted S18 e7de691e0; one-session sequential coordinator/executor. |
| Admission And Approval | Owner's complete MyRaidenII implementation approval and delegated ordinary same-T planning. Use the explicitly approved S18 math vocabulary; no further Shared edit is admitted. |
| Objective | Consume the full S12 nine-ID numeric ledger: 130e/2208/338e/3bb0/42c2/4aa0/6200/8100/8900, arithmetic status/distance/angle results and full chains/failure/reset/admission proof. |
| Non-goals | No physical COP timing claim, uploaded microcode interpreter, copied reference implementation/table, Shared API change, RNG/DMA/2a05/unresolved algorithm repair, game startup or playable qualification. |
| Reference Baseline | Clean pushed e7de691e0; S12 complete ledger, S13 common entry-status correction, S17 angle approach and S18 four math aliases. Fixed MAME c1425a1af00b88aa07477b1b23a30ed5e5f856ff cmd.ipp SHA B1E84AEF5BFDEBCA53D8F0AF0B1BE481B8218157BA460C1714853D322AC92EFC, factual L2 research only. |
| Candidate Proposal | ../proposals/m1-board-cop.md B07/B08; ../etc/evidence/t3-s12-cop-geometry-contract.md complete nine-member receiver, superseded only by explicit S13 correction and S19 documented arithmetic choices. |
| Files And ABI Surface | Existing src/myraidenii/core/cop.c and its CMakeLists.txt; existing test/myraidenii/unit/core/board_bus_smoke.c; product records only. No public API/state mirror. Existing register store grows28 bytes through5b5h, not S12's historical176-byte arithmetic estimate. Production +150--220/-5--15; existing test +220--320/-5--15. |
| Applicable Rules | Architecture sole COP register/result/admission owner and existing Core DMA/safe point; Coding bounded conversions, unsigned modulo differences, no negative shifts, existing table-driven test reuse and optional Types math only; Source original implementation from declared mathematical contracts, no reference body/table import; Execution full batch before/after, exact proof, cached build, complete P and actual coordinator review; Documentation L2/latency and live-read uncertainty explicit. |
| Verification | Independent literal quadrants/wrap/word sign/distance extremes/division zero and scales/trig cardinals and neighbours; all three ports. Actual Board chain, byte/word/result read-only/observe/reset/busy/upload behavior. Existing isolated actual COP fixture covers each operand read and late/partial output failure with old/new result publication and common error-bit clearing. Reuse old transfer/update/collision matrices. Stable x64/x86 strict builds and focused test, then full687 units sequentially four workers/600-second aggregate; docs/whitespace/protected inputs/ten release guards. |
| Expected Markers | One numeric dispatch family shares coordinates/checked DMA/result store. Common status error clearing happens once on admitted service. Result words5b0/5b2/5b4 have one read-only owner; reset zeroes them. No live-memory mirror or duplicate math-library implementation. |
| Asset Needs | ROM-free literals only, no task temporary root. Fixed reference responses in memory only. Reuse .ninja/.ccache, preserve all ten current product EXEs and protected inputs; no MyRaidenII frontend exists yet. |
| Reporting Requirements | Before code report complete batch, selected L2 choices and estimates; record actual delta and full failure/publication order, independent oracles, dual-width proof, retained T3 gates, source hashes and actual-change review. |
| Stop Conditions | Shared/public extension, undefined arithmetic or unbounded floating conversion, copied reference source/table, altered protected input, another execution/time/result owner or unsupported physical/gameplay claim. |
| Exit Criteria | All nine ledger members and common results have direct actual-bus proof with correct byte/partial failure/reset/status chains; old regressions and full dual-width units pass. Complete implementation P pushed, coordinator actual-change acceptance and governance P pushed; T3 remains open for its other obligations. |
| Original Owner Request | Implement complete playable MyRaidenII, single-session dual-role governance, coherent batch implementation and reuse/extend existing tests rather than repeated per-opcode tests. |
| Similar-Issue Sweep | All arithmetic result stores/conversions and angle/distance/division/trig variants, all command ports, existing handler dispatch error clearing and read/write boundary prefixes; no duplicate angle calculator or legacy branch. Inspect retained unmodelled routes and any changed rejection expectations rather than weaken old tests. |
