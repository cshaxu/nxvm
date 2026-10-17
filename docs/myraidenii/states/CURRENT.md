# MyRaidenII Status

## Current Work

M1 T2 S15 native segment-transfer/inhibition delivery is verified; coordinator acceptance follows.
Single-session coordinator/executor mode remains in effect.

## Active S15 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: MyRaidenII M1 T2 S15, next unused S after accepted S14; single-session coordinator/executor, no delegation. |
| Admission And Approval | Owner's standing full implementation, automatic S and V30 integration approvals retained on 2026-10-17. Shared CPU/tests, eight receiving NXVM PC EXEs and MyRaidenII evidence only. No public API/layout, Lib/Emulator, MyNES, INI/media or third-party source modification. |
| Objective | Consume V06 native segment-transfer and V11 associated NMI/INT-inhibition batch: both MOV directions in GP/memory forms, PUSH/POP segment, LDS/LES, complete segment/default/override/alias/lane/tail and failure contexts. Correct all three documented MOV/POP shadow publication sites through the existing interrupt-shadow/entry owner; preserve scalar storage and Intel behavior. |
| Non-goals | Complete V06/V11, PSW/MD and mode/8080, general interrupt priorities/BRK fidelity, timing, Machine, board and game qualification remain open. No shadow on every segment-cache write, second interrupt owner, new state/API or arbitrary broad MOV/ALU qualification. |
| Reference Baseline | Clean pushed 9f1928ca4. Pinned October 1986 NEC manual printed 6-4 and 12-7--12-10/12-146--12-153; S14's recorded native DS/ES load and MOV source-direction gap. Existing segment shadow supplies the one next-instruction inhibition mechanism. The summary specifies external NMI/INT for stores; retaining the existing shadow's BRK suppression there is an explicit L2 model, not silicon-certified BRK behavior. |
| Candidate Proposal | [T2 proposal](../proposals/m1-v30.md) and [full ledger](../etc/t2-v30-ledger.md), native segment-transfer/inhibition subfamily of V06/V11. Every original unqualified native/mode/time/board receiver remains open. |
| Files And ABI Surface | Existing CPU instruction MOV/POP handlers, removal of identical MOV load decode branches if confirmed; one CPU-owned segment unit using the existing real-storage fixture, registration/manifests; eight PC 0563 artifacts; product status/ledger/history/evidence. No public function/type/layout or persistent production state. |
| Applicable Rules | Product guide, Execution/Architecture/Coding/Document, architecture/layout/source policy; architecture-governance then coding-governance. Sole segment-load, shadow, memory/entry and failure owners; original handler style, Lib Types, inward dependencies, original ROM-free expectations and accepted-prefix effects; no framework or copied implementation. |
| Verification | Baseline actual native DS/ES MOV/POP and source-direction witnesses; independent full CPU/memory results, legal native selectors/GP forms, segment override/default, aligned/odd/FFFF/overlap and each applicable transfer failure. Actual NMI/INT deferral through a following instruction, exact saved return frame and IRET, chained transfers, reset and failed-instruction nonpublication; negative PUSH/LDS/LES cases. Preview and old Intel regressions retained. Original Release Ninja/ccache graphs: all-target builds and complete repo-only units sequentially x64/x86; manifests/gates/docs/links, eight PE/name/version/INI/hash checks and actual-diff review. |
| Expected Markers | V30:SEGMENT:OK; retained V30:MEMORY-TRANSFER:OK and V30:STACK:OK; complete dual-width repo-only units and eight PC 0563 identities with default profile restored. |
| Asset Needs | No game inputs. Rebuild eight CPU-consuming PC EXEs at retained 0563, preserve MyNES pair and all INIs/ROM/media/snapshots. Retain bounded ignored build/unit/target/source-sweep logs for final T2 qualification; no raw game trace. |
| Reporting Requirements | Initial source/design and estimate, baseline versus repaired behavior, same-class sweep, source grades including store-BRK model boundary, counted source/test additions/deletions/net, complete verification/artifacts and remaining contexts. Estimate production net -6--+6 and tests net +300--450, excluding manifests/docs/artifacts. |
| Stop Conditions | Stop for public API/layout/new state, third-party source import, unsupported behavior model, Lib/Emulator/outside-target change or silent Intel change. No global segment-write shadow, new executor/interrupt route, recovery journal or timing owner. |
| Exit Criteria | All admitted native segment forms have independent result/alias/lane/failure proof in both widths. Every documented MOV/POP external-inhibition form defers NMI/INT through the next instruction and resumes through actual IRET; non-inhibiting PUSH/LDS/LES remain distinct. Correct publication only after successful operation, Intel preservation, full units/artifacts/gates and complete scoped pushed delivery. T2 remains open. |
| Original Owner Request | Implement fully playable MyRaidenII using governed single-session dual-role execution, approved V30 integration and automatic complete-mechanism S admission. |
| Similar-Issue Sweep | Inspect every segment MOV/POP, PUSH, far-pointer load and segment-cache writer plus existing shadow publication/consumption/reset/entry. Classify genuine far control/exception/debug loads separately; do not attach inhibition at the generic cache loader. Verify failed provider effects and unchanged Intel profiles; retain general BRK/entry/PSW/mode work at V10/V11/V13. |

| Task progress | Accepted result / remaining work |
| --- | --- |
| T2 S1 | Complete V30 domain/current-source audit accepted at 50eaf75b7: three opcode spaces and seventeen mechanism/context rows. |
| T2 S2 | Removed 83 identical advancement forks; source/test net -575 lines, dual-width 655/655 units and eight PC artifacts. No new CPU behavior admitted. |
| T2 S3 | Explicit private capabilities, native V30 profile/reset/PSW and representative 16-bit execution/preview; Machine remains ineligible. Dual-width 656/656 units and eight PC artifacts. |
| T2 S4 | NEC bit/nibble 10--1F/28/2A through existing execution/metadata/preview; dual-width 657/657 units and eight PC artifacts. |
| T2 S5 | Packed BCD 20/22/26; Shared 0fbc2cf9e, acceptance 3ceb11ef9, dual-width 658/658 units and eight PC artifacts. |
| T2 S6 | INS/EXT 31/33/39/3B; Shared 5ce9a3a9b, acceptance 56f7df605, dual-width 659/659 units and eight PC artifacts. |
| T2 S7 | Shared 297ae1637, NXVM 860fa1282 and evidence 1656b79fb accepted after actual-change review. Native carry/zero comparison repetition, dual-width 660/660 units and eight current PC 0563 artifacts. |
| T2 S8 | Accepted Shared 70dde1d31, NXVM 0ed0a9e30 and evidence b9f9c0110 after actual-change review: native memory lanes/addressing and source-graded L2 tail model, dual-width 661/661 units and eight PC 0563 artifacts. |
| T2 S9 | Accepted Shared 7a9433dc1, NXVM 2f0ac6b63 and evidence 4fdfe424d: native scalar/string I/O lanes and exact completion/failure effects; final dual-width 662/662 complete units and eight PC 0563 artifacts. First-run gate timeouts and complete unchanged-budget rechecks recorded; actual-change review passed. |
| T2 S10 | Accepted Shared 871fdfe1b and evidence 22fe9ffcb: six native data-block forms qualified without production changes; dual-width 663/663 complete units, existing PC/MyNES artifact hashes unchanged; actual-change review passed. |
| T2 S11 | Accepted Shared 36bba8907, NXVM b1fd77101 and evidence 395d126d0: native full-count shift/rotate and long-shift CF qualification; dual-width 664/664 complete units, eight PC 0563 artifacts. Reserved memory admission remains V03; actual-change review passed. |
| T2 S12 | Accepted Shared 9d069e6e4/310bd9aba, NXVM 663ce268d and evidence 87207373d/2c53350e6: common native primary ModRM admission, nineteen-opcode encoding matrix and failure/wrap witnesses; final dual-width 665/665 units and eight PC artifacts. Actual-change review passed. |
| T2 S13 | Accepted Shared 5c9695356, NXVM c7ab4fdd4 and evidence 597780b1a: native fetch-start normalization and original span/cache reuse; 5,700 instruction/preview witnesses, fetch failures, first dual-width 665/665 units and eight PC artifacts. Actual-change review passed. |
| T2 S14 | Accepted Shared 1ec55bd3d, NXVM d3ad9d003 and evidence 8011c084c: native GP-stack/register-set/frame qualification and seven-transfer POPR; independent order/alias/overlap/tail/failure proof, first dual-width 666/666 units and eight PC artifacts. Actual-change review passed. |
| T2 S15 | Shared ef0a6f30e and NXVM 82a3aee05 pushed: native segment MOV/PUSH/POP/LDS/LES and complete associated external-inhibition receiver; independent result/lane/alias/failure and actual NMI/INT/step/IRET proof, first dual-width 667/667 units and eight PC artifacts. Coordinator acceptance follows. |
| T2 remaining | Complete native semantics and slot/form disposition, mode/8080/entry, unqualified block/port-prefix/LOCK/long-prefix contexts, physical prefetch/bus timing, source-graded timing/neutral Machine and real board proof remain. Bit-field AX/AL/AH output collision is not silicon-exact; S8 native data-tail behavior remains L2. |

## Accepted Evidence And Review

[T2 history](../history/M1-T2-v30.md), [ledger](../etc/t2-v30-ledger.md),
[S15 delivery](../etc/evidence/t2-s15-segment.md) and
[eight artifact identities](../etc/evidence/t2-s15-pc-artifacts.json) retain
source/model grades, complete batch proof, actual deltas and verification.

Executor actual-diff review covers all three native shadow publication sites,
unchanged Intel and generic cache/stack/entry owners, removed duplicate MOV
decode branches, entire owned unit/registration, manifests, eight artifacts
and documentation. The sixteen-field packet, original full goal, complete
segment-transfer/inhibition batch, remaining receivers and scoped P sequence
are reconciled for coordinator acceptance. Production +7/-10/net -3 and
tests/registration +435/-0/net +435 exclude manifests/docs/artifacts. No
production helper, public API/layout, state mirror or alternate path is added.

Both original Release Ninja/ccache graphs build all targets. First complete
repo-only suites pass x64 667/667 (304.83 seconds) and x86 667/667 (109.00
seconds), including 178 X86 Core units. New-unit runtime is 0.73/2.01 seconds.
Complete suites need no retry, budget change or assertion removal. All eight
PC 0563 deployments and PE/name/version/INI/hash checks pass, CPU objects are
reused and default restored. MyNES pair, all INIs/media/ROMs and other Shared
packages remain unchanged; assets still retain exactly ten EXEs.

Manifests, Types/dependency/ownership/negative gates, governance, links and
whitespace pass. Existing production warning exemptions remain unchanged.
Independent CPU/storage/lane/failure expectations and actual NMI/INT/step
entry, wrapped frames, IRET, consecutive transfers, IF/STI and reset cover
the admitted batch. Store-BRK suppression retains an explicit existing-shadow
L2 model, not silicon-exact proof. PUSH/LDS/LES do not acquire the shadow.
Generic segment-cache writes remain separate from instruction policy.

Full native slots/semantics, PSW/control frames, mode/8080, general entry/
priority, prefetch/bus/source-graded timing, Machine and board/game remain
open. No external-ROM integration, manual desktop/RDP, physical audio or game
run is claimed. No MyRaidenII EXE exists. The lossless-colour
[High TODO](TODO.md) still gates final release; no quantization or second
presenter is admitted.

## Previous Work

| Task | Accepted outcome |
| --- | --- |
| T1 | Source/board/set and integration-contract freeze closed at 7f7d23938. Thirteen runtime image identities and F01--F13 reconciled; not game execution. |

[T1 history](../history/M1-T1-board-and-source-freeze.md) retains its proofs.
M0 Td S1 closed at `f21a22997`; M1 remains open toward the full playable goal.

## Queue

[Queue](QUEUE.md) contains five unadmitted receivers after active V30 T2.
