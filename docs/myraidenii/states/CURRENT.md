# MyRaidenII Status

## Current Work

M1 T2 remains open after accepted S7. S8 implements native memory transfers.
Single-session coordinator/executor mode remains in effect.

## Active S8 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: MyRaidenII M1 T2 S8, next unused S after accepted S7; one-session coordinator/executor, no delegation. |
| Admission And Approval | Owner's standing full implementation, automatic S and V30 integration approval retained by goal continuation; admitted 2026-10-17. Shared CPU plus eight receiving PC artifacts; no new public interface or Lib/Emulator approval. |
| Objective | Consume V14 native memory-address/transfer batch: V30 segment-tail scalar/pair/stack access, 20-bit physical wrapping, even-word versus odd-byte lanes, code/data/observation provenance, accepted-prefix failure and existing bit-field integration. |
| Non-goals | No I/O lane/port restart, electrical bus timing/prefetch queue certification, HLT/WAIT/LOCK completion, 8080/mode, full native qualification, Machine/board/game admission, Lib/Emulator or MyNES change. These retain V09/V14/V15 receivers. |
| Reference Baseline | Clean pushed 0082edaae. Pinned NEC October 1986 manual 3-5 and 4-1--4-3; MAME 8a18b751b388390db32812be43b8802de8cac839 necpriv.ipp/nec.cpp read-only and hashed. Source-backed L2 contiguous physical span at a segment-tail operand; not silicon-exact boundary proof. |
| Candidate Proposal | [T2 proposal](../proposals/m1-v30.md), [ledger](../etc/t2-v30-ledger.md), complete native memory transfer subfamily of V14; no domain removal. |
| Files And ABI Surface | src/x86-core/cpu/cpu_instructions.c; owned CPU memory-transfer unit, affected bit-field bus-count assertions, registration/manifests; eight PC 0563 EXEs; MyRaidenII evidence/history/status. No public API/type/layout or persistent state. |
| Applicable Rules | Shared Execution/Architecture/Coding/Document, product architecture/layout/source policy, architecture-governance then coding-governance skills. One logical-address and physical-bus owner, original trace/handler style, Lib Types, ROM-free proof, no copied reference implementation. |
| Verification | Original x64/x86 Release Ninja/ccache graphs and complete repo-only unit suites; source/test manifest and registered inward/Types/ownership gates; eight real PC links/deployments with PE/name/version/INI/hash validation; governance/links/whitespace and actual-change review. |
| Expected Markers | V30:MEMORY-TRANSFER:OK, V30:BIT-FIELD:OK, full dual-width units pass; eight PC 0563 artifacts; graphs restored to default, MyNES/INIs unchanged. |
| Asset Needs | No ROM/media input. Retain current PC revision 0563; rebuild affected eight EXEs, preserve all other artifacts/configuration/snapshots/external inputs. |
| Reporting Requirements | Report source/model boundary and estimate before code, semantic/failure progress and final actual source/test deltas and artifacts. Estimate production +70--120/-10--25 and tests +250--400. |
| Stop Conditions | Stop for public API/layout, protected-source import, Lib/Emulator/outside-target change or incompatible source model. No claimed hardware-accurate timing, unqualified I/O behavior or second bus/executor. |
| Exit Criteria | Native logical access does not invent protection faults; all memory bus chunks use 20-bit width and documented byte/word lanes via existing bus callbacks, including forced observations; actual instruction/pair/stack/bit-field and every split failure position proved; full units/artifacts pass, complete target-scoped delivery pushed and reviewed. T2 stays open. |
| Original Owner Request | Implement full MyRaidenII in single-session dual-role mode under governance, with approved V30 integration and automatic bounded S admission. |
| Similar-Issue Sweep | Inspect read/write, preflight, pair, stack, code/prefetch, observation and physical-width paths; preserve all Intel paths and accepted-prefix semantics. Explicitly retain I/O, prefetch timing and undocumented hardware boundary receivers. |

| Task progress | Accepted result / remaining work |
| --- | --- |
| T2 S1 | Complete V30 domain/current-source audit accepted at 50eaf75b7: three opcode spaces and seventeen mechanism/context rows. |
| T2 S2 | Removed 83 identical advancement forks; source/test net -575 lines, dual-width 655/655 units and eight PC artifacts. No new CPU behavior admitted. |
| T2 S3 | Explicit private capabilities, native V30 profile/reset/PSW and representative 16-bit execution/preview; Machine remains ineligible. Dual-width 656/656 units and eight PC artifacts. |
| T2 S4 | NEC bit/nibble 10--1F/28/2A through existing execution/metadata/preview; dual-width 657/657 units and eight PC artifacts. |
| T2 S5 | Packed BCD 20/22/26; Shared 0fbc2cf9e, acceptance 3ceb11ef9, dual-width 658/658 units and eight PC artifacts. |
| T2 S6 | INS/EXT 31/33/39/3B; Shared 5ce9a3a9b, acceptance 56f7df605, dual-width 659/659 units and eight PC artifacts. |
| T2 S7 | Shared 297ae1637, NXVM 860fa1282 and evidence 1656b79fb accepted after actual-change review. Native carry/zero comparison repetition, dual-width 660/660 units and eight current PC 0563 artifacts. |
| T2 S8 | Delivered Shared 70dde1d31 and NXVM 0ed0a9e30: native memory lanes/addressing and source-graded L2 tail model, dual-width 661/661 units and eight PC 0563 artifacts; coordinator review pending. |
| T2 remaining | Native full-form qualification, mode/8080/entry, other block/port/LOCK/long-prefix contexts, physical bus, source-graded timing/neutral Machine and real board proof remain. Bit-field AX/AL/AH output collision is not silicon-exact; FFFF-start word remains V14. |

## Accepted Evidence And Review

[T2 history](../history/M1-T2-v30.md), [ledger](../etc/t2-v30-ledger.md),
[S7 delivery](../etc/evidence/t2-s7-conditional-repeat.md) and
[eight artifact identities](../etc/evidence/t2-s7-pc-artifacts.json) retain
complete sources, scope, actual deltas and verification.

Coordinator inspected the actual private enum/prefix predicate, profile
admission, twelve comparison-condition replacements, scan/runtime target
validation, interrupt restart, complete new CPU unit, registration, manifests,
eight binary identities and task documents. One recognizer serves execution/
preview/restart; one repeat predicate retains original comparison operands
and one-iteration safe points. No public API/type/layout, persistent state,
second parser/executor/clock/queue/worker is introduced. Production
+71/-56/net +15 and tests +281/-0/net +281 exclude manifests/docs/artifacts.
Original handler style and Lib Types vocabulary remain.

Both original Release Ninja/ccache graphs build all targets. Complete repo-only
unit suites pass on their first full run: x64 660/660 in 291.27 seconds and
x86 660/660 in 155.62 seconds, including 171 X86 Core units. No retry, budget
increase or assertion removal is used. All four-profile links/deployments
complete per architecture, reusing production objects and restoring default.
PE/product/version/INI/hash identities pass. MyNES pair, all INIs/media and
other Shared packages remain unchanged. Manifests, Types/dependency/ownership/
negative gates, documentation, relative links and whitespace pass.

The accepted batch proves native CMPS/SCAS byte/word carry/zero repetition,
prefix selection, failed data reads and NMI/INT-to-IRET continuation with up to
three retained prefixes. It does not retire the complete V05/V09 or other
ledger domains. Undocumented carry targets remain diagnostic, not hardware
claims. No external-ROM integration, desktop/RDP, physical audio or game run
was performed. No MyRaidenII EXE exists. The lossless-colour
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
