# T2 S30 V30 Native Transfer Nominal Timing

Baseline `ec7e857b5`, after accepted S29. This is the native data-transfer
nominal-clock subfamily of V15, not complete V30 timing or Machine admission.
Final qualification is complete. The full original CPU/board/game scope remains.

Shared implementation P1 is `daf4883f4`; receiving NXVM P2 is `fcfaf67f3`.

## Primary Source And Model Boundary

NEC V20/V30 User's Manual, October 1986, section 12-1 and printed pages
12-4..12-13 (PDF pages 49..58), pinned SHA-256
`FABBFCD19D4A2FCDAAC9F0BC06B00BF0A6A4BEEDAFBD3FF16ACCD075A374EA4C`.
[Original source](https://www.dosdays.co.uk/media/nec/NEC-V20-V30-Users-Manual.pdf).
The complete ten pages were rendered and visually inspected, including both
columns and the distinct V20/V30 odd/even rows. No external implementation,
manual payload, rendering or protected game data enters tracked source.

The source assumes prefetched instruction bytes and includes decode,
effective-address calculation, operand access and execution. These constants
are therefore not an Intel base plus EA clocks or a cost per callback.
Word-odd MOV adds four clocks; a two-word pointer or read/write XCH adds
eight. Segment bases are real-mode paragraph-aligned, so decoded offset
parity is sufficient even after LDS changes a segment register.

## Complete Admitted Source Family

| Encoding / source operation | Register | Byte / aligned word memory | Extra odd word |
| --- | --- | --- | --- |
| 88/89 MOV to r/m, 12-4 | 2 | 9 | 4 |
| 8A/8B MOV from r/m, 12-5 | 2 | 11 | 4 |
| B0..BF immediate to register, 12-6 | 4 | not applicable | not applicable |
| C6/C7 immediate to memory, 12-5 | no documented nominal alias | 11 | 4 |
| A0/A1 direct load, 12-6 | not applicable | 10 | 4 |
| A2/A3 direct store, 12-7 | not applicable | 9 | 4 |
| 8E segment load, 12-7..12-8 | 2 | 11 | 4 |
| 8C segment store, 12-8..12-9 | 2 | 10 | 4 |
| C4/C5 far-pointer loads, 12-9..12-10 | invalid form | 18 | 8 |
| 8D effective address, 12-11 | invalid form | 4 (no data read) | 0 |
| 9E AH to PSW, 12-11 | 3 | not applicable | not applicable |
| 9F PSW to AH, 12-10 | 2 | not applicable | not applicable |
| D7 translation, 12-12 | not applicable | 9 (byte lookup) | 0 |
| 86/87 XCH, 12-12..12-13 | 3 | 16 | 8 |
| 90..97 accumulator XCH, 12-13 | 3 | not applicable | not applicable |

The new private selector consumes the existing completed decoder record. It
publishes the documented nominal subtotal through the sole CPU timing
selector, with the appended approved `V30_NOMINAL_SOURCE` origin. Existing
origins retain their values; no function or structure layout changes.
Keys are primary opcode plus bit 8 for memory ModRM forms; direct/implicit
memory does not claim ModRM/EA formula inputs. No new persisted CPU state.

Every V30 result remains `source_timing_unallocated`, maximum query remains
zero and Machine's explicit profile switch still rejects V30. A nonzero
nominal subtotal is not elapsed time, a bus/prefetch model or production
eligibility. Prefix/undocumented aliases, other native families, 8080 and
all timing-envelope work retain their original V15 receiver. Unknown results
are zero/unattributed rather than fabricated Intel compatibility time.

## Actual Proof And Initial Corrections

The new CPU-owned unit executes actual guest instructions. Its independent
source table drives register/memory widths and odd penalties, not a copy of
the production selector. It covers 24,160 inputs: every applicable ModRM
register/address form, two displacement/parity scenarios, no prefix and four
segment prefixes; all direct/immediate/accumulator/AH/translation forms;
LOCK/REP/carry-repeat/NEC/out-of-family exclusions; actual BRKEM to an 8080
instruction; fetch/read/write refusals; same-live-CPU publication lifetime;
pure preview and all six old Intel profiles.

C6/C7 register aliases intentionally remain unallocated. C4/C5/LEA register
forms and invalid segment selectors are covered by the existing admission
unit, not executed as documented source forms here. Existing architectural,
transfer/phase/accepted-prefix tests remain intact; the new test does not
replace them with clock-only assertions. Memory is reused only after each
case initializes its code and addressed data for the explicit memory forms.
Fixed nominal costs do not depend on incidental lookup/operand values; their
architectural values already have the independent S16/S21 proof.

Initial focused proof caught a real metadata defect in the candidate:
`flagMem` also covers direct MOV, so using it alone reported the A0 form as
ModRM. The source owner now distinguishes lexical ModRM before applying
formula inputs/form identity; the regression explicitly checks that bit.
A test incorrectly used REPC/REPNC MOVS, prohibited by existing source form
admission; it now uses documented comparison forms. The production rejection
was not weakened. One premature CTest invocation before its build finished
was Not Run, not a product failure or pass; the same build was polled to exit
and final qualification runs only after complete build completion.
An incorrect standalone test-manifest command was replaced by the existing
source verifier with the test corpus root; no new checker was introduced.

The first complete x64 suite passes 678/678. Its following specialized gate
rejects the candidate comment's word READY as a forbidden board input. The
actual selector reads no signal or board state; the comment is corrected to
extra waits without weakening the existing gate. Final rebuild proves the
CPU timing object remains byte-identical:
`E48984393EFEA509086C7492D76E3D8E7D90C62FDC4FCF2DE57707CC143AE299`.
The complete x64 unit result therefore remains proof of the unchanged
executable mechanism, not an unexplained retry or silently discarded failure.
All specialized gates are rerun successfully after that comment correction;
the final x86 full build/unit/gates and receiving publications follow normally.
The original failed gate log and terminal handle are retained with the resume
driver, not represented as a final pass. No assertion/budget is weakened.

## Similar-Issue Sweep

Queries:

- `rg -n 'V30|source_timing_unallocated' src/x86-core/cpu src/x86-core/machine test/x86-core`
- `rg -n 'timing_select|timing_maximum_ticks|capture_timing|preview_lexeme' src/x86-core test/x86-core`

CPU selection remains unique. Its old Intel chain is unchanged and the
NEC branch cannot fall through to it. Actual instruction completion invokes
selection before asynchronous entry reuses the decoder record. Preview does
not execute the new selector or mutate its result. Refresh/reset clears
old timing, failure never selects a successful nominal result, and the
same-live-CPU test proves no old subtotal survives later unknown/failure work.
The original successful/fault/event distinctions from S29 remain.

The maximum query and Machine admission are intentionally unchanged: both
remain closed until the full envelope/form domain is ready. Machine's
physical qualification also rejects unallocated results. Existing compatibility
timing semantics for old Intel profiles are not globally reinterpreted by
the new V30 contract comment. Bus callbacks, READY waits, prefetch, LOCK
signals and event timing remain their actual existing owners; this batch
does not sum those partial facts or manufacture a second timeline.

## Counted Change

Tracked C/H/CMake source/test diff against baseline, excluding manifests,
records, generated inputs and artifacts: production +83/-6/net +77;
tests/registration +276/-0/net +276; total +359/-6/net +353.
Manifests separately +4/-3/net +1. The production helper has one actual
selector caller and cohesive source-family responsibility; no test-only
entry point or new persistent state is added. The 275-line unit adds real
independent source/form/lifetime proof rather than another execution path.
Final actual-diff review must reconcile these counts before acceptance.

## Final Qualification

Both original all-target Release Ninja/ccache graphs pass. Complete repository
units pass 678/678 per width, including 191 X86 Core units, in
312.58/152.34 seconds. Both twelve-unit related groups pass. The new 24,160
input unit takes 0.17714/0.302949 seconds in the complete suites. All 498 strict
rows and Types/dependency/ownership/negative/manifest gates pass; the global
Types check includes 1,388 tracked code/script files. Existing instruction
warning exceptions remain explicit; new code/tests use normal strict warnings.

Eight PC 0564 EXEs are rebuilt/deployed. Fourteen selected receiving cases
plus restored-default identity pass per width; both original graphs are
default and profile switches recompile zero CPU objects. All ten actual
PE/name/width/size/hash identities pass; MyNES 0044 and all INIs are unchanged.
DeskPro boot passes in 136.96/170.79 seconds without retry or budget changes.
The resumed final process exits zero. The initial x64 comment gate failure
and exact object-equivalence basis for retaining its full unit proof are
recorded above, not silently treated as an initial all-green run.

[Measured results](t2-s30-results.json) and
[ten deployed identities](t2-s30-artifacts.json) come from final JUnit/logs
and actual deployed PEs, checked against frozen inputs. The retained ignored
qualification/resume/collector scripts, logs and original manual support this
scope and the next timing receiver. No raw guest trace is generated or retained.
Documentation/whitespace checks pass. Shared raw/index bytes are identical LF.
No full external/native integration, manual desktop/RDP/audio or gameplay
claim is made; original complete timing and product gates remain open.

## Complete Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation, MyRaidenII M1 T2 S30, next unused S after accepted S29. |
| Admission And Approval | Owner requests automatic continued single-person coordinator/executor implementation. Existing 2026-10-17 approval covers necessary V30 timing-origin enum extensions, preserving existing values/signatures/layout. Coordinator admits the source-qualified transfer timing batch on 2026-10-18: Shared X86 Core CPU/tests; NXVM eight receiving releases/status; MyRaidenII evidence. No Lib/Emulator/MyNES source change. |
| Objective | Consume V15 native data-transfer nominal clocks from original NEC section 12-1 and 12-4 through 12-13 at the existing sole timing selector. Cover MOV general/immediate/direct/segment, LDS/LES, LEA, XLAT, XCH and AH/PSW transfers, widths and odd/even data addresses. Keep incomplete total timing explicitly non-eligible; never substitute Intel clocks. |
| Non-goals | No full V30 time, prefetch/READY/bus-lock model, extra-wait aggregation, arithmetic/string/control/8080 timing, Machine/board/game admission, public function/layout, production state, Lib/Emulator or MyNES edit. Prefix/undocumented alias forms remain unallocated until their own complete source-qualified mechanism is connected. No fixed tick compatibility or invented L2 timing. |
| Reference Baseline | Clean pushed ec7e857b5; NEC October 1986 manual SHA FABBFCD19D4A2FCDAAC9F0BC06B00BF0A6A4BEEDAFBD3FF16ACCD075A374EA4C, section 12-1 assumptions and printed 12-4..12-13, PDF pages 49..58 visually inspected. Existing V15 ledger and S16 transfer results remain authoritative for architecture, not clocks. |
| Candidate Proposal | proposals/m1-v30.md and V15 nominal transfer subfamily. The CPU timing selector remains the sole publisher. A private source evaluator supplies only documented prefetched zero-wait nominal clocks. Existing source_timing_unallocated stays true until the complete envelope is available; V30 maximum remains zero and Machine admission remains rejected. |
| Files And ABI Surface | src/x86-core/cpu/cpu_timing.c, cpu_interface.h appended nominal-source origin and contract comment, manifests; new owned CPU timing unit/registration and only directly affected assertions if needed. Estimate production +75..110/-5..12, tests +180..280/-0..10. No new public function, layout, state or external dependency. NXVM artifacts/status and MyRaidenII records separately committed. |
| Applicable Rules | MyRaidenII guide/architecture/coding/roadmap/source policy; Execution lifecycle/coverage/artifacts; Architecture/Coding skills; Documentation; PDF read-only source review. Receiving NXVM guide/design/source/artifact rules before release. Single selector, Lib Types, preserved executor style, no third-party implementation import, one target per P. |
| Verification | Actual guest CPU execution for every admitted opcode and documented form; all ModRM addressing modes/registers, both widths, segment choices, parity and wrap. Verify exact independent manual clocks, rejected prefix/alias/emulation/out-of-family forms, provider failures/no stale timing, pure preview and old Intel contrast. Both original Ninja/ccache builds -j8, related batch then complete repository units -j4 and specialized gates; 14 selected PC receiving cases plus restored identity per width; ten PE/hash checks and unchanged MyNES/INI. |
| Expected Markers | V30:NOMINAL-TRANSFER:OK with measured vector count; complete suites zero failures/skips, 498 strict rows and all gates retained. Every partial nominal result remains source_timing_unallocated with zero maximum; non-applicable results retain zero ticks and unattributed sentinel. |
| Asset Needs | ROM-free synthetic tests and pinned read-only manual. Eight current PC 0564 EXEs rebuilt/deployed; ten identities including unchanged MyNES 0044. Preserve all owner INI/media/ROM/snapshot inputs. No MyRaidenII executable or game ROM publication. |
| Reporting Requirements | Report source scope, diff estimate, focused result and actual changes, source-versus-total timing distinction, final dual-width qualification/artifact hashes, failures/corrections and unverified physical/desktop/game boundaries. |
| Stop Conditions | Unresolved source contradiction affecting admitted clocks, required new timing model/public contract outside approved enum, unsafe stale decoder facts or regressions prevents acceptance. Do not guess timing or broaden nominal eligibility to satisfy tests. |
| Exit Criteria | Complete admitted transfer-clock family has independent actual-execution proof, non-eligible/prefix/alias boundaries are tested, no Intel fallback or duplicate state, full dual-width units/gates/receiving releases pass, manifest/evidence updated and target-separated complete Ps pushed. Coordinator actual-change review then governance closure; T2 remains open. |
| Original Owner Request | Implement full playable MyRaidenII under single-person two-role governance, including NEC V30 and required facilities, automatically continuing bounded S work without redefining completion. |
| Similar-Issue Sweep | Audit all V30 timing publishers/preview/max/Machine consumers and transfer instruction decode/address facts. Compare all source page variants, source prefix and undocumented alias boundaries, old Intel selectors, failure and reset clears. Record every retained zero/unallocated path as original timing receiver, not a falsely complete CPU. |
