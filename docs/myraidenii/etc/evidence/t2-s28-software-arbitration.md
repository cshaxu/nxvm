# T2 S28 Software-Entry Arbitration

Baseline `a50905530`, after accepted S27. This consumes V11's complete
software-entry/completion-boundary priority subfamily, not all V11 or T2.
Final results and release identities below record the completed qualification;
T2 and the playable objective remain open.

Shared implementation P1 is `2a51bee0e`; receiving PC artifact P2 is
`67b32dab5`. The latter retains PC revision 0564 and changes no owner INI.

## Primary Source And Before Witness

The pinned [NEC October 1986 V20/V30 user manual](https://www.dosdays.co.uk/media/nec/NEC-V20-V30-Users-Manual.pdf), SHA-256
`FABBFCD19D4A2FCDAAC9F0BC06B00BF0A6A4BEEDAFBD3FF16ACCD075A374EA4C`,
Table 6-1, printed 6-1 and sections 8/12 identify software entry before NMI,
INT and BRK single step. BRKEM/CALLN's individual flag tables preserve IE/BRK
while changing MD; they are not disabling asynchronous entries.

Before repair the actual BRK3/IE+BRK/NMI+INT witness reports
`pc=408/40c sp=6f5/6fb`: vector 2 immediately nests inside vector 3,
pushing two frames before either handler executes. The ignored before-test
log records the failure; this is CPU execution, not a synthetic converter.

## One Owner And Required Private Result

Existing protected-transfer timing metadata is not a software-entry outcome.
Fault-delivery bookkeeping suppresses retirement and cannot represent an
ordinary software interrupt; using it would hide a timing problem. Opcode
exclusion lists also cannot distinguish a taken condition or successful
entry from a failed frame/vector transfer.

The sole successful real-entry commit therefore records one private Boolean
`instruction_software_entry` when a V30 instruction is executing. Reset and
the start of every refresh clear it. Debug completion and asynchronous
admission consume that same result. A later real handler instruction reopens
admission. It is neither another pending interrupt nor guest state, and has
no public layout/API, manager, retry, clock or thread. Intel membership and
asynchronous entry outside instruction execution remain unchanged.

DE/CHKIND still use the existing delivered-fault retirement classification;
its V15 accounting receiver is explicit and unchanged. This batch qualifies
priority and entry effects, not physical retirement or V30 Machine eligibility.
BRKEM/CALLN preserve their flag semantics; the boundary guard does not clear
IE/BRK merely to obtain the desired scheduling result.

## Finite Proof And Failure Boundary

The existing real-one-MiB fixture and typed transaction/phase checker are
reused. Nine operations cover BRK3, immediate BRK, taken/untaken BRKV,
zero-divisor DIVU, taken/untaken CHKIND, BRKEM and emulation CALLN. Eight flag
seeds, four pending NMI/INT combinations, three ordinary/tail PCs and four
aligned/odd/wrapped stacks give 3,456 inputs plus the direct witness.

Full CPU, saved flags/CS/next-PC, native stack effects and vector reads are
predicted independently. Actual BRKEM establishes CALLN's MD write-enable.
Software entry must retain external requests without acknowledgement; the
next actual native/8080 handler instruction admits the retained winner. Untaken
conditions must not acquire software priority or suppress ordinary stepping.
DR6.BS expectations describe existing CPU step bookkeeping, not a claimed
architectural NEC DR6 register.

Every observed read/write/fetch/preview refusal is injected for all seven
taken forms and both stack parities. Failed entry never publishes the completion
fact, preserves pending external requests, stops at the old checkpoint and
keeps the exact accepted write prefix without replay. Preview preserves all
CPU/pending state and the previous refresh's completion result. There is no
new memory implementation, planner, production entry or test framework.

Initial test-only corrections reconstruct each refusal plan against that
attempt's actual RAM (including the retained contiguous word-tail model),
and preserve the previous BRKEM completion fact during pure preview instead
of incorrectly expecting an earlier result to disappear before refresh.
Neither weakens the CPU/frame/pending/failure assertions.

## Similar-Issue Sweep

Queries: `rg -n '_ser_int_real\\(|_e_intr_n\\(|_e_except_n\\('`
against CPU instructions, plus debug completion, ExecFinal, refresh, reset,
mode owners, metadata and private/public CPU headers. Seven real-entry callers
have one commit: native BRK3/BRKV/immediate BRK, external entry wrapper,
exception wrapper, BRKEM and CALLN. External and halted wake entry execute
outside the instruction scope and do not claim software priority. Native
DE/BR enter during ExecFinal and do. Provider failure never reaches the
commit; undefined NEC forms remain diagnostics. Protected Intel entry and
its opcode-based step behavior are distinct retained architecture, not a
second NEC path. V15 DE/BR retirement and full physical timing remain the
original ledger receivers rather than new debt or an implicit pass.

## Executor Self-review And Boundary

The counted Git source/test scope is exactly the two CPU instruction files,
new software-arbitration unit and its CMake registration. Production is
+9/-2/net +7, tests/registration +287/-0/net +287, code total +296/-2/net +294.
Manifests separately are +4/-3/net +1; records and artifacts are excluded.
The 286-line unit accounts for the positive code delta and does not add a
production helper, public operation, clock, queue, worker or alternate entry.
The sole new private Boolean is a successful per-refresh result, required to
avoid a second opcode classifier or abuse of the unretired-fault outcome.

Complete V30 clocks, DE/BR retirement, loaded-BRK/prefix/general arrival and
nesting, unresolved source/model questions, Machine admission, board and actual
gameplay remain the original ledger receivers. No fresh complete external/native
integration suite, manual desktop/RDP, physical audio or playable-game result
is claimed by the unit/selected receiving batch.

## Final Qualification

Both original Release Ninja/ccache all-target graphs and whole-repository
unit suites pass 676/676 per width, including 189 X86 Core units. Full suite
times are 317.96/118.01 seconds; this new unit takes 0.0507752/0.0629108
seconds and reports 3,457 success inputs plus all refusal routes. Both seven-
unit focused groups pass. No final full-suite failure, skip, retry, budget
increase or partial-run pass is hidden. Existing instruction warning
exceptions remain explicit; the new unit uses normal strict warnings.

All 498 strict rows and Types/dependency/ownership/negative/manifest gates
pass; global vocabulary scans 1,386 tracked code/script files, including the
new staged unit. Raw/index Shared bytes match as LF. Documentation and
whitespace gates pass for MyRaidenII and receiving NXVM.

Eight PC 0564 EXEs are rebuilt/deployed. Fourteen selected receiving cases
and restored-default identity pass per width. Both original graphs are
restored; receiving profile switches recompile zero CPU objects. All ten
PE/name/width/size/hash identities pass, MyNES 0044 hashes and owner INIs
remain unchanged. No external firmware/media or snapshot source is edited.
The final qualification handle exits zero. Logs/scripts and the pinned manual
are retained ignored for this proof and the next timing receiver; no new raw
trace or rendering is created.

The [results](t2-s28-results.json) and [artifact identities](t2-s28-artifacts.json)
are collected from the final terminal logs/JUnit and actual deployed PE files,
not estimated test counts. A collector-template substring replacement first
mistyped the expected count as 776 and retained focused count 6. Those two
local evidence-reader assertions were corrected to actual 676/7; no source,
test result, assertion, budget or run was changed or repeated to obtain proof.

## Complete Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation, MyRaidenII M1 T2 S28; next unused S after accepted S27. |
| Admission And Approval | Owner requests continuation implementing MyRaidenII's V30 and facilities; standing V30 implementation and enum approval, no Lib/Emulator modification. Coordinator admits the complete software-entry arbitration batch on 2026-10-18. Targets Shared CPU/tests, NXVM eight receiving EXEs/evidence, MyRaidenII governance/evidence. MyNES is unchanged. |
| Objective | Consume V11 software-entry versus simultaneous NMI/INT/BRK arbitration: a successfully committed software entry wins this completion boundary, retains external requests, and admits them only after subsequent guest execution. Include BRK3/BRK/BRKV, DE/CHKIND and native/emulation mode calls at their existing entry owner. |
| Non-goals | No new public API/enums/layout, Lib/Emulator changes, timing model, loaded-BRK sampling upgrade, Intel policy change, board/game execution or T closure. DE/BR retirement/timing classification remains V15; this batch fixes arbitration only. |
| Reference Baseline | Clean pushed a50905530; original NEC October 1986 manual SHA FABBFCD19D4A2FCDAAC9F0BC06B00BF0A6A4BEEDAFBD3FF16ACCD075A374EA4C, Table 6-1 and mode-call flag tables; S22/S23/S27 entry proofs and complete T2 convergence ledger. |
| Candidate Proposal | proposals/m1-v30.md; V10/V11 successful software-entry priority subfamily. Existing timing transfer metadata describes protected transfers, not software-entry acceptance; use one CPU-private per-refresh completion fact written only by successful real entry while an instruction is executing. No opcode shadow classifier. |
| Files And ABI Surface | src/x86-core/cpu/cpu_instructions.c/.h, test/x86-core/cpu software arbitration unit and registration, both manifests; product-scoped receiving artifact/evidence and MyRaidenII records. Estimate production +15..25/-2..5, tests +250..400/-0..5. Private context bookkeeping only; public opaque interface unchanged. |
| Applicable Rules | Read product guide/architecture/coding/roadmap/source policy, shared Execution/Architecture/Coding/Document, local architecture/coding skills and receiving NXVM guide/source/artifact authorities. One entry owner, Lib vocabulary, source-backed priority, synthetic units, explicit partial-write failure semantics, one-target P, reusable graphs/cache, actual-diff coordinator acceptance. |
| Verification | Before-fix actual CPU witness; both original Release Ninja/ccache graphs -j8, affected units then complete repository unit suites -j4, specialized Types/dependency/manifest/strict gates, fourteen selected PC receiving integrations plus restored-default identity per width, all ten PE/hash identities; MyNES/INI/media/ROM unchanged. Documentation and diff checks. |
| Expected Markers | V30:SOFTWARE-ARBITRATION:OK; complete units with zero failures/skips per width; existing strict rows retained; all receiving cases and identity checks pass. No timeout retry or partial-run pass claim. |
| Asset Needs | No game input; pinned ignored primary manual only. Rebuild/deploy eight current PC 0564 EXEs; all ten identities, MyNES pair unchanged. No runnable MyRaidenII artifact exists yet. |
| Reporting Requirements | Confirm mechanism/estimate, report reproduction and focused proof, then full qualification and actual added/removed/net source/test lines, helpers/state/API necessity, remaining timing/board/game obligations and unexecuted desktop/RDP/full external tests. |
| Stop Conditions | Public contract expansion beyond approved boundary, new source/license/model necessity, unrelated dirty tree, regression or failed required release/gate prevents acceptance; diagnose in scope, never weaken assertion or silently transfer failure. |
| Exit Criteria | Frozen software-entry mechanism and failure batch proven, external requests preserved, no same-boundary double entry, old Intel tests pass, full units/gates/receiving releases pass, manifests and evidence current, target-separated P commits pushed; coordinator actual-change review then pure governance closure. T remains open. |
| Original Owner Request | Switch to MyRaidenII context and continue the next S implementing NEC V30 and required facilities; automatic sequential S execution under governance. |
| Similar-Issue Sweep | Inspect every _ser_int_real caller, debug completion and ExecInt admission, exception/mode entry and halted/reset refresh. Fix at successful entry commit, retain distinct asynchronous/non-V30/failure contracts, document DE/BR retirement receiver rather than misusing fault flags or fabricating timing. |
