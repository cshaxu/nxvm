# MyRaidenII Status

## Current Work

M1 T2 S31 implementation and qualification are delivered. Shared source/test
P1 is `9a438a0b9`; NXVM receiving P2 is `5ecd61022`. Coordinator actual-change
acceptance remains; the active packet is below. T2 and the full playable
objective remain open.

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation, MyRaidenII M1 T2 S31 after accepted S30, next unused S. |
| Admission And Approval | Owner directs automatic continued single-person coordinator/executor implementation. Coordinator admits the fixed-cost native scalar ALU/flag nominal-clock batch on 2026-10-18. Targets Shared X86 Core CPU/tests; NXVM eight receiving releases/status; MyRaidenII records. Existing Shared CPU authorization applies; no Lib/Emulator/MyNES source edit or new public interface is admitted. |
| Objective | Consume V15 fixed-cost native ADD/ADDC/SUB/SUBC/AND/OR/XOR/CMP/TEST, INC/DEC/NEG/NOT and CY/DIR/IE control nominal clocks, with documented register/memory/immediate/width/address-parity forms. Extend S30's one private source selector/publication mechanism; preserve all original semantic and physical-time receivers. |
| Non-goals | No dynamic multiply/divide/shift/decimal/conversion/NEC-bit/string/control-stack/8080 timing, prefix/alias timing envelope, new CPU semantics, Machine/board/game admission, public API/enum/layout, state, Lib/Emulator or MyNES edits. Total V30 time remains unallocated, maximum zero; no guessed clock or Intel fallback. |
| Reference Baseline | Clean pushed e6c1925d9. NEC October 1986 manual SHA FABBFCD19D4A2FCDAAC9F0BC06B00BF0A6A4BEEDAFBD3FF16ACCD075A374EA4C: 12-1 assumptions; 12-29..41, 12-47..49, 12-65..81, 12-89, 12-94, 12-99 and 12-172 fixed scalar/flag columns, visually inspected PDF pages. Accepted S17/S21 semantic proof and S30 nominal contract. |
| Candidate Proposal | proposals/m1-v30.md and V15 fixed scalar nominal subfamily. The completed decoder record feeds the existing selector; no execution or flag logic is copied. Existing V30_NOMINAL_SOURCE reports only documented prefetched zero-wait subtotals, not full elapsed-time eligibility. Group subopcodes must have distinct source-form keys where they select different rows. |
| Files And ABI Surface | src/x86-core/cpu/cpu_timing.c and manifest; extend the existing CPU ALU and flags units plus directly affected transfer timing-only assertions and test manifest. After reading their actual complete semantic/failure matrices, reuse them instead of duplicating a new execution harness. Refined estimate production +55..85/-3..10; tests +70..120/-5..15. No new public declaration/layout/state/helper framework. NXVM artifacts/status and MyRaidenII evidence separate Ps. |
| Applicable Rules | MyRaidenII guide/design/roadmap/source policy; Execution lifecycle/coverage/artifacts; Architecture/Coding and their local skills; Documentation and read-only PDF source review. Receiving NXVM guide/Current/design/source before release. Lib Types, sole selector/state owner, source/test owned together, one target per P and actual-diff coordinator review. |
| Verification | Independent original-manual clock expectations through real CPU execution over all eight ALU groups, documented ModRM/width/direction/immediate forms, addressing modes, parity/wrap, carry/value patterns, flags and every scoped provider refusal. Prove read-only CMP/TEST versus RMW costs, prefix/alias/dynamic exclusions, group-key identity and consecutive publication lifetime. Preserve S30 full transfer proof, S17/S21 results and old Intel paths. Both original cached Ninja builds -j8, complete repository units -j4, specialized gates and 14 PC receiving cases plus restored-default identity per width; ten PE/hash checks, MyNES/INI unchanged. |
| Expected Markers | V30:NOMINAL-ALU:OK over the retained 917,460 ALU success inputs, strengthened existing flag cases and retained V30:NOMINAL-TRANSFER:OK cases=24160; all partial V30 results remain unallocated with zero maximum. Complete dual-width units zero failures/skips, 498 strict rows and all related/receiving checks pass. |
| Asset Needs | Synthetic ROM-free CPU vectors and pinned read-only manual. Rebuild/deploy eight existing PC 0564 EXEs; verify ten identities with MyNES 0044 unchanged. No new ROM/media/snapshot/INI input or MyRaidenII executable. |
| Reporting Requirements | Before edits confirm source rows, sole owner and estimate; after implementation report actual counted added/removed/net C/H/CMake lines, focused/full qualification, receiving artifacts, failures/corrections and remaining complete timing/game boundaries. |
| Stop Conditions | Unresolved source row or necessary new model/public contract, changed old Intel semantics, external inputs or failed qualification blocks acceptance. Do not weaken assertions or promote partial nominal clocks to production eligibility. |
| Exit Criteria | Entire admitted scalar/flag nominal family has independent executed source/form and failure/lifetime proof, no duplicate execution/state/publication, old paths retained only for their real semantics; complete dual-width units/gates/receiving releases and manifest/doc checks pass, target-separated complete deliveries pushed and coordinator reviews actual changes before governance S closure. T2 remains open. |
| Original Owner Request | Implement full playable MyRaidenII in governed single-person/two-role mode, automatically continuing bounded NEC V30 and facility work without narrowing the objective to passing CPU tests. |
| Similar-Issue Sweep | Audit all scalar opcode directions/group extensions, read-only versus RMW transfer classes, immediate signed width, prefixes/aliases and nominal key lifetime; follow every V30 selector/preview/max/Machine consumer. Repair the owning nominal calculation or retain an explicit original V15 receiver; do not duplicate instruction semantics. |

## Task Progress

| Task progress | Accepted result / remaining work |
| --- | --- |
| T2 S31 | Delivered fixed native scalar/flag nominal clocks and qualified receiving releases; coordinator review pending. Complete V30 timing, Machine, board and game remain open. |

[S31 evidence](../etc/evidence/t2-s31-nominal-alu.md),
[results](../etc/evidence/t2-s31-results.json) and
[ten artifact identities](../etc/evidence/t2-s31-artifacts.json) preserve proof.
[T2 history](../history/M1-T2-v30.md) and
[ledger](../etc/t2-v30-ledger.md) retain the full original domain.

## Delivered Verification (S31)

Both original Release Ninja/ccache all-target builds and complete repository
unit suites pass 678/678 per width, including 191 X86 Core tests, in
307.55/129.99 seconds. Both fourteen-unit related groups, all 498 strict rows
and Types/dependency/ownership/negative/manifest/documentation gates pass.
The existing ALU unit checks 917,460 inputs plus fifteen adjacent exclusions;
the flags unit retains 406,800 inputs with clock checks at actual execution,
not pure debug queries. All 24,160 S30 transfer inputs remain. Initial test
ordering/admission mistakes and corrections are preserved in evidence.
No assertion or test budget is weakened.

Eight PC 0564 EXEs are rebuilt/deployed. Fourteen selected receiving cases
plus restored-default identity pass per width; both original graphs are
default and profile switches recompile zero CPU objects. Ten PE/hash identities
pass. MyNES 0044, owner INIs, firmware/media and snapshots are unchanged.

Production +41/-3/net +38; tests +120/-9/net +111; total code
+161/-12/net +149. Manifests separately +4/-4/net zero. The existing private
nominal selector and origin are reused; no new production helper, state,
public function, enum or layout. Shared P1 is `9a438a0b9`;
NXVM receiving P2 is `5ecd61022`.

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
