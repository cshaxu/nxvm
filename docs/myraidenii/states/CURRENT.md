# MyRaidenII Status

## Current Work

M1 T2 S32 implementation and qualification are delivered. Shared source/test
P1 is `847c98c58`; NXVM receiving P2 is `86645b339`. Coordinator actual-change
acceptance remains; the active packet is below. T2 and the full playable
objective remain open.

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation, MyRaidenII M1 T2 S32, next unused S after accepted S31. |
| Admission And Approval | Owner authorizes automatic continued single-person coordinator/executor MyRaidenII implementation. Coordinator admits native stack and scalar-port source-clock batch on 2026-10-18. Shared X86 Core CPU/tests, NXVM eight receiving EXEs/status and MyRaidenII records only. Existing Shared CPU approval applies; no Lib/Emulator/MyNES source or public-contract expansion. |
| Objective | Consume the complete stack/scalar-port nominal source family of V15: native GP/segment/PSW PUSH/POP, immediate PUSH, memory PUSH/POP, PUSHR/POPR, PREPARE/DISPOSE and eight scalar IN/OUT encodings. Allocate only directly supported original NEC rows; explicitly prove unsupported alignment/alias/level/prefix cases stay unallocated. |
| Non-goals | No instruction semantics, new CPU state/helper framework/API/enum/layout, L2 timing inference, string-I/O/repeat/control/8080/full physical timing, Machine/board/game eligibility or Lib/Emulator/MyNES edits. Nominal subtotal is never complete elapsed time; total eligibility false and maximum zero remain. |
| Reference Baseline | Clean pushed 2438b61ff. Pinned original NEC October 1986 manual SHA FABBFCD19D4A2FCDAAC9F0BC06B00BF0A6A4BEEDAFBD3FF16ACCD075A374EA4C, section 12-1 assumptions; visually inspected printed 12-25..27,12-146..153. S14/S15/S21 existing semantics and S30/S31 sole nominal selector/publication. |
| Candidate Proposal | proposals/m1-v30.md and original V15 source family. Extend the existing one selector using completed decoder, original SP/BP/DX and lexical immediate data. Exact table values, including POPR 43/75 despite seven data reads. Do not infer mixed-address slopes or missing PREPARE level-one rows; retain those complete unresolved classes in V15 for later source/L2 review. |
| Files And ABI Surface | src/x86-core/cpu/cpu_timing.c and manifest; test/x86-core/cpu/cpu_v30_stack_smoke.c, cpu_v30_segment_smoke.c, cpu_v30_flags_smoke.c, cpu_v30_port_transfer_smoke.c and manifest, plus only directly affected prior timing exclusions if real. Estimate production +55..85/-0..8, tests +100..170/-5..15. No registration or new shared declaration. |
| Applicable Rules | Product guide/architecture/coding/roadmap/source policy, Execution lifecycle/coverage/artifacts/roles, Architecture/Coding skills, Documentation and PDF original-source review. Receiving NXVM guide/design/source policy. Lib Types, sole timing/state owner, source/test closure, one target per P and actual committed-diff review. |
| Verification | Independent source/form expectations at existing actual CPU seams: stack parity/wrap/original SP versus final SP and LEAVE BP; all immediate/GP/segment/PSW/set/frame variants; explicit source-unknown mixed alignments/level one/aliases/prefixes; scalar immediate/DX port width/parity/wrap, provider failures and no stale completion; S30/S31 proof retained. Both original Ninja/ccache all-target builds -j8, full repository units -j4, specialized gates/498 strict rows, fourteen receiving integrations and restored-default identity per width; ten PE/hash/owner-input checks. |
| Expected Markers | Existing V30:STACK:OK, V30:SEGMENT:OK, V30:FLAGS:OK cases=406800, V30:PORT-TRANSFER:OK and S30/S31 markers retained, with new nominal stack/port markers at the same tests. Both widths zero complete-unit failures/skips, all source gates and receiving cases pass; V30 max zero and Machine rejection preserved. |
| Asset Needs | Synthetic ROM-free vectors and existing pinned manual only. Rebuild eight current PC 0564 EXEs; MyNES 0044 unchanged; ten identities checked. No new firmware/media/snapshot/INI input or MyRaidenII EXE. |
| Reporting Requirements | Before edits source rows/unknown classes and estimated diff; during work actual findings and process progress; delivery actual production/test added/removed/net, preserved paths, source and verification evidence, artifact hashes and remaining full V30/game receivers. |
| Stop Conditions | New source model/public contract needed, old Intel semantic change, owner-input change or failed qualification blocks acceptance. No arbitrary timing, assertion relaxation, alias promotion, duplicate bus counting or partial-clock production eligibility. |
| Exit Criteria | Entire admitted source family has explicit allocated-source or retained-source-unknown disposition, actual executed semantic/failure/source-form proof, one selector/publication and no duplicated state/executor. Full dual-width unit/gate/receiving/artifact/doc qualification, target-separated pushed deliveries and actual-change coordinator acceptance. Only S32 may close; original full T2/M1 remains open. |
| Original Owner Request | Implement complete playable MyRaidenII in governed single-person/two-role mode, automatically continuing NEC V30 and required facilities without replacing game completion with CPU tests. |
| Similar-Issue Sweep | Audit every native implicit-stack/group alias/address/level/PSW/segment and scalar-port width/encoding, prefix/8080 isolation and all selector/preview/max/Machine consumers. Keep string-I/O/control/async and full bus time at their original V15 receivers; preserve accepted-prefix faults rather than compensating or re-executing. |


## Task Progress

| Task progress | Accepted result / remaining work |
| --- | --- |
| T2 S32 | Delivered native stack/scalar-port nominal clocks and qualified receiving releases; coordinator review pending. Complete V30 timing, Machine, board and game remain open. |

[S32 evidence](../etc/evidence/t2-s32-nominal-stack-port.md),
[results](../etc/evidence/t2-s32-results.json) and
[ten artifact identities](../etc/evidence/t2-s32-artifacts.json) preserve proof.
[T2 history](../history/M1-T2-v30.md) and
[ledger](../etc/t2-v30-ledger.md) retain the full original domain.

## Delivered Verification (S32)

Both original Release Ninja/ccache all-target builds and complete repository
unit suites pass 678/678 per width, including 191 X86 Core tests, in
320.78/136.83 seconds. Both sixteen-unit related groups, all 498 strict rows
and Types/dependency/ownership/negative/manifest/documentation gates pass.
The existing Stack unit checks 3,356 inputs; scalar-port execution expands
to 160 inputs. Segment and PSW clocks are checked at their real existing
execution seams. S30/S31 transfer, ALU and flag proofs remain. Source-unknown
mixed alignment, PREPARE level one, register aliases and prefix envelopes
remain explicitly unallocated, not assigned guessed clocks. An initial test
fetch-transaction count error and correction are preserved in evidence.
No assertion or test budget is weakened.

Eight PC 0564 EXEs are rebuilt/deployed. Fourteen selected receiving cases
plus restored-default identity pass per width; both original graphs are
default and profile switches recompile zero CPU objects. Ten PE/hash identities
pass. MyNES 0044, owner INIs, firmware/media and snapshots are unchanged.

Production +48/-5/net +43; tests +120/-9/net +111; total code
+168/-14/net +154. Manifests separately +5/-5/net zero. The existing private
nominal selector and origin are reused; no new production helper, state,
public function, enum or layout. Shared P1 is `847c98c58`;
NXVM receiving P2 is `86645b339`.

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
