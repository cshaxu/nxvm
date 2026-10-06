# T546 S2 Decode Admission Implementation And Proof

## Approval And Baseline

Owner approves the [concrete design](t546-s2-decode-admission-design.md) after
S1 `55348a583`, including the specific-source 286 UD disposition and necessary
next-fetch postcheck removal. Implementation is active; this record does not
accept S2 or claim complete family qualification. Current owns status.

## Complete Mechanism Map

| Receiver | Existing-owner change | Direct regression and present proof state |
| --- | --- | --- |
| Thirteen unchecked early decodes | Every direct `_d_*` call uses existing CHECK_RETURN before dependent effects. | One table: all thirteen forms, every required suffix byte, all five profiles. Provider distinguishes required fetch from prefetch/data; checks original CE, CPU rollback and absent dependent reads/writes/port completions. First x64 probe passes; final-source dual-width proof pending. |
| Competing code advances | All 224 direct IP increments use existing `_adv`; `_kdf_skip` retains early 16-bit wrap and later continuous sequential cursor. | FFFF NOP retires without testing unused SS pointer; next late-family fetch faults at its own advanced cursor. Final-source proof pending. |
| Family length | `_s_read_cs` checks existing start/cursor distance and requested span before cached/provider reads: 286 ten/UD, 386 fifteen/GP0. No early invented limit or mirrored counter. | Cached/required prefix matrix and immediate, ModRM/displacement, SIB, operand/address-size legal/illegal endpoints. 186 long-prefix cases preserve current policy, not proof of an undocumented maximum. |
| Premature following-page fault | Initial prefetch is bounded by page and CS; required operands use existing checked fetch. | Physical page-table fixture: page-end NOP succeeds without next page; required immediate faults on absence or reads mapped bytes. No modulo-memory shortcut. |
| Zero-padded preview/observation | `oplen` reflects actual capture; successful required fetch extends the existing copied opcode image. Preview scans captured bytes, never handlers. Early wrap uses modular observation distance. | Page-end NOP preview is available, incomplete immediate unavailable; CPU/page tables unchanged. Final-source proof pending. |
| Init error overwritten by debug/decode | ExecIns finalizes init failure before breakpoint matching or dispatch. | Failed initial fetch plus enabled matching execution breakpoint stays CE; no required/data work or changed CPU. Final-source proof pending. |
| Unused next-EIP/ESP tests | Delete generic postchecks and sole-call helpers; retain actual access/branch/frame checks. | Endpoint retirement/next-fault attribution plus established transfer/frame regressions. S4 retains complete later qualification. |
| LLDT/LTR and LGDT/LIDT permission | Preserve form decode; reject CPL before operand read; retain selector/cache load owner. LGDT/LIDT's VM-excluding predicate is corrected as detailed below. | Four forms: 286 protected and 386 protected16/protected32/VM86, absent operand reads/LDTR/TR publication. Invalid IDT exposes downstream outcomes separately, not all delivery-priority proof. |

Budget applies to required code spans, not array size or dispatch count.
Lookahead does not count the same byte twice. No new API/parser/callback/framework
or architectural state. The 237 advance/check substitutions preserve original
handler tables/arithmetic bodies; nonmechanical changes are mapped above.

Additional original proof was rendered and inspected: 1987 286 programmer
PDF 139, printed 7-13 section 7.4.2, states that the last byte is within the code
limit and prefetch itself does not cause a limit fault; attempted execution
beyond the end does. Hardware PDF 52, printed 3-2, states prefetch stops at the
code limit. This supports boundary-limited fill and removal of premature
post-retirement validation, not an arbitrary emulator convention. Hardware
original SHA-256 is
`D3ECE037A200B17EF32D78A055D0D96C3E47474D755D94569B9576A9A68C6915`.

## Similar-Issue And Target Sweep

All-direct-decode/direct-IP searches now return zero production hits. The new
owner-local static gate rejects those bypasses and obsolete postcheck names.
Shared edits are CPU C, test/x86 registration/runtime/static proof and their two
manifests only. No public header, Lib/Common, IBMPC production, App/firmware,
INI, external media or MyNES input changes. MyNES pair hashes still match S1;
it is not rebuilt.

## Verification Ledger

- New regression before final incremental refinements: x64 passes once, 0.50 s;
  not final-source execution or complete-unit proof.
- Static decode-admission, x86 corpus and test Types checks pass.
- Eight manifests checked; test/x86 became stale after another test edit and
  must refresh/reverify after source freeze. Other seven pass; no failure waived.
- x64 build handle 3076 and x86 58056 remain live at this progress checkpoint.
  Configure handles completed, generation x64 105.2 s/x86 332.3 s. Do not restart
  a live build because observation yielded.
- Late source/test refinement followed the first x64 CPU objects. Final
  incremental rebuild must precede acceptance; check object/executable times
  and registrations. Stale root binaries/first probes are not final evidence.

Complete dual-width units, specialized/supplemental gates, actual receiving
0546 targets, eight artifact identities and original external contexts once
remain pending. No implementation P delivered.

## Retained Task Boundaries

1987 286 B-9 remains a recorded conflict; owner accepts specific 1985 section
9.6.1 and agreeing appendices for UD. Missing 186 limit proof is not Manual-L3
or hidden by long-prefix success; retain it in original source convergence.
S3/S4/S9/S10/S13/S14/S17 retain full segment, transfer/frame, delivery,
descriptor, paging, restart and timing proof; this local test does not resolve
those whole domains.

## First Complete Receiving Run And Corrections

First final-source x86 complete unit attempt: 527/534 pass, 291.42 s. Seven
registrations fail: two aliases of operand/address, bus observation, preview,
cross-page paging, new admission test and FDC negative timeout. No failure is
waived. The quiet FDC negative rerun passes in 4.35 s without changing its
timeout; concurrent full builds caused the earlier resource contention.

Source/context reconciliation finds three false expectations: 386 sequential
16-bit fetch wrapped to zero, 8088 capture always contained fifteen real bytes,
and a required first-page prefix left A clear when the next required page failed.
Correct the owning predicates, preserve all valid tests, and distinguish valid
complete RAM-end preview from a genuinely truncated instruction. Existing
Core preview needs actual readable suffix bytes: extend the copied peek through
the same checked CS owner with observe-only semantics, rather than invent zeros
or return unavailable merely because the cache was short.

The cross-mode matrix corrects an earlier audit statement: LGDT/LIDT's existing
`_IsProtected && (...VM...)` predicate excludes VM86, so it was not a complete
early permission check. Both now use the existing CPL definition before operand
access; LLDT/LTR retain their mode/form check and corresponding early CPL check.
Fault-delivery reads in the VM fixture are not operand reads; independently
count the selected operand address. Invalid delivery remains a S9 receiver,
not falsely qualified here. The new admission test then passes once on x86.

Original x64 receiving groups: XT 1/1 (22.19 s), AT 3/3 (38.79 s), Model40
2/3 with its boot context failing at 180.98 s in ROM F000:D130. No timeout,
checkpoint, INI, media or BIOS rule is relaxed. After the later CPU corrections,
all products/tests require final-source rebuild/proof; those earlier passes
alone are not final acceptance. The existing NXVM boot harness gains bounded
CRTC/fault observations for one thirty-second diagnosis using the same inputs.
No partial P, source-family qualification or S2 closure is claimed.

## Bounded Model40 Diagnosis And Pending Data-Owner Decision

Release development trace counters stay zero; they are not evidence of absent
CPU/port work. The existing public copied retirement observer was then enabled
for a bounded thirty-second diagnosis, retaining only 32 scalar samples.
It proves IN 61h at D12A, not CRTC I/O: successive starts are 79738695,
79738727, 79738759, etc. Per-loop source values are 12, 2, 2, 3 and 13, total
32; every observed input value remains 10h. CPU remains actively retiring.
No first CPU fault is recorded. The earlier display-check description is
corrected to refresh-bit polling; unchanged DX=3B4 was not the immediate port.

Current Model40 primary PIT ratio is 1:1, reset counter 1 is mode 2/divisor 18,
and D4 directly exposes OUT1 in bit 4. Its one-input-clock low interval and
fixed sample parity produce the identified aliasing dependency. Correctly
captured register ModRM must not be reverted to fake zero/memory timing to
change the loop period. Full board timing remains a separate authority.

Original D3PE was rechecked in the existing owner archive scan 1 of 2:
SHA-256 `5D37A105E45A91FDD9C2552514F27C5F9A92177DF956035A5FE356B7FD6F7B52`.
PDF page 100, printed page 5 (5 January 1987), was rendered and visually
verified: OSC 14.31818 MHz, /2 then /6 TIMCLK. The existing macro axis is
16 MHz. This supports a proposed L2 proportional input, not physical closure.
The [proposal](../../proposals/m5-cpu-audit-gap-repair.md) records the exact
candidate fraction and whole related-clock review. Owner scope approval is
requested before touching App profile data or any new shared interface.

The corrected five targeted x86 regressions pass 5/5 (3.72 s). Complete
final-source dual-width builds remain live (8902 x86, 29288 x64 at this
checkpoint); neither full unit acceptance nor Model40 boot success is claimed.

Original clock formula was subsequently visually verified on D3PE page 5;
the primary candidate uses the exact printed OSC/divider formula reduced to
715909/9600000 against the existing macro axis. This conversion remains L2.
The historical physical-axis rejection in T387 is not silently reinterpreted
as physical qualification. Related auxiliary pin/clock inputs still need their
own reconciliation rather than an arbitrary phase/period adjustment.
The misleading CRTC counters were removed; bounded fault and actual retirement
samples remain in the existing integration harness. No profile edit is made.

Latest complete x86 rebuild finishes successfully. Final-source complete units
pass 534/534 in 193.35 s, eight jobs under the unchanged 300-second suite budget.
All seven first-run registrations now pass, including the unchanged FDC
negative timeout. The x64 complete rebuild finishes and its final-source full
units pass 534/534 in 219.60 s with the same eight-job/300-second bound. All
eight manifests and the source corpus, Types and decode-admission gates pass.
These results qualify neither Model40 boot nor the remaining T batches.

Supplemental registered CTest checks pass 33/33 per width, x64 248.04 s and
x86 202.93 s. All eight manifests independently match current source/test
bytes; corpus, Types and checked-decode static sweeps pass. No rule or timeout
was weakened. Final-source fixed-product/boot receivers for default, AT and XT
finish rebuilding independently; Model40 profile data still awaits the separate
scope decision. Product candidates are not accepted by static/unit passes alone.

Final-source original non-Model40 integration groups pass once: default 22/22
per width (x64 13.59 s, x86 14.30 s), XT 1/1 per width (17.73 s/22.24 s), AT
3/3 per width (30.95 s/39.02 s). Total 52/52. A first combined shell invocation
exited after the first group because its managed runner did not set native
LASTEXITCODE; logs proved only default x64 ran. The remaining five groups ran
with terminating-error handling, without repeating that successful group.
No input, checkpoint or timeout changes. Six PC products are current candidates;
the remaining Model40 six receiving contexts stay unaccepted, not deleted.

Both final specialized gate aggregates complete successfully. T345's printed
duplicate-row rejection is the expected negative self-test followed by its
pass marker, not a waived failure. Original owner INIs and excluded Lib/Common/
MyNES trees remain unchanged. Six non-Model40 optimized 0546 candidates rebuild
with verified PE architecture. Their current hashes are:

| Candidate | SHA-256 |
| --- | --- |
| default x64 | 7A53D0685D5FC05774C6E910DDA2AF63C65F7A6D1ECBDB60FC5CD2F899402392 |
| default x86 | 555FA4949712867E9165BD6EA3A6A1B456C66C88C7905D58862C299FA214C88C |
| XT x64 | B0A4E35ACD4B7CC86C89EB5ACF9F9A6A36F32F20B320C0A686650BEFBF8819A1 |
| XT x86 | C1DE3D64DF26E6A472DB65F7C8E69E80518EFA1B86DC35D229E2ED85FE0C50E0 |
| AT x64 | 3C9BB8E232EC4A17D62CF3523AA40E2FC4942379A4B7079D9BF21DD8F87494D7 |
| AT x86 | 40B3CD5488301099281977E2C7E9EC7BAE14BE444853959F13F7168C7479E09D |

All owned build/test handles have completed. Scope approval for Model40
clock-input reconciliation is the remaining implementation prerequisite;
no unapproved profile edit or partial P is delivered. T546 remains open.

## Owner-Approved Model40 Clock Reconciliation

On 2026-10-06 the owner approves the whole related-clock input batch. Original
Compaq D3PE (January 5, 1987), scan SHA-256
5D37A105E45A91FDD9C2552514F27C5F9A92177DF956035A5FE356B7FD6F7B52,
is reviewed visually: printed pp. 4-5, p. 22, schematic sheets 5-6 (PDF
8-9). Sheet 5 connects both DMA CLK pins to DCLK. Sheet 6 connects U44's
three CLK pins to DCLK and U45's three CLK pins to TIMCLK. U44 is the
auxiliary timer (failsafe/extra/speed); U45 is the primary system/refresh/
speaker timer. Printed p. 31 (PDF 126) visually confirms the RTC crystal.
No per-counter clock API is needed.

| Input | Existing | Corrected/disposition |
| --- | --- | --- |
| Primary PIT | 1/1 | OSC/(12*16 MHz) = 715909/9600000, zero phase. |
| DMA | 1/1 | Nominal DCLK/16 MHz = 1/4. Actual BCLK resynchronization makes this an approximation. |
| Auxiliary PIT | 5/16 with incorrect 10 MHz BCLK comment | Same nominal DCLK ratio 1/4 for all three pins; no invented pin model. |
| RTC | 1/1 into a 32768-tick second | 32768/16 MHz = 256/125000. Original CMOS circuit describes the 32768 Hz crystal. |
| FDC duration conversion | 8 MHz source-unit conversion | 16 MHz source-unit conversion; this field scales microseconds into the sole Core axis, not the FDC crystal. |
| KBC | 1/1 service ticks | Retain: delivery/keyboard timers consume configured service units, not 8042 instruction-clock edges. PCLK frequency is not a replacement for that API's units. |
| VADP | 1/1 service ticks | Retain: configured text timing consumes service units, not processor-board OSC edges. No video frequency claim is added. |
| Provider | 1/1 source ticks | Retain: callbacks consume the sole Core axis directly. |

All conversion to the existing MACRO_PROPORTIONAL execution axis remains L2.
Printed hardware numbers/formulas remain source facts, not proof that each CPU
elapsed tick is a physical clock. CPU instruction costs, refresh pulse shape,
firmware, INIs, Lib/Common/MyNES and all public interfaces are unchanged by
this board correction. Existing App tests check exact frozen ratios and FDC
units; a repository-only input-clock regression also proves one macro second
delivers 32768 RTC ticks and a 32-tick refresh poll observes low within its
bounded sample matrix. Full receiving verification remains pending.

Clock-source Model40 original contexts pass: x64 boot 130.57 s plus Console
and CMOS 2/2 (0.62 s); x86 all three 3/3, 164.85 s total, boot 163.74 s.
Both boot predicates retain their original 180-second limit. The six contexts
complete the original 58/58 receiving matrix together with the final-source
52/52 non-Model40 contexts above. An initial incorrect bare CTest name matched
zero tests and is not counted; corrected registered names use no-tests=error.
No successful boot group is repeated. Product pairs rebuilt with PE-width checks.

Both App clock regressions pass; the first FDC test attempt fails because its
old fixed 128-tick byte interval assumes an 8 MHz source axis. Replace that
test-only advance with the existing 15 microsecond byte contract converted
through the selected source rate; do not undo the corrected production unit.
Complete post-clock dual-width unit builds/tests remain pending.

Model40 artifact hashes after this correction:

- x64: 68329073BFE8C0F54DD03581C8A3389FD4669170D54B108929E3CCAA1F8111B5.
- x86: E441A8E8DCA04540790AEAB1138E0627862F2D41541964E176888FCCF129482C.

All five external media hashes freshly match T545/S1. Owner INIs and MyNES
pair are unchanged. The bounded trace-only retirement ring is retained for
the admitted CPU follow-on diagnosis; it never runs in ordinary acceptance
and adds no production state/API. Its 32 copied samples are RAM-only, not an
unbounded recording route. First-fault reporting uses the existing public
copied diagnostic. The inspected source graph has one CPU fetch/admission
owner and one board clock conversion; no compatibility CPU cost or BIOS path.

First post-clock x64 full unit run is 533/534 in 241.11 s. The sole failure,
Model40 BYOB reset, advances one source tick and assumes that is a PIT input
edge. Preserve its original count-18/reset assertion but advance to the first
actual input edge, ceil(denominator/numerator), using the frozen clock ratio.
This is the same old identity-clock fixture assumption as the FDC fixed-byte
interval; no production compatibility branch or weakened predicate is added.
Final full-suite proof must follow the corrected fixture rebuild.

The other-width receiver run also completes 533/534 (x86 228.59 s),
with the same sole old BYOB assumption; no second defect class is hidden.
Both corrected FDC receivers pass, and the corrected BYOB source is rebuilt
per width before final verification. The similar-assumption sweep includes
Model40 App tests and default App unit fixtures: other one-tick advances are
explicit identity-clock fixtures or other unchanged profiles, not receivers
of this Model40 ratio. No unrelated board test is rewritten.

Both corrected BYOB tests pass (x64 0.65 s/x86 0.38 s). A subsequent x64
full run passes every behavior receiver but the FDC negative static test
times out while the x86 target rebuild overlaps it: 533/534, 208.61 s.
The identical negative test passes quietly in 4.90 s without any source or
timeout change. Final full suites therefore run with four jobs and no live
build competition, within the unchanged 300-second budget; the earlier
failed aggregate is not accepted or replaced by the isolated pass.

Final post-clock/post-fixture x86 complete units pass 534/534, 107.07 s,
four jobs with no build overlap. FDC and CPU negative static tests both pass
with their original deadlines; all source-conditioned behavior assertions
remain intact. The x64 final aggregate is still pending.

Counted source/test/build descriptions (Git numstat plus new test files,
excluding manifests/docs/artifacts): fourteen paths +872/-314, net +558.
Production alone is three existing C files +295/-294, net +1. The positive
test delta supplies the missing failure/mode/byte/preview and clock-unit
proof, not a production framework or mirrored architectural state. Original
handler tables and CPU timing-selection algorithm remain the sole paths.

## Final Executor Verification And Delivery Review

Final post-clock/post-fixture full units pass 534/534 per width: x64 107.68 s
and x86 107.07 s, four jobs under the unchanged 300-second suite bound.
No source/build operation overlaps them. All old failing fixture assumptions
and both negative static gates pass. Original external receivers are 58/58
as detailed above, once per successful final-source group. CPU-source
supplemental checks 33/33 and both specialized aggregates remain valid: their
shared inputs have not changed since those passes. Final App clock data and
fixture corrections are additionally compiled and covered by these full units.
All eight manifests, checked-decode/corpus/Types sweeps, final documentation
gate and diff-check pass. Eight optimized stripped PC EXEs have verified
8664/014C PE machines and the recorded hashes. INIs, external masters and
the exact MyNES pair remain unchanged; there is no owned live build/test.

Executor self-review inspects the actual CPU diff, complete new table-driven
regression, all corrected receiver predicates, App clock graph/pin sources,
manifest registration and artifact/documentation changes. The architecture-
and coding-governance skills enforce existing owner/one-path boundaries,
original handler style and source-conditioned tests instead of wrappers or
BIOS conditions. No public header/API, Lib/Common or MyNES change is present.
The required explicit receiver correction removes the sampling dependency
without changing a sourced CPU cost. Remaining S3-S19 qualification and the
original source conflicts stay in the complete T ledger; no whole-family or
physical-L3 claim follows from S2. Deliver Shared then NXVM P commits; a later
pure governance P records coordinator acceptance and closes S2 only.

Shared implementation P1 is 34867b960, pushed. The current artifacts contain
that exact CPU source plus the reviewed Model40 App input diff; source commit
and the recorded binary hashes identify them independently of build timestamps.

NXVM implementation P2 is 01709c418, pushed. Coordinator actual-change review
accepts both target-separated deliveries and every assigned exit above. The
retained 186/source-conflict and full later mechanism receivers are not hidden
by unit/boot success. Governance acceptance closes S2 only; T546 stays open.
