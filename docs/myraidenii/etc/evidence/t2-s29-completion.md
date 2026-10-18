# T2 S29 Completion Versus Event-Only Entry

Baseline `7c0143472`, after accepted S28. This consumes the V15
software-entry completion classification subfamily and ordinary HALT
failure publication, not complete V30 timing or Machine qualification.
Final qualification and release identities below are complete. T2 and the
full playable objective remain open.

Shared implementation P1 is `921a7ffd8`; NXVM receiving P2 is `1c1e0c761`.

## Source And Actual Reproduction

The pinned [NEC October 1986 V20/V30 user manual](https://www.dosdays.co.uk/media/nec/NEC-V20-V30-Users-Manual.pdf),
SHA-256 `FABBFCD19D4A2FCDAAC9F0BC06B00BF0A6A4BEEDAFBD3FF16ACCD075A374EA4C`,
Table 6-1 and the DIV/CHKIND pages classify these as software interrupts.
Accepted S19/S22 independently establish following-PC frames and actual RETI.
The diagnostic snapshot is not a claim that the instruction remains unexecuted.

The strengthened actual software-arbitration unit fails before repair at
`op=3 flags=0000 events=0 pc=400/400 sp=fffa/fffa flags=8002/8002`.
The architectural frame/result already matches; completion metadata does not.
Successful V30 DIV/CHKIND is marked as an unretired fault, preventing the
normal selector/completed-observation publication even though other software
entries complete. This was not covered by existing correct result/frame tests.

The result sweep separately reproduces a failed HALT-wake entry reporting
`completed=0/0 event=1/0 consumed=0 pc=0 halt=0 stop=1`.
The pre-dispatch readiness snapshot survived an error inside ExecInt and
falsely marked that failed attempt as a delivered event.

## Sole Owners And Repair

At successful synchronous delivery, the existing committed NEC software-entry
fact prevents the unretired-fault marker. Diagnostics, saved CPU/frame, failed
write prefixes and old Intel architectural faults are unchanged. Shutdown
continues through the original generic fault helper. No opcode classification,
replacement exception owner or additional stored outcome is introduced.

After ordinary asynchronous dispatch, the existing no-instruction/HALT-wake
publisher now checks the actual stop result before marking delivery. Failed
memory/acknowledgement still stops through the original provider fault path;
it does not publish a successful event. A successfully completed instruction
which is followed by a failed step entry retains its completed observation:
failure of later work is not retroactive cancellation of the instruction.

V30's selector still truthfully returns unallocated time. This repair restores
completion/observation eligibility, not clocks, maximum budget, physical grade
or Machine admission. The separate recorded diagnostic never becomes another
guest control or timing path.

## Existing And New Owned Proof

Four existing CPU units acquire one common fixture assertion over actual
results. It checks saved completion, the public event-only result and its
one-shot consumption, and absence of instruction ticks in non-completed rounds.
The original operand/flag/stack/vector/phase/checkpoint and accepted-prefix
assertions are preserved, not replaced by this Boolean check.

The multiply/divide unit covers native signed/unsigned, byte/word, immediate,
register/memory/segment/alias values and every existing data refusal.
Software-entry covers taken/untaken bounds/BRKV, every software vector,
RETI, PSW and existing refusal positions. Software-arbitration covers its
3,457 native/mode/conditional/pending-source inputs and failures. Ordinary
interrupt proof covers native/8080 completion, event-only HALT wake,
acknowledgement/transfer refusals and failure of an own-step entry after
successful completion. Actual mode entry is retained.

A 104-line CPU completion unit independently checks 43 cases: seven runnable
CPU profiles each have DIV classification contrast, successful NMI/INT wake,
failed memory wake and failed INT acknowledgement. DEFAULT is explicitly
tested as a non-runnable construction sentinel, not guessed to be an Intel
chip. Each successful wake then executes a real handler instruction to prove
the event-only result does not leak into later completion.

Initial test work placed a completed-instruction assertion on a no-instruction
acknowledgement failure; it was moved to the actual own-step failure path.
Adding useful fixture diagnostics required its own Lib Types file declaration,
rather than relying on include order. The first generic probe incorrectly tried
to execute DEFAULT; source family/form authority requires rejection and the
final case proves that rejection. No production failure policy is weakened.

## Similar-Issue Sweep And Dispositions

Queries over production, owned tests and direct consumers:

- `rg -n 'instruction_fault_delivered\s*=|_e_mark_instruction_fault_delivered' src/x86-core test/x86-core`
- `rg -n 'consume_instruction_fault_delivery|record_delivered_exception|completed_observation' src/x86-core`

The successful synchronous publisher is fixed using the existing entry result;
its generic shutdown/non-NEC marker remains. The ordinary post-dispatch
publisher is fixed at the stale-precondition boundary for every profile.
The explicit shutdown-wake publisher already requires successful entry.
Reset/refresh/consume clears are lifecycle operations, not additional owners.
Machine's delivered-exception callback records diagnostics without faulting;
its actual fault callback is distinct. Machine checks provider-faulted state
before its timing/event-only consumer; no Machine adapter change is required.

Two selected-SX-only publishers (SMM entry and STOP-GRANT arbitration) retain
their distinct accepted-prefix/event_ticks qualifications. Possible failed-
attempt Boolean divergence needs their original source/L2 failure proof, not
a V30-clock patch. The bounded [TODO receiver](../../states/TODO.md) records
both sites, risk, lack of demonstrated guest failure, owner and admission.
V30 cannot enter them; no whole-CPU-system-event consistency claim is made.

## Counted Change And Remaining Boundary

Counted Git source/test paths exclude manifests, records, generated inputs
and artifacts. Production is +4/-2/net +2; tests/registration +143/-5/net +138;
total code +147/-7/net +140. Manifests separately +8/-7/net +1. No production
helper/state/API/layout/enum, clock, thread or alternate execution path is added.
The one fixture assertion is a real multi-test invariant, not a new framework.
Normal strict warnings remain; existing instruction exceptions are not erased.

Complete V30 nominal/bus/prefetch/dynamic timing, loaded-BRK/prefix/general
arrival, unresolved packed/non-ten/CHKIND models, complete native forms,
Machine, board and playable game remain the original ledger receivers.
No full external/native integration, manual desktop/RDP/audio or gameplay
qualification is inferred from unit or selected receiving proof.

## Final Qualification

Both original all-target Release Ninja/ccache graphs pass. Complete repository
unit suites pass 677/677 per width (190 X86 Core), in 307.07/123.25 seconds.
Both nine-unit related groups pass, including CPU:COMPLETION:OK cases=43 and
the strengthened existing native/8080 result/refusal routes. All 498 strict
rows and Types/dependency/ownership/negative/manifest gates pass; global Types
checks include all 1,387 tracked code/script files. New/changed tests use
normal strict warnings; historical instruction exceptions remain explicit.

Eight PC 0564 EXEs are rebuilt/deployed. Fourteen selected receiving cases
plus restored-default identity pass per width, with zero CPU object recompiles
during profile switches. Both original graphs are restored. All ten actual
PE/name/width/size/hash identities pass; MyNES 0044 hashes and owner INIs are
unchanged. No external firmware/media or snapshot source is edited. The final
qualification process exits zero; no final full suite or receiving case needs
a retry, skip or increased budget. A local runner-template quoting error was
corrected before any command in that malformed driver could execute; it did
not change source, tests or a completed result.

Documentation and whitespace gates pass for MyRaidenII and receiving NXVM.
Raw/staged Shared bytes are identical LF. The measured
[results](t2-s29-results.json) and [ten identities](t2-s29-artifacts.json)
come from final JUnit/logs and actual deployed PEs with frozen input hashes.
Ignored logs/scripts/manual are retained for current proof and the following
timing receiver; no new raw trace/rendering or residual owned process remains.

## Complete Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation, MyRaidenII M1 T2 S29, next unused S after accepted S28. |
| Admission And Approval | Owner requests automatic continued MyRaidenII implementation under governance, with approved Shared V30 work. Coordinator admits this native/8080 completion-contract batch on 2026-10-18. Targets Shared CPU/test mechanisms, NXVM eight receiving EXEs/evidence, MyRaidenII records. MyNES source/assets and Lib/Emulator are excluded. |
| Objective | Reconcile V15's DE/BR completed-software-entry versus unretired-fault outcome using S28's sole successful-entry fact. Prove the entire affected result class: ordinary and software completion, event-only HALT wake, failed entry, and failed asynchronous entry after a successfully completed instruction. Delivered diagnostics must not imply unretired execution. |
| Non-goals | No clock allocation, physical timing/model grade, Machine V30 admission, public API/enum/layout, new state/helper in production, Intel architectural fault-policy change, selected-SX system-event/prefix accounting, game/board activation or T closure. The two SX-only metadata candidates have an explicit separate source/L2 review receiver in TODO; V30 cannot enter those paths. Remaining nominal/bus/prefetch/dynamic timing is the original V15 receiver, not claimed complete. |
| Reference Baseline | Clean pushed 7c0143472; NEC October 1986 user manual SHA FABBFCD19D4A2FCDAAC9F0BC06B00BF0A6A4BEEDAFBD3FF16ACCD075A374EA4C, software-entry table and DIV/CHKIND following-PC semantics; accepted S19/S22/S27/S28 proofs and T2 convergence ledger. |
| Candidate Proposal | proposals/m1-v30.md, complete V15 software-entry retirement classification subfamily. Preserve the common fault helper for shutdown and non-NEC failures; only a successful delivery already tagged as a completed NEC software instruction avoids the unretired-fault marker. Original timing selector still reports unallocated V30 time and Machine remains gated. |
| Files And ABI Surface | src/x86-core/cpu/cpu_instructions.c and manifest; existing CPU V30 memory fixture plus multiply/divide, software-entry, software-arbitration and asynchronous-interrupt units and manifest. The in-scope failed-HALT-wake discovery requires a CPU-owned completion unit/registration across all profiles: a pre-dispatch snapshot must not mark failed entry as delivered. This is failure metadata only, not an Intel architectural fault-policy change. Revised estimate production +4..8/-2..3, tests +125..180/-0..8. No public signature/layout or new production state/helper. Receiving artifacts and product-scoped evidence only. |
| Applicable Rules | MyRaidenII guide/architecture/coding/roadmap/source policy, shared Execution/Architecture/Coding/Document, local architecture/coding skills; receiving NXVM guide/Current/design/source/artifact authority. One completion owner, truthful unallocated time, Lib Types, synthetic owned regressions, no copied hardware state/second executor, one-target P and actual-change review. |
| Verification | Actual before witness through existing CHKIND/divide unit; both original Release Ninja/ccache all-target builds -j8, affected units then complete repository unit suites -j4 and specialized gates, 14 selected PC receiving cases plus restored-default identity per width; all ten PE/hash identities, MyNES/INI unchanged; documentation/manifest/whitespace checks. |
| Expected Markers | CPU:COMPLETION:OK cases=43 plus existing V30 MULTIPLY-DIVIDE, SOFTWARE-ENTRY, SOFTWARE-ARBITRATION and INTERRUPT success markers; complete units zero failures/skips per width and 498 strict rows retained; every receiving/restored case passes. A consumed event-only result returns true exactly once. |
| Asset Needs | ROM-free CPU tests and pinned manual; rebuild/deploy eight current PC 0564 EXEs, verify ten identities with MyNES 0044 unchanged. No game ROM, new firmware, INI/media/snapshot edit or runnable MyRaidenII artifact. |
| Reporting Requirements | Confirm class/estimate, before witness and focused proof, final counts and actual added/removed/net lines, existing result/state ownership, receiving artifacts and unverified physical timing/desktop/RDP/gameplay boundary. |
| Stop Conditions | Required public contract/model/source-policy change, unrelated dirty changes or failed regression/release/gate prevents acceptance; no weakened assertions, invented clocks, deferred failure disguised as pass or silent scope expansion. |
| Exit Criteria | DE/BR complete like the other accepted software entries; event-only and accepted-prefix failure semantics remain distinct; all four affected units, the new 43-case completion unit and full dual-width units/gates/releases pass, manifests/evidence current and target-separated P deliveries pushed. Coordinator actual-change review then pure governance S closure; T2 stays open. |
| Original Owner Request | Continue the full single-person/two-role governed MyRaidenII implementation, implementing NEC V30 and required facilities; automatically admit each bounded S without shrinking the original playable objective. |
| Similar-Issue Sweep | Inspect all fault-marker calls, successful real-entry callers, selector/completed-observation creation, HALT wake, own-step/external failure and public consume operation plus Machine use. Reuse one fixture assertion across real existing test consumers. The observed failed-HALT-wake false event is repaired after dispatch for all profiles, with direct ordinary Intel divide-fault contrast and failed/successful wake tests. Intel architectural fault policy/shutdown and physical timing receivers remain unchanged. |
