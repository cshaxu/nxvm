# MyRaidenII Status

## Current Work

M1 T2 remains open after accepted S8. S9 implements native port transfers.
Single-session coordinator/executor mode remains in effect.

## Active S9 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: MyRaidenII M1 T2 S9, next unused S after accepted S8; one-session coordinator/executor, no delegation. |
| Admission And Approval | Owner's standing full implementation, automatic S and approved V30 integration retained on 2026-10-17 continuation. Targets Shared CPU/tests, NXVM eight receiving PC artifacts and MyRaidenII evidence only. No new public API/layout or Lib/Emulator approval. |
| Objective | Consume V09/V14 native port-transfer subfamily: eight scalar IN/OUT forms, four port strings, V30 byte/even-word/odd-word lanes, repeat/DF/segment and 16-bit address contexts, exact accepted-prefix failures and interrupt continuation. |
| Non-goals | No 8080/mode implementation, undocumented carry-repeat port semantics, electrical bus/prefetch/timing certification, full native or Machine/game eligibility, Lib/Emulator/MyNES or protected input change. |
| Reference Baseline | Clean pushed 337eb47b7; pinned NEC October 1986 manual 4-3--4-4 and 12-25--12-29. Segment-free 16-bit I/O, odd-word two cycles and INM/OUTM register rules; reserved port addresses are not a peripheral hardware claim. |
| Candidate Proposal | [T2 proposal](../proposals/m1-v30.md), [complete ledger](../etc/t2-v30-ledger.md), bounded V09/V14 native I/O subfamily. All other domain rows remain. |
| Files And ABI Surface | Existing src/x86-core/cpu/cpu_instructions.c port owners, owned new CPU port-transfer unit/registration and source/test manifests; eight PC 0563 EXEs and MyRaidenII status/ledger/evidence/history. No public function/type/layout, persistent state or second port path. |
| Applicable Rules | Shared Execution/Architecture/Coding/Document, product architecture/layout/source policy, architecture-governance then coding-governance. One bus owner, inward dependencies, original handler/trace style, Lib Types and synthetic independent vectors; no third-party implementation copy. |
| Verification | Original x64/x86 Release Ninja/ccache full builds, complete repository-only units both widths, relevant scalar/string/entry regressions, source/test manifests and registered Types/dependency/ownership/negative gates; eight PC real links/deployments and PE/product/version/INI/hash checks, documentation/links/actual diff review. |
| Expected Markers | V30:PORT-TRANSFER:OK; both full unit suites pass; eight current PC 0563 artifacts, default graph selection restored, MyNES and INIs unchanged. |
| Asset Needs | No game ROM/media input. Rebuild eight CPU-consuming PC EXEs at retained 0563; preserve all INIs/media/snapshots and unrelated artifacts. |
| Reporting Requirements | Explain sole-owner fix and estimate before code, then semantic/failure milestones and actual source/test plus/minus/net, full verification and unverified boundaries. Estimate production +20--40/-0--10, tests +300--450. |
| Stop Conditions | Stop for new public interface/layout, copied/protected source, Lib/Emulator/outside-target change or unsupported source model. Do not silently grant undocumented prefix/reserved-port/timing hardware behavior. |
| Exit Criteria | Scalar and string port paths share existing byte/even-word callbacks; odd V30 word splits low then high with exact completion and failure effects, no accepted-prefix replay; register/count/segment/DF/restart contracts proven through actual execution; complete units, affected artifacts and target-scoped pushed delivery reviewed. T2 remains open. |
| Original Owner Request | Implement full MyRaidenII in single-session dual-role mode under governance, approved V30 integration and automatic bounded S admission. |
| Similar-Issue Sweep | Inspect all transfer_port/complete_port callers, scalar/string owners, repeat admission, memory preflight and external-cycle publication; preserve Intel/486 buffered semantics, no instruction-local bus workaround. Retain mode/timing/undocumented carry and long-prefix contexts in their existing ledger receivers. |

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
| T2 S9 | Delivered Shared 7a9433dc1 and NXVM 2f0ac6b63: native scalar/string I/O lanes and exact completion/failure effects; final dual-width 662/662 complete units and eight PC 0563 artifacts. First-run gate timeouts recorded; coordinator review pending. |
| T2 remaining | Native full-form qualification, mode/8080/entry, other block/port/LOCK/long-prefix contexts, physical bus, source-graded timing/neutral Machine and real board proof remain. Bit-field AX/AL/AH output collision is not silicon-exact; FFFF-start native data spans now have S8 L2 proof, not silicon-exact certification. |

## Accepted Evidence And Review

[T2 history](../history/M1-T2-v30.md), [ledger](../etc/t2-v30-ledger.md),
[S8 delivery](../etc/evidence/t2-s8-memory-transfer.md) and
[eight artifact identities](../etc/evidence/t2-s8-pc-artifacts.json) retain
complete sources, scope, actual deltas and verification.

Coordinator reviewed the actual logical/physical/fetch changes, new memory
unit, affected bit-field/nibble assertions, registration, manifests, eight
binary identities and task documents. One private lane predicate serves the
existing read/write owners; existing fetch provenance separates instruction
wrap from data spans. Original callbacks and BEGIN/COMMIT/CANCEL retain accepted
prefix effects without replay. No public API/type/layout, persistent state,
second executor/clock/queue/worker or copied reference implementation is added.
Production +45/-1/net +44 and tests +335/-4/net +331 exclude manifests/docs/
artifacts. Original handler style and Lib Types vocabulary remain.

Both original Release Ninja/ccache graphs build all targets. Complete repo-only
unit suites pass on their first full run: x64 661/661 in 310.12 seconds and
x86 661/661 in 180.98 seconds, including 172 X86 Core units. No retry, budget
increase or assertion removal is used. All four-profile links/deployments
complete per architecture, reusing production objects and restoring default.
PE/product/version/INI/hash identities pass. MyNES pair, all INIs/media and
other Shared packages remain unchanged. Manifests, Types/dependency/ownership/
negative gates, documentation, relative links and whitespace pass.

The accepted batch proves native memory byte/word lanes, 20-bit addressing,
data/pair/stack tails, separate instruction wrap, observation and split-failure
contracts. The contiguous FFFF data span is explicitly source-backed L2, not
original-chip measurement. It does not retire complete V14, I/O, electrical
bus/prefetch, timing, native-form or mode/8080 qualification. No external-ROM
integration, desktop/RDP, physical audio or game run was performed. No
MyRaidenII EXE exists. The lossless-colour [High TODO](TODO.md) still gates
final release; no quantization or second presenter is admitted.

## Previous Work

| Task | Accepted outcome |
| --- | --- |
| T1 | Source/board/set and integration-contract freeze closed at 7f7d23938. Thirteen runtime image identities and F01--F13 reconciled; not game execution. |

[T1 history](../history/M1-T1-board-and-source-freeze.md) retains its proofs.
M0 Td S1 closed at `f21a22997`; M1 remains open toward the full playable goal.

## Queue

[Queue](QUEUE.md) contains five unadmitted receivers after active V30 T2.
