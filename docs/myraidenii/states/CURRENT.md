# MyRaidenII Status

## Current Work

M1 T2 S29 is accepted and closed after coordinator actual-change review of
Shared `921a7ffd8`, NXVM `1c1e0c761` and MyRaidenII evidence `1e6022462`.
The complete sixteen-field packet is retained in the linked S29 evidence.
T2 and the full playable objective remain open. S30 is admitted below.

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

## Task Progress

| Task progress | Accepted result / remaining work |
| --- | --- |
| T2 S30 | Delivered native-transfer nominal clocks and qualified receiving releases; coordinator actual-change review remains before closure. T2 remains open. |

[S30 evidence](../etc/evidence/t2-s30-nominal-transfer.md),
[results](../etc/evidence/t2-s30-results.json) and
[ten artifact identities](../etc/evidence/t2-s30-artifacts.json) preserve proof.
[T2 history](../history/M1-T2-v30.md) and
[ledger](../etc/t2-v30-ledger.md) retain the full original domain.

## Final Verification

Both original Release Ninja/ccache all-target builds and complete repository
unit suites pass 678/678 per width, including 191 X86 Core tests, in
312.58/152.34 seconds. Both twelve-unit related groups, all 498 strict rows
and Types/dependency/ownership/negative/manifest/documentation gates pass.
The new source-family unit proves 24,160 actual execution cases per width.
An initial static gate failure caused by a source comment is preserved in
evidence; after the comment correction the x64 CPU object remains identical,
and all final gates pass. No assertion or test budget is weakened.

Eight PC 0564 EXEs are rebuilt/deployed. Fourteen selected receiving cases
plus restored-default identity pass per width; both original graphs are
default and profile switches recompile zero CPU objects. Ten PE/hash identities
pass. MyNES 0044, owner INIs, firmware/media and snapshots are unchanged.

Production +83/-6/net +77; tests/registration +276/-0/net +276; total code
+359/-6/net +353. Manifests separately +4/-3/net +1. One cohesive private
nominal selector helper; no new production state/public function/layout.
The approved appended origin preserves all previous enum values.
Shared P1 is `daf4883f4`; NXVM receiving P2 is `fcfaf67f3`.

## Remaining T2 Boundary

The implemented nominal subtotal is explicitly not complete elapsed time:
V30 results stay unallocated, maximum stays zero and Machine rejects V30.
Remaining prefix/alias/native/8080 clocks, full bus/prefetch/dynamic/entry
accounting, source-model questions, general arrival/nesting, complete forms,
Machine/board and playable game retain the original ledger receivers.
Selected-SX system-event metadata remains its explicit TODO receiver.
No full external/native integration, manual desktop/RDP/audio, physical timing
or gameplay qualification is inferred. No runnable MyRaidenII EXE yet.

## Previous Work

S27 closed at `a50905530`; [its evidence](../etc/evidence/t2-s27-interrupt.md)
retains the earlier asynchronous-arbitration qualification.
T1 source/board/set and integration-contract freeze closed at `7f7d23938`;
[T1 history](../history/M1-T1-board-and-source-freeze.md) retains thirteen
runtime image identities and F01--F13, not game execution.
M0 Td S1 closed at `f21a22997`; M1 remains open toward the full playable goal.
Accepted NXVM T564 supplies RGB32/four-slot publication; actual RAIDEN II
palette/frame/rotation integration remains unqualified.

## Queue

[Queue](QUEUE.md) contains five unadmitted receivers after open V30 T2.
