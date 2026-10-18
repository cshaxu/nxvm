# MyRaidenII Status

## Current Work

M1 T2 S23 is admitted at clean `500976cd8`, after accepted S22. The full
playable RAIDEN II goal is unchanged; this batch implements the complete
mode-transition mechanism, not full 8080 execution or gameplay.

## T2 S23 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation, MyRaidenII M1 T2 S23 after pushed/accepted S22; no new T allocation. |
| Admission And Approval | Owner's durable single-person coordinator/executor MyRaidenII implementation goal and standing V30 CPU authority. Shared CPU source/tests and MyRaidenII records; NXVM eight receiving artifacts/status after production changes. No Lib/Emulator, MyNES source or public API/layout expansion. |
| Objective | Consume V10/V13 complete mode-transition mechanism: BRKEM, CALLN, RETEM, native RETI/POP PSW MD restoration while write-enabled, native NMI/INT entry from emulation, reset abort and prohibited nesting disposition. Reuse existing fetch, frame/vector, return, rollback and execution boundary. |
| Non-goals | General 8080 opcode/ALU/stack/I/O implementation remains the next V12 receiver. Only actual 8080 NOP is admitted as a transition continuation witness; other unfinished ordinary slots stop diagnostically, never emulate native instructions or silently NOP. General priority/HLT/step, full prefixes/LOCK, physical timing, Machine/board/game remain open. No model or silicon timing upgrade. |
| Reference Baseline | Clean 500976cd8; original NEC October 1986 manual section 8 and printed 12-169, 12-176, 12-177 plus PSW restore footnotes. S22's reused actual-memory fixture, 672 repository units, PC 0564 and MyNES 0044 identities. |
| Candidate Proposal | [V30 proposal](../proposals/m1-v30.md), [V10/V12/V13 ledger](../etc/t2-v30-ledger.md). Complete native/NEC/8080/time/board universe remains intact. |
| Files And ABI Surface | Existing cpu_instructions.c/h private CPU context and cpu.c initialization, one native NEC format/dispatch, one mode-aware context preview; existing real entry/return/flags/reset. New CPU mode unit and necessary old admission-vector updates, registration/manifests. Estimate production +100..180/-15..40, tests +350..600/-0..30. One hardware MD-write-enable lib_bool; no second mode bit, public function/enum/layout or worker. Receiving eight PC EXEs/status and MyRaidenII evidence/history/ledger. |
| Applicable Rules | Product guide, architecture/layout/source policy, proposal/ledger, Contributing; shared Execution/Architecture/Coding/Document; architecture-governance then coding-governance, PDF read-only skill. Existing NXVM source-policy embedded-PC-firmware artifact exception applies; no raw protected input import. |
| Verification | Original x64/x86 Release Ninja/ccache graphs -j8; mode lifecycle matrix and native/Intel receivers, then complete repository units -j4, 498 strict rows and Types/dependency/ownership/negative/manifests/governance. New finite mode unit TIMEOUT 90, existing budgets unchanged. Rebuild/deploy eight PC 0564 EXEs, relevant four-profile receiving integrations, restore default graphs and verify all ten identities; MyNES unchanged. |
| Expected Markers | V30:MODE-TRANSITION:OK finite inputs, exact native frame/vector/MD/write-enable/full CPU and return outcomes, no emulation SP/BP conflation, preview purity, each observed host refusal and no accepted-write replay. Ordinary unsupported 8080 slots remain explicit diagnostics, not qualified instructions. |
| Asset Needs | Already pinned NEC manual and original synthetic bytes; read-only original Intel 8080 NOP reference for the sole ordinary continuation. Render relevant primary diagrams before implementation; retain provenance and remove owned temporary images. No external payload/code/game/ROM import, media/INI changes or new build graph. |
| Reporting Requirements | Report architecture, state necessity, estimated/actual source/test diff; retain primary facts/ambiguities, before admission state, complete mode/failure proof and all receiving identities. Full sixteen-field packet retained in evidence P before separate coordinator closure. |
| Stop Conditions | Unresolved primary instruction semantics, public contract expansion, second mode/entry/decoder authority, third-party source/protected input or out-of-family behavior requires review before implementation. Undefined emulation nesting stops diagnostically, not invented hardware delivery. |
| Exit Criteria | Complete admitted mode lifecycle passes both widths with actual instruction bytes, all selectors/PSW images and wrap/odd/alias/failure contexts, native IRQ/RETI and POP PSW reentry/reset/MD relock. Existing Intel/native units and receiving artifacts pass, Ps pushed and actual-change review accepted. V12 ordinary 8080 plus general priority/time/board/game remain explicitly unqualified. |
| Original Owner Request | Continue single-person dual-role governed MyRaidenII implementation toward the playable game; do not shrink success to CPU tests. |
| Similar-Issue Sweep | Audit all PSW load/image callers, real interrupt entry callers, CPU reset/context initialization, native/NEC lexical admission, context preview and ExecIns/ExecFinal rollback. One MD owner in PSW and one hardware write-enable bit; no copied stack/vector/return implementation or native fallback for 8080 bytes. |

## S23 Executor Delivery

Mode transitions are implemented through the existing executor/frame/return
owners, with one private hardware MD-write-enable bit and no new public
contract. Final qualification passes 673/673 repository units per width
(186 X86 Core), plus 498 strict rows each. The new mode matrix passes
328,192 successful inputs plus host refusals and lifecycle/diagnostic guards;
the complete final qualification includes physical-alias coverage, without
another production change. Suite elapsed times are 94.51/103.27 seconds;
the mode matrix takes 0.862028/1.16264 seconds.
The dirty-context initialization correction and deliberate preliminary-run
interruption are retained in [S23 evidence](../etc/evidence/t2-s23-mode-transition.md)
and history. Production +105/-24/net +81 and tests/registration +406/-1/net
+405 yield code +511/-25/net +486, excluding manifests/records/artifacts.
Receiving eight-PC publication, 28 selected integrations and two restored
identities pass. Both default graphs are restored with zero CPU recompiles
inside receiving-profile loops. MyNES 0044 and user INIs are unchanged;
all ten identities pass. Final [results](../etc/evidence/t2-s23-results.json)
and [artifacts](../etc/evidence/t2-s23-artifacts.json) retain proof. All final
handles are terminal. Shared P1 `ad55ccf3f` and NXVM receiving P2 `ad47d64f9`
are pushed. The complete packet above is retained for coordinator review;
S23 awaits the product evidence delivery and acceptance.

## Accepted Progress

| Task progress | Accepted result / remaining work |
| --- | --- |
| T2 S22 | BRK3, BRK imm8, conditional BRKV, CHKIND BRK5 and ordinary-native RETI frames/returns are qualified through existing owners. Native CHKIND now saves following PC, not the Intel restart PC. Mode/8080, general priority/step, physical retirement/time and Machine/board/game remain open. |

[T2 history](../history/M1-T2-v30.md) retains S1--S22 and scoped P deliveries.
The [convergence ledger](../etc/t2-v30-ledger.md) preserves the complete
native/NEC/8080, entry, physical bus/timing and Machine/board domain.

Shared P1 `470391cf0`, NXVM receiving P2 `2f227277c` and MyRaidenII evidence
P3 `1d40f3ce8` are pushed. P3 retains the complete sixteen-field S22 packet.
Production +5/-4/net +1 and tests/registration +258/-0/net +258 give code
+263/-4/net +259, excluding records/artifacts/manifests. There is no new
production helper, public API, state, worker, provider or execution path.
Lib/Emulator and private App source remain unchanged.

## Accepted Evidence And Verification

[S22 evidence](../etc/evidence/t2-s22-software-entry.md),
[results](../etc/evidence/t2-s22-results.json) and
[ten artifact identities](../etc/evidence/t2-s22-artifacts.json) retain proof.

Both original Release Ninja/ccache graphs build all targets. First complete
repository unit suites pass 672/672 per width, including 185 X86 Core units,
in 323.40/136.98 seconds. The new finite matrix passes 655,265 inputs plus
host refusals in 8.16742/9.66592 seconds. Both specialized targets retain
498 strict rows. Manifests, Types, dependency/ownership/negative,
documentation and whitespace gates pass; existing warning exceptions remain.

Eight PC 0564 EXEs are rebuilt/deployed. Fourteen selected receiving
integrations and one restored-default identity check pass per width.
Receiving-profile loops recompile zero CPU objects; both original graphs
are restored to default. All ten PE/name/width/size/hash checks pass.
MyNES 0044 and user INIs/media/ROMs remain unchanged. The existing receiving
PC embedded-firmware artifact exception applies; no raw ROM is imported.

Coordinator actual-change review accepts P1--P3 after examining the original
request/full packet, source repair, complete new unit and reused fixture,
primary manual, before witness, raw logs, manifests, receiving artifacts
and ledger. The positive CHKIND witness fails before and passes after the
central repair without weakening assertions. Intel restart and S19 NEC
divide policy are retained. All owned build/test handles have exited.
Final governance P closes S22 only, not T2.

CHKIND negative-limit signedness retains its existing model, not a stronger
silicon claim. Packed-adjust questions, non-ten conversions, remaining
native/NEC semantics, mode-sensitive PSW/IRET, general BRK/priority/step,
full prefixes/LOCK, 8080, physical prefetch/clocks and neutral Machine/board/
game remain open. S8 data-tail and existing BRK-sampling grades are unchanged;
DE/BR delivered-fault retirement accounting remains explicit in V15 before
Machine admission. No desktop/RDP, physical audio, fresh complete external
integration suite, complete V30 or gameplay qualification is implied.
No runnable MyRaidenII EXE exists yet.

## Previous Work

T1 source/board/set and integration-contract freeze closed at `7f7d23938`;
[T1 history](../history/M1-T1-board-and-source-freeze.md) retains thirteen
runtime image identities and F01--F13, not game execution.
M0 Td S1 closed at `f21a22997`; M1 remains open toward the full playable goal.
Accepted NXVM T564 supplies RGB32/four-slot host publication; actual RAIDEN II
board palette/frame/rotation integration remains unqualified.

## Queue

[Queue](QUEUE.md) contains five unadmitted receivers after open V30 T2.
