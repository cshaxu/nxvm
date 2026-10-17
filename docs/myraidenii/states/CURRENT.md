# MyRaidenII Status

## Current Work

M1 T2 remains open after accepted S12. S13 qualifies native fetch start/span.
Single-session coordinator/executor mode remains in effect.

## Active S13 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: MyRaidenII M1 T2 S13, next unused S after accepted S12; one session, coordinator then executor, no delegation. |
| Admission And Approval | Owner's standing full implementation/automatic S/V30 integration approvals retained on 2026-10-17. Shared CPU and owned tests, eight affected NXVM PC artifacts and MyRaidenII evidence only; no public API/layout, Lib/Emulator or MyNES modification. |
| Objective | Consume V04/V14 native instruction-fetch start/span batch: normalize the V30 program offset at the common fetch owner, reuse existing code-span wrapping and physical lanes, qualify NEC lookahead, ordinary immediate/displacement/prefix paths, preview/cache and fetch failures without altering data-span semantics. |
| Non-goals | No physical prefetch arbitration or clock certification, complete primary semantics/slot disposition, long-prefix/LOCK/mode/8080 qualification, FPU, Machine/board/game admission or new source import. |
| Reference Baseline | Clean pushed f96c1b3cb; S12 source finding of NEC suffix-at-FFFF unmasked lookahead; pinned October 1986 NEC manual printed 3-4--3-5 and 4-1--4-3: sixteen-bit PFP, program segment addressing and native bus lanes. Retain S8's explicit data-tail L2 model. |
| Candidate Proposal | [T2 proposal](../proposals/m1-v30.md) and [complete ledger](../etc/t2-v30-ledger.md), native fetch start/span subfamily of V04/V14. No original native/mode/timing/Machine receiver is removed. |
| Files And ABI Surface | Existing cpu_instructions.c common fetch and redundant primary lookahead clamp; extend existing cpu_v30_memory_transfer_smoke.c and source/test manifests; eight PC 0563 EXEs; MyRaidenII status/ledger/evidence/history and directly stale guide commit-target wording. No new file-level production owner, public function/type/layout or persistent state. |
| Applicable Rules | Execution/Architecture/Coding/Document, product architecture/layout/source policy, architecture-governance then coding-governance. One fetch/translation owner, inward dependencies, Lib Types, original handler style and shared real-storage fixture; no handler-specific patch, second cache/decoder/bus or copied implementation. |
| Verification | Baseline reproducer then x64/x86 original Release Ninja/ccache full builds and complete repo-only units sequentially. Real primary/NEC streams at aligned/odd/segment-tail/20-bit bases, all short segment prefixes, pure preview versus execution bytes/state/data and actual fetch addresses, cache reuse, every relevant fetch/observation failure position. Source/test manifests/gates, eight PE/product/version/INI/hash identities, docs/relative links/actual-diff review; preserve all unrelated inputs. |
| Expected Markers | V30:MEMORY-TRANSFER:OK with added fetch matrix and failure contexts; complete dual-width units and eight current PC 0563 artifacts. Original build graphs/cache retained and default selected after deployments. |
| Asset Needs | No game ROM/media. Rebuild eight CPU-consuming PC EXEs at retained 0563; MyNES binaries, INIs/media/snapshots/ROMs unchanged. Ignored batch logs retained for final T2 evidence. |
| Reporting Requirements | Report initial design/estimate, baseline versus repaired behavior, sole-owner/similar-issue disposition, actual source/test plus/minus/net and complete validation/artifacts. Estimate production +2--6/-1--2 and tests +170--260, excluding manifests/docs/artifacts. |
| Stop Conditions | Stop for new public API/layout/state, protected/copied source, Lib/Emulator/outside-target edits or incompatible source model. No second code reader, instruction cache, timing owner, per-opcode wrap or Intel fetch/fault-order change without separate admission. |
| Exit Criteria | Native starting offsets and spans are resolved by their existing common owners; actual wrapped NEC lookahead and representative complete form matrix match real code/preview without out-of-segment fetch or hidden data wrap. Failures retain diagnostic/CPU effects and accepted-prefix contracts. Dual-width full units, affected artifacts, gates and complete scoped delivery pass; T2 remains open. |
| Original Owner Request | Implement fully playable MyRaidenII in governed single-session dual-role mode, with approved V30 integration and automatic complete-mechanism S admission. |
| Similar-Issue Sweep | Inspect every _s_read_cs caller, native immediate/displacement/NEC lookahead, prefix and code advance, logical span/provenance, prefetch initialization/reservation and observational preview. Reuse S8's span owner; distinguish Intel-only LOCK/control paths and native data versus fetch. Record every excluded timing/mode/long-stream class in its original ledger receiver. |

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
| T2 S13 | Delivered Shared 5c9695356 and NXVM c7ab4fdd4: common native fetch-start normalization and existing span/cache reuse; 5,700 real instruction/preview witnesses and fetch failures; first dual-width 665/665 units and eight PC artifacts. Coordinator acceptance remains pending. |
| T2 remaining | Complete native semantics and slot/form disposition, mode/8080/entry, unqualified block/port-prefix/LOCK/long-prefix contexts, physical prefetch/bus timing, source-graded timing/neutral Machine and real board proof remain. Bit-field AX/AL/AH output collision is not silicon-exact; S8 native data-tail behavior remains L2. |

## Accepted Evidence And Review

[T2 history](../history/M1-T2-v30.md), [ledger](../etc/t2-v30-ledger.md),
[S12 delivery](../etc/evidence/t2-s12-form-admission.md) and
[eight artifact identities](../etc/evidence/t2-s12-pc-artifacts.json) retain
complete sources, scope, actual deltas and verification.

Coordinator reviewed actual native dispatch, reused private format/minimum
predicates, retained NEC/Intel paths, complete owned unit and fixture cleanup,
registration/manifests, eight artifact identities and task documents. The
sixteen-field packet, complete constrained-opcode batch, original full goal,
target/P boundaries and outstanding receivers were reconciled. The baseline
reserved memory read is repaired without per-handler guards, a new public
API/layout, persistent state, decoder, instruction cache or executor. Production
+17/-4/net +13 and final tests +218/-0/net +218 exclude manifests/docs/artifacts.

Both original Release Ninja/ccache graphs build all targets. Initial complete
repo-only units pass x64 665/665 in 284.78 seconds and x86 665/665 in 102.77
seconds. After review removes two unused fixture counters without changing
assertions or production behavior, final complete units pass x64 665/665 in
72.90 seconds and x86 665/665 in 79.29 seconds. No failed run is hidden by
retry, budget change or assertion removal. Each suite includes 176 X86 Core
units and receiving App units. All eight PC 0563 links/deployments and
PE/product/version/INI/hash checks pass, reusing objects and restoring default.
MyNES pair, all INIs/media and other Shared packages remain unchanged.

Manifests, Types/dependency/ownership/negative gates, governance, relative links
and whitespace pass. Existing warning exceptions remain; the newly used trace
macro has its existing empty-else warning, not a new exemption. Subsequent
batches review fixture field ownership before full validation.

This qualifies actual primary ModRM shape/minimum admission, not every legal
native instruction's semantics or physical fetch timing. The source sweep
records NEC suffix-at-FFFF unmasked lookahead under V04/V14; its dynamic
reproduction and complete fetch start/span repair remain required. Native
slot/form semantics, mode/8080, timing, Machine and real-board/game work remain
open. No external-ROM integration, desktop/RDP, physical audio or game run was
performed. No MyRaidenII EXE exists. The lossless-colour [High TODO](TODO.md)
still gates final release; no quantization or second presenter is admitted.

## Previous Work

| Task | Accepted outcome |
| --- | --- |
| T1 | Source/board/set and integration-contract freeze closed at 7f7d23938. Thirteen runtime image identities and F01--F13 reconciled; not game execution. |

[T1 history](../history/M1-T1-board-and-source-freeze.md) retains its proofs.
M0 Td S1 closed at `f21a22997`; M1 remains open toward the full playable goal.

## Queue

[Queue](QUEUE.md) contains five unadmitted receivers after active V30 T2.
