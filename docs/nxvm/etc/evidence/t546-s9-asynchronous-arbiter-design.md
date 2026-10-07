# T546 S9 Asynchronous Arbiter Design

## Admission And Whole Batch

Continue from accepted S8 267c1d405, Shared 9d5e475f7 and NXVM 626d504f5.
CPU source BE62A9BB... and all eight 0546 products are the accepted baseline.
Owner's standing automatic-S and x86/chips/Core/ibmpc approval covers this
consecutive S9. Lib/Common/MyNES, public extensions, INI/media/firmware and
clock redesign are excluded. No production change or P occurs at admission.

Consume the complete T544 B03 and all linked family interrupt/debug/return
findings, including S8's explicit arbiter/comparator receivers. The independent
unit is family, producer, incoming IF/TF/RF, request combination, short shadow,
NMI mask/service state, boundary, accepted/failed entry and expiry/reset/return.

| Member | Required source/caller and direct proof |
| --- | --- |
| Family priority | Original 8086/8088, 186, 286 and 386 priorities; separate pre-instruction faults from post-instruction debug traps and pending NMI/INTR. Prove simultaneous requests and retained lower-priority requests. |
| SS shadow | Successful MOV SS, POP SS and LSS; same/changing selector, repeated producers and rejected loads. Qualify debug/NMI/INTR inhibition and exact following-instruction expiry by family, not by treating all segment loads alike. |
| STI shadow | Prior IF0/IF1, CLI/STI combinations, repeated STI, rejection and reset. Qualify its actual event classes independently of SS loading. |
| NMI lifetime | External board mask, pending edge and CPU service blocking are distinct. Prove masked signal handling, in-service edges, unmask, IRET success/failure and reset with no second board-owned CPU truth. |
| Debug cause/status | Enabled matches cause delivery; source-defined matching comparator bits and BS/BT status remain distinct. Prove simultaneous enabled/disabled comparator matches, RF instruction-breakpoint suppression and one pending cause owner. |
| Execution boundaries | Normal instruction, halted CPU and REP iteration/progress use the same arbiter; prove return point, unchanged completed instruction outcome and no acknowledgement/consumption before successful admitted delivery. Full WAIT/NPX and time publication retain S16/S18. |
| Cross-owner reception | CPU/Core/board NMI and INTR producers, reset and public copied diagnostics. Preserve the documented mask API and PIC acknowledgement semantics; no App BIOS-specific path. |

## Existing Owner And Initial Evidence

Read the complete current ExecInit/ExecIns/ExecFinal/ExecInt and debug match/
schedule/delivery paths, all SS/STI producers, IRET and reset before choosing
the candidate. rg identifies one flagMaskInt used by MOV/POP SS and STI,
cleared at ExecInit. ExecInt currently attempts NMI before pending debug trap;
SS/STI only gate NMI/INTR. CPU request_nmi rejects an external masked signal
and records one pending edge, but no CPU NMI-in-service state exists. Core's
unmask callback asks the board to re-evaluate its own source; preserve that
existing boundary rather than folding board mask into service blocking.

Original 386 Table 9-2/chapter 12 and retained T544 records identify the
priority and comparator-mask mismatches. Those findings are evidence leads,
not a substitute for fresh original-page review or early/186 predicates.
The same source/receiver grid must resolve the whole affected class before
production or a P; no first-observed-event patch or new arbiter framework.

## Verification And Exits

Use existing owner-local fixtures and code-defined handlers/frames to prove
actual delivered order, preserved pending requests, stack return point,
inhibit expiry and status. Reject missing-IDT shutdown as a successful event
oracle. Preserve all original assertions except documented source-proven
false oracles; a test-derived state is not a hardware specification.

Final qualification retains complete dual units, eight manifests and Types/
corpus/positive-negative checks, 33 supplemental cases per width, both full
gate aggregates, original 58 external contexts once and eight stripped 0546
products. Reuse validated per-width Ninja/ccache and sequential fixed-profile
selection; restore Default and isolate final full units from compiler/static
work. Preserve MyNES 0044, INIs and external masters exactly. Count actual
tracked code/test/build delta and review each changed owner before complete
target-separated P delivery and coordinator acceptance. T546 stays open.

## Fresh Original Sources And Direct Pre-fix Proof

Recomputed four original identities match the retained source records:
1981 8086/8088 3EEA6CA7..., 1985 family 2516D66C...,
1987 286/287 AD487BA9... and 1990 386DX 9A8188F9.... Read-only PDF skill
review renders remain in ignored build/t546-s9-research. No original, OCR or
reference implementation text is imported.

Fresh visual 8086 PDF45/2-26 Table 2-3 orders NMI, INTR, then single-step;
its text allows NMI interrupting an interrupt procedure. 1985 PDF216/2-50
Table 2-21 puts 186 single-step below maskable sources and explicitly handles
it as in 8086. PDF186/2-20 Table 2-10 describes its internally unmaskable,
edge-latched NMI. Do not impose the later blocked-until-IRET contract on
these early profiles. 286 PDF176/9-14 explicitly takes simultaneous single
step before external requests, checks those again before the handler's first
instruction and stops single stepping the external handler. PDF329/C-3
item 14 expressly marks blocked NMI until first IRET as an early-family
compatibility difference. 386 PDF209/9-5 Table 9-2 independently orders
debug traps ahead of NMI and INTR; lower priority interrupts remain pending.

Code-owned existing CPU-owner fixtures reproduce these distinct behaviors.
The 35-case all-family TF/NMI/INTR Cartesian subset checks actual stack return
points, handler order, IRQ acknowledgement and retained requests. Seven
contexts fail: early TF+INTR never acknowledges IRQ because debug is delivered
first, and 286/386 TF+NMI combinations put the debug frame in the NMI handler
instead of the opposite required ordering. Other controls remain passing.

The five-family nested-NMI fixture uses a real code-owned NOP/IRET handler.
Early re-entry is a passing sourced control. On both 286/386, a second request
enters again before IRET and grows SP from 7FFA to 7FF4; the two later rows
fail. The initial probe wrongly expected blocking on all five families; source
review corrected that unqualified expectation before any implementation.
No existing accuracy grade or production behavior is changed by this correction.

Five 286/386 MOV-SS/POP-SS/LSS TF contexts reproduce four improper immediate
step traps after MOV/POP SS. The LSS single-step control passes; its external
interrupt inhibition still needs separate direct proof. Move the new fixture's
instruction storage away from IVT vector one: the first LSS probe accidentally
overwrote that vector's first byte. Its corrected memory layout eliminates
the fixture error without relaxing the source contract.

Strict x64 builds of all three existing test targets complete; bounded
development executions report the expected failures above. These are failing
reproducers, not full-suite acceptance or an S9 P. Test manifest is refreshed
under S9; CPU source and all products remain exact accepted S8. All research,
build and test handles in this round are terminal. Continue reconciling STI,
SS/IRET expiry, failed entry/return and comparator cause/status before selecting
the whole existing-owner production scheme. No partial commit is eligible.

## Existing-Owner Candidate Scheme

Replace flagMaskInt, not supplement it with a competing arbiter. One private
NONE/INTR/SEGMENT shadow records the current producer's short inhibition;
sample its segment condition before the next actual instruction for that
instruction's pre-execution breakpoint admission. Reset the short shadow only
at actual instruction start, not lexical/frame ExecInit. Successful MOV/POP
segment producers select their documented family policy; LSS contributes no
shadow. STI and sourced early IRET select INTR-only inhibition. A separate
private NMI-in-service bit applies only to the source-qualified later families,
never to the externally controlled board mask or pending edge.

The existing ExecInt is the sole arbiter: later debug first, NMI, INTR;
early NMI, INTR, debug. Accepted entries consume their own request only;
stop/failure must prevent later branches from running in a terminal state.
Sampled segment inhibition gates the next instruction's debug fault, while
the current producer gates its end-boundary debug trap/NMI/INTR. Reset covers
all new private lifetime fields. IRET releases later NMI service at instruction
entry; the original "first IRET executed" wording and read-only Bochs
ctrl_xfer16/32 corroborate release before return-frame validation, not a
successful-return-only latch.

Original 386 PDF490/17-172 and 286 1985 B-102 describe IRQ delay through the
next instruction. Do not transplant Bochs's later-model IF-already-one
exception as a new claimed 386 manual rule. Keep the sourced delay, distinguish
its event classes, and qualify repeated/old-IF contexts explicitly. Fresh
386 12-4/12-5 defines all matching DR6 comparator bits independently of which
enabled match causes delivery. That mask/result repair stays at the existing
comparators and one debug publication owner. 186 non-SS segment/IRET shadows
still require the full primary-source predicate; preserve the current behavior
until that reconciliation, and do not claim those pending rows qualified.

This is a developing cohesive candidate, not a partial P or complete S9
qualification. Complete the remaining source predicates and direct receiver
matrices before any P, including all failure/expiry/simultaneous contexts.

## Developing Implementation And Direct Results

The current candidate removes flagMaskInt and keeps one private three-state
shadow plus sampled prior segment inhibition, rather than adding parallel
per-device flags/dispatch. ExecInt now chooses the sourced early/later order;
segment producers and pre-instruction breakpoint admission use the same
shadow lifetime. LSS no longer declares a shadow. Later NMI service and IRET
release are private CPU-execution state; the early in-service behavior remains
different. No public interface, new executor or clock/timing constant changes.

Actual failure-path review exposes a shared lifetime issue: the externally
controlled NMI mask and pending edge lived inside the register rollback image.
Move those two facts into the sole execution context and remove their old
storage. They are input state, not registers to restore after a failed handler
entry. Recognition clears its old pending request before building the frame,
so a new request during that entry can remain latched. Preserve the existing
failed-entry pending-request contract explicitly, independently of register
rollback, and retain the source-qualified service block until IRET/reset.
The migrated tests keep their assertions and use the existing context owner;
no second mask/state getter or board bypass is added.

INTR acknowledgement necessarily obtains a vector before descriptor/frame
entry; it is an irreversible accepted bus operation, not something to defer
until successful entry or roll back afterward. S9 must prove no ACK for
inhibited/lower-priority requests and exactly one admitted ACK, including
failed-entry reception. Full ordered delivery/time behavior stays S10/S18.
This clarifies the initial admission wording about successful consumption,
not a weaker hardware contract or unapproved timing downgrade.

Strict x64 builds complete and six existing CPU-owner development cases pass,
including the seven formerly failing priority contexts, both later NMI-service
rows and four MOV/POP-SS TF rows, with LSS and earlier-family controls retained.
Existing VM86 NMI/IRQ and rejected-entry register/pending rollback controls
also pass. These are narrow development results, not final qualification.
All handles in this round are terminal. Source/test manifests are refreshed;
MyNES/Lib/Common, external masters, owner INIs and deployed eight S8 products
remain unchanged. Continue comparator cause/status, full shadow/return/reset
and actual Core/board receiver matrices before any P; unresolved 186 non-SS
and IRET source predicates are still pending within this same S9, not waived.

## Comparator And External-Shadow Development Proof

Separate enabled causes from all matching status bits in the two existing
instruction/data comparators. They return the enabled mask and a private
matched-mask output; publish matched DR6 bits only when an enabled condition
actually causes the debug exception. Preserve BS/BT and sticky prior DR6,
RF suppression and the sole pending-trap owner. No new state/API or duplicate
comparator is introduced. The existing disabled-match read oracle expecting
only bit 1 is corrected to both matching bits under original 386 12-4.
An independent 1,024-context local/global enable-by-match instruction/write
matrix passes, including no-delivery controls and preserved prior BS.

The 288-context all-family external-shadow matrix checks MOV/POP ES/DS/SS,
STI, 386 LSS, both prior IF values and four NMI/IRQ combinations. It proves
first-boundary admission, no inappropriate ACK, remembered NMI and actual
following-instruction expiry. Five-family reset lifetime controls also pass.
The current three-state policy retains source-proven early broader segment
inhibition, later SS-only inhibition and STI's separate INTR-only shadow.

Primary hardware descriptions did not explicitly settle every 186 non-SS/
IRET predicate. Inspect official MAME's actual 186-specific 8E override,
its common POP ES/DS/SS/IRET dispatch and 186 execution loop rather than
assuming the generic 8086 MOV handler applies. MAME's 186 override inhibits
SS only; common non-SS POP and IRET declare no corresponding shadow. These
specific 186 rows retain the same project behavior with an explicit L2
reference-model basis, not newly asserted Manual-L3. Do not copy MAME's
broader generic 8086 policy over its specific 186 handler or the project's
original 8086 manual. Exact instruction clock grades/constants remain unchanged.
The owner automatically approves upward source/model qualification; no existing
L2/L3 row is downgraded or falsely retagged. This is behavior research only,
not an imported/transliterated source or runtime dependency.

Official source review on 2026-10-07:
[MAME i186.cpp](https://raw.githubusercontent.com/mamedev/mame/master/src/devices/cpu/i86/i186.cpp)
(186 MOV override and common dispatch) and
[MAME i86.cpp](https://raw.githubusercontent.com/mamedev/mame/master/src/devices/cpu/i86/i86.cpp)
(common POP/IRET handlers). The attempted GitHub master-commit API read returned
an internal error; no fixed upstream-commit identity is claimed. Original Intel
pages and project-owned behavior probes remain the stronger qualifying inputs.

Read the actual Core ACK transaction and provider-failure tests: CPU's ACK
branch silently ignored non-OK status, unlike its checked memory/port paths.
Route this admitted failure through the existing internal CE/finalizer at the
post-completion context. Preserve completed instruction IP/registers and a
provider's unacknowledged IRQ; do not invent a guest #GP or successful ACK.
The old smoke requiring no error from LIB_STATUS_IO_ERROR is a false oracle;
its updated all-five-family controls require stopped CE/IO status while
retaining the original completed-IP/SP/pending/ACK assertions.

Six developing CPU-owner cases pass on x64/x86 before the final ACK change;
the changed bus, debug and signal cases then pass on x64. Final complete
unit/receiving qualification remains pending, including broader return/failure,
SS debug-fault/data, repeated-producer and actual Core/board matrices.
All handles in this round are terminal. Source/test manifests will reflect
these exact candidate bytes; no P or product refresh is eligible yet.

## Shadow And NMI Entry/Return Boundary Proof

Add six existing-owner 386 MOV/POP-SS debug-fault/data boundary controls.
The following instruction's execution breakpoint is suppressed, but a
breakpoint at the next instruction is admitted. Data writes after the shadow
produce their actual copied post-instruction trap point and committed byte.
Consecutive successful SS producers are checked separately from an ordinary
following instruction; no pre/post debug fault is mistaken for the other.
These controls pass on the current x64 candidate, preserving DR6/status and
all prior debug-state assertions.

Two later-family real NMI entry probes inject a second actual request from
the existing memory provider at vector fetch. They prove the edge remains
pending during the admitted NMI handler, is not overwritten by register
rollback/image preparation, and is consumed only after IRET releases service.
This directly supports the single context-owned input state rather than a
second board mask or patched saved-register image.

Two protected 386 IRET controls install real code-owned NP/NMI gates and
separate handler descriptors. A non-present return descriptor produces actual
NP delivery, not missing-IDT shutdown. NMI service release precedes return
validation: without another edge the NP handler runs unblocked; with a pending
edge it can admit NMI before executing that handler's first instruction.
Their existing-owner test passes; full ordered exception/gate geometry still
belongs to S10/S12 and is not claimed here.

Strict changed-target builds and bounded development execution finish with
success. These results advance S9's direct boundary grid, not complete unit,
Core/board, external or product acceptance. No source/API/timing change occurs
in this proof increment. Keep the active packet and remaining failure/return/
receiving dispositions open; no partial P. MyNES/Lib/Common, deployed S8 EXEs,
INIs and master media remain unchanged. All handles are terminal.

## Resume: Sequential Producers And Receiving Oracles

Forty code-owned all-family STI sequence controls cover both incoming IF
values and STI/NOP, consecutive STI, STI/CLI and STI/HLT. They prove retained
IRQ/no ACK inside the shadow, CLI's disabled control and the actual saved return
IP after expiry; HLT wakes through the same admitted IRQ. Five REP MOVSB
controls prove one committed byte/count/index update followed by exactly one
ACK, with the saved IP at the repeat prefix and the next byte untouched.
These development controls pass on x64, without a second arbiter or REP path.

Eight protected 386 rejected MOV/POP-SS controls cover inherited/no shadow
and pending/no NMI. Actual GP entry preserves the original SS and does not
re-arm a failed producer; a pending NMI can enter before the GP handler's
first instruction. Existing gates are valid, so missing-IDT shutdown cannot
masquerade as this proof. The x64 development case passes.

Actual receiving review identifies one false board oracle: LSS was expected
to delay PIC admission through the following NOP. Original 386 3-41 and 9-4
explicitly exclude that inhibition. Correct the existing LSS/LFS/LGS receiving
test to the common immediate IRQ return point (IP 5), retaining PIC IRR/ISR,
memory frame and handler/HLT checks. Its current x64 execution passes.
Board source remains unchanged: AT parity and XT PPI latch/re-evaluate their
own signal and call the existing Core NMI boundary; Core unmask refreshes that
source without changing the CPU's separate service latch. Existing RTC-mask,
parity/XT unmask, HLT and protected/VM86 delivery tests are required receiving
proof in the full sweep, not replaced by CPU private-state tests.

The resumed x64 complete repository-only unit run passes 505/505 in 231.92 s
through the existing 300-second contained aggregate, without competing builds.
Its receiving tests include actual Core attachment/unmask, AT parity, XT PPI,
PIC failed entry, HLT and real/protected/VM86 hardware delivery. Both the changed
chip and receiving LSS bodies are included. The x86 complete dependency build
then finishes successfully and its separate complete aggregate passes 505/505
in 224.48 s. Both aggregates are terminal, without relaxed containment or
overlapping compiler/static work.
Source, x86-test and IBMPC-test manifest checks and NXVM documentation governance
pass. These are complete dual-width unit results, not external/product or
S9 acceptance. Remaining final gates, external contexts, artifact refresh and
actual-diff review are pending; no S9 P exists. The halted-debug boundary still
requires explicit source/actual-path disposition before the whole execution
boundary member can be accepted. Lib/Common/MyNES and accepted S8 deployed
products remain unchanged.

## Halted Debug Arbitration

Fresh original 386 PDF382/17-64 specifies HALT's enabled interrupt/NMI/reset
wake and the post-HLT saved return address; 12-8 supplies the ordinary sampled
TF rule. The combined deferred-step/HALT recognition order is not an exact
numeric timing row. Read-only Bochs 2.6 event.cc handles the HALT wait before
normal asynchronous priority; a pending step alone does not end that wait.
This is the selected L2 combination model, not newly asserted physical L3.
PCjs cpux86.js checkINTR provides an independent early/later priority check,
but its trap branch does not clear HALT; it cannot qualify this combination.
No reference source is copied or imported. Existing exact HLT instruction
clock allocation and all prior accuracy tags remain unchanged.

The normal priority matrix extends to HLT with all seven request combinations
across five profiles (70 contexts total). Before repair, seven additional
contexts fail: step-only enters a handler despite remaining halted on five
profiles, and later step+IRQ enters with the HALT state still set. Keep the
pending step while no eligible external wake exists. At the existing ExecInt
owner, admit wake without consuming/acknowledging a request, then use that
same family-qualified arbiter. Re-evaluate actual inputs at delivery, since
frame entry can itself cause another edge. No additional executor or flag.

The complete 70-context development matrix now passes on x64, including ten
follow-up masked-IRQ/eligible-wake stages after a retained step-only HALT.
Frames/PIC acknowledgement retain the proper early/later priority and saved
post-HLT IP. The prior 505/505 unit results belong to the pre-HALT candidate;
the final changed candidate must receive fresh complete qualification before
any P. Original renders remain ignored in build/t546-s9-research.

## Complete Candidate Disposition And Self-review

The candidate is frozen at CPU instructions SHA-256
BEBC79F8DB7C33E93EA26A089D5A98B6E252DDEAF33FA5EFFE7297E8E15067B7.
Review all four changed production files, all eight changed regression bodies,
and actual Core/board producers, callbacks and copied observation consumers.
No public header/API, instruction clock allocation, Lib/Common or MyNES body
changes. The original instruction tables and sole ExecInt/finalizer remain.

| Admitted member | Before / candidate direct disposition |
| --- | --- |
| Family priority | Seven of 35 normal simultaneous contexts failed. One early/later selection repairs them; the normal/HLT extension proves 70 contexts with retained lower-priority requests and real frames. |
| SS shadow | Four later MOV/POP-SS step contexts failed; LSS control stayed valid. One private shadow supplies source-qualified early broad/later SS-only production and next-instruction expiry. 288 external, six debug boundary and eight rejected-load contexts prove success, repetition, failure and actual following instruction; changed-selector board tests remain receivers. |
| STI shadow | Separate INTR-only state replaces its accidental NMI inhibition. Incoming IF0/IF1 and repeated STI/CLI/HLT sequences, existing protected/VM86 rejection matrices and reset prove no inappropriate ACK or accepted failed producer. |
| NMI lifetime | Two later service rows failed. One context owns pending, external mask and service as different facts; five-family service/reset controls, two edges during entry and two faulting IRET controls prove their lifetimes. Existing failed-entry/private rollback and board unmask cases remain direct receivers. |
| Debug cause/status | Disabled matching DR6 bits were missing from an enabled exception. The two existing comparators separate cause from status; 1,024 enable/match/local/global instruction/write contexts, existing RF/BS/BT cases and SS tests qualify that boundary. Full task-state entry remains S13. |
| Execution boundaries | One arbiter serves normal, HLT and REP. Five actual REP-element controls preserve commit and prefix return; HLT adds seven reproduced failures and ten retained/masked/wake stages. Combined HALT/step selection is explicitly reference-model L2. Full WAIT/NPX and retirement/time publication remain S16/S18. |
| Cross-owner reception | CPU ACK failure was silently ignored; all-five-family checked CE/IO controls now preserve completed IP/SP and unacknowledged request. Core attachment/RTC mask, AT parity/XT PPI, PIC entry, protected/VM86 delivery and HLT bodies execute in complete units. One LSS false receiving oracle is corrected from the original manual, not weakened to observed code. |

This exhausts the admitted mechanism batch, not all CPU/gate/task/page/timing
contexts. Ordered fault-pair/shutdown, gate geometry, full task/paging/NPX,
REP failure and successful instruction/delivery time retain their named
S10-S18 receivers; no S9 compatibility claim replaces them. No timing grade
is downgraded and no L1 fallback is introduced.

Count tracked source/test paths with git diff --numstat, excluding manifests,
documentation and generated/artifact files: twelve paths, +752/-51, net +701.
Four production paths are +97/-28, net +69; regression bodies are net +632.
The positive increase is direct source-conditioned matrix/boundary proof,
not another state owner, executor, framework or product special case.
Retired flagMaskInt/flagMaskNMI/flagNMI have no tracked production/test callers.

Both full specialized gate aggregates pass, including 514 strict source rows
and global Types coverage. Eight manifests pass. The remaining static/negative
supplemental routes pass 33/33 per width (23.20/21.62 s). Final complete units
are now running serially after all compiler/static jobs have terminated.
External/product qualification, immutable P review and closure remain pending.

## Final Candidate Verification In Progress

The final frozen candidate's contained complete units pass 505/505 per width,
x64 206.77 s and x86 194.28 s. Logs remain ignored under
build/t546-s9-final-qualification/unit-x64.log and unit-x86.log. The earlier
231.92/224.48 s passes remain pre-HALT-correction evidence, not final proof.
Both full specialized aggregates, supplemental 33/33 per width, eight manifests,
Types coverage and documentation checks are terminal and passing.

Final product/integration qualification now executes each original selected
profile group once per width through its actual fixed binding and owner INI.
The one live job iterates the validated Ninja/ccache trees, builds the 0546
product and contained integration target, and restores Default in finally.
No new checkpoints or boot budgets, owner INI changes, media copies, MyNES
rebuild or parallel external runs are introduced. This paragraph records a
live qualification, not a passing 58-context result or artifact acceptance.

Similar-issue queries include rg -n for flagMaskInt/flagMaskNMI/flagNMI over
tracked src/test C and headers (no hits), and nmi/interrupt_shadow/debug_segment_shadow
over CPU, Core and IBMPC producers/receivers. Every actual state/callback hit
maps to the member table above; no second arbiter or board CPU-service owner.

## Final Receiving Qualification And Artifacts

The product/integration job terminates successfully. Original 58/58 external
contexts pass once: per width Default 22, AT 3, XT 1 and Model40 3. Group
elapsed seconds are x64 54.59/31.97/20.27/110.71 and x86
62.56/42.33/23.83/151.30. No budget, checkpoint, owner INI, master or access
mode changes. Both cached trees are restored to Default and MyNES remains
disabled in their build graph. Complete final units and all other gates above
refer to this same frozen production source, not the earlier candidate.

Eight optimized stripped Release 0.5.0546 products are rebuilt and deployed
only to their existing flat App roots. Native post-build architecture checks
and an independent PE/section review prove x64 8664 and x86 014C, empty compiler
debug directory and no debug/zdebug/stab sections. Runtime Debug remains.
An initial ad-hoc PE review had a PowerShell expression error; only its corrected
strict, terminating-error execution qualifies the debug-directory check.

| App | Final x64 SHA-256 | Final x86 SHA-256 |
| --- | --- | --- |
| NXVM | 75CCAF0FF13B5D79215B5CDBAD5802D3DDD9019FAD4FA2661AC068A7A3000487 | D08CB8205D7BAB713B511A8BB6818A4C6E087D137F4CCEB22306C00B43B6D5C5 |
| My5160 | E655690D77E8DBAB47F241BFA4D0E355ACFEB1B4F0E78D0C16BD19E86720BAF4 | D2394BAFD4762B6BF90D55133F353A9E58F7E0E8C1BC78CA3B1ED2273996F8C7 |
| My5170 | 4ABBEAFDD22E5FC240E33068DCFB972B8809F4E6339672FCA55C1082BA42D47D | 8E5C64584D195B4396A3DF1310EEF083CE4BD5C4FD04C24D443672994ADBAE78 |
| MyDeskPro386 | 49FEF82CF8799713218AD6964C9EA78B984D67EE8E5CC87153010D04B90A7A45 | 61790C83532B8A3C1B3614AAF4C6169553F03F8549042F1C4F2BF4E6959A08A0 |

Four owner INIs, five external media masters and exact accepted MyNES 0044
x64/x86 hashes match S8's recorded identities. Lib/Common/MyNES and their
tests/docs/assets have no task diff. No owned build/test handle remains live.
Validated caches/research/final logs remain for immediate S10 use. Final
executor source/caller, artifact, documentation and complete-batch self-review
passes; immutable target-separated P delivery and coordinator acceptance are
the remaining closure steps. T546's S10-S20 remains open.

Shared implementation P1 is 746e3e214cc4e723b85d491f34b39a5e816fe7f0,
pushed immediately to origin/master. Its fifteen paths contain only CPU source,
owner-local x86/IBMPC tests and their three manifests. The eight artifact hashes
above were produced from these exact frozen source bytes; task documentation
and product delivery are the separate NXVM P2, not a Shared-source side effect.
