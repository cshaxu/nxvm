# MyRaidenII Status

## Current Work

M1 T2 remains open after accepted S9. S10 qualifies native data-block transfers.
Single-session coordinator/executor mode remains in effect.

## Active S10 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: MyRaidenII M1 T2 S10, next unused S after accepted S9; one-session coordinator/executor without delegation. |
| Admission And Approval | Owner's standing full implementation/automatic S/V30 integration approvals retained on 2026-10-17. Targets Shared CPU-owned tests and MyRaidenII evidence; NXVM artifacts only if executable inputs actually change. No public API, Lib/Emulator or MyNES modification admitted. |
| Objective | Consume V09's complete six-form native MOVS/LODS/STOS transfer subfamily, through the existing handlers and S8 memory owner: single/REP, zero/count, DF, segment choices, index/bus tails, overlap, accepted-prefix failures and actual interrupt/IRET continuation. |
| Non-goals | No full V03/V09 retirement, 8080/mode, timing/electrical queue qualification, undocumented carry/F2/LOCK or long-prefix semantics, Machine/board/game admission or external ROM test. |
| Reference Baseline | Clean pushed 01c5cd23a, pinned NEC October 1986 manual 12-15--12-20 and section 6; S8 segment-tail L2 model retained, not promoted to silicon measurement. |
| Candidate Proposal | [T2 proposal](../proposals/m1-v30.md), [complete ledger](../etc/t2-v30-ledger.md), V09 six-form data-block transfer batch with V14 contextual proof. |
| Files And ABI Surface | New owned test/x86-core/cpu/cpu_v30_block_transfer_smoke.c, registration and test manifest; existing CPU handlers only if demonstrated defect requires correction, then source manifest/eight PC 0563 artifacts. Product evidence/status/history/ledger. No new public type/layout/function or persistent production state. |
| Applicable Rules | Execution/Architecture/Coding/Document, product architecture/layout/source policy, architecture-governance then coding-governance: inward test ownership, sole handler/memory owner, original source style, Lib Types, original synthetic data and no external implementation copy. |
| Verification | Existing x64/x86 Release Ninja/ccache graphs and full repository-only units, sequential architectures with bounded parallelism; actual instruction/preview/interrupt/failure proof, manifests and registered Types/dependency/ownership gates, documentation/links/actual-change review. |
| Expected Markers | V30:BLOCK-TRANSFER:OK and dual-width full units; retained PC/MyNES artifacts match current recorded hashes if no executable inputs change. |
| Asset Needs | No game ROM/media or new output artifact needed for test-only change. If production changes, rebuild/deploy all eight PC 0563 EXEs and verify PE/product/version/INI/hash; preserve MyNES, all INIs/media/snapshots. |
| Reporting Requirements | Report initial reuse/estimate, observed semantic and failure outcomes and actual source/test plus/minus/net, full verification and unverified boundaries. Estimate production zero, tests +280--380; do not fabricate production edits to satisfy a repair label. |
| Stop Conditions | Stop for new public API/layout, external/protected source, Lib/Emulator/outside-target or incompatible source-model change. No replacing existing handlers or second bus/executor/test framework. |
| Exit Criteria | All six forms execute through existing CPU routes with independent expected data, complete flags/register/count/index and actual provider address/width/failure checks; REP overlap and interrupt continuation retain exactly-once accepted effects, full dual-width units and applicable artifact proof pass, delivery pushed/reviewed. T2 stays open. |
| Original Owner Request | Implement full MyRaidenII with governed single-session dual roles, approved V30 integration and automatic complete-mechanism S admission. |
| Similar-Issue Sweep | Inspect all six handlers, _m_movs/_m_lods/_m_stos, _kas_move_index, repeat admission/restart and S8 logical/physical owners. Preserve comparison and port owners' distinct semantics; remaining modes/timing/undocumented prefixes retain existing ledger receivers. |

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
| T2 S10 | Delivered Shared 871fdfe1b: six native data-block forms qualified without production changes; dual-width 663/663 complete units, existing PC/MyNES artifact hashes unchanged. Coordinator review pending. |
| T2 remaining | Native full-form qualification, mode/8080/entry, other block/unqualified-port-prefix/LOCK/long-prefix contexts, physical bus, source-graded timing/neutral Machine and real board proof remain. Bit-field AX/AL/AH output collision is not silicon-exact; FFFF-start native data spans now have S8 L2 proof, not silicon-exact certification. |

## Accepted Evidence And Review

[T2 history](../history/M1-T2-v30.md), [ledger](../etc/t2-v30-ledger.md),
[S9 delivery](../etc/evidence/t2-s9-port-transfer.md) and
[eight artifact identities](../etc/evidence/t2-s9-pc-artifacts.json) retain
complete sources, scope, actual deltas and verification.

Coordinator reviewed actual port input/output changes, all twelve scalar/
string handlers and their sole provider route, complete new unit, registration,
manifests, eight artifact identities and task documents. Odd V30 words reuse
the original byte path with bounded one-level splitting, not a second I/O
engine. Accepted external effects and per-cycle completion remain authoritative;
architectural failure rollback stays at the existing instruction checkpoint.
No helper, public API/type/layout, persistent state, second executor/clock/
queue/worker or copied reference implementation is added. Production
+13/-0/net +13 and tests +383/-0/net +383 exclude manifests/docs/artifacts.

Both original Release Ninja/ccache graphs build all targets. Final complete
repo-only unit rechecks pass x64 662/662 in 76.23 seconds and x86 662/662 in
80.14 seconds, including 173 X86 Core units. Both first full runs passed
661/662 with the unchanged dependency-negative gate timing out; its isolated
x86 check and sequential full rechecks pass without code, assertion or budget
changes. First-run failures remain recorded, not relabeled green. All eight
PC 0563 links/deployments and PE/product/version/INI/hash checks pass, reusing
objects and restoring default graph selection. MyNES pair, all INIs/media and
other Shared packages remain unchanged. Manifests, Types/dependency/ownership/
negative gates, documentation, relative links and whitespace pass.

The accepted batch proves native scalar/string port lanes, exact accepted-
prefix completion/failure, REP/DF/index/segment and actual NMI/INT-to-IRET
continuation. Reserved-tail wrap and ignored prohibited input overrides retain
explicit model boundaries; no silicon-exact claim is made. It does not retire
complete V09/V14, electrical bus/prefetch, timing, native-form or mode/8080
qualification. No external-ROM integration, desktop/RDP, physical audio or
game run was performed. No MyRaidenII EXE exists. The lossless-colour
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
