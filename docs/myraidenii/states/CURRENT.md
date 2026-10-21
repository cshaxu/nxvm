# MyRaidenII Status

## Current Work

M1 T7 is active. It receives T6's one Product composition route and qualifies
external-ROM startup, then progressively receives gameplay and release work.
It does not redefine unimplemented Board behavior as a release result.

## Active Packet — M1 T7 S29

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation after closed S28; one session performs coordinator and executor roles. |
| Admission And Approval | The owner directed continued MyRaidenII work; this packet changes only MyRaidenII's existing Sound-to-Board aggregate observation. |
| Objective | Classify existing OKI writes as sample commands, voice selections, stops or locally rejected selections without recording guest payload, so the observed zero voice starts can be assigned to the current receiver or to upstream Z80 behavior. |
| Non-goals | No Shared/API change, raw command/sample output, host audio redesign, emulator reference import, new thread/queue, ROM/PAL change, mixer behavior change, title/gameplay claim or release. |
| Reference Baseline | S26 proves both OKI ports receive writes but active voice starts and generated PCM are zero. The existing OKI state machine already distinguishes its command, selection and stop paths internally. |
| Candidate Proposal | Add bounded counters to the existing private Sound observation and copied Board observation. Increment them at the existing branch owners only; extend the existing Sound/Board tests and selected-set neutral summary. |
| Files And ABI Surface | MyRaidenII `core/sound.[ch]`, `core/board.[ch]`, existing Sound/Board tests, existing selected-set Probe, Current and evidence. Expected production +20..40/-0..8, tests +30..70/-0..15. No Shared/public cross-product interface. |
| Applicable Rules | MyRaidenII guide, Architecture/Coding/UI, source policy, shared Execution/Coding/Documentation rules and T7 qualification proposal. Real-set counts are L2 observations. |
| Verification | Focused Sound/Board units x64/x86; existing Probe build; one bounded selected-ROM run per width; compare neutral summaries; affected MyRaidenII unit suites; documentation/whitespace checks. |
| Expected Markers | Every OKI write has one named local disposition; no raw values leave Sound; both widths agree. A count alone does not establish audible playback or a physical chip contract. |
| Asset Needs | Read-only owner archive through the existing wrapper only; no acquisition or persistence. |
| Reporting Requirements | Before/after branch/accounting matrix, actual diff, dual-width counts, retained uncertainty and cleanup. |
| Stop Conditions | Need for Shared/public cross-product API, raw payload capture, source import, behavior change to force a voice, unexplained counter ambiguity or staging failure stops and records the gap. |
| Exit Criteria | Existing observation exposes every named OKI disposition, synthetic owner tests cover them, bounded results are compared, checks pass, stages/logs are removed and a complete P is pushed. |
| Original Owner Request | Continue independently using bounded probes and external emulators as L2 references rather than waiting for physical hardware. |
| Similar-Issue Sweep | Both devices, pending/nonpending command state, valid/invalid selection, every stop mask, reset, aggregation copy and no-payload output. |

## M1 T7 S29 Closure

S29 is closed after actual-diff review.  The existing Sound observation now
classifies all local OKI receiver paths without exposing payload.  Existing
synthetic tests cover valid selection, stop, rejected selection and ignored
control through the production decoder; both host widths passed the affected
tests and the complete 12-test MyRaidenII unit label.

The bounded selected-set runs agree exactly: both OKI devices received 373
writes, all through the existing stop branch, with zero sample commands,
selections, rejections, starts and generated nonzero PCM.  S25's reconciled
Board-to-Sound lane map remains unchanged.  Thus no local mapping or decoder
change is justified merely to manufacture audio.  See [S29 evidence](../etc/evidence/t7-s29-oki-write-disposition.md).

The external stages were removed after the probes.  No Shared component,
public cross-product interface, executable, protected media or Assets release
changed.  T7 remains active; no successor S is admitted by this closure.

## Archived Packet — M1 T7 S28

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation after closed S27; one session performs coordinator and executor roles. |
| Admission And Approval | The owner directed continued MyRaidenII implementation and permits bounded probes rather than Computer Use. This is a MyRaidenII test-only packet. |
| Objective | Add one mutually exclusive, ordered Coin then Start pulse to the existing selected-set Probe so its deterministic Board input route can be compared with the prior simultaneous pulse without adding a guest-input path. |
| Non-goals | No Shared, Product, Machine, Core production, API, ROM/PAL, title-state mirror, host automation, timing claim, gameplay claim, external-reference dependency, or release change. |
| Reference Baseline | S19 proved only the simultaneous quarter/half instruction-budget pulse. S24's external input pulse orders Coin before Start at frame checkpoints; its frame schedule is not transferred as local hardware timing. |
| Candidate Proposal | Extend the existing Probe argument enum with one exclusive `--coin-start-sequence` mode. At fixed normalized instruction-budget fractions it presses/releases Coin, then later presses/releases Start through the existing Machine driver. Report the four application flags and thresholds in the existing neutral summary. |
| Files And ABI Surface | `test/myraidenii/integration/selected_set_startup_probe.c`, its existing wrapper only if forwarding needs adjustment, Current and one evidence record. No production or public ABI change. Expected test-only delta +45..75/-8..18 lines. |
| Applicable Rules | MyRaidenII guide, Architecture/Coding/UI, source policy, shared Execution/Coding/Documentation rules and T7 qualification proposal. The schedule is L1 containment/diagnostic policy; resulting owner-ROM observations are L2. |
| Verification | Build the existing Probe on x64/x86; reject conflicting/unknown options; run the owner-ROM wrapper once per width at its existing 20M cap; compare neutral summaries; run affected MyRaidenII tests and documentation/whitespace checks. |
| Expected Markers | Four ordered driver deliveries succeed once each, both widths agree, and either a finite difference or a finite neutral result is recorded without identifying a title/credit state. |
| Asset Needs | Read-only owner archive through the existing staging wrapper only; no acquisition or persistence. |
| Reporting Requirements | Record schedule, implementation/test delta, dual-width output comparison, cleanup and the explicit non-equivalence to external frame timing. |
| Stop Conditions | A need for production input changes, new public state/API, host automation, raw guest-state capture, unbounded run, protected output retention or a failing existing input route stops the S and records the precise gap. |
| Exit Criteria | The existing Probe has the ordered mode with exclusive parsing and all four edge results; verification passes; stages are removed; actual diff is reviewed and a complete P is pushed. |
| Original Owner Request | Continue independently using bounded probes and external emulators as L2 references rather than waiting for physical hardware. |
| Similar-Issue Sweep | Existing held/simultaneous/ordered mode exclusivity, every press/release failure, instruction threshold overflow/order, output summary framing, wrapper forwarding and staging cleanup. |

## M1 T7 S28 Closure

S28 is closed after actual-diff review.  The existing selected-set Probe now
has one exclusive ordered Coin/Start diagnostic mode.  Both widths completed
the full 20M run with byte-identical neutral summaries and all four driver
deliveries accepted.  This rules out only the old simultaneous-pulse shape;
it does not prove a guest title, credit, start, gameplay or sound outcome.
See [S28 evidence](../etc/evidence/t7-s28-ordered-input-probe.md).

The affected MyRaidenII unit suites passed on both host widths.  External
stages and temporary logs were removed.  No production, Shared, public API,
asset or executable changed.  T7 remains active; no successor S is admitted
by this closure.

## Archived Packet — M1 T7 S27

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation after closed S26; one session performs coordinator and executor roles. |
| Admission And Approval | The owner directed continued MyRaidenII work, permits probes instead of Computer Use, and permits external emulator behavior as L2 evidence. This packet is MyRaidenII research only. |
| Objective | Establish whether the selected set exposes a finite observable COPX-D2 receiver boundary during bounded reference startup, or record the evidence-backed absence of such a contract without inventing a local COPX route. |
| Non-goals | No production/test/CMake/API change, COPX loader, lookup-table import, ROM/PAL download, raw ROM/RAM/trace/pixel/audio retention, MAME source import, protected-media commit, Shared/other-App change, or release claim. |
| Reference Baseline | S5/S8 establish that the local twelve-image transaction deliberately excludes COPX-D2 because no finite receiver exists. S26 proves sound register traffic but no PCM activity. The current Board owns the only local COP model. |
| Candidate Proposal | Inspect the already-pinned external reference only for map/device identity, then run a bounded MAME Lua black-box probe against the owner-supplied selected set. Observe only named device/bus write counts and neutral lifecycle markers for candidate COP boundaries during fixed startup. Adopt no behavior from an unbounded trace or opaque payload. |
| Files And ABI Surface | Current and one MyRaidenII evidence record only. Temporary Lua/configuration/staged archive material is external under `O:\repos.hobby\nxvm-assets\media-myraidenii\t7-s27-mame`; it is removed after process exit. No source, test, manifest, interface, asset or executable file changes. |
| Applicable Rules | MyRaidenII guide, Architecture/Coding/UI, source policy, shared Execution/Documentation/Coding rules and T7 qualification proposal. External observations are L2 only. |
| Verification | Pin external tool/revision identities; bounded reference observations once per specified schedule; retain only aggregate counts and irreversible neutral markers; verify owned process/stage cleanup; run documentation governance and whitespace checks. |
| Expected Markers | Either a repeatable finite candidate request/result boundary with enough facts for a later independently designed contract, or a documented no-contract result. Neither outcome proves gameplay, COPX accuracy, title progress, sound or release readiness. |
| Asset Needs | Read-only owner ROM archive and already owner-approved external PAL files, staged only outside the repository. No asset is acquired, modified or committed. |
| Reporting Requirements | Record exact reference revision/tool identity, candidate observation scope, schedules, aggregate results, evidence grade, cleanup, and what remains unknown. Do not retain protected payloads. |
| Stop Conditions | Any need for source/table/code import, raw guest-state inspection, unbounded tracing, an inferred local lookup rule, unavailable reference boundary, nondeterminism, or a production change stops this S and records the gap. |
| Exit Criteria | A finite evidence-backed disposition is recorded; owned temporary output is removed; applicable documentation checks pass; actual diff is reviewed and a complete P is pushed. |
| Original Owner Request | Continue independently toward MyRaidenII using bounded probes and external emulators as L2 references rather than waiting for physical hardware. |
| Similar-Issue Sweep | Candidate COP map/register traffic, selected-set boot state, device availability, staging isolation, output size limits and cleanup. |

## M1 T7 S27 Closure

S27 is closed after the actual-diff review.  The bounded MAME observation
shows existing COP control, command and trigger windows are active, while the
pinned reference still exposes no COPX-D2 request/result boundary.  It would
therefore be misleading to add an INI role, loader or guessed Board lookup.
The result is recorded as L2 evidence in
[S27 evidence](../etc/evidence/t7-s27-copx-boundary-observation.md); COPX
remains the explicitly deferred receiver gap in TODO.

No production, test, CMake, manifest, interface, asset or executable changed.
The external ROM/PAL staging root and task Lua script were removed after the
reference process exited.  Documentation governance and whitespace review are
the applicable verification for this documentation-only result.  T7 remains
active; no successor S is admitted by this closure.

## M1 T7 S26 Closure

S26 is closed after the coordinator review of `4a68b49fb`.  The single existing
Sound → Board observation route now proves local YM and both OKI write
reachability, active voice starts and generated nonzero PCM without exposing
payload or mutable internals.  Existing Sound/Board regressions cover both
the direct device model and the public Board bridge, including reset.

One bounded Coin/Start selected-set pulse per host width reached YM and both
OKI write ports but reached no active OKI voice or nonzero generated PCM; the
results were identical on x64 and x86.  A finite 600-frame MAME Lua write-tap
run observed the same three address ranges as L2 reference behavior.  The
evidence does not identify a demonstrated local YM/OKI/PCM defect, so no
audio model was changed merely to create output.  Temporary MAME/probe stages
were removed after their processes ended.

The two stable Ninja graphs passed all 697 repository-only unit tests.  The
current x64/x86 MyRaidenII candidates were rebuilt, but no Assets release was
published because T7's initial gameplay/release qualification remains open.
No Shared, other-App, INI or protected-media file changed.  See
[S26 evidence](../etc/evidence/t7-s26-sound-path-observation.md).

T7 remains active.  The remaining receiver is title progression/qualification
beyond the bounded startup state; this result is not an audio, gameplay or
release success claim.  No new S is admitted by this closure record.

## Archived Packet — M1 T7 S25

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation after S24; one session performs coordinator and executor roles. |
| Admission And Approval | Owner's continuing MyRaidenII implementation approval, reaffirmed by the latest continue request; external emulator facts may be used as L2. Target is MyRaidenII only. |
| Objective | Reconcile the complete main/sound command, response, pending and interrupt handoff at the actual byte-address boundary; repair only demonstrated local discrepancies. This consumes the sound/input receiving-path portion of G04/G05/G11, not gameplay qualification. |
| Non-goals | No Shared or other App changes, new public API, CPU fork, game-state mirror, COPX guess, ROM patch/import, host UI automation or unrelated sound synthesis redesign. |
| Reference Baseline | S24 `2c3ce8354`; existing Board low-byte mapping, Sound transport, board-bus/sound tests, frozen T1 checkpoint ledger. Local address division and MAME delegate offset units need direct verification, not interpretation by name. |
| Candidate Proposal | Use bounded MAME Lua address-space operations with synthetic command/reply values, without guest execution between write/read observations. Compare physical-address results with the local real Board route. Sweep commands, replies, pending/acknowledge, IRQ and unused lanes. If mismatch is demonstrated, repair its existing owner and extend existing synthetic tests. |
| Files And ABI Surface | src/myraidenii/core/board.c, sound.c, private sound.h and existing board-bus/sound tests; evidence and Current. Probe confirms a four-byte register stride with even-lane mirrors, plus two distinct pending directions. Reference IRQ inspection also identifies missing in-service inhibition. Revised estimate: production +65/-25, tests +140/-30. Existing private callback is renamed to its actual response-pending meaning; no public or Shared interface is added. |
| Applicable Rules | Product guide, Architecture/Coding/UI, source policy, shared Execution/Architecture/Coding/Documentation and T7 proposal. External implementation is reference only; synthetic observations are L2, not a silicon claim. |
| Verification | Bounded external synthetic handoff probe; existing Board/Sound units on x64/x86; required complete repository-only unit suites; affected local selected-set startup observation; existing Ninja/ccache trees; rebuild the existing Product candidate pair. T7 still owns initial release/configuration qualification, so this repair does not promote the unqualified candidate into Assets. |
| Expected Markers | Explicit physical addresses and synthetic values agree across each receiving path, or each discrepancy has an owner and regression. No checksum or elapsed budget is labelled gameplay success. |
| Asset Needs | Read-only previously inventoried owner ROM/PAL set for MAME; no acquisition or protected payload retention. |
| Reporting Requirements | Before/after address/handshake matrix, reference revision/tool identity, actual code delta, test results, artifacts if affected and remaining gameplay limits. |
| Stop Conditions | Need for Shared change, protected code/table import or undocumented new CPU/device contract stops that implementation and reports the precise missing authority. A disproved mapping hypothesis is recorded, not patched. |
| Exit Criteria | Entire finite handoff batch is reconciled and any demonstrated in-scope mismatch fixed with receiving-path regressions; applicable verification passes; temporary output removed; complete P pushed and reviewed. |
| Original Owner Request | Continue independently implementing MyRaidenII using probes and explicit L2 external-emulator references rather than waiting for physical hardware. |
| Similar-Issue Sweep | All main sound-window byte/word offsets, inactive lanes, both latch bytes, both directions, pending/acknowledge/IRQ/reset and coin input polarity; retain unrelated video/COP work without attributing an unproven cause. |

Transient scripts are under `build/myraidenii-t7-s25/`; external tool output is
under `<external-assets>/media-myraidenii/t7-s25-mame/`. Each reference probe has
a 20-second emulated ceiling, 60-second wall budget and 1 MiB output budget.
The current session completed cleanup after process exit; no protected output is retained.

## M1 T7 S25 Closure

S25 is closed after coordinator actual-diff review of `d99c89ef4`. The finite
Board/Sound handoff repair and both existing regression suites preserve one
production route. Both host widths passed all 697 repository-only units and
the 20M-instruction real-set pulse produced identical neutral results. Both
Product candidates were rebuilt; initial Assets release remains T7's unfulfilled
qualification, not this bounded repair's outcome. No Shared or other App
files changed. See [S25 evidence](../etc/evidence/t7-s25-sound-handoff.md).

T7 remains active: the sampled PCM is still silent and gameplay is unqualified.
The next receiving investigation should distinguish Sound CPU execution,
device register writes and PCM generation using bounded observations along the
existing route. The corrected latch `00,82` alone proves neither command
completion nor an audible result; it must not be used to blame COPX without
independent evidence. No new S is admitted by this closure record.

## Archived Packet — M1 T7 S24

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; S24 follows closed S23. One session performs coordinator and executor roles. |
| Admission And Approval | The owner directed continued MyRaidenII implementation, prefers probes over Computer Use, and has authorized external emulator behavior as L2 evidence. This packet is a bounded external black-box diagnostic only. |
| Objective | Use MAME's documented Lua input API to establish whether a finite Coin 1 then Start 1 press/release changes the selected set's visible checkpoint relative to an otherwise identical no-input run. |
| Non-goals | No Computer Use, source/code/table import, local production/test/API change, ROM/PAL acquisition or retention, debugger/memory/trace/screenshot/audio capture, input mapping change, gameplay or release claim. |
| Reference Baseline | S23 proves the local Machine-driver pulse reaches Board controls but cannot infer guest acceptance. S35 provides the owner-approved external PAL BYOB inputs required for current MAME verification. |
| Candidate Proposal | Create one transient Lua script outside tracked source. It enumerates the named Coin 1/Start 1 fields through MAME's documented I/O-port API, applies a bounded press/release schedule, and prints only field discovery plus visible-screen dimensions and non-reversible checksums at fixed frames. Run the same fixed budget once with and once without the pulse; remove all scripts/configuration/logs afterwards. |
| Files And ABI Surface | Documentation/evidence only after the experiment. No repository source, test, CMake, manifest, public interface, asset or executable change. Temporary external MAME output is confined to `<external-assets>/media-myraidenii/t7-s24-mame/`. |
| Applicable Rules | MyRaidenII guide; product Architecture, Coding and UI; shared Execution, Documentation and Coding rules; source policy. MAME Lua is an L2 black-box tool. Its API and visible result do not establish a physical Board contract or local game correctness. |
| Verification | Record MAME version and the previously inventoried read-only ROM/PAL inputs; run bounded no-input and pulse observations with no payload retention; compare the neutral output records; remove owned external output; run documentation governance, whitespace checks and repository-only units on both host widths. |
| Expected Markers | The two cases either produce a repeatable finite difference or a repeatable no-difference. Either result is L2 evidence only and cannot alone prove title, credit, start, gameplay or a local defect. |
| Asset Needs | Read-only owner archive and already owner-approved external PAL files. No asset is added, copied, modified or committed. |
| Reporting Requirements | Record tool/input identities, input-field names, schedule, dimensions, irreversible checksums, host-cleanup and the exact scope limitation. Do not retain pixel, ROM, RAM, sound or trace payload. |
| Stop Conditions | Missing/ambiguous input fields, API inability to schedule the bounded pulse, nondeterministic results, any need for debugger/memory/source inspection, or any local behavior change stops this packet and records the gap rather than guessing. |
| Exit Criteria | One documented, reproducible black-box disposition exists for no-input versus Coin/Start pulse; all owned transient files are removed; required documentation checks pass; no local product behavior is misrepresented as qualified. |
| Original Owner Request | Continue implementing MyRaidenII under the stated L3/L2/L1 classification; probes are the normal diagnostic mechanism, while Computer Use is not required for this task. |
| Similar-Issue Sweep | Inspect both player-one input fields, active-low/active-high semantics through the API rather than assuming polarity, no-input baseline scheduling, frame callback termination, and every transient external output path. |

## M1 T7 S24 Closure

S24 is accepted and closed after coordinator actual-diff review of `27533db46`.
Both host widths passed the full 697 repository-only units, including the
45 MyNES cases omitted by the initial cache. The MAME I/O-field probe identifies the exact
external Coin 1 and Start 1 field names, and a bounded press/release schedule
produces a repeatable 320 x 240 screen checksum distinct from its no-input
baseline.  The external root and transient scripts were removed.  This is L2
evidence that the input schedule is meaningful in the reference, not a local
guest-state or gameplay claim. The local and external schedules differ, so this
is not a controlled cross-emulator comparison. See [S24 evidence]
(../etc/evidence/t7-s24-mame-input-pulse.md).

## M1 T7 S23 Closure

S23 is closed. The existing Machine-driver Coin/Start pulse completed the
full bounded 20M instructions on x64 and x86 with byte-identical neutral
output, then all staging, logs and temporary OS tasks were removed. The result
preserves the unresolved distinction between electrical ingress and guest
acceptance. See [S23 evidence](../etc/evidence/t7-s23-full-budget-machine-pulse.md).

## M1 T7 S22 Closure

S22 is closed. The indexed reconciliation maps every frozen G01--G12
checkpoint to current evidence without inventing a title, credit or gameplay
state. It confirms that KVM Coin/Start electrical ingress is already proven
through the Machine driver, while guest acceptance and title/attract remain
blocked by a missing finite COPX-D2 Board contract. See [S22 evidence]
(../etc/evidence/t7-s22-qualification-reconciliation.md).

## M1 T7 S21 Closure

S21 is closed. The selected-set held/pulse diagnostic now injects KVM `5`/`1`
events through the existing MyRaidenII Machine driver, while retaining its one
existing Core run loop. Both host widths produced identical bounded L2 results
at the default 2M containment cap; all twelve MyRaidenII unit tests passed per
width and the external staging root is clean. This proves neither Window/RDP
focus nor title acceptance, gameplay, graphics fidelity or audible output. See
[S21 evidence](../etc/evidence/t7-s21-machine-input-pulse.md).

## M1 T7 S20 Closure

S20 is closed. Product composition retains one driver route, and the existing
Machine driver consumes ordinary KVM `5`/`1` events into Coin 1/Start 1 Board
controls. The selected ROM Product remained alive for ten seconds on x64 and
x86, with staging removed each time. This does not prove Window focus, title
acceptance or gameplay. See [S20 evidence](../etc/evidence/t7-s20-product-input-route.md).

## Archived Packet — M1 T7 S19

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; S19 follows closed S18. One session performs coordinator and executor roles. |
| Admission And Approval | The owner directed continued MyRaidenII implementation under the L3/L2/L1 rule and delegated ordinary same-T packet planning. |
| Objective | Replace the existing held Coin/Start-only selected-set diagnostic with one bounded scheduled press/release diagnostic, so the real Board input route can be observed without treating a continuously asserted electrical input as an accepted gameplay action. |
| Non-goals | No Core, Machine, Product, Shared, ROM/PAL, source/table import, longer instruction cap, host-window input, sound-fidelity, title, gameplay or release claim. |
| Reference Baseline | S4 only asserted Coin 1 and Start 1 before execution and held them through the fixed budget. S18 now proves Board-to-Sound IM0 transport acceptance, but command bytes and copied PCM remain zero. The Board already has a public active-low `set_control()` API. |
| Candidate Proposal | Add one mutually exclusive Probe mode that presses Coin 1 and Start 1 at the first quarter of the unchanged budget, releases them at the second quarter, and reports the named schedule alongside existing neutral output. Reuse the existing run loop and Board control API; do not create a guest-input path, scheduler, trace, or state mirror. |
| Files And ABI Surface | Test-only `selected_set_startup_probe.c`, its wrapper option, Current and one evidence record. No production ABI, CMake, manifest, asset or artifact change. |
| Applicable Rules | MyRaidenII guide, Architecture, Coding, UI and Source Policy; shared Execution, Architecture, Coding and Documentation rules. The scheduled selected-set observation is L2 runtime evidence; the budget is L1 containment only. |
| Verification | Extend existing argument validation and synthetic Probe argument coverage if present; build the existing Probe x64/x86; run it once per width with the unchanged 20M cap and scheduled mode; run complete MyRaidenII units x64/x86, documentation governance and whitespace checks. |
| Expected Markers | Both widths produce the same finite output and report exactly one press/release schedule. Any visual, command or PCM change is recorded as L2 observation only; unchanged output remains a valid result. |
| Asset Needs | Read-only owner archive only for the final bounded Probe; wrapper-owned staging remains transient and is removed on exit. |
| Reporting Requirements | Record schedule, exact host-width output comparison, code/test delta, cleanup and the distinction between an electrical-edge diagnostic and actual Window/RDP/gameplay input qualification. |
| Stop Conditions | Any need for guest-state inspection, raw trace/payload retention, a new input API, host automation, source/table import, timing-grade escalation or production behavior change stops and transfers the gap. |
| Exit Criteria | The existing Probe has one explicit finite edge mode, its parser rejects incompatible modes, both widths are compared under the existing cap, evidence records the result and all named checks pass. |
| Original Owner Request | Only an explicit directly applicable formula, constant or rule in an inspected primary manual PDF is L3; external/range/reference behavior is L2; otherwise it is L1. |
| Similar-Issue Sweep | Inspect all existing selected-set Probe input options and wrapper forwarding for duplicated control setup, incompatible combinations or held-state assumptions. |

## M1 T7 S19 Closure

S19 is closed. The existing Probe now has one bounded Coin/Start pulse mode;
its parser rejects mutually exclusive input modes. Its x64/x86 selected-set
runs apply the same press/release schedule and yield identical neutral frame,
Sound-command and PCM summaries. This proves only the existing Board input
route's finite deterministic edge handling, not title acceptance, host input
delivery, gameplay or release. See [S19 evidence](../etc/evidence/t7-s19-input-pulse-diagnostic.md).

## Archived Packet — M1 T7 S18

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; S18 follows closed S17. One session performs coordinator and executor roles. |
| Admission And Approval | The owner directed continued L2 implementation from permitted external-emulator reference behavior when no directly applicable primary-manual-PDF rule is available. |
| Objective | Replace the local selected-Sound IM1-only interrupt assumption with one bounded L2 Seibu-style IM0 arbitration model, using the existing Board latch and YM pending source. |
| Non-goals | No Shared change, imported/transliterated source/table, primary-manual claim, second interrupt loop, second clock, thread, Sound synthesis, ROM/PAL retention, audio-success or gameplay claim. |
| Reference Baseline | S17 proves the local Board source reaches the local transport but the selected ROM retains it. The inspected MAME Raiden II configuration routes the audio CPU through `im0_vector_cb`; inspected Seibu Sound behavior selects RST18 (`0xdf`) before RST10 (`0xd7`), assigns main-CPU request to RST18 and FM/YM to RST10. This is L2 external behavior only. |
| Candidate Proposal | Keep one local interrupt acceptance point. In IM0, choose Board pending first and enter `0x18`; otherwise choose YM pending and enter `0x10`. Board source clears at its accepted RST18 transition; guest acknowledge addresses end service only and no longer conflate all sources with Board latch clear. IM1 is deliberately outside this selected-Sound L2 model. |
| Files And ABI Surface | Local MyRaidenII Core only: `sound.h/c`, `board.c`, existing `sound_smoke.c`, Current and one evidence record. The local transport renames its Board-clear callback to accepted-source semantics; no Shared/public product interface changes. |
| Applicable Rules | MyRaidenII guide, Architecture, Coding, UI and Source Policy; shared Execution, Architecture, Coding and Documentation rules. The vector/source/priority model is L2 and is not promoted by a passing ROM run. |
| Verification | Update existing Sound smoke: IM0 Board RST18, IM0 YM RST10, Board-over-YM priority and Board clear at accepted delivery. Build/test MyRaidenII x64/x86, run the bounded selected-set Probe on both widths, documentation governance and whitespace checks. |
| Expected Markers | A selected-set run may clear the Board latch or change PCM, but either observation is L2 runtime evidence only; unchanged output remains a valid finite disposition. |
| Asset Needs | Read-only owner archive only for the final bounded Probe; wrapper-owned staging is removed on exit. |
| Reporting Requirements | Pin the external revision/path and facts without retaining its source. Report exact local model and selected-set observation separately from audible/gameplay evidence. |
| Stop Conditions | Any need for undocumented third source, guessed opcode/vector, source/table import, raw ROM disassembly/logging, additional execution loop or L3 claim stops this packet. |
| Exit Criteria | One Sound interrupt acceptance owner implements the stated L2 IM0 source/priority model; existing and new unit coverage passes in both widths; bounded Probe is compared on both widths; evidence retains limits. |
| Original Owner Request | Only an explicit directly applicable formula, constant or rule in an inspected primary manual PDF is L3; external/range/reference behavior is L2; otherwise it is L1. |
| Similar-Issue Sweep | Review every Sound interrupt address for source-clear versus service-end semantics and confirm no Board latch is cleared by unrelated YM/RST10 activity. |

## M1 T7 S18 Closure

S18 is closed. The one local Sound interrupt acceptance point now uses the
bounded L2 IM0 source model: Board RST18 has priority over YM RST10, and the
Board latch clears only once its transition is accepted. Existing Sound and
all MyRaidenII unit tests pass on both widths. The bounded selected-set Probe
observes the Board latch cleared at all four checkpoints identically on x64
and x86; copied PCM remains zero, so this does not claim audible output or
gameplay. See [S18 evidence](../etc/evidence/t7-s18-sound-im0-transport.md).

## Archived Packet — M1 T7 S17

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; S17 follows closed S16. One session performs coordinator and executor roles. |
| Admission And Approval | The owner directed continued work under the L3/L2/L1 evidence rule and permits L2 external-emulator behavior reference without source/table import. |
| Objective | Connect the existing Board Sound-interrupt-pending latch to the existing local Z80 IM1 interrupt arbitration through its single Board-to-Sound transport contract. |
| Non-goals | No Shared change, new thread, clock, executor, audio renderer, YM/OKI behavior, ROM/PAL import or retention, source import, timing-grade promotion, audio-success or gameplay claim. |
| Reference Baseline | S16 observes the Board interrupt latch pending through all four bounded selected-set checkpoints while local Sound accepts only YM timer pending. The inspected external sound-map reference is L2 only; the local Z80 IM1 vector and existing acknowledge map are already-owned code. |
| Candidate Proposal | Add one read-only `interrupt_is_pending` transport callback. Board supplies its existing latch. The existing Sound interrupt gate accepts IM1 when either the existing YM source or the Board latch is pending; the existing guest-side acknowledge writes remain the only clear owner. |
| Files And ABI Surface | Local MyRaidenII Core only: `sound.h/c`, `board.c`, existing `sound_smoke.c`, Current and one evidence record. The local transport structure expands; no Shared/public product API changes. |
| Applicable Rules | MyRaidenII guide, Architecture, Coding, UI and Source Policy; shared Execution, Architecture, Coding and Documentation rules. The Board-to-Sound union is L2 unless a directly applicable explicit primary-manual-PDF fact is inspected. |
| Verification | Extend the existing ROM-free Sound smoke to prove Board-pending IM1 entry, guest acknowledge clear and no interrupt without a source; retain YM interrupt coverage. Build/test MyRaidenII x64/x86, run documentation governance and whitespace checks; rerun the bounded selected-set Probe if local Sound behavior changes successfully. |
| Expected Markers | The synthetic Z80 reaches its existing IM1 vector for a pending Board source and clears that source only by writing its existing acknowledge register. |
| Asset Needs | None for unit work. The existing owner archive is read only only if the final bounded Probe is run; wrapper staging remains transient and removed on exit. |
| Reporting Requirements | Report L2 evidence separately from local deterministic unit behavior. State whether selected-set PCM changes; neither result proves audible sound. |
| Stop Conditions | Any need for a second interrupt path, guessed vector/mode beyond existing IM1 behavior, synthesized audio, external source/table import, protected payload logging or L3 claim stops this packet. |
| Exit Criteria | One transport source owner, no automatic latch clear, existing YM coverage retained, Board-source regression passes on both widths, all touched checks pass and evidence records exact L2 limit. |
| Original Owner Request | Only an explicit directly applicable formula, constant or rule in an inspected primary manual PDF is L3; external/range/reference behavior is L2; otherwise it is L1. |
| Similar-Issue Sweep | Audit all transport callbacks for read-only query versus guest-owned acknowledge, and all interrupt acceptance failure paths for propagated status rather than silent source loss. |

## M1 T7 S17 Closure

S17 is closed. The local Board Sound latch now reaches the existing Sound CPU
IM1 gate through its one transport contract, while guest acknowledge remains
the only clear owner. Existing YM interrupt coverage and the new Board-source
regression pass on both widths. The bounded selected ROM still retains its
pending latch and zero-nonzero PCM summary, identically on x64/x86; therefore
the actual mode/vector/acknowledgement receiver remains open as L2 work rather
than being guessed. See [S17 evidence](../etc/evidence/t7-s17-sound-im1-transport.md).
Coordinator actual-change review accepted the local Core-only contract expansion:
no Shared component, external asset, new execution path or release artifact changed.

## Archived Packet — M1 T7 S16

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; S16 follows closed S15. One session performs coordinator and executor roles. |
| Admission And Approval | The owner directed continued MyRaidenII work under the stated L3/L2/L1 evidence rule. This packet is a bounded T7 runtime qualification diagnostic. |
| Objective | Identify whether the selected-set main CPU reaches the existing Board-to-Sound command transport during the same bounded startup observation that produces advancing frames but zero copied PCM. |
| Non-goals | No Sound synthesis, YM/OKI model, ROM/PAL import or retention, production behavior change, public API expansion, execution-budget increase, source import, screenshot/trace, Shared change, release artifact or audio-success claim. |
| Reference Baseline | S15 establishes identical x64/x86 frame progression at the existing 20M cap while its one final PCM copy has zero nonzero frames. Board already exposes non-destructive command-byte and interrupt-pending queries. |
| Candidate Proposal | Extend the existing four Probe samples with the two current Board sound-command bytes and the Board sound-interrupt-pending state. The same single helper takes the frame and Sound transport snapshot after each fixed sample. No query acknowledges, clears or consumes any guest state. |
| Files And ABI Surface | Expected only `test/myraidenii/integration/selected_set_startup_probe.c`, Current and one evidence record; no public/production ABI or manifest owner changes. |
| Applicable Rules | MyRaidenII guide, Architecture, Coding, UI and Source Policy; shared Execution, Architecture, Coding and Documentation rules. Observed selected-set behavior is L2; no result is L3 without an explicit inspected primary-manual PDF fact. |
| Verification | Build the existing Probe on x64/x86; run the existing external staging wrapper once per width at the unchanged cap; require identical finite sample state and no terminal failure; run MyRaidenII units x64/x86, documentation governance and whitespace checks. |
| Expected Markers | A finite result distinguishes no main-to-Sound transport activity from active/pending transport. It may select a later Sound receiver, but cannot itself establish audio fidelity. |
| Asset Needs | Read-only owner archive `O:\assets\raidenii\raiden2.zip`; wrapper-owned transient staging only, removed on exit. |
| Reporting Requirements | Record only command bytes, pending state, existing neutral summaries, host-width comparison and cleanup. Preserve the distinction between transport activity, PCM output and audible sound. |
| Stop Conditions | Any need to alter guest state, acknowledge a command, inspect/dump protected payload, retain a trace, alter production Sound semantics, use MAME/source behavior, or make a gameplay/audio claim stops this packet and transfers the receiver. |
| Exit Criteria | The existing Probe exposes one non-destructive Sound transport sample per fixed frame sample, both widths are compared under the existing budget, all touched checks pass and evidence identifies the next owner without an unsupported implementation. |
| Original Owner Request | Continue with strict evidence: only an explicit directly applicable rule/constant/formula in an inspected primary manual PDF is L3; external/range/board observations are L2; unsupported borrowing is L1. |
| Similar-Issue Sweep | Verify all Board sound queries are read-only, return failures consistently and do not accidentally acknowledge/clear the pending transport; retain the single final destructive PCM copy. |

## M1 T7 S16 Closure

S16 is closed. The bounded selected-set Probe now records existing Board Sound
command bytes and the existing Sound-interrupt latch at S15's four fixed frame
samples. Both host widths observed the same pending interrupt throughout the
unchanged 20M budget; the prior zero-nonzero PCM result remains. This is L2
runtime evidence only. It transfers a finite Board-to-Sound interrupt-arbitration
receiver to a later S; it neither qualifies audio nor introduces production
behavior. See [S16 evidence](../etc/evidence/t7-s16-sound-transport-observation.md).
Coordinator actual-change review accepted the test-only Probe extension: no
production ABI, artifact, protected-input retention or Shared component changed.

## M1 T7 S15 Closure

S15 is closed.  The existing selected-set Probe now samples four non-destructive
frame checkpoints within S9's unchanged finite instruction budget.  Both
widths have the same three observed frame transitions and no CPU, COP or
Board-control failure.  This is L2 runtime progress evidence only; it does not
qualify title/attract, input, sound, blend fidelity, COPX-D2 or gameplay.  See
[S15 evidence](../etc/evidence/t7-s15-progress-sampling.md).
Coordinator actual-change review accepted S15 P1 `01598b1ce`: the only code
path is the existing integration Probe (`+67/-10` source lines); its added
sampling has no production ABI, artifact or protected-input effect.

## M1 T7 S14 Closure

S14 is closed with no source, test, ABI, asset or artifact change. The complete
external L2 Board-map reconciliation found no unowned finite local route: the
existing COP, Video, sprite, sound, input, bank and memory owners cover every
selected Raiden II address class. COPX-D2 and sprite blend remain explicit
non-finite receivers rather than guessed behavior. See [S14 evidence](../etc/evidence/t7-s14-board-map-reconciliation.md).

## Archived Packet — M1 T7 S14

S14 performs one bounded L2 Board address-space reconciliation.  It compares
the locally implemented V30 Board routes with the inspected external Raiden II
reference map, classifies every address-class difference, and admits no source
change unless the difference has a finite independently implementable receiver
contract.  An external map is L2 context only: it cannot promote a constant,
formula, timing value or behavior to L3, and no external implementation,
table, ROM or PAL data may be copied.

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; S14 follows closed S13. One session performs coordinator and executor roles. |
| Admission And Approval | The owner directed continued MyRaidenII work and fixed the evidence rule: only an inspected primary-manual PDF's explicit directly applicable formula, constant or rule is L3; a range, board observation or external reference is L2; unsupported borrowing is L1. |
| Objective | Reconcile the complete finite V30 Board address map against the local Board routes, identify any real finite receiver omission, and preserve the two known non-finite receivers (COPX-D2 and sprite blend) without guessing behavior. |
| Non-goals | No external source/table/data import, ROM/PAL operation, generic board framework, guest behavior claim, raster retune, COPX attachment, sprite blend implementation, or release artifact. |
| Reference Baseline | Local `myraidenii_core_board` owns the selected V30 memory and device routes. The inspected external Raiden II map is a permitted L2 address-ownership reference, not a primary hardware manual or behavioral specification. |
| Candidate Proposal | Enumerate the external map's finite route classes, inspect the corresponding local Board reader/writer paths and tests, then classify each as equivalent, intentionally unimplemented with an existing owner, or a newly finite receiver. Only the last case transfers to a later implementation S with its own bounded contract. |
| Files And ABI Surface | Expected `docs/myraidenii/states/CURRENT.md`, one evidence record and its index only. No source/test/build/API/asset/artifact change is admitted by this audit packet. |
| Applicable Rules | MyRaidenII guide, Architecture, Coding and Source Policy; shared Execution and Documentation rules. Evidence grades follow the owner-defined L3/L2/L1 rule exactly. |
| Verification | Inspect local Board routes and unit coverage; inspect the pinned external map only as L2; reconcile ranges/widths/read-write direction; run documentation governance, whitespace and diff review. |
| Expected Markers | A complete disposition table with no undocumented map-class difference; either no finite omission or a named later receiver with owner and boundary. |
| Asset Needs | None. No payload is downloaded, extracted, logged or retained. |
| Reporting Requirements | State every unresolved route class, its grade and reason. Explicitly distinguish address ownership from operation semantics and avoid any L3 claim from the external reference. |
| Stop Conditions | Any need to infer a formula/constant/semantics from the map, copy/transliterate external code or tables, attach COPX without request/index/result semantics, or alter production behavior stops this S and transfers the receiver. |
| Exit Criteria | Every finite map class is reconciled, evidence grades are correct, any real receiver is explicitly transferred, and governance checks pass. |
| Original Owner Request | Continue under the strict evidence definition: direct explicit primary-manual-PDF constants/formulas/rules are L3; external/range/board evidence is L2; unsupported borrowing is L1. |
| Similar-Issue Sweep | Inspect both read and write paths, byte/word lane handling, local no-op/failure gaps, and existing Board unit tests rather than comparing only address labels. |

## M1 T7 S13 Closure

S13 is closed with no source, test, ABI, asset or artifact change.  The
external FPGA reference independently confirms the existing L2 cadence after
clock-domain conversion: its 8 MHz pixel clock and its 64 MHz, 4096-cycle
renderer line are both the local 64-us line interval.  The local 16 MHz Board
axis must therefore retain 1024 ticks/line.  This is not L3 hardware timing,
nor a sprite/COPX/gameplay qualification.  See [S13 evidence](../etc/evidence/t7-s13-raster-cadence-reconciliation.md).

## Archived Packet — M1 T7 S13

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; S13 follows closed S12. One session performs coordinator and executor roles. |
| Admission And Approval | The owner directed continued MyRaidenII work and reaffirmed the L3/L2/L1 rule: only an inspected primary-manual PDF's directly applicable formula, constant or rule is L3; a range, board observation or external reference is L2; an unsupported local approximation is L1. |
| Objective | Reconcile the existing local video-raster cadence against an independently inspected external L2 hardware implementation, and determine whether a concrete timing repair is warranted without importing source, tables or protected content. |
| Non-goals | No third-party code/table import, no ROM/PAL acquisition or retention, no sprite-blend implementation, no COPX-D2 attachment, no new timer/worker/frame path, no release claim and no change merely to match an external implementation's internal clock labels. |
| Reference Baseline | Local Video advances a 16 MHz nominal Board axis by 1024 ticks per 512-pixel line, for 282 lines/frame and VBlank after 240 lines. The inspected external FPGA reference reports an 8 MHz pixel clock, 512 pixels/line, 282 lines/frame and separately counts 4096 cycles/line on its 64 MHz internal renderer clock. |
| Candidate Proposal | Independently calculate each source's line/frame duration and only modify local timing if their physical-duration contracts disagree. Record the exact grade and keep the current implementation unchanged when they agree. |
| Files And ABI Surface | Expected only `docs/myraidenii/states/CURRENT.md`, `docs/myraidenii/etc/evidence/` and its index. No source/test/build/API/asset/artifact change is admitted. |
| Applicable Rules | MyRaidenII guide, Architecture, Coding, UI and Source Policy; shared Execution and Documentation rules. L3/L2/L1 labels must match the owner's definition; source policy forbids external code/table/data import. |
| Verification | Inspect the current Video constants and external published clock statements; calculate line/frame/VBlank duration independently; run documentation governance, whitespace and diff review. |
| Expected Markers | A finite conclusion that the local cadence either differs with a named owner or already represents the same duration through a different clock axis. No unsupported timing-grade promotion. |
| Asset Needs | None. External web/source inspection is read-only; no external payload is downloaded or retained. |
| Reporting Requirements | State the two clock-domain calculations, evidence grade and whether code changed. Explicitly distinguish the external internal renderer clock from Board timing. |
| Stop Conditions | Any need to copy/transliterate external HDL/code/table, claim a primary-manual fact, change blend/COPX semantics, or infer a guest checkpoint stops this S and transfers the receiver. |
| Exit Criteria | The cadence relation is documented with exact arithmetic and grade; any justified repair has an identified owner, otherwise no production code changes; governance checks pass. |
| Original Owner Request | Continue under the rule that only a direct explicit primary-manual-PDF formula/constant/rule is L3, external/range evidence is L2, and unsupported borrowing is L1. |
| Similar-Issue Sweep | Inspect existing local raster comments/constants and the external reference's separately stated pixel, line, frame and internal-clock values; reject any conflation of clock domains. |

## M1 T7 S12 Closure

S12 is closed with no source change. Current design and source-map authorities
now name the existing Core, Machine and Product implementations without
promoting their bounded evidence to gameplay or release qualification. The
active T7 receivers, including COPX-D2 and sprite blend fidelity, remain open.
See [S12 evidence](../etc/evidence/t7-s12-current-authority-reconciliation.md).

## Archived Packet — M1 T7 S12

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; S12 follows closed S11. One session performs coordinator and executor roles. |
| Admission And Approval | The owner directed continued MyRaidenII work under the established L3/L2/L1 evidence definition. This packet corrects current-authority documentation only; it does not upgrade any evidence grade or runtime qualification. |
| Objective | Reconcile current MyRaidenII design/source-map documentation with the existing Product -> Machine -> Core implementation, while retaining the active T7 gameplay/release gaps exactly as unqualified. |
| Non-goals | No production/test/build/API/asset change, no historical-record rewrite, no executable rebuild, no ROM/PAL operation, no claim that composition, bounded startup or a native frame proves gameplay. |
| Reference Baseline | `src/myraidenii/CMakeLists.txt` owns active Core, Machine and Product subcomponents; their corresponding unit roots are registered. Current design/source-map text still describes those paths as future or unimplemented, which can misdirect later qualification work. |
| Candidate Proposal | Correct only present-tense architecture, coding, roadmap and source-root statements. Preserve historical proposal records and explicitly distinguish implemented finite Board/Video/Sound/Product mechanisms from pending external-ROM gameplay, COPX and blend-fidelity qualification. |
| Files And ABI Surface | Expected only `docs/myraidenii/design/`, `docs/myraidenii/states/CURRENT.md`, `src/myraidenii/README.md` and one evidence record. No source ABI or artifact changes. |
| Applicable Rules | MyRaidenII guide, Architecture, Coding, UI and Source Policy; shared Documentation and Execution rules. Current design documentation remains authority; history remains evidence. |
| Verification | Compare each amended statement against current source/CMake ownership and existing closure records; run documentation governance, whitespace and diff review. |
| Expected Markers | No current authority says Product/configuration/controls or the local Board Video/Sound path is absent; no current authority calls T7's required gameplay/release evidence complete. |
| Asset Needs | None. |
| Reporting Requirements | List corrected current statements and retained qualification gaps. Do not quote or retain protected content. |
| Stop Conditions | Any required source change, historical correction, Shared change, evidence-grade promotion or game-behavior claim stops this packet. |
| Exit Criteria | Current-source documentation agrees with the active source graph and current task state; governance checks pass; history is unchanged. |
| Original Owner Request | Continue implementation under the strict rule: only an inspected primary-manual PDF's direct formula/constant/rule is L3; external reference/runtime evidence is L2; unsupported local behavior is L1. |
| Similar-Issue Sweep | Search current design/source-map authorities for stale future/unimplemented wording and retain it only when it names a real open receiver. |

## M1 T7 S11 Closure

S11 is closed with no source change.  The single bounded external MAME
reference launch stopped before game execution because the owner archive lacks
two MAME-required binary PAL dumps.  The archive's JED is not a substitute and
was not used as one.  This L2 asset prerequisite neither proves a local defect
nor changes the selected Board contract; its owned external output was removed.
See [S11 evidence](../etc/evidence/t7-s11-mame-reference-prerequisite.md).

## Archived Packet — M1 T7 S11

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; S11 follows closed S10. One session performs coordinator and executor roles. |
| Admission And Approval | The owner directed continued MyRaidenII implementation and restated the evidence rule: only a directly applicable explicit formula, constant or rule from an inspected primary manual PDF is L3; external implementations, board observations and ranges are L2; unsupported local choices are L1. The owner permits MAME as an external L2 runtime reference only. |
| Objective | Run the owner-supplied selected archive once through the already-present external MAME binary in a finite isolated observation, to distinguish a local early-startup limitation from a reference startup/environment result without importing code or protected payload. |
| Non-goals | No MAME code/table/data import, no ROM extraction retained in the repository, no source-model change, no claimed gameplay qualification, no screenshot/audio dump, no Shared change and no reference-run dependency. |
| Reference Baseline | The local selected-set Probe reaches its bounded 20M-instruction budget without a terminal CPU/COP/control error but cannot label a guest checkpoint. MAME's current Raiden II driver is an allowed L2 external reference, not an L3 hardware authority. |
| Candidate Proposal | Launch the existing external MAME executable with `O:\\assets\\raidenii\\raiden2.zip` available only through its ROM search path; direct all MAME configuration, NVRAM, snapshots and logs to a fresh owner-managed directory below `O:\\assets\\media-myraidenii\\`; use a finite wall-clock budget and remove generated data afterward. Record only neutral process/window/exit facts. |
| Files And ABI Surface | Expected only Current and one evidence record. No source, test, public API, build, executable or shared-component change is admitted. |
| Applicable Rules | MyRaidenII guide, Architecture, Coding, UI and Source Policy; shared Execution, Architecture, Coding and Document rules. Every reference-run observation is L2. |
| Verification | Verify the selected archive identity before launch; verify the MAME executable identity/version; make one bounded run; wait for owned process exit; confirm all generated output remained below the declared external root; remove it; run documentation governance, whitespace and diff checks. |
| Expected Markers | A finite MAME process result or concrete owned startup error, with no repository files or protected payload retained. The result may establish only that this external reference did or did not remain alive during the finite observation. |
| Asset Needs | Read-only `O:\\assets\\raidenii\\raiden2.zip`; owned temporary MAME output only below `O:\\assets\\media-myraidenii\\t7-s11-mame\\`, removed before closure. |
| Reporting Requirements | State exact external executable/version, archive identity, elapsed budget and process result; do not log ROM bytes, infer a hardware timing fact, or conflate an external result with local qualification. |
| Stop Conditions | Any need to copy/provide MAME source, retain a ROM/config/snapshot outside the owned external root, alter local production behavior, add an unbounded trace, or claim title/gameplay from process survival stops this packet. |
| Exit Criteria | One finite reference observation is recorded or a concrete owned prerequisite failure is documented; all generated external data is removed and no runnable source claim is changed. |
| Original Owner Request | Continue under the strict L3/L2/L1 definition: only an inspected primary manual PDF's explicit applicable formula/constant/rule is L3; external references, ranges and runtime observations are L2; unsupported local borrowing is L1. |
| Similar-Issue Sweep | Check that configuration/log/snapshot paths are all external and owned, that no reference result enters source behavior, and that archive contents are never printed or retained. |

## M1 T7 S10 Closure

S10 is closed with no source change.  Video has one existing sprite-flags owner
and no hidden blend path, but the external L2 reference selects its half-alpha
operation through a non-importable reference colour table.  Without a Board
selection contract or non-ROM oracle, a local implementation would be L1
guesswork; the fidelity receiver remains explicitly deferred.  See [S10 evidence](../etc/evidence/t7-s10-sprite-blend-contract-review.md).

## M1 T7 S9 Closure

S9 is closed without a source change.  The existing staged Probe completed its
maximum 20M-instruction held-control budget identically on x64 and x86, with
no CPU/COP/control terminal and no retained staging files.  This is L2
determinism evidence only, not a game-start or timing result; another longer
run would not select a finite receiver.  See [S9 evidence](../etc/evidence/t7-s9-maximum-budget-probe.md).

## M1 T7 S8 Closure

S8 is closed with no production change.  It repeated the COPX-D2 source
sweep under the owner-restated evidence grades and found only L2 identity/
board-observation facts, not a finite Board receiver contract.  The separate
image therefore remains deliberately unconfigured and deferred rather than
being represented by a misleading path-only input.  See [S8 evidence](../etc/evidence/t7-s8-copx-source-sweep.md).

## Archived Packet — M1 T7 S7

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; S7 follows closed S6. One session performs coordinator and executor roles. |
| Admission And Approval | The owner directed continued MyRaidenII implementation and established the strict evidence grades: only an inspected primary manual PDF's directly applicable formula, constant or rule is L3; external references/ranges/runtime observations are L2; unsupported local choices are L1. |
| Objective | Make the existing external selected-set probe's bounded instruction budget explicit and safely variable, then observe a longer identical selected-set run on both widths without converting elapsed diagnostic work into a guest-hardware timing claim. |
| Non-goals | No CPU/Board/Sound/Video model change, ROM import/retention, trace/screenshot/PCM dump, new input path, COPX behavior, playable-game claim or release artifact. |
| Reference Baseline | The current probe always executes a fixed two-million-instruction budget. Its aggregate output proves a nonfaulted bounded run, but cannot distinguish a later guest checkpoint from a missing receiver. |
| Candidate Proposal | Preserve the two-million default. Add one strictly parsed optional budget argument capped at a finite diagnostic maximum; have the existing staging wrapper pass it only in Probe mode. Output the requested budget with existing aggregate metadata. |
| Files And ABI Surface | Expected only the MyRaidenII integration probe, its existing staging tool, Current and one evidence record. No public API or Shared/Product/Machine/Core ABI changes. |
| Applicable Rules | MyRaidenII guide, Architecture, Coding and Source Policy; shared Execution, Architecture, Coding and Document rules. The budget cap is L1 containment policy. Runtime results are L2 and cannot establish physical timing or gameplay. |
| Verification | Build the existing probe for both widths; prove default compatibility and invalid-budget rejection; run exactly one longer owner-input probe per width through the existing verifier/staging route; run complete repository-only unit selection per width; verify cleanup, documents and diff. |
| Expected Markers | Default output retains two-million budget; invalid input is rejected before image loading; both long runs report the same requested/executed budget or a concrete owned terminal result; staging remains empty. |
| Asset Needs | Read-only `O:\\assets\\raidenii\\raiden2.zip`; temporary files remain only below `O:\\assets\\raidenii\\myraidenii-diagnostics\\` and are removed. |
| Reporting Requirements | Separate the L1 diagnostic budget choice from L2 runtime observations; report both widths, terminal reason, aggregate frame/PCM metadata and all nonclaims. |
| Stop Conditions | A need for unbounded execution/trace, retained payload, a guest behavior change, Shared change, inferred timing constant or unsupported production receiver stops this packet. |
| Exit Criteria | The probe remains one bounded diagnostic with default compatibility; both widths complete the one longer observation or expose a concrete owned terminal; cleanup and applicable tests pass. |
| Original Owner Request | Continue under the strict L3/L2/L1 definition: only an inspected primary manual PDF's explicit applicable formula/constant/rule is L3; external references, ranges and runtime observations are L2; unsupported borrowing is L1. |
| Similar-Issue Sweep | Check every external diagnostic argument is bounded and parsed once, staging still validates before extraction, and no changed output exposes protected content. |

## M1 T7 S7 Closure

S7 is closed.  The one selected-set Probe now accepts an explicit bounded
instruction budget while retaining its two-million default and one execution
route.  A five-million held-control observation completed with identical x64
and x86 aggregates and no terminal CPU, COP or control fault.  The budget is
L1 containment policy and the run is L2 only; neither is a game/hardware timing
or gameplay claim.  Existing complete repository unit selections from S6 remain
valid because S7 changes only the opt-in integration Probe and its wrapper; the
affected Probe was rebuilt on both widths and invalid-input handling was
executed on both.  See [S7 evidence](../etc/evidence/t7-s7-bounded-budget.md).

## Archived Packet — M1 T7 S6

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; S6 follows closed S5. One session performs coordinator and executor roles. |
| Admission And Approval | The owner directed continuation and clarified the evidence-grade rule: only an inspected primary manual PDF with a directly applicable explicit formula, constant or rule is L3; ranges, external emulators and runtime observations are L2; unsupported local approximations are L1. |
| Objective | Qualify the already implemented Product composition through a bounded, external selected-set Window launch, then determine whether the existing KVM input path can be observed without inventing a gameplay or hardware claim. |
| Non-goals | No ROM import/embedding, external-source import, COPX model, protection bypass, Shared change, synthetic title/gameplay success, release deployment or claim that a Window observation proves game correctness. |
| Reference Baseline | S1/S4 exercised the Core/Machine selected-set probe only; S2/S3 established a bounded Product launch and optional host PCM endpoint. Product remains Window-only and owns no alternative input route. |
| Candidate Proposal | Add only the minimum reproducible external staging/launch support needed to place the existing Product executable, its adjacent lowercase INI and the twelve already validated external inputs in one temporary owner-managed directory. Observe one bounded launch per width and, when a Window is available, use existing KVM input only; retain no payload and clean the stage. |
| Files And ABI Surface | Expected MyRaidenII tool/test/evidence/current files only. Product/Machine/Core/Shared ABI changes require a separately evidenced defect. |
| Applicable Rules | MyRaidenII guide, Architecture, Coding, UI and Source Policy; shared Execution, Architecture, Coding and Document rules. All runtime and external-reference conclusions are L2 unless an inspected primary manual PDF directly supplies the applicable rule. |
| Verification | Verify archive identity before staging; build existing Product once per width; check generated INI uses only adjacent relative filenames; bounded process/Window observation and owned-process cleanup per width; documentation/governance/whitespace/diff review. This is external integration evidence, not a CTest or release qualification. |
| Expected Markers | Product locates `myraidenii.ini`, validates all twelve images, remains alive through the bounded observation or yields a concrete owned failure; no stage remains after cleanup. Any visible input observation records only host-to-KVM delivery, not gameplay response. |
| Asset Needs | Read-only `O:\\assets\\raidenii\\raiden2.zip`; temporary input extraction only under `O:\\assets\\raidenii\\myraidenii-diagnostics\\`, removed after the owned process exits. |
| Reporting Requirements | Record commands, widths, staging cleanup, Product exit/Window disposition and exact evidence grade. Distinguish a UI/input observation from guest title, coin/start, sound or stage qualification. |
| Stop Conditions | A retained protected payload, a need to modify Shared contracts, a need for a guessed Board behavior, an unbounded process/trace, or a launch failure whose owner is outside Product/Machine stops this packet and records the receiver. |
| Exit Criteria | Both widths have a bounded real-Product launch result with safe cleanup; any observed input fact is bounded and accurately graded; no unsupported behavior or release claim is introduced. |
| Original Owner Request | Continue under the strict L3/L2/L1 definition: only an inspected primary manual PDF's explicit applicable formula/constant/rule is L3; external references, ranges and runtime observations are L2; unsupported borrowing is L1. |
| Similar-Issue Sweep | Verify all Product configuration paths use the adjacent INI/base directory, both widths use the same image-role transaction, and no diagnostic leaves ROM data, configs or child processes behind. |

## M1 T7 S6 Closure

S6 is closed.  The selected-set wrapper now owns both its established Probe
route and a bounded Product route without adding another Product loader.  Both
x64 and x86 real Product processes remained live for ten seconds from fresh
adjacent-INI stages and were then stopped and cleaned.  The external stage root
was empty after each run.  Complete repository-only unit selections passed
652/652 on each width; the temporary-junction build-cleanup selftest required
the host execution capability rather than the restricted sandbox.  This is L2
startup evidence only: the desktop automation surface did not expose the native
Product Window, so no visual, host-key, title, gameplay or sound claim is made.
See [S6 evidence](../etc/evidence/t7-s6-product-launch.md).

## Archived Packet — M1 T7 S5

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; S5 follows closed S4. One session performs coordinator and executor roles. |
| Admission And Approval | The owner instructed continued work under the strict L3/L2/L1 definition and expressly permits external emulator implementations as L2 references. The source policy still prohibits copying/transliterating MAME code, tables or ROM data. |
| Objective | Establish whether the selected set's separately inventoried COPX image has a concrete, independently implementable Board consumer contract, or record a precise deferral instead of adding a path-only image role. |
| Non-goals | No MAME source/data import, no ROM embedding/retention, no guessed lookup function, no configuration role with no Board consumer, no protection bypass, no Shared change, and no release/gameplay claim. |
| Reference Baseline | The archive inventory identifies `copx-d2.u0313` as a 262144-byte runtime image that current Machine configuration does not consume. S4 proves the twelve-image route has no immediate bounded control fault, but not that the COPX-dependent behavior is complete. |
| Candidate Proposal | Inspect the already available external L2 reference only to determine whether it supplies a finite COPX access contract. Cross-check our Board/COP address ownership and consumer paths. If no exact independently implementable contract exists, retain the image as a documented deferred receiver and add no inert configuration/API path. |
| Files And ABI Surface | Expected Current/evidence/TODO only. Any production/test/API change requires a concrete consumer contract and a new bounded implementation packet. |
| Applicable Rules | MyRaidenII guide, Architecture, Coding, UI and Source Policy; shared Execution, Architecture, Coding and Document rules. Only an inspected primary-manual PDF's explicit applicable rule/formula/constant may be L3. The external reference and observed ROM identity are L2; any unsupported local model would be L1. |
| Verification | Static ownership/call-path audit, external-reference provenance record without copying source/data, Current/TODO/documentation gates and diff review. No CTest or binary deployment is claimed for a research-only packet. |
| Expected Markers | Either one finite Board-owned consumer contract that can be independently implemented later, or a written L2 deferral explaining why the role must not yet be passed through Machine/Board. |
| Asset Needs | Read-only metadata already recorded for `O:\\assets\\raidenii\\raiden2.zip`; no extraction is needed. The external source reference remains outside the repository and is not a dependency. |
| Reporting Requirements | Separate the exact L2 facts from speculation, name every current COPX absence point, state why a path-only input would be false support, and place the concrete deferred receiver in TODO if no consumer is available. |
| Stop Conditions | A request to copy/transliterate external source or tables, retain external payload, claim L3 without a primary manual PDF, add a guessed model, or modify a Shared contract stops this packet. |
| Exit Criteria | COPX ownership is either tied to a finite later receiver or truthfully deferred; no inert code path or unsupported behavior enters the product. |
| Original Owner Request | Continue under the strict definition: only an inspected primary-manual PDF's explicit applicable formula/constant/rule is L3; an external reference, range or runtime observation is L2; unsupported local borrowing is L1. |
| Similar-Issue Sweep | Review every selected image role for a complete Machine-to-Board consumer and distinguish a verified input identity from an implemented hardware behavior. |

## M1 T7 S5 Closure

S5 is closed.  The current twelve-role Machine transaction and Board COP have
no hidden COPX consumer.  The inspected external L2 source confirms the
separate 262144-byte image identity but exposes no finite access contract and
describes its purpose tentatively.  The image therefore remains an explicit
High-priority Board/COP receiver in TODO rather than an inert configuration
role or guessed lookup model.  See
[S5 evidence](../etc/evidence/t7-s5-copx-receiver-review.md).

## Archived Packet — M1 T7 S4

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; S4 follows archived S3. One session performs coordinator and executor roles. |
| Admission And Approval | The owner instructed continuation under the strict L3/L2/L1 evidence definition and permanently approved ordinary in-scope MyRaidenII work. No Shared change, ROM import, external-source import or artifact deployment is admitted. |
| Objective | Exercise the already implemented Coin 1 and Start 1 Board-control path through the existing bounded selected-set startup probe, once per host width, and record whether it preserves the established non-faulted bounded result. |
| Non-goals | No claim of title/attract/gameplay/stage qualification; no automated host keyboard/window test; no new controller mapping, ROM reader, COPX integration, protection model, video/audio fidelity change, raw trace, screenshot or binary release. |
| Reference Baseline | S1 established the neutral twelve-image selected-set probe on both widths. The probe already owns an opt-in `--coin-start-held` path that calls the public Board controls before its bounded execution; Core and Machine units separately cover the control mapping. |
| Candidate Proposal | Reuse that exact probe and external staging wrapper with `--coin-start-held`. Compare its terminal reason, fault observation, control-failure observation and published frame/PCM metadata with S1, then delete staging. A successful bounded run establishes only that the current selected control path does not immediately fault this bounded external-input route. |
| Files And ABI Surface | Expected only Current and a new MyRaidenII evidence record. No production, test, public API, build, ROM/archive, shared-component or asset change. |
| Applicable Rules | MyRaidenII guide, Architecture, Coding, UI and Source Policy; shared Execution, Architecture, Coding and Document rules. This is L2 because it uses owner-supplied runtime input; only an inspected primary-manual PDF with an explicit applicable rule/formula/constant can establish L3. |
| Verification | Build the existing probe only if needed, verify the unchanged inventory once, run the bounded wrapper once on x64 and once on x86, verify staging cleanup, then perform documentation/whitespace/diff review. This is an opt-in diagnostic, not a CTest or release qualification. |
| Expected Markers | `coin-start-held=1`, no reported CPU/COP/control terminal fault, one bounded budget result per width and an empty external staging root after each run. |
| Asset Needs | Read-only `O:\\assets\\raidenii\\raiden2.zip`; temporary files only under `O:\\assets\\raidenii\\myraidenii-diagnostics\\` and removed by the existing wrapper. |
| Reporting Requirements | Preserve output without protected payload, label it L2, compare it to S1 without claiming equivalence to real user input or gameplay, and state the still-unresolved COPX ownership separately. |
| Stop Conditions | Identity failure, retained payload, a need to alter Board behavior/controls or external source, nonempty staging residue, or a terminal fault transfers the concrete receiver to a later packet. |
| Exit Criteria | Both widths complete the existing bounded control diagnostic, staging is clean, no source change is needed, and evidence truthfully states its narrow L2 scope. |
| Original Owner Request | Continue under the strict definition: only an inspected primary-manual PDF's explicit applicable formula/constant/rule is L3; an external reference, range or runtime observation is L2; unsupported local borrowing is L1. |
| Similar-Issue Sweep | Check that the wrapper stages only declared roles, control assertions are not mistaken for host input, external paths are bounded/read-only and cleanup remains complete. |

## Archived Packet — M1 T7 S3

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; S3 receives the precise S2 real-Product early-exit outcome. One session performs coordinator and executor roles. |
| Admission And Approval | The owner instructed continuation, previously required host mute/endpoint absence not to make testability depend on physical audio, and permanently approved ordinary in-scope MyRaidenII changes. No Shared change is admitted. |
| Objective | Make the MyRaidenII Machine's existing Board PCM bridge optional when Lib cannot create a host endpoint, while retaining the one Board PCM drain route and preventing endpoint absence from stopping the emulator. |
| Non-goals | No audio synthesizer, alternate mixer, endpoint retry/recovery framework, second queue/thread, Shared API, ROM/archive reader, release artifact, audio-fidelity claim or hardware timing change. |
| Reference Baseline | S2's verified external Product bundle exited before the ten-second observation. Product constructs Lib PCM before Machine; Machine and its Audio bridge currently require a non-null stream and unconditionally call stream methods. The Board already owns produced PCM; missing host output is a host capability condition, not a guest-board state change. |
| Candidate Proposal | Keep one Machine Audio bridge. Let it accept a null host stream and drain/discard Board PCM locally; all stream operations become conditional on the existing borrowed stream. Product treats only `IO_ERROR`/`UNSUPPORTED` from stream creation as the no-endpoint mode, while other creation failures remain fatal. No public function/type is added. |
| Files And ABI Surface | Expected only `src/myraidenii/product/`, `src/myraidenii/machine/`, their existing unit tests, Current and evidence. Existing signatures remain; their null-stream behavior is clarified and tested. No Lib/Emulator/X86/Core/asset/ROM/external source change. |
| Applicable Rules | MyRaidenII guide, Architecture, Coding, UI and Source Policy; shared Execution, Architecture, Coding and Document rules. This host-fallback policy is L1: no primary manual or external model establishes a Raiden II endpoint requirement. |
| Verification | Focused existing Machine audio/driver and Product composition tests plus complete MyRaidenII unit selection on x64/x86. Re-run the bounded external Product launch only after production tests pass, then inspect cleanup. |
| Expected Markers | Exactly one Audio bridge; null stream never reaches a Lib stream operation; Board PCM is still consumed; non-endpoint failures remain fatal; normal stream behavior is unchanged. |
| Asset Needs | The same read-only owner archive only for the final external launch repro. Temporary files stay beneath the external diagnostic root and are removed. |
| Reporting Requirements | Report the exact fallback statuses, paths that remain fatal, test deltas, actual launch disposition and L1 limitation. |
| Stop Conditions | Need for a Shared audio contract, retained external payload, second host audio route, retry service, or a host failure outside the two existing endpoint-unavailable statuses stops and transfers the gap. |
| Exit Criteria | Both unit widths prove the optional-output path and retained normal path; the external Product launch is rechecked; no duplicate audio path or host-dependent startup failure remains for the identified statuses. |
| Original Owner Request | Continue MyRaidenII under the strict L3/L2/L1 definition: only an inspected primary-manual PDF's explicit applicable formula/constant/rule is L3; range/reference facts are L2; unsupported borrowing is L1. |
| Similar-Issue Sweep | Check configuration filename/case, relative-path base, image role coverage, process ownership, temporary-root safety and cleanup; do not retain protected bytes in build/repository locations. |

## M1 T7 S4 Closure

S4 is closed.  The existing bounded selected-set probe accepted its direct
Coin 1/Start 1 request on x64 and x86 without a CPU, COP or unowned-control
terminal fault; both runs reached the normal two-million-instruction budget.
Its frame/PCM observations matched S1, and the external staging root was
empty after each run.  This is L2 evidence only: it demonstrates neither
host input delivery nor a playable credit/start sequence.  The separate COPX
image remains an explicit later Board-ownership receiver; no speculative ROM
input or lookup model was added.  See
[S4 evidence](../etc/evidence/t7-s4-selected-set-controls.md).

## M1 T7 S2 Closure

S2 verified the real Product path uses the adjacent lower-case INI and temporary
selected image bundle, but both widths exited before the ten-second observation
with code 1.  The bundle was removed in every case.  Product currently
collapses construction failure to the process result, so S2 does not claim the
specific internal status; its construction-order review identifies the
mandatory host PCM stream as the next bounded local receiver.  See S3 for the
repair and recheck; no external payload or release artifact was retained.

## Archived Packet — M1 T7 S1

| Field | Required record |
| --- | --- |
| Identifier Mode | New qualification task; S1 is its bounded external-input startup diagnostic. One session performs coordinator and executor roles. |
| Admission And Approval | The owner accepted continuation of MyRaidenII implementation, supplied the existing `O:\\assets\\raidenii\\raiden2.zip` input, and authorized external emulator behavior only as L2. The source policy permits bounded read-only selected-set diagnostics; no protected data may enter the repository. |
| Objective | Re-run the existing verified selected-set startup probe through the actual current Core/Machine graph, record its first terminal or bounded-budget outcome, and reconcile the result with the qualification ledger. |
| Non-goals | No ROM download/import/embedding, archive runtime reader, Product release INI, executable deployment, source derivation, trace/pixel/sample dump, game-completion claim, or evidence-grade promotion. |
| Reference Baseline | T1's checked inventory identifies the owner-supplied archive and its 12 runtime roles. T6 supplies the one configuration/Product path but has not claimed a runnable release. `Invoke-SelectedSetStartupProbe.ps1` already validates then stages a short-lived external input bundle for the existing diagnostic executable. |
| Candidate Proposal | Build the existing x64/x86 startup probe from the stable Ninja graph, run the existing inventory verifier and staging wrapper once per width against the unchanged owner archive, retain only its bounded neutral output/evidence, and clean the generated external staging directory. Treat any resulting fault as a receiver ledger fact, not an opportunity to invent a device model. |
| Files And ABI Surface | Expected only MyRaidenII Current/evidence/queue records unless diagnostic evidence identifies an already-approved bounded receiver. No production/test/API/build/asset/ROM/external source file change is admitted in S1. |
| Applicable Rules | MyRaidenII guide, Architecture, Coding, UI and Source Policy; shared Execution, Architecture, Coding and Document rules. L3 requires an inspected primary-manual PDF with an explicit applicable formula/constant/rule. External observations are L2; unsupported local behavior is L1. |
| Verification | Existing inventory verifier plus existing selected-set startup probe on x64/x86; source-policy review; staging cleanup verification; documentation, whitespace and diff review. This is an opt-in diagnostic, not a CTest result. |
| Expected Markers | One verified archive identity, twelve runtime image roles, temporary files only under the external diagnostic root, one bounded probe per width, no payload retained, and a precise terminal/budget disposition. |
| Asset Needs | Read-only owner-supplied `O:\\assets\\raidenii\\raiden2.zip`; generated staging/log output may exist only under `O:\\assets\\raidenii\\myraidenii-diagnostics\\` and must not become tracked data. |
| Reporting Requirements | Record command identities, archive identity check, bounded neutral results, cleanup state, grade limits, and whether a concrete existing receiver is eligible for a later S. |
| Stop Conditions | Archive identity failure, a request to retain protected payload, a need to modify Shared/external code, or a result requiring an unapproved Board/CPU model stops and transfers the gap. |
| Exit Criteria | Both widths complete the existing bounded diagnostic using verified external inputs; no protected payload or misleading release claim remains; the next receiver boundary is explicitly accepted, deferred, or refused. |
| Original Owner Request | Continue MyRaidenII under the strict grade definition: only an inspected primary-manual PDF's explicit applicable formula/constant/rule is L3; range/reference facts are L2; unsupported borrowing is L1. |
| Similar-Issue Sweep | Check every external path, archive handle, temporary extraction location and output record for payload retention, working-directory dependence, unbounded trace output, and false L3 language. |

## Archived Packet — M1 T6 S3

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; S3 follows accepted S2 commit `2c0796648`. One session performs coordinator and executor roles. |
| Admission And Approval | The owner accepted M1 Product work, delegated ordinary same-T S planning, and instructed continuation on 2026-10-20. No Shared change, protected input, third-party source import, asset deployment, or executable release is admitted. |
| Objective | Add the one real MyRaidenII process entry and local executable target that invoke the established Product configuration and composition path. |
| Non-goals | No INI template, ROM/archive opening, release artifact, deployment, snapshot/debug implementation, console graphics fallback, new lifecycle path, Shared/Core/Machine API or external-source import. |
| Reference Baseline | S1 supplies the one Product composition owner; S2 supplies its one monitor/hotkey policy. Configuration already resolves exactly the adjacent lowercase `myraidenii.ini`; Machine remains the one image transaction owner. |
| Candidate Proposal | Add a narrow `main.c` that loads the existing Product configuration and invokes the existing Product composition. Add one local executable CMake target linking the existing Product library. It creates no second configuration, ownership or runtime route and is not copied to Assets until a separately qualified external configuration/artifact packet. |
| Files And ABI Surface | Expected only `src/myraidenii/product/main.c`, its CMake declaration, Current and S evidence. No public API, Shared/Core/Machine/test/asset/ROM/external repository file change is expected. |
| Applicable Rules | MyRaidenII guide; Architecture, Coding, UI and Source Policy; shared Execution, Architecture, Coding and Document rules. Product -> Machine -> Core remains the only application direction. L3 requires an inspected primary-manual PDF with the explicit applicable fact; external evidence is L2; local policy is L1. |
| Verification | Configure/build the actual entry target on x64/x86, then rerun the complete MyRaidenII unit selection on both widths plus documentation/whitespace/diff review. No external-ROM, desktop, executable deployment or gameplay claim. |
| Expected Markers | One `main`, one configuration load, one Product run, one executable target and no asset-copy/post-build release command. |
| Asset Needs | None. The owner-supplied external selected-set archive remains unopened and read-only. The required adjacent INI and individual ROM files are not invented or committed. |
| Reporting Requirements | Report entry/control-flow ownership, build names, tests and explicit non-claims. |
| Stop Conditions | A need to parse/archive ROM inputs, add a release INI, deploy an artifact, change a Shared contract, alter Machine/Core, or claim successful guest startup stops and transfers the gap. |
| Exit Criteria | The build contains one real process entry into the existing Product route on both widths; it introduces no duplicate configuration/composition path and all scoped checks pass. |
| Original Owner Request | Continue MyRaidenII under the strict grade definition: L3 only for an inspected primary-manual PDF's explicit applicable formula/constant/rule; reference/range evidence is L2; unsupported local policy is L1. |
| Similar-Issue Sweep | Review MyRaidenII for duplicate `main`, configuration-load or composition-run paths and ensure the target does not deploy an executable, INI or protected payload. |

## Archived Packet — M1 T6 S1

| Field | Required record |
| --- | --- |
| Identifier Mode | New implementation task; T6 is allocated from the head Product candidate after T5 closed. S1 is the first bounded composition increment. One session performs coordinator and executor roles. |
| Admission And Approval | The owner accepted M1 Product work in the queue and instructed continuation on 2026-10-20. Ordinary in-scope S planning and implementation are delegated. No Shared change, protected payload import, or source-license exception is admitted. |
| Objective | Establish the one MyRaidenII Product-owned composition context that consumes the existing parsed configuration and Machine image transaction, constructs the existing Core Board and Machine driver, and supplies that one owned Machine to Emulator Product. |
| Non-goals | No second executor, Session provider, audio route, renderer, ZIP reader, ROM embedding, command grammar, snapshot/debug implementation, gameplay claim, App artifact, or Shared API change. |
| Reference Baseline | T5 S16 parses the fixed adjacent INI; T5 S17 materializes/validates the complete external image bundle; T5 closes the sole Board-to-Machine PCM bridge and one Emulator Machine driver. Emulator Product already owns composition, Session, UI creation and teardown. |
| Candidate Proposal | Add a cohesive Product composition owner and the minimum configuration-directory fact needed to pass the already parsed path set to Machine. It creates Board from the one complete image bundle, creates the existing Machine driver, copies its existing driver descriptor into `emulator_product_machine`, and lets `emulator_product_run()` own the neutral composition lifetime. Bind/unbind only establishes the required opaque Emulator Machine borrow. |
| Files And ABI Surface | Expected: `src/myraidenii/product/` and matching `test/myraidenii/unit/product/`, its CMake/manifest and MyRaidenII records. Product config may gain a copied INI-directory output required by the existing Machine transaction. Existing public Emulator, Machine and Core interfaces are consumed unchanged. |
| Applicable Rules | MyRaidenII guide; Architecture, Coding, UI and Source Policy; shared Execution, Architecture, Coding and Document rules. Product -> Machine -> Core remains the only dependency direction. L3 requires an inspected primary-manual PDF with the explicit applicable rule; reference/range facts are L2; unsupported local decisions are L1. |
| Verification | Before S1 closure: focused Product composition/configuration unit tests on x64/x86, complete MyRaidenII unit selection on x64/x86, manifest/whitespace/documentation gates, and actual diff/ownership review. No external-ROM desktop or EXE qualification is claimed by S1. |
| Expected Markers | One Product context owns images, Board and Machine until Emulator Product has completed ordered teardown; one existing driver descriptor; one bind/revoke link; failed construction leaves no candidate reachable; no Product-to-Core file route or second Session/worker. |
| Asset Needs | Existing owner-supplied `O:\assets\raidenii\raiden2.zip` remains read-only and unused by S1 unit work. No asset is copied, retained or committed. |
| Reporting Requirements | Record actual files/API delta, composition/cleanup order, source-grade classifications, test results, retained single paths and explicit non-claims. |
| Stop Conditions | A need for Shared change, archive decoding, payload retention, public Core file API, second execution/session route, protected source import, or evidence-grade escalation stops and transfers the gap. |
| Exit Criteria | The Product has one test-covered composition owner that can construct and cleanly release the existing Machine through Emulator Product without bypassing its lifecycle; all scoped checks pass and no obsolete/parallel route remains. |
| Original Owner Request | Continue MyRaidenII while applying the strict L3/L2/L1 definition: only an inspected primary-manual PDF's explicit applicable formula/constant/rule is L3; range-only/reference evidence is L2; unsupported borrowing is L1. |
| Similar-Issue Sweep | Review all current MyRaidenII Product/Machine construction and cleanup call sites for duplicated path resolution, duplicate resource ownership, direct Core file access, second driver creation, or raw platform/CRT vocabulary. |

## M1 T6 S1 Closure

S1 is closed.  The Product now has exactly one owned construction route from
the runtime configuration through Machine image validation, Board, PCM stream
and Machine driver into Emulator Product.  The matching composition smoke
proves normal bind/revoke/destruction plus image, Machine and configuration
failure boundaries without a test-only production route.  The focused Product
and complete MyRaidenII unit selections pass 12/12 on x64 and x86; the
documentation and whitespace gates pass.  See
[S1 evidence](../etc/evidence/t6-s1-product-composition.md).

T6 remains active.  Later bounded S packets receive the real App entry,
input/hotkey policy and release qualification; they must consume this owner
rather than construct another Board, Machine, Session or PCM path.

## M1 T6 S2 Closure

S2 is closed.  Product now has exactly one generic monitor provider and one
Escape pause-toggle route through the existing Emulator Session/lifecycle
contracts.  Its seven structured help rows describe the existing Machine input
policy without creating a second guest-input or command path.  Fixed snapshot
and debug commands remain honestly unsupported.  Complete MyRaidenII units
pass 12/12 on both x64 and x86, with documentation and whitespace gates
passing.  See [S2 evidence](../etc/evidence/t6-s2-product-monitor-policy.md).

T6 remains active.  A later S must establish the real App entry, adjacent
release configuration and product artifacts before any runnable-product claim.

## M1 T6 S3 Closure

S3 is closed.  `myraidenii-0-0-0006` is the sole process entry and calls the
existing adjacent-INI loader and Product composition exactly once.  It is
configured and linked on x64 and x86, but has no Asset copy command: an
external, user-owned `myraidenii.ini` plus individual ROM inputs remain a
separate release-qualification prerequisite.  No archive or ROM was opened.
Complete MyRaidenII unit selections pass 12/12 on both widths; see
[S3 evidence](../etc/evidence/t6-s3-product-entry.md).

T6 remains active.  A later S must qualify the external release configuration
and artifact workflow without introducing a second configuration or runtime
path.

## M1 T7 S1 Closure

S1 verified the owner-supplied selected set and ran the existing bounded
startup probe once per width.  Both widths reached the same two-million-
instruction budget without CPU, COP or control fault and produced the same
neutral frame/PCM observations.  This is limited L2 execution evidence, not
gameplay, audio-fidelity, desktop or release qualification.  The staging root
was empty after each wrapper run.  See
[S1 evidence](../etc/evidence/t7-s1-selected-set-startup.md).

T7 remains active.  The next packet must address only a concrete, independently
reviewed qualification boundary; it must not reinterpret the bounded probe as
a complete startup or playable-game proof.

## M1 T7 S3 Closure

S3 makes only host PCM output optional.  The same Machine Audio bridge still
consumes Board PCM; it locally discards that data only when Lib reports the
existing endpoint-unavailable statuses during stream creation.  All other
creation failures remain fatal.  Focused and complete MyRaidenII units pass on
both widths, and the real temporary Product launch survived the ten-second
observation on both widths before controlled shutdown/cleanup.  See
[S3 evidence](../etc/evidence/t7-s3-optional-host-audio.md).

T7 remains active.  This removes only host endpoint absence from startup; it
does not qualify audible output, guest input, visuals, gameplay or release.

## M1 T6 Closure

T6 is closed after three bounded deliveries: S1 establishes the single
configuration-to-Board/Machine composition owner; S2 establishes its single
generic monitor and Escape lifecycle route; S3 establishes the sole process
entry and local executable target.  All three retain Product -> Machine -> Core
ownership and consume Emulator contracts unchanged.  No protected input,
external archive, asset deployment, runnable release claim or gameplay claim
entered T6.

The next queued qualification task receives the deliberately separate work:
verified external individual-ROM configuration, production startup, lifecycle
and input observation, release artifacts and owner gameplay acceptance.  It
must not recreate the T6 composition, monitor or entry routes.

## Archived Packet — M1 T5 S18

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; S18 follows accepted S17 closure `4c45e54f5`. One session performs coordinator and executor roles. |
| Admission And Approval | The owner’s standing MyRaidenII implementation direction and latest “continue” instruction admit this final bounded T5 closure audit. No source-grade exception is requested. |
| Objective | Reconcile the finite T5 Sound ledger against current source and regressions, close every in-scope Sound/Machine receiver, and explicitly transfer the real user-command qualification to the queued Product task. |
| Non-goals | No new decoder, chip, clock, Product composition, executable, external payload, Shared change, timing-grade upgrade, or playable-game claim. |
| Reference Baseline | S1--S15 implement the one Board transport/time seam, local Z80/device/PCM path, Machine PCM bridge/driver/input adapter, and S16--S17 add strict Product config plus Machine-owned image materialization. |
| Candidate Proposal | Audit the production route and existing tests against the four accepted Sound receivers. Retain one Core Sound owner and one Machine-to-Lib bridge. Transfer only the missing real monitor/input-to-command qualification to the existing Product proposal because no Product composition exists. |
| Files And ABI Surface | Evidence, Current, T5 history/proposal/queue references as needed. No production/test/build/API/asset file is expected to change. |
| Applicable Rules | MyRaidenII guide, Architecture, Coding, UI, Source Policy and shared Execution/Architecture/Coding/Document rules. L3 requires an inspected primary-manual PDF explicit applicable rule; external/reference facts remain L2; unsupported local mechanisms remain L1. |
| Verification | Met: both cached unit targets had no work; full repository-only units pass 651/651 on x64 and x86; selected-set probe passes through the Machine transaction on both widths; documentation governance and whitespace checks pass. |
| Expected Markers | One production Sound owner, one Board transport/time route, one Machine PCM bridge/driver, no direct Product/Core file route, and one named transfer of real command-path qualification to `m1-product.md`. |
| Asset Needs | Existing owner-supplied external `O:\assets\raidenii\raiden2.zip` is read-only, transiently staged only by the existing verified tool, and never committed. |
| Reporting Requirements | Record a finite receiver/disposition ledger, actual run results, retained production route, source grades and explicit Product transfer. |
| Stop Conditions | Need for a new sound contract, Shared change, protected payload, source import, new execution route, or evidence-grade escalation stops and transfers the gap. |
| Exit Criteria | Met: every finite T5 receiver is proven by its owner regression or explicitly transferred to Product; full repository-only dual-width units and the owner-input probe pass; no T5 Sound obligation remains implicit. |
| Original Owner Request | Continue MyRaidenII under the strict L3/L2/L1 definition, without treating external implementations as L3. |
| Similar-Issue Sweep | Review all MyRaidenII Core Sound, Board sound transport, Machine audio/driver and associated tests for duplicate routes or untracked residual Sound receivers. |

Sequential coordinator closure review accepts S18. The production scan retains
one Core Sound owner, one Board transport/time route and one Machine-to-Lib
PCM bridge. The sole unresolved real command-path qualification is transferred
to the queued Product proposal, which must consume this driver rather than add
a second execution or audio route. Full repository-only units pass 651/651 on
both widths; the owner-input selected-set probe passes on both widths through
the Machine transaction. M1 T5 is closed. See [closure evidence](../etc/evidence/t5-s18-ledger-closure.md).

## Archived Packet — M1 T5 S17

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; S17 follows accepted S16 closure `22b59920d`. One session performs coordinator and executor roles. |
| Objective | Design and implement the single transactional runtime-image materialization boundary that consumes S16 configuration, validates the fixed selected-set images, and hands a complete owned bundle to later Board composition. |
| Reference Baseline | S16 owns strict adjacent-INI parsing but deliberately neither resolves paths nor opens inputs. Core Board copies a complete supplied bundle and exposes no external-file contract. |
| Candidate Proposal | Keep configuration syntax in Product, but move the fixed image-role enum and materialization transaction to Machine, where external-resource lifetime and later Board construction belong. Product maps named INI keys to the Machine request only. Machine resolves each configured path against the INI directory, opens all twelve individual external files read-only, checks the audited fixed byte count and code-owned SHA-256 digest, and destroys the entire candidate bundle on any failure. It exposes only a complete opaque bundle to later composition. No ZIP reader, fallback filename, archive path, partial Board replacement, payload logging or Shared checksum API is introduced. |
| Evidence Grades | Hashes/sizes are audited asset identity metadata, not CPU/device evidence. Any later model fact follows the user-approved ladder: inspected primary-manual PDF explicit applicable formula/constant/rule is L3; external/reference or range-only evidence is L2; unsupported local engineering is L1. |
| Files And ABI Surface | Product configuration maps keys to the Machine-owned role enum; Machine-local materialization and matching owner tests, Current and S evidence may change. No Shared component, Core ABI, external asset, ROM payload or product artifact is changed. |
| Applicable Rules | MyRaidenII guide, Architecture, Coding, UI, Source Policy and shared Execution/Architecture/Coding/Document rules. One transaction owns candidate lifetime; later Composition remains the sole Board/Machine/Session owner. |
| Verification | Met: dual-width focused wrong-size/digest loader checks, complete repository-only MyRaidenII unit selection (11/11 per width), documentation/whitespace checks, and an owner-supplied selected-set probe through the production Machine transaction. The probe does not claim desktop, host-audio, gameplay, Product composition, or EXE qualification. |
| Stop Conditions | Need for external archive decoding, protected payload retention, altered Shared hashing API, a Board public file API, unapproved asset/source import, or a second composition/session route stops and transfers the gap. |
| Exit Criteria | A complete candidate is either available or no candidate survives; every configured path is relative to the single INI directory unless explicitly absolute; all records are verified against one code-owned contract; no later owner needs to reopen or guess an image. |
| Temporary Artifacts | None. Synthetic test inputs live only within the test process. Existing Ninja/ccache trees are reused. |
| Original Owner Request | Continue implementation while preserving the explicit L3/L2/L1 definition. |

Sequential coordinator actual-change review accepts S17's Machine-only image
materialization. The existing selected-set probe no longer has a test-only
image reader or direct Core configuration route: its controlled temporary
staging supplies ordinary paths to the one production transaction. Both
widths pass the same bounded external-set run, while all 11 repository-only
MyRaidenII units per width, documentation governance and whitespace checks
pass. The run retained no protected payload and its staging root was removed.
This closes S17; M1 T5 remains open for its explicit remaining Sound work, and
the queued Product task remains the sole receiver for runnable composition.
See [S17 evidence](../etc/evidence/t5-s17-machine-image-materialization.md).

## Archived Packet — M1 T5 S16

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; S16 follows accepted S15 closure `aca35192d`. One session performs coordinator and executor roles. |
| Objective | Establish MyRaidenII's sole App-owned `myraidenii.ini` grammar and its complete runtime-image role contract, without creating a second machine, session, execution loop, archive reader or media path. |
| Reference Baseline | Core Board already accepts one complete copied image bundle; Machine now exposes its one Emulator driver and input adapter. No Product source, configuration parser or executable target currently exists. |
| Candidate Proposal | Add Product-local configuration parsing for the fixed twelve runtime image roles plus `display=window|console` and `console_control=0|1`. Resolve the adjacent lower-case INI through Lib Base/Storage. Require every role, reject duplicates and unknown keys, and leave file opening, relative-path resolution and content validation to the next composition transaction. |
| Evidence Grades | This parser has no hardware model claim. The fixed role list is an audited selected-set asset contract; it is not promoted to L3. The user-approved ladder applies to all later device facts: only an inspected primary manual PDF's explicit applicable formula/constant/rule is L3; external/reference or range-only evidence is L2; unsupported local engineering is L1. |
| Files And ABI Surface | `src/myraidenii/product/config_interface.h`, `config.c`, Product CMake registration and matching `test/myraidenii/unit/product/` only, plus Current. No Shared component, Core/Machine ABI, archive support, external media, ROM, Product executable or asset is changed. |
| Applicable Rules | MyRaidenII guide, Architecture, Coding, UI, Source Policy and shared Execution/Architecture/Coding/Document rules. Product remains the only configuration owner; Core stays platform/media-free; Machine remains the one Board-to-Emulator adapter. |
| Verification | Met: focused Product configuration test and complete repository-only MyRaidenII unit selection pass 9/9 on x64 and x86; documentation-governance and whitespace checks pass. No external-ROM, desktop, endpoint, integration or EXE claim applies because no runnable Product target exists. |
| Stop Conditions | Need for ZIP decoding, runtime ROM-name guessing, Shared API change, machine/session construction, a second command route, protected payload retention, or a source-grade claim beyond the stated ladder stops and transfers the gap. |
| Exit Criteria | Met: one strict, test-covered grammar owns all twelve roles and UI options; loading finds only adjacent `myraidenii.ini`; no production route loads a file yet; scoped gates and diff review pass. |
| Temporary Artifacts | None. Existing Ninja/ccache trees are reused. |
| Original Owner Request | Continue implementation while preserving the explicit L3/L2/L1 definition. |

Sequential coordinator actual-change review accepts S16's Product configuration
foundation. The sole parser owns the lower-case adjacent `myraidenii.ini`, the
twelve fixed selected-set image roles and the two shared presentation options.
It rejects missing, duplicate and unknown assignments without guessing a role
or opening any external input. File opening, relative-path resolution,
size/hash validation, Board construction and Emulator composition remain a
separate receiving increment; no archive reader, protected payload, Product
executable or Shared route was introduced. Both cached widths pass the complete
MyRaidenII repository-only selection 9/9, with documentation governance and
whitespace checks passing. S16 is closed; M1 T5 remains open for the
transactional image loader and one Product/Machine composition route.

M1 T4 Video is closed. S50 verifies that the selected-set probe loads the
complete external graphics-role bundle through the sole Core Board API,
executes the real V30 path, and reports one bounded neutral native frame
observation. It retains no ROM bytes and creates no second renderer, Machine,
Product or Shared path. The revised Video proposal explicitly leaves
Machine/Window publication to the later Product/Machine task; unqualified
sprite blend fidelity remains a separately named TODO rather than a false T4
blocker.

M1 T5 Sound remains active. S1 mapped the Seibu Z80, command/response latch,
interrupt, YM2151 and dual-OKI ownership against the existing Board wiring.
Published emulator behavior is accepted as L2 where primary material is absent;
the implementation remains independent, deterministic and Core-local. S2
closed the single Core-owned transport/time seam. S3 closed the bounded private
Z80 bootstrap, fixed map and bank route. S4 now closes the local YM2151 and
dual-OKI register/status/timer/sample boundary; deterministic mixing and host
PCM delivery remain later bounded Sound receivers. S5 now closes the first
Core-only PCM publication seam: guest time produces a bounded copied queue,
but the L1 diagnostic mixer is not a real-chip claim and Machine/Lib delivery
is still deferred.

Sequential coordinator actual-change review accepts S5 P1 `f5b9f0a23`.
The local Sound owner is the sole PCM queue/mix owner; Board has only the
one forwarding operation. The committed evidence records the L3/L2/L1
classification, bounded oldest-frame drop semantics and the retained limits.
Cached x64/x86 MyRaidenII unit selections each pass 6/6; focused Core
Sound/Board rows, documentation governance and `git diff --check` pass.
No external ROM was opened, no Shared component or release artifact changed,
and no host audio, desktop or playable-game conclusion is claimed. S5 is
closed; M1 T5 remains open for a separately admitted Machine/Lib delivery and
real-set qualification receiver.

Sequential coordinator actual-change review accepts S6 P1 `9abe996e4`.
The one existing selected-set probe retains its sole Board route and reports
only aggregate PCM observations; no sample payload, source import or product
path was added. Both widths reached the same established early stop with 616
PCM frames, zero drops and zero nonzero frames, so this is a transport
observation rather than an audio-result claim. Cached x64/x86 MyRaidenII units
pass 6/6 and documentation/whitespace checks pass. S6 is closed. M1 T5
remains open for the later Machine/Lib delivery and qualified real command
path; neither is silently implied by this diagnostic.

Sequential coordinator actual-change review accepts S7 P1 `e946c2d9b`. The
sole Core Sound owner now distinguishes the manual's nine attenuation levels
from L2 field-code disposition and its retained L1 fixed-point diagnostic
gain. The same command owner correctly gives pending phrase data priority over
the overlapping stop bit, then decodes a non-pending stop mask by channel.
The existing smoke executable proves the retained mapped path rather than a
test-only entry. Cached focused Sound/Board rows pass 2/2 per width; complete
MyRaidenII units pass 6/6 per width, with documentation/whitespace gates
passing. The task-owned rendered manual pages are removed. No Shared change,
external payload, host delivery, desktop result or real-game audio claim is
implied. S7 is closed; M1 T5 remains open for qualified decoder, Machine/Lib
delivery and real command-path receivers.

Sequential coordinator actual-change review accepts S8 P1 `822b15f24`. The
sole Core Sound owner now owns per-voice decoded signal and adaptation step
state; it consumes nibbles exactly once in its existing advance path and PCM
uses that state rather than raw nibble amplitude. The inspected MAME behavior
is explicitly L2, while the local fixed-point implementation is L1; neither
is presented as manual or silicon proof. The existing actual-route smoke now
shows opposite PCM signs for opposite synthetic nibble streams. Cached focused
Board/Sound rows pass 2/2 per width; complete MyRaidenII units pass 6/6 per
width, and documentation/whitespace gates pass. No Shared/API/media/artifact
change, external-set run, host delivery, desktop result or temporal-resampling
qualification occurred. S8 is closed; M1 T5 remains open for resampling
alignment, Machine/Lib delivery and real command-path receivers.

Sequential coordinator actual-change review accepts S9 P1 `4da7a5ecd`. PCM frame
conversion now consumes each existing delivered Z80/device slice through one
rational phase; the old whole-Board-interval path is gone. Board remains the
only elapsed-time source. The existing actual-route phrase smoke now confirms
that PCM arrives in the same bounded advance that executes the phrase start.
Focused Board/Sound rows pass 2/2 and complete MyRaidenII units pass 6/6 on
each width; documentation/whitespace gates pass. No Shared/API/media/artifact
change, external-set run, host delivery, desktop result, physical timing or
playable-game qualification is claimed. S9 is closed; M1 T5 remains open for
Machine/Lib delivery and qualified real command-path receivers.

Sequential coordinator actual-change review accepts S10 P1 after dual-width
qualification. The first MyRaidenII Machine component is a bounded PCM bridge:
it borrows the sole copied Board PCM consumer and the existing Lib stream,
stages one ordered suffix, and retires exactly the prefix Lib accepted. It has
no guest clock, worker, callback, mixer, alternate output route, Shared change
or imported material. `core/audio_interface.h` now names the pre-existing
48 kHz, interleaved stereo boundary once, so the Machine does not borrow a
Sound-private format declaration.

The retained FIFO/retry behavior is L1 local engineering; S10 adopts no manual
formula (therefore no L3 fact) and no external emulator behavior (therefore no
L2 fact). Its focused actual-route unit proves partial-prefix retirement and
failed-clear retention. Cached x64 and x86 builds pass the Machine target and
the complete MyRaidenII repository-only unit selection, 7/7 each; documentation
governance and whitespace checks pass. The owned temporary logs are removed.
No native endpoint, desktop, external-ROM, audible-output, Product-driver or
playable-game qualification is claimed. The later Machine driver must bind this
adapter as the sole live Board-to-Lib route. S10 is closed; M1 T5 remains open
for that driver and genuine real-command-path qualification.

Sequential coordinator actual-change review accepts S11 P1 `13aa69b47`. One opaque
MyRaidenII Machine driver is now the sole adapter from the constructed Board to
Emulator Machine and the already-created Lib PCM stream. It has one execution
route (the existing Core machine), one Board-to-Lib PCM bridge, and one
complete RGB32 frame copy into the granted Window slot; it neither retains the
slot nor creates a second guest clock, worker, output path or App dependency.

The caller transfers stream ownership only after driver creation succeeds;
construction failure leaves it with the caller. A later native-stream teardown
failure retains the Machine object for retry. The 256-instruction host-control
quantum, normal control wake and its cancellation of the existing writable
wait are all explicitly L1 local mechanisms. No manual PDF fact is claimed as
L3 and no external-reference behavior is claimed as L2.

Cached x64/x86 MyRaidenII unit selections pass 8/8 each, including the actual
Machine driver callback unit; documentation governance and whitespace checks
pass. No endpoint, desktop, Product/Session, control, ROM, audible-output or
playable-game verification occurred. S11 is closed; M1 T5 remains open for
Product composition and qualified real command-path work.

Sequential coordinator actual-change review accepts S12 P1 `9f8574f28`. The
sole approved external-set probe now emits only non-payload CPU-fault summary
fields and corrects its staging description; it has no production or Shared
change. Cached x64/x86 unit selections pass 8/8 each, documentation governance
and whitespace checks pass, and each width reaches the same bounded Core
`STOP_FAULT` with `LIB_STATUS_UNSUPPORTED` after 12,721 instructions and
205,420 elapsed ticks. The public CPU diagnostic and COP observation are both
empty; Board's fixed Sound-before-COP advance order therefore transfers the
earliest current receiver to the local Core Sound decoder.

This is an implementation diagnosis, not a hardware conclusion: S12 adopts no
manual-PDF L3 fact and no external-reference L2 behavior. The next decoder
receiver must obtain an eligible L3 formula/constant/rule, a clearly bounded
L2 external behavior, or state an L1 local model. The probe retained no ROM
payload, trace, frame or PCM sample; its task staging was removed. No audible
output, startup, graphics, input, desktop or gameplay qualification is
claimed. S12 is closed; M1 T5 remains open for the decoder receiver, Product
composition and qualified real command-path work.

Sequential coordinator actual-change review accepts S13 P1 `be53c2556`. Sound now
owns the two missing fixed-map behaviors and nothing else: the L2 MAME board map
marks `0x4004` no-op read/write and routes both YM ports; the no-device `0xff`
read value is explicitly L1 because neither inspected manual defines it. The
existing smoke was extended through the actual Z80 path, not by adding another
decoder entry; it proves no-device write stability and second-YM-port status
read. No public API, Shared component, Board timing route, Product, Machine or
external material changed.

The bounded owner-supplied selected-set probe now reaches its two-million
instruction budget on both widths instead of a Sound fault. This proves only
that the fixed-map refusal was removed: it does not qualify graphics, zero
PCM output, host audio, startup, desktop, controls or gameplay. Cached x64/x86
MyRaidenII unit selections pass 8/8 each; focused Sound checks, documentation
governance and whitespace checks pass. Debug staging was deleted without
retaining program bytes, opcode/address/PC output, trace, frame or PCM payload.
S13 is closed; M1 T5 remains open for Product composition and qualified real
command-path work.

Sequential coordinator actual-change review accepts S14's bounded diagnostic.
The public Board control route remains the only control owner, and the normal
selected-set probe is unchanged unless explicitly given its diagnostic option.
Holding Coin 1 and Start 1 through the fixed budget is explicitly L1; the
existing active-low mapping is L2. Both widths reached the same budget with no
CPU/COP/control failure and zero nonzero PCM frames. This is not an audio,
startup, input, desktop or gameplay result. S14 is closed; M1 T5 remains open
for later qualified Sound work, while Product/Machine configuration and the
real user-input route remain the queued receiving task.

## Archived Packet — M1 T5 S15

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; S15 follows accepted S14 closure `eb440284a`. One session performs coordinator and executor roles. |
| Admission And Approval | The owner directs continued implementation under the explicit L3/L2/L1 source ladder. Product-visible host key bindings are a local L1 policy unless separately supported; the Board's semantic JAMMA controls remain the previously recorded L2 board contract. |
| Objective | Replace the Machine driver's current input discard with its one required KVM-to-Board control adapter, including release/reset hygiene. |
| Reference Baseline | `myraidenii_machine_deliver_input()` currently ignores every KVM event despite the Board already owning active-low semantic controls. No Product path is created by this S. |
| Candidate Proposal | Map a finite L1 host default (arrows, Z/X, 1/5 and F2) to existing Player 1 direction/button, Start 1, Coin 1 and Service controls. Forward only KVM key transitions. Emulator Session already turns INPUT_RESET/SOURCE_RETIRED into source-owned key releases, so the driver creates no second held-key ledger. No second queue, product parser or public interface is added. |
| Files And ABI Surface | `src/myraidenii/machine/driver.c`, its existing Machine-driver unit, evidence and Current only. No Shared, Product, Core ABI, ROM/media or artifact change. |
| Applicable Rules | MyRaidenII guide; source policy; Architecture, Coding, Documentation and Execution rules. Machine remains the sole KVM-to-Board adapter; Board remains sole electrical/polarity owner. |
| Verification | Met: focused Machine-driver and complete MyRaidenII unit suites pass 8/8 on x64 and x86; documentation governance and whitespace checks pass. |
| Stop Conditions | Need for a configurable binding registry, product/session parser, additional input source, unavailable KVM event fact, external payload, Shared API change or nonlocal key policy stops and transfers the gap. |
| Exit Criteria | Met: actual Machine-driver tests prove every listed make, a break, unrelated-event ignore and no reset/retirement-ledger duplication. |
| Original Owner Request | Continue toward MyRaidenII under strict evidence grades without promoting an external/reference or local policy to manual-backed L3. |

Sequential coordinator actual-change review accepts S15's Machine-only input
adapter. It recognizes a finite L1 host policy and delegates all recognized
make/break transitions to the existing Board semantic-control owner. Emulator
Session remains the sole source-reset/release ledger; no second held-key state
or input route was added. Both cached widths pass focused Machine-driver and
complete MyRaidenII 8/8 unit selections, documentation governance and
whitespace checks. There is no Product executable, desktop input or gameplay
claim. S15 is closed; M1 T5 remains open for qualified Sound work and queued
Product/Machine configuration.

## Archived Packet — M1 T5 S14

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; S14 follows accepted S13 closure `f0034a7a6`. One session performs coordinator and executor roles. |
| Admission And Approval | The owner directs continued MyRaidenII work and clarifies the source-grade rule: only an inspected primary manual PDF's explicit applicable formula or constant is L3; external/reference behavior or a range-only fact is L2; an otherwise unsupported local model is L1. No source, table, ROM, firmware or generated-output import is authorized. |
| Objective | Establish the next bounded receiver for the selected-set sound path without conflating a budget-limited, zero-PCM observation with a sound-chip or product failure. |
| Reference Baseline | S13 removed the two observed fixed-map refusals. The external-set probe now reaches its declared instruction budget on both widths, with no unsupported decoder result and with zero-valued PCM observation. Current product composition and real input/control routing are still absent. |
| Candidate Proposal | Audit the sole existing Board-to-Sound command origin and the private Z80/device advance route. Adopt only a finite fact supported by the approved L3/L2/L1 ladder; if no receiver can be isolated without a Product control path or protected payload observation, record the transfer rather than widening an unproven device model. |
| Files And ABI Surface | Initially read-only research/evidence and Current. Any later source/test delta must stay within MyRaidenII Core Sound/Board and its existing owning unit tests, unless a separately demonstrated Product dependency is needed. No Shared, external media or artifact change is in scope. |
| Applicable Rules | MyRaidenII guide; source policy; Architecture, Coding, Documentation and Execution rules. Board remains the only guest-time and command transport owner; Sound remains the sole decoder/device/PCM owner. |
| Verification | Met: cached x64/x86 probe builds and one bounded held-control diagnostic per width completed; repository-only MyRaidenII units pass 8/8 on both widths; documentation governance and whitespace checks pass. |
| Stop Conditions | Need for a Product input route, a second execution path, protected payload/trace retention, external-source/table import, Shared API change, or an unsupported timing/decode assertion stops and transfers the gap. |
| Exit Criteria | Met: the held-control experiment is explicitly L1 and leaves the normal probe unchanged. It isolates no further Sound decoder receiver; the required real Product/Machine input route is precisely transferred without speculative Sound behavior. |
| Original Owner Request | Continue under the strict L3/L2/L1 classification: explicit applicable manual-PDF formulas/constants only for L3; external/range-only references L2; unsupported local engineering L1. |


## Archived Packet — M1 T5 S6

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; S6 follows accepted S5 closure `6c2efe7a5`. One session performs coordinator and executor roles. |
| Admission And Approval | The owner authorized continued M1 Sound work and explicitly permits bounded external-reference use as L2 while retaining the L3/L2/L1 distinction. No Shared or media import is authorized. |
| Objective | Extend the existing selected-set startup diagnostic to observe the sole Board PCM output once after bounded real V30 execution, recording only neutral aggregate facts. |
| Non-goals | No production Sound/Board change, decoder change, host PCM delivery, Machine/Product implementation, ROM retention, PCM dump, new renderer, timing claim, audible/playable claim or Shared change. |
| Reference Baseline | S5 owns the only local bounded PCM queue; Board has one forwarding operation. The existing startup probe already stages fixed external input roles transiently, runs a bounded V30 budget and reports neutral frame facts. |
| Candidate Proposal | Reuse that probe and its one Board instance. After execution, copy at most the fixed queue capacity via `myraidenii_core_board_pcm_copy()`, compute only frame count, cumulative dropped count, nonzero-frame count and a deterministic digest, then release the local copy. A zero/nonzero result is observation, not pass/fail evidence of audio fidelity. |
| Files And ABI Surface | Expected only `test/myraidenii/integration/selected_set_startup_probe.c`, its existing launcher/evidence/Current and no source ABI change. No Shared, Core production, ROM/media, artifact or app file changes. |
| Applicable Rules | MyRaidenII guide; source policy, Architecture, Coding, Execution and Documentation rules. Existing probe remains the one external-set diagnostic owner; Board/Sound remain the only Core route. |
| Verification | Met: cached x64/x86 probe builds and the external selected-set launcher both complete with identical neutral output. The closing source will also run the complete MyRaidenII unit selection on both widths plus documentation and whitespace checks. |
| Expected Markers | Probe calls exactly one Board PCM copy after bounded execution, emits no sample payload, releases copied storage, and returns successful diagnostic completion regardless of zero/nonzero PCM content unless the existing Core route actually fails. |
| Asset Needs | The owner-supplied external archive remains read-only. Existing temporary staging is deleted by the current launcher. No protected bytes, hashes, sample data or trace enter tracked files. |
| Reporting Requirements | Record command/result by width, the observed neutral PCM fields, source/test delta, and explicit distinction between a transport observation and audio fidelity/host delivery. |
| Stop Conditions | Need for a second audio path, retained external payload, raw audio dump, host output, timing workaround, external-source import or a changed Board contract stops and transfers the gap. |
| Exit Criteria | Met: the existing external-set probe has one safe PCM observation on both widths. It reports 616 copied frames, zero drops and zero nonzero frames at the existing early stop; no protected payload, production change or component boundary change occurred. |
| Original Owner Request | Continue using direct manual facts only as L3, external behavior as L2 and unsupported local mechanisms as L1. |
| Similar-Issue Sweep | Inspect all external-set diagnostic output/staging/cleanup fields for protected payload leakage, duplicate Board routes, hidden host-output behavior or a pass predicate based on observed audio content. |

## Archived Packet — M1 T5 S5

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; S5 follows accepted S4 delivery `db8f88167`. One session performs coordinator and executor roles. |
| Admission And Approval | The owner approved the finite T5 Sound sequence, authorized external emulator behavior as L2 when primary material is unavailable, and clarified the L3/L2/L1 classification on 2026-10-20. This packet consumes that approval only. |
| Objective | Add one Core-local, bounded PCM publication seam driven exclusively by the existing Board guest-tick axis, then establish a deterministic mixing proof for the existing local YM/OKI ownership without host delivery. |
| Non-goals | No Lib/Emulator change, host audio stream, thread, second clock, generic audio framework, imported source/table/ROM, physical-chip fidelity claim, desktop playback or playable-game claim. |
| Reference Baseline | S4 has one local Sound owner with Z80, YM register/timer state and two OKI channel/sample-address states. The current `advance()` is called only by Board attachment time. Architecture reserves Core PCM creation before later Machine-to-Lib adaptation. The supplied OKI manual establishes the device class and channel role but does not supply a directly reusable decoder formula in the inspected material; any decoder/mix approximation must be L1 unless separately backed by a specific eligible reference. |
| Candidate Proposal | Extend the existing local Sound owner with one fixed-rate bounded PCM queue and one copied, read-only Core output operation. Advance converts only Board ticks to output frames, creates samples through the current device state and defines explicit overflow/drop accounting. Board exposes that existing Sound output without duplicating state. A local synthetic vector proves deterministic non-silent command-triggered output, queue order, reset and bounded overflow behavior. |
| Files And ABI Surface | Expected MyRaidenII-only paths: `src/myraidenii/core/{sound.c,sound.h,board.c,board_interface.h}`, existing Core Sound/Board tests, evidence, Current and manifests. Any additive API is Core-local and uses copied bounded samples. No Shared, Product, Machine, ROM/media or artifact change. |
| Applicable Rules | MyRaidenII guide; Architecture, Coding, source policy and repository execution/architecture/coding/documentation rules. Board remains the one Core composition/API root; Sound remains the sole local audio state/mix owner; no external payload is retained outside existing copied role inputs. |
| Verification | Met: cached x64/x86 focused Sound/Board tests pass; the complete six-row MyRaidenII unit selection passes on both widths; `git diff --check` passes. No external set, host audio or desktop result is counted. |
| Expected Markers | Board time is the only producer clock; a device state transition yields deterministic copied PCM in FIFO order; reset discards unconsumed samples; overflow has one stated outcome and does not silently duplicate frames; Board output observes exactly the Sound-owned queue. |
| Asset Needs | No additional asset. Existing owner-supplied archive stays read-only and is not opened by unit vectors. |
| Reporting Requirements | Record every adopted audio fact with explicit L3/L2/L1 grade, source/edition where applicable, actual code/test deltas, queue ownership/overflow semantics, x64/x86 results and the absence of host/real-game qualification. |
| Stop Conditions | A needed Shared contract, imported algorithm/table, second time owner, retained external payload, generic framework, or a demand for physical audio fidelity stops and transfers the gap. |
| Exit Criteria | Met: one local Sound PCM queue has a single guest-time producer and copied Board consumer; the retained existing Sound smoke covers output/order/reset/overflow; no second audio/timing path exists. The evidence record names the L3/L2/L1 basis and the deferred host/real-chip work. |
| Original Owner Request | Continue MyRaidenII sound work using manuals only for explicit L3 facts, external references only as L2, and unsupported approximations only as L1. |
| Similar-Issue Sweep | Inspect every Sound time/state/output path, Board callback, reset/destroy path and existing test for duplicate frame ownership, host-time use, silent overflow or an ungraded evidence claim. |

## Archived Packet — M1 T5 S4

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; S4 follows accepted S3 closure `95df75bbe`. One session performs coordinator and executor roles. |
| Admission And Approval | The owner approved the finite T5 Sound sequence and repeatedly authorized mature external emulator behavior/layout as L2 evidence when primary board material is unavailable, most recently 2026-10-20. This packet consumes that approval; no source, ROM, table or generated-output import is permitted. |
| Objective | Give the existing private Sound owner deterministic, local YM2151 and two MSM6295 register/status/channel models at their one fixed mapped addresses, including their bounded timer/interrupt and sample-address behavior. |
| Non-goals | No MAME/source import or transliteration, ROM/sample retention, host PCM delivery, mixing, second guest clock/thread, Shared API, generic chip framework, physical-silicon claim or playable-game claim. |
| Reference Baseline | S3 owns the copied Z80 program, sole Board-derived tick axis, fixed 0000--ffff map and transport. YM/OKI map addresses currently reject. Published Raiden II board wiring identifies YM at 4008--4009, OKI devices at 6000/6002, YM at Z80 clock and each OKI at 28.636363 MHz/28 with pin 7 high; these facts are L2 only. |
| Candidate Proposal | Extend the one local Sound object—not Board, Lib or a new device framework—with private YM and two OKI state. Map accesses through the existing sole Z80 read/write switch; drive all device time from already delivered Z80 ticks; expose only map-visible status/IRQ effects to the existing local CPU/transport path. Copy supplied sample images only if the Board config later admits their fixed role/size contract; otherwise preserve explicit unavailable behavior. |
| Files And ABI Surface | Expected: `src/myraidenii/core/{sound.c,sound.h,board.c,board_interface.h}`, the existing Sound/Board test families, selected-set probe/staging only if sample-role ingress is needed, evidence and Current. Any additive Board config roles remain Core-local and copied. No Lib, Emulator, other Shared or runnable App changes. |
| Applicable Rules | MyRaidenII guide; Goal, Architecture, Coding, source policy; repository architecture/coding/documentation/execution rules. Sound remains one Core-local owner; Board remains the only main-side transport/time source. |
| Verification | Met: focused synthetic register/timer/status/channel vectors, mapped transport paths and reset/bounds proof pass on x64/x86. All six MyRaidenII unit tests pass on both widths. The selected external archive probe supplies both sample roles through Board, releases its buffers after creation and completes on both widths without retaining payload. |
| Expected Markers | Reads/writes at 4008--4009, 6000 and 6002 reach exactly one local owner; device time changes only from the Board-to-Z80 timeline; channel/status/IRQ outputs are deterministic; invalid forms fail without mutating accepted state; missing sample inputs cannot look like successful playback. |
| Asset Needs | The owner-supplied external `O:\assets\raidenii\raiden2.zip`, read-only, only if fixed sample-role ingress is admitted. Temporary unpacking is under ignored `build/`, deleted after execution; no protected media, bytes or hashes enter tracked source. |
| Reporting Requirements | Record L2 sources/facts, actual source/test line delta, retained owners/routes, width-by-width verification, and the distinction between register semantics and deferred mixing/host PCM/real-game audio. |
| Stop Conditions | A necessary Shared contract, unresolved fixed sample role/size, need to retain external payload, conflicting route, second clock/thread, source-license issue or behavior requiring a generic framework stops and transfers the bounded gap. |
| Exit Criteria | Met: one local Sound owner implements the fixed YM/two-OKI map and deterministic guest-time semantics with synthetic proof; no duplicate device/timing path remains. The L2 facts, caller-buffer ownership and width evidence are recorded in `docs/myraidenii/etc/evidence/t5-s4-local-sound-devices.md`. Mixing, Lib PCM delivery and qualified selected-game sound are explicitly deferred. |
| Original Owner Request | Implement MyRaidenII under governance; use external emulator implementations as L2 where required, without repeated L3 back-and-forth. |
| Similar-Issue Sweep | Inspect all Sound map reads/writes, reset/destroy paths, Board time consumers, transport acknowledgements, tests and selected-set staging for duplicate device state, unowned success paths or hidden host-time dependence. |

## Archived Packet — M1 T5 S3

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; follows S2 delivery `e011c6998`; one session performed coordinator and executor roles. |
| Objective | Add the first bounded, private Z80 execution owner to the existing Core Sound seam and connect the fixed L2 sound-side address map without creating a second command/clock path. |
| Non-goals | No copied external CPU source, generic Z80 framework, YM/OKI synthesis, host PCM, second clock/thread, Shared API, ROM/PAL retention or playable-game claim. |
| Reference Baseline | Board owns the main-V30 command/response/pending registers; S2 owns deterministic rational Z80 elapsed ticks but deliberately does not consume them or alter latch state. |
| Candidate Proposal | Independently implement the minimum local Z80 state/fetch/execute/map owner needed to consume S2 ticks. It alone acknowledges commands/interrupts when its mapped Z80-side operations execute; bank mapping remains private and follows accepted L2 map facts. |
| Files And ABI Surface | MyRaidenII Core Board/local Sound source and its existing Board-owned unit family only. No Lib, Emulator, Shared or public Board API change. |
| Applicable Rules | MyRaidenII guide; architecture, coding, documentation, execution and source-policy rules. |
| Verification | Met: the existing Sound/Board synthetic vectors cover reset, fetch/decode, bounded elapsed execution, fixed map/bank routing and mapped transport operations. The selected external archive ingress probe ran in both widths without retaining payload. Complete repository unit suites passed x64/x86; each restricted run had only the deliberate Junction-fixture sandbox failure, and `repository.build-cleanup` was then rerun successfully under normal Windows permissions on both widths. |
| Stop Conditions | A required Shared contract, ROM retention, host-time callback, second clock, copied source/table or silent latch completion stops and records the gap. |
| Exit Criteria | Met: one private Z80 owner consumes S2 ticks through the sole Board timeline, has one fixed L2 map/bank route, and cannot acknowledge a command or interrupt outside its actual mapped execution path. This is not a claim that the selected sound ROM, YM2151, either OKI, PCM delivery, or game audio is complete. |

## Archived Packet — M1 T5 S2

| Field | Required record |
| --- | --- |
| Delivery | `e011c6998` adds the private rational Board-to-Z80 clock seam and the local sound unit. |
| Exit Criteria | Met: only the Board elapsed callback advances the rational sound clock; reset clears it; no command/interrupt state is prematurely read, acknowledged or completed; x64/x86 focused and full repository unit qualification passed. |

## Archived Packet — M1 T5 S1

| Field | Required record |
| --- | --- |
| Delivery | `67420101d` closes T4 Video, corrects the active-packet record, and records the finite S1 sound contract. |
| Exit Criteria | Met: every Sound subsystem has one owner, grade, route and synthetic proof owner; S2--S5 are explicit with no source/table/media import or second time owner. |

## Archived Packet — M1 T4 S50

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: follows accepted S49 P2 `571e03f9e`; one session performs coordinator and executor roles. |
| Objective | Extend the existing selected-set startup integration probe so it validates the complete immutable graphics input bundle and one Board-owned native RGB32 frame after bounded real V30 execution. |
| Non-goals | No production behavior/API change, shared change, new renderer, Machine/Product executable, external payload retention, ROM dump, screenshot, raw trace, sound/protection completion or claim of playable qualification. |
| Reference Baseline | S44--S49 establish one copied graphics ingress, sprite decrypt/pixel reader, latched object RAM and Video compositor. The existing program-only probe already reaches two million real V30 instructions on both widths without CPU/COP/control fault. |
| Candidate Proposal | The probe accepts the two program lanes plus the seven fixed-role graphics files, validates exact existing sizes through Board creation, runs its existing budget, calls the existing Board frame-copy API once, and emits only dimension, non-black count and a deterministic frame digest. It stages inputs transiently outside the repository. |
| Files And ABI Surface | `test/myraidenii/integration/selected_set_startup_probe.c` and MyRaidenII evidence/current only. No production file, declaration, manifest, shared component or artifact changes. |
| Applicable Rules | MyRaidenII guide; architecture, coding, documentation, execution and source-policy rules. Board remains the only graphics ownership/API root; Video remains the only compositor. |
| Verification | Passed: cached x64/x86 probe builds; selected external set on both widths through two million instructions; complete five-entry MyRaidenII unit suite on both widths; one wrong-size graphics-role negative run; documentation governance and `git diff --check`. No desktop/RDP/gameplay result is claimed. |
| Expected Markers | Missing/wrong graphics role rejects before Board execution; correct role bundle survives caller-buffer release; both widths complete bounded execution without a terminal CPU/COP/control fault; a complete sized frame is copied through Board once without retaining protected pixels. |
| Asset Needs | The owner-supplied external `O:\\assets\\raidenii\\raiden2.zip`, read-only. Temporary unpacked lanes/images live under ignored `build/`, are deleted after each run, and no payload/hash is committed. |
| Reporting Requirements | Record exact command form without raw paths where unnecessary, neutral width-by-width observations, changed source/test/doc totals, and the limitation that this is Board/Core integration rather than product/window/gameplay evidence. |
| Stop Conditions | A missing/incorrect role size, terminal Board fault, need for a second frame path, or need to retain payload stops and becomes a bounded follow-up; it is not papered over with fallback graphics. |
| Exit Criteria | Met: the only Board frame path accepts the complete selected image bundle and returns a sized RGB32 frame after real selected-set execution on both widths; no protected media is retained; named tests and governance checks pass. |

## Archived Packet — M1 T4 S49

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S49 follows accepted S48 P2 `cf6215917`; one session performs coordinator and executor roles. |
| Objective | Correct Video's only latched-sprite painter to scan the fixed 4 KiB object image in selected descending-entry order and omit non-empty descriptors whose tile code is zero. |
| Non-goals | No external source/table import, ROM/media, sprite-bank callback, blend/shadow, protection behavior, Board-map change, Machine/Product work, Shared change or physical-hardware claim. |
| Reference Baseline | The MAME BSD-3-Clause `sei25x_rise1x_spr.cpp` documents the compatible eight-byte layout and scans object entries downward; its loop skips zero tile code. The existing decoder already represents this layout, while S48's initial painter used ascending order and only skipped all-zero descriptors. |
| Files And ABI Surface | `src/myraidenii/core/video.c`, its existing Core graphics smoke, L2 evidence and task record only. No declaration, public API, allocation, component boundary or mutable state changes. |
| Verification | Passed: focused Core tests plus all five MyRaidenII units on cached x64/x86 Ninja trees, documentation governance and `git diff --check`. |
| Exit Criteria | Met: the sole compositor has one bounded descending scan, tile code zero preserves the previous pixel, lower-address descriptors win within a priority phase, and named verification passes. |

## Archived Packet — M1 T4 S48

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S48 follows accepted S47 P1 `0cd527357`; one session performs coordinator and executor roles. |
| Admission And Approval | The owner directs continued MyRaidenII implementation using external mature emulator behavior as explicit L2, without recurring L3 back-and-forth. Existing source policy permits independently written behavior derived from the inspected reference. |
| Objective | Add one Video-owned compositor path that copies only latched Sprite Buffer entries, decodes existing copied descriptors and already-decoded pixels, and merges their non-transparent RGB32 output at the selected five priority phases. |
| Non-goals | No MAME source import/transliteration, ROM/media payload, sprite-protection emulation, blend/shadow behavior, tile extension/banking interpretation beyond the existing copied descriptor, board-map change, Machine/Product work, Shared change or physical-hardware claim. |
| Reference Baseline | S45 owns live/latched object RAM, S46 owns the eight-byte descriptor and S47 owns one decrypted packed sprite stream. Existing Video owns the only RGB32 tile compositor; Board owns the only frame API. |
| Candidate Proposal | Video receives the opaque Sprite Buffer only for `frame_copy`, reads one copied eight-byte latched entry at a time through its existing bounded API, and paints decoded 16-by-16 tiles in descriptor order. Four descriptor priorities are inserted before background, between Background/Midground/Foreground, and before Text; pen 15 stays transparent and Sprite palette groups use the existing 0x000 palette region. |
| Files And ABI Surface | `src/myraidenii/core/{video.c,video.h,board.c}` and focused Core unit tests/evidence/Current only. Core-local `video_frame_copy` gains an opaque read-only Sprite Buffer input; Board adapts its existing owned buffer. Lib, Emulator, other X86, Machine, Product and App code stay unchanged. |
| Applicable Rules | MyRaidenII guide; architecture, coding, documentation, execution and source-policy rules. Video is the sole pixel/composition owner; Sprite Buffer remains the sole mutable/latched RAM owner; Board remains the sole composition/API root. |
| Verification | Add synthetic tests for latched-not-live source, descriptor tile extent and flips, transparent pen, palette group, priority phases, clipping and error preservation. Run focused x64/x86 Core tests, cached complete MyRaidenII selection and repository units, documentation governance and `git diff --check`. No protected ROM or desktop result is claimed. |
| Expected Markers | A live post-latch mutation cannot alter the copied frame; all four priority values have one defined phase; out-of-frame tiles clip; invalid Sprite Buffer reads fail without publishing a partial destination; existing tile-only frame remains unchanged. |
| Asset Needs | Read-only inspected MAME behavior/reference facts only. No ROM, PAL, media, screenshot, trace or external generated data enters the repository. |
| Reporting Requirements | Record the L2 facts, single retained compositor path, interface delta, source/test/doc line totals, verification by width and deferred blend/bank/protection/real-game work. |
| Stop Conditions | A required unresolved bank/extension, blend/shadow algorithm, protected payload, Shared contract, second render path or Board/Machine/Product expansion transfers to a later packet rather than being guessed. |
| Exit Criteria | One Video path consumes only the Sprite Buffer snapshot and existing Graphics/Palette owners; synthetic tests reject wrong phase, live-RAM use, pen visibility, flip/clip and partial publication; named verification passes. |
| Original Owner Request | Use external implementations as L2 evidence and proceed toward a running Raiden II without treating unavailable physical documentation as a repeated blocker. |
| Similar-Issue Sweep | Inspect all frame callers, descriptor consumers, sprite-buffer reads, tile palette bases, output/error paths and any duplicated object-pixel paths. |

## Archived Packet — M1 T4 S47

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S47 follows S46 P1 `a79641f64`; one session performs coordinator and executor roles. |
| Admission And Approval | The owner reiterated on 2026-10-20 that external mature emulator implementations are approved L2 evidence and directed progress without repeated L3 back-and-forth. This packet uses the separately licensed BSD-3-Clause Raiden II sprite-decrypt unit as a narrowly recorded L2 model; exact provenance and notice are reviewed before any retained constants enter the repository. |
| Objective | Add one Core-owned, deterministic decryption transform for the selected Raiden II sprite stream so a later Video compositor receives decoded four-bit sprite pixels rather than encrypted words. |
| Non-goals | No MAME runtime dependency, ROM/media import, executable, new Shared API, second renderer, generic crypto framework, physical-hardware accuracy claim, sprite priority/composition, Board-map change, or host presentation work. |
| Reference Baseline | S44 owns the 32-bit lane-interleaved 8 MiB private sprite stream; S45 owns latched object RAM; S46 owns descriptor decoding. `graphics_interface.h` deliberately has no sprite pixel decoder because its source bytes remain encrypted. |
| Candidate Proposal | Add one private Core decrypt implementation and one public Core sprite-pixel reader. It transforms the existing private stream once after ingress using the finite Raiden II L2 model, preserves all other graphics ingress, and exposes packed 16-by-16 four-bit pixels through the existing Graphics owner. No caller retains or mutates a decoded stream. |
| Files And ABI Surface | `src/myraidenii/core/{graphics.c,graphics.h,graphics_interface.h,sprite_decrypt_data.h,video.c,video.h,CMakeLists.txt}`, owning Core graphics unit, MyRaidenII source-policy/evidence and exact third-party notice only. The additive Core sprite-pixel declaration is the sole public surface; Lib, Emulator, X86 and Board interfaces remain unchanged. |
| Applicable Rules | MyRaidenII guide, architecture, coding, documentation, execution and source policy. Video remains the sole graphics-stream owner; Graphics remains the sole pixel-decoder owner. BSD-3-Clause provenance is retained without linking to, vendoring, or executing MAME. |
| Verification | Met: synthetic known-answer vectors cover word transform, nibble order and bounds; focused graphics/Board tests, complete MyRaidenII selection and complete repository unit selection pass on cached x64/x86 Ninja trees. The restricted runner cannot create the cleanup-test junction; that exact test passes under normal Windows execution on both widths. Documentation governance and `git diff --check` pass. No external ROM or desktop run is claimed. |
| Expected Markers | Every transformed 32-bit word follows the one L2 model; every sprite tile has 16 by 16 four-bit pixels; invalid pointers/index/coordinates reject; release/reset remain one allocation path; no live object RAM access occurs from Graphics. |
| Asset Needs | Read-only local MAME reference checkout only. No ROM, PAL, media or external generated payload enters the repository. |
| Reporting Requirements | Record exact revision/path/license, the L2 limitation, source/test/document delta, public surface, x64/x86 results and deferred sprite compositing/priority/real-game verification. |
| Stop Conditions | A need for an LGPL unit, unreviewed license, ROM contents, a second data path, Board/Shared change, priority/blending semantics or Product/Machine work transfers to a later S. |
| Exit Criteria | Met: one Video-owned private stream is transformed once, Graphics has one bounded sprite-pixel reader, synthetic vectors reject a wrong transform/nibble order/boundary, existing graphics paths stay valid, and all named checks pass. |
| Original Owner Request | External implementations are usable as L2; do not repeatedly block progress by demanding unavailable hardware-level evidence. |
| Similar-Issue Sweep | Inspect all selected graphic ingress/stream consumers, ownership/release paths, table provenance/license, byte order, error returns and duplicate decrypt/pixel paths. |

## Archived Packet — M1 T4 S46

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S46 follows S45 P1 `328e760bf`; one session performs coordinator and executor roles. |
| Admission And Approval | The owner directed use of external mature emulator implementations as L2 without repeated L3 approval churn, 2026-10-20. |
| Objective | Give one MyRaidenII Core sprite component a bounded decoder for an eight-byte latched sprite descriptor, including tile index, palette/color index, priority, position, dimensions and both flip bits. |
| Non-goals | No graphics decrypt, sprite pixel decode/draw, bank behavior, blending, frame priority composition, Board-map change, ROM read, Shared change, external payload or source/table import. |
| Reference Baseline | S45 now makes the complete 4 KiB latched object RAM available to the later Video owner, but no Core component can yet name or validate a descriptor's fields. |
| Candidate Proposal | Add one independent Core sprite descriptor function that accepts exactly eight little-endian V30 bytes and emits a copied value object. It validates only the bounded input/output contract; later rendering owns object iteration and visibility. |
| Files And ABI Surface | `src/myraidenii/core/{sprite.c,sprite_interface.h,CMakeLists.txt}`, matching Core unit test, evidence and Current only. This adds a Core-local copied descriptor API, with no Board or Shared public interface change. |
| Applicable Rules | MyRaidenII guide, architecture, coding, documentation, source policy and execution rules. Sprite-buffer remains sole RAM owner; the decoder owns only field extraction; tests retain no external bytes. |
| Verification | Passed focused descriptor test x64/x86; complete five-entry MyRaidenII unit selection x64/x86; documentation governance and `git diff --check`. No runnable Product/Machine EXE exists. |
| Expected Markers | Valid synthetic descriptors preserve all named fields; signed 9-bit coordinate boundaries, one-to-eight tile spans, flip bits and input-size failures are explicit; no mutable descriptor or graphics state is retained. |
| Asset Needs | None. Synthetic eight-byte vectors only. |
| Reporting Requirements | Report source/test/doc added/removed/net lines, finite L2 layout facts, width-by-width verification, public API scope and deferred decrypt/draw behavior. |
| Stop Conditions | Any needed decryption table, real ROM content, board-specific sprite bank, priority algorithm, renderer/frame change, Shared API or copied/transliterated source transfers to a later S. |
| Exit Criteria | Met: one Core decoder exposes the finite descriptor fields from exactly eight bytes; synthetic regression covers bit-field/sign/size boundaries; no duplicate parser or rendering path exists. |
| Original Owner Request | Continue implementation toward RAIDEN II using external implementations as L2 instead of treating absent physical L3 evidence as a recurring blocker. |
| Similar-Issue Sweep | Recheck endianness, coordinate sign-extension, all descriptor bit fields, mutable-layout exposure, duplicate field extraction and accidental graphics/buffer ownership overlap. |

## Archived Packet — M1 T4 S45

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S45 follows S44 P2 `a238d3a84`; one session performs coordinator and executor roles. |
| Admission And Approval | The owner directed continued implementation with external emulator behavior/layout accepted as L2 on 2026-10-20, without repeated approval churn for absent L3 sources. |
| Objective | Establish one MyRaidenII Core owner for the selected board's 4 KiB live sprite RAM and its display-latched snapshot, then wire the real Board address and latch trigger to it. |
| Non-goals | No sprite decrypt/decode/draw, object attribute semantics, priority, frame publication, Shared change, ROM import, external payload, second memory path or generic device framework. |
| Reference Baseline | The Board currently leaves `0xC000–0xCFFF` as generic RAM and swallows `0x68E–0x68F` inside a stateless object-write aperture. Accepted L2 references describe sprite RAM and a buffer trigger at those ranges. |
| Candidate Proposal | Add one opaque Core sprite-buffer owner with live/latched storage, bounded read/write/latch/reset operations and a future-consumer snapshot copy. Board replaces the generic RAM range, routes the trigger, resets the owner and removes the trigger from its former stateless aperture. |
| Files And ABI Surface | `src/myraidenii/core/{sprite_buffer.c,sprite_buffer_interface.h,board.c,CMakeLists.txt}`, matching Core unit tests, evidence, Current and no Shared/App files. The Core-local opaque sprite-buffer interface is the only new API. |
| Applicable Rules | MyRaidenII guide, architecture, coding, documentation, source policy and execution rules. Board remains the sole bus owner; sprite buffer remains the sole live/latched data owner; tests use synthetic bytes only. |
| Verification | Passed focused sprite-buffer and Board bus smokes on x64/x86, then the complete four-entry MyRaidenII unit selection on both widths, documentation governance and `git diff --check`. No product EXE exists yet. |
| Expected Markers | Guest writes/readbacks at `0xC000–0xCFFF` use the Board-owned live RAM; a write at either byte of `0x68E–0x68F` copies live into the snapshot; later live writes do not mutate the snapshot; reset clears both. |
| Asset Needs | None. Tests allocate synthetic data only; external game media remains read-only and untouched. |
| Reporting Requirements | Report source/test/doc added/removed/net lines, the two adopted L2 facts, verification by width, Core-local API scope and deferred sprite work. |
| Stop Conditions | Required decrypt tables, object interpretation, source/table import, Shared API, real-ROM read or any independent rendering path transfers to a later bounded S. |
| Exit Criteria | Met: one owner retains both buffers; Board has one live-RAM route and one trigger route; prior stateless trigger handling is removed; unit regression proves copy isolation/reset and the two L2 map facts. |
| Original Owner Request | Continue MyRaidenII toward RAIDEN II using external implementations as L2 rather than treating absent physical L3 material as a recurring blocker. |
| Similar-Issue Sweep | Recheck Board route precedence/overlap, byte/word handling, reset/destruction, RAM fallback removal, snapshot lifetime and all former `0x68E` aperture claims. |

S43 was accepted after actual-change review of `6b5def499`: two owner-local
helpers read only safe CRTC word pairs and apply them to the existing layer
draw path. Synthetic regression proves enable, per-layer X/Y scroll and
modulo-512 source behavior; Board regression retains its public byte-route and
reset proof. Both cached widths passed all three MyRaidenII unit entries and
documentation governance. No external media, artifact, desktop or playable
game claim is made.

## Archived Packet — M1 T4 S44

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S44 follows coordinator-accepted S43 P2 `9468f4cca`; one session performs coordinator and executor roles. |
| Admission And Approval | The owner explicitly directs continued implementation using external emulator behavior/layout as L2 without further authority churn, 2026-10-20. |
| Objective | Correct the one Video graphics ingress so four immutable 2 MiB sprite lanes become their selected 32-bit little-endian 8 MiB stream for a later sprite owner. |
| Non-goals | No sprite decryption, object decode/draw, Board object semantics, priority/blending, Sprite flags interpretation, Shared change, real media read, or external source/table/ROM import. |
| Reference Baseline | Video currently copies the four sprite inputs consecutively into an 8 MiB allocation, while inspected L2 board layout establishes a four-lane 32-bit little-endian sprite region. No current reader depends on their incorrect contiguous-per-file arrangement. |
| Candidate Proposal | Retain one graphics allocation and one loader; interleave the four owned input lanes into the existing sprite-region capacity, name the stream layout explicitly, and extend the existing synthetic graphics smoke with a finite lane-order proof. |
| Files And ABI Surface | `src/myraidenii/core/{video.c,video.h}`, `test/myraidenii/unit/core/graphics_smoke.c`, L2 evidence and task record only; no external function/type ABI change. |
| Applicable Rules | MyRaidenII guide, architecture, coding, documentation and source policy; execution rules. Video retains sole graphics-copy ownership; synthetic tests use no external payload; L2 records only the finite lane organization. |
| Verification | Passed focused graphics/Board smoke on x64/x86; then passed all three cached MyRaidenII unit entries on each width, documentation governance and `git diff --check`. |
| Expected Markers | Every four-byte synthetic stream group preserves lane order and repeated source indices; character/background layouts remain unchanged; loader failure/release behavior remains one allocation/owner. |
| Asset Needs | None; tests allocate synthetic bytes only and do not access external ROM/media. |
| Reporting Requirements | Report source/test/doc added/removed/net lines, the finite L2 lane fact, verification per width, and deferred decrypt/object/render work. |
| Stop Conditions | Any needed decryption table/algorithm, actual sprite object protocol, Board API, Shared change, or external payload is deferred rather than guessed. |
| Exit Criteria | Met: the only sprite ingress is explicitly lane-interleaved; owning Video regression rejects the former file-concatenation layout; Board route regression, prior graphics ownership and all unit entries remain intact. |
| Original Owner Request | Continue implementing MyRaidenII toward RAIDEN II using external implementations as L2 rather than treating absent physical L3 sources as a repeated blocker. |
| Similar-Issue Sweep | Recheck all seven graphics-role offsets, allocation size/overflow, loader/release failures and consumers for duplicate sprite ingress or accidental interpretation. |

## Archived Packet — M1 T4 S43

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S43 follows coordinator-accepted S42 P3 `b1dd46f0f`; one session performs coordinator and executor roles. |
| Admission And Approval | The owner explicitly directed continued MyRaidenII implementation under external emulator L2 evidence without back-and-forth on 2026-10-20. |
| Objective | Make the existing Video composition path consume the finite CRTC active-low Background/Midground/Foreground/Text enables and three tile-layer scroll pairs. |
| Non-goals | No sprite decode/decryption/drawing, base-scroll behavior, text scroll, blend/priority behavior, Machine publication, Shared change, ROM/media input, or copied/transliterated source/table. |
| Reference Baseline | S42 P2 `37cba797a` establishes correct static map/palette/order ownership, while the existing CRTC register file is routed through Board but has no frame effect. |
| Candidate Proposal | Add owner-local CRTC word and enabled helpers; conditionally draw the four existing layers and apply modulo-512 source coordinates for the three 16 by 16 tile layers. Reuse the sole compositor, Board CRTC route, and existing smoke files. |
| Files And ABI Surface | `src/myraidenii/core/video.c`, its public comment only if required, `test/myraidenii/unit/core/{graphics_smoke.c,board_bus_smoke.c}`, L2 evidence, manifests and task record; no function/type ABI change. |
| Applicable Rules | MyRaidenII guide, architecture, coding, documentation and source policy; execution rules. Video remains the single raster/composition owner, Board remains the one CRTC route, and external evidence supplies only finite L2 behavior/layout facts. |
| Verification | Run focused graphics and Board tests on x64/x86; then cached x64/x86 MyRaidenII unit selections, manifests/dependency gates, documentation governance and `git diff --check`. |
| Expected Markers | Active-low layer masking leaves the frame clear when all tiles are disabled; each three scroll pairs shifts only its own layer; a Board bus write changes the same sole Video frame output. |
| Asset Needs | None; all regressions use synthetic buffers and do not read or modify external ROM/media. |
| Reporting Requirements | Report source/test/doc added/removed/net lines, the exact adopted L2 facts, verification per width, and deferred sprite/base-scroll/text-scroll/Machine work. |
| Stop Conditions | Any needed sprite/decryption/base-scroll contract, Shared API, new renderer, external data payload, or behavior beyond the named finite CRTC fields is transferred rather than guessed. |
| Exit Criteria | Met: one Video owner consumes the named enables/scroll pairs, Board retains the only public CRTC route, and synthetic regression proves the previous inert-register behavior cannot recur. Evidence is updated; this product has no file manifest. |
| Original Owner Request | Continue implementation toward RAIDEN II with external implementations admitted as L2, without treating the lack of a physical L3 source as a blocker. |
| Similar-Issue Sweep | Recheck all existing CRTC frame-affecting fields, Video/Board ownership, source-coordinate bounds, layer order/transparency, and public API changes for a second route or unapproved behavior. |

## Archived Packet — M1 T4 S42

| Field | Required record |
| --- | --- |
| Identifier Mode | Corrective continuation: M1 T4 S42 P2 corrects the unaccepted P1 tilemap/palette L2 mapping; one session performs coordinator and executor roles. |
| Admission And Approval | The owner approved MAME/other emulator implementations as L2 evidence and explicitly directed continued use without further back-and-forth on 2026-10-20. |
| Objective | Correct the four non-sprite tile-layer RAM ordering and palette bases in the sole MyRaidenII Video composition path, and make its owning smoke reject the former swapped mapping. |
| Non-goals | No sprite decoding/decryption, scroll/enable behavior, blending, rotation, Machine frame publication, external ROM data, copied reference source/table, or Shared change. |
| Reference Baseline | S42 P1 `6944777f4` publishes an initial 320 by 240 frame, but its Mid/Foreground tilemap offsets and palette bases disagree with the recorded L2 source layout. |
| Candidate Proposal | Retain one Video compositor; draw Background from 000h/400h palette, Midground from 800h/420h palette, Foreground from 400h/410h palette, and Text from c00h/700h palette. Strengthen the existing synthetic smoke with distinct entries/colours and transparent peel-down checks. |
| Files And ABI Surface | `src/myraidenii/core/video.c`, `test/myraidenii/unit/core/graphics_smoke.c`, S42 evidence and task record only; no public API/ABI changes. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source policy. The L2 implementation is independent, records revision/path facts only, and does not import or transliterate source, tables, ROM data or decryption material. |
| Verification | Run the owning Core graphics and Board tests x64/x86, static/manifest checks, then the required repository unit selections under the documented junction execution context. |
| Expected Markers | Distinct foreground/midground offsets and palette groups are exercised; transparent layers reveal the correct lower layer in the correct order; no alternative composition owner exists. |
| Asset Needs | None; tests use synthetic buffers only and external ROM/media remain untouched. |
| Reporting Requirements | Report source/test added/removed/net lines, exact L2 limitation, verification by width, and all deferred sprite/scroll/frame-publication work. |
| Stop Conditions | Any needed sprite decrypt source/table, ROM extraction, Shared API change, or behavior beyond finite L2 layer mapping is transferred rather than guessed. |
| Exit Criteria | Met: the sole tile composition path conforms to the recorded L2 map/order/palette facts; the regression rejects the former swap; evidence and manifests are updated. |
| Original Owner Request | Continue MyRaidenII implementation with external emulator behavior/layout as approved L2, without repeatedly requesting that authority. |
| Similar-Issue Sweep | Recheck all four tile layers for one owner, source/test mapping agreement, bounds, transparent-pen behavior and absence of copied source/table/decryption material. |

## Archived Packet — M1 T4 S41

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S41 follows accepted S40 P2 `91f7f76fa`; one session performs coordinator and executor roles. |
| Admission And Approval | The owner explicitly authorizes MAME/other emulator implementations as L2 references. Source policy forbids copying/transliterating their source, tables, ROM data or decryption material. |
| Objective | Derive and independently implement the first bounded Core graphics decode/composition contract from recorded L2 source facts, then qualify it with synthetic ROM-free vectors. |
| Files And ABI Surface | MyRaidenII Core video/graphics, its owning unit test, evidence, manifest and task records only. No Shared, Product, external asset or ROM change. |
| Verification | Record source revision/path and adopted finite facts; run x64/x86 focused Core video tests plus the complete repository-only unit suite before S closure. |
| Exit Criteria | Met: the independently implemented character/background decoder passes synthetic regression and records its L2 scope; no reference source/table/ROM payload entered the repository. |

## Archived Packet — M1 T4 S40

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S40 follows accepted S39 P2 `9add13000`; one session performs coordinator and executor roles. |
| Admission And Approval | The owner approved bounded L2 behavioral modeling with MAME as a black-box reference. MAME's public Lua documentation identifies `emu.pause()` and video frame updates as session control; no reference implementation/source is read. |
| Objective | Determine whether a paused external reference can render two synthetic graphics states without CPU/frame animation, yielding a finite repeatable element coordinate/value observation. |
| Non-goals | No MAME/FPGA source or debugger-memory inspection, no ROM/RAM/register/pixel payload retention, screenshot, trace, decoder/render/API/test/artifact change, decryption claim or playable-game claim. |
| Reference Baseline | S40's initial paused repaint produced no byte effect; its advancing mutation/control pair proves the earlier S37 screen difference was time advancement. S39 leaves no original layout source. MAME documents public pause/session and screen/video Lua capabilities. |
| Candidate Proposal | At a bounded post-startup frame, pause the reference, replace only its external tile region with a synthetic state, call the public video update, hold the baseline only in process, toggle one synthetic byte, call video update again, and print only dimensions, difference count/bounds and a checksum. |
| Files And ABI Surface | Documentation/evidence only; one ignored temporary script under a unique `build/` path. No repository production/test/manifest/API/shared/artifact or external media changes. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source policy. MAME stays black-box only; all mutations occur in its process and no protected payload leaves it. |
| Verification | Run two identical paused-mutation trials and one no-mutation control; prove the callback/termination remains bounded; retain only declared neutral summary; delete all temporary script/output; run documentation governance and `git diff --check`. |
| Expected Markers | Either a finite stable element coordinate/value fact or an exact documented reason pause/frame-update cannot yield one under containment. |
| Asset Needs | Existing owner-supplied external archive and S35 external PALs, read-only; generated MAME configuration remains only under `nxvm-assets/media-myraidenii/raiden2/mame-config/`. |
| Reporting Requirements | Record MAME version/API class, pause/update outcome, neutral dimensions/checksum/bounds, repeatability and cleanup only. No raw pixel, ROM, RAM, register, trace or implementation content. |
| Stop Conditions | Pause/update needs debugger/source inspection, a saved state, protected payload retention, MAME modification/plugin, archive change, unbounded run, or non-repeatable output. |
| Exit Criteria | A complete bounded element contract is documented for a decoder S, or a truthful blocked field set is transferred without any production source claim. |
| Original Owner Request | Continue MyRaidenII under the approved L2 model and external BYOB assets until RAIDEN II can be run, rather than stopping after a planning-only result. |
| Similar-Issue Sweep | Confirm no temporary script/log/NVRAM/snapshot persists and no second graphics owner, decoder implementation, MAME-derived source/table or protected payload reaches repository state. |

## Archived Packet — M1 T4 S39

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S39 follows accepted S38 P2 `b32ce2ed9`; one session performs coordinator and executor roles. |
| Admission And Approval | The product source policy permits original NEC/Zilog/Yamaha/OKI manuals and board observations as research. This packet does not import, download, commit or derive executable source from any reference. |
| Objective | Locate original/vendor board documentation or independently recorded physical measurement that specifies a finite Raiden II graphics-element layout/attribute contract suitable for an L2 Core decoder admission. |
| Non-goals | No MAME/FPGA/reference-source inspection, no decoder/render implementation, no third-party code import, no ROM/manual payload retention, no source/test/API/artifact change and no playable-game claim. |
| Reference Baseline | S38 proves the existing no-payload MAME black-box method cannot isolate a finite element coordinate result. S40 later corrects S37's unpaired causal attribution. |
| Candidate Proposal | Search only primary/archival metadata and public original-board documentation indexes. Retain URLs, publisher/holder, document identity, license/access facts and only the finite hardware facts directly needed for a later proposal. |
| Files And ABI Surface | `docs/myraidenii/{states,etc/evidence}` only. No product, Core, shared, asset, test, API, manifest or external-media changes. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source policy. Original documentation remains external research unless an independent acquisition/usage decision is later approved. |
| Verification | Recheck provenance and whether each result is original/vendor material rather than derived emulator material; record positive evidence or the exact evidence gap; run documentation governance and `git diff --check`. |
| Expected Markers | A finite provenance-qualified graphics contract, or a documented lack of one and explicit transfer boundary. |
| Asset Needs | None. Existing external game/PAL inputs are out of scope and untouched. |
| Reporting Requirements | Record only source identity, ownership/provenance, URL/access, permitted factual scope and limitations. Do not retain a copyrighted manual, screenshots, ROM content, MAME/FPGA implementation detail or raw game data. |
| Stop Conditions | A candidate is derivative/emulator-source based, access requires acquisition without policy approval, the document lacks element-layout facts, or a result would require source/table/transliteration. |
| Exit Criteria | A finite independently sourced contract is ready for a later decoder S, or the unresolved evidence field is transferred without a production claim. |
| Original Owner Request | Continue MyRaidenII under the approved L2 model and external BYOB assets until RAIDEN II can be run, rather than stopping after a planning-only result. |
| Similar-Issue Sweep | Confirm no local research payload, external tool configuration, MAME artifact, decoder implementation, second graphics owner or non-approved reference-derived content is introduced. |

## Archived Packet — M1 T4 S38

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S38 follows accepted S37 P2 `6eb554da4`; one session performs coordinator and executor roles. |
| Admission And Approval | The owner authorized M1/T4 L2 behavioural modeling and external MAME as a black-box research reference. This is a same-T bounded continuation; existing source-policy prohibitions remain in force. |
| Objective | Determine whether controlled synthetic tilemap, palette and graphics mutations can establish a finite coordinate/value graphics-element oracle sufficient to specify one initial Core decoder contract. |
| Non-goals | No MAME source/table/handler import, reading/dumping/retaining ROM or memory content, screenshots, raw traces, production/test/API/artifact change, layer/sprite composition, decryption or playable-game claim. |
| Reference Baseline | S31 fixed the seven image roles; S32 established L2 RGB555 lookup; S36 fixes 320×240; S37 proves a graphics-region mutation changes later visible output reproducibly. |
| Candidate Proposal | In a transient MAME Lua run, install only synthetic in-process graphics state and one known graphics-bit pattern, then retain only a bounded set of pixel coordinates/colours or an all-screen neutral summary. Repeat each admitted observation twice and delete all script/output files. MAME uses the external `nxvm-assets/media-myraidenii/raiden2/mame-config/` directory explicitly; it must not create repository-root configuration. |
| Files And ABI Surface | Documentation/evidence only; one ignored temporary script under a unique `build/` path. No repository production/test/manifest/API/shared/artifact or external media changes. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source policy. MAME is black-box only; all mutations occur in its process and no protected bytes or pixel payload leave it. |
| Verification | Prove synthetic isolation by repeatable observations with and without the selected mutation; verify only declared coordinates/colours/checksums are printed; delete all temporary data; run documentation governance and `git diff --check`. |
| Expected Markers | Either a finite replicated element-layout fact table, or an exact reason a coordinate oracle cannot be obtained under containment rules. |
| Asset Needs | Existing owner-supplied external archive and S35 external PALs, read-only; generated external MAME configuration is stored only under `nxvm-assets/media-myraidenii/raiden2/mame-config/`. |
| Reporting Requirements | Record only MAME version/API class, synthetic state classes, neutral coordinates/colours/checksums, repeatability and cleanup. No raw address-to-content dump, image, ROM byte, memory dump or MAME implementation detail. |
| Stop Conditions | The experiment needs source inspection, protected payload retention, an archive change, an unbounded observation, a MAME modification/plugin or non-repeatable output. |
| Exit Criteria | A complete bounded element contract is documented for a decoder S, or a truthful blocked field set is transferred without any production source claim. |
| Original Owner Request | Continue MyRaidenII under the approved L2 model and external BYOB assets until RAIDEN II can be run, rather than stopping after a planning-only result. |
| Similar-Issue Sweep | Confirm no temporary script/log/NVRAM/snapshot persists and no second graphics owner, decoder implementation, MAME-derived source/table or protected payload reaches repository state. |
| Later Correction | S40's repeated no-mutation control proves that S37's later checksum difference was ordinary reference-frame advancement, not established graphics-region sensitivity. |

## Archived Packet — M1 T4 S37

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S37 follows accepted S36 P2 `b6b80ccda`; one session performs coordinator and executor roles. |
| Admission And Approval | The owner approved continued M1/T4 implementation under L2 behavioural evidence, MAME as a black-box reference only, and external BYOB PAL inputs. |
| Objective | Derive a bounded reproducible L2 graphics-element contract from controlled external MAME runtime mutation, or record the exact missing element without guessing a decoder. |
| Non-goals | No MAME source/table/handler import, ROM/image dump or retained payload, screenshot, raw trace, source/test/API/artifact change, renderer/frame publication, decryption claim or playable-game claim. |
| Reference Baseline | S31 owns fixed-role image ingress; S32 owns RGB555 lookup; S36 fixes the external 320×240 screen checkpoint. MAME's published Lua interface exposes memory-region writing and screen observation. |
| Candidate Proposal | Mutate only synthetic in-process bytes in an external reference and retain only neutral dimensions/checksums; use the result to decide whether a decoder contract is actually established. |
| Files And ABI Surface | `docs/myraidenii/{states/CURRENT.md,etc/evidence/}` plus a transient ignored build script only. No source, test, manifest, shared component, asset or executable changes. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source policy. MAME remains research-only, external inputs read-only, and protected payload is never retained. |
| Verification | Run two identical bounded trials, verify official Lua API capability, remove the transient script/output, run documentation governance and `git diff --check`. |
| Expected Markers | A finite L2 fact table or a precise missing-contract transfer; no production-behaviour claim. |
| Asset Needs | Existing owner archive and S35's external PAL files, read-only and untracked. |
| Reporting Requirements | Record tool/API class, synthetic mutation class, neutral repeatability and cleanup; never record raw memory/pixel/ROM content. |
| Stop Conditions | Source inspection, retained protected data, archive mutation, non-repeatability, unbounded trace, or a claim stronger than L2 is required. |
| Exit Criteria | Evidence establishes a complete bounded graphics-element contract, or truthfully records the exact unresolved element with all transient data removed. |
| Original Owner Request | Continue MyRaidenII under owner-approved L2 evidence and external BYOB PAL setup toward running RAIDEN II. |
| Similar-Issue Sweep | Confirm no MAME configuration/NVRAM/snapshot/log/script survives and no second graphics owner, renderer or source-derived layout enters the repository. |

## Archived Packet — M1 T4 S36

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S36 follows accepted S35 P2 `4744a53e4`; one session performs coordinator and executor roles. |
| Admission And Approval | The owner approved continuing implementation under L2 behavioral evidence and explicitly authorized external BYOB PAL inputs. This packet uses MAME's documented Lua black-box API only; it imports no source, table, handler or protected payload. |
| Objective | Establish one reproducible, neutral selected-set screen checkpoint: visible dimensions plus a non-reversible byte-stream checksum at a declared emulated frame. |
| Non-goals | No screenshot, pixel dump, raw trace, ROM/memory/register capture, decoder/layout/decryption implementation, renderer/frame, shared change, product artifact or playable-game claim. |
| Reference Baseline | S35 makes current MAME 0.289 accept the owner archive plus the two external PAL files. MAME documents `-autoboot_script`, `screen:pixels()` and `machine:exit()` as public scripting facilities. |
| Candidate Proposal | Create one transient Lua file outside the repository, wait for a fixed video-frame count, compute Adler-32 over the returned visible-pixel bytes, print only tag/width/height/checksum/frame count, schedule exit, retain those neutral values and delete the script. |
| Files And ABI Surface | `docs/myraidenii/{states/CURRENT.md,etc/evidence/}` only. No source, test, manifest, API, asset, archive or executable changes. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source policy. The probe is black-box, bounded and deletes all transient output; BYOB PALs remain external/untracked. |
| Verification | Run the probe twice with identical current-MAME options and external paths; require same dimensions/checksum and a bounded clean exit each time. Confirm the Lua file/output are deleted; run documentation governance and `git diff --check`. |
| Expected Markers | A stable selected screen tag, dimensions and checksum at the exact declared frame; no retained pixel data and no repository asset changes. |
| Asset Needs | Existing owner archive plus S35's owner-approved external PAL files, all read-only. |
| Reporting Requirements | Record MAME version, command class, frame number, neutral metadata/checksum, repeatability, cleanup and limitations. Do not retain payload, pixels, screenshots or memory state. |
| Stop Conditions | Lua API cannot yield a stable visible frame, output differs between identical runs, probe needs protected content capture, or any proposed decoder/layout claim would exceed neutral black-box evidence. |
| Exit Criteria | A repeatable, cleaned-up neutral screen checkpoint or an exact documented failure with no code changes; any decoder work remains a separate reviewed S. |
| Original Owner Request | Continue MyRaidenII after PAL acquisition and connect it through compilation/runtime evidence. |
| Similar-Issue Sweep | Confirm no MAME config/NVRAM/snapshot/log or script remains in repository/assets; verify MAME reads both external roots only and no raw data enters task evidence. |

## Archived Packet — M1 T4 S35

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S35 follows accepted S34 P2 `bce13d2b8`; one session performs coordinator and executor roles. |
| Admission And Approval | The owner explicitly authorizes downloading the two required PAL images as external BYOB inputs, placing them only under `O:\repos.hobby\nxvm-assets`, without Git/manifest/EXE inclusion. This is a recorded exception to the default MyRaidenII source-policy acquisition restriction. |
| Objective | Acquire, hash-verify and stage only the PAL images required by current MAME's Raiden II verifier, then prove the current black-box runner accepts the existing owner archive plus those external inputs. |
| Non-goals | No source import, decoder/renderer/frame implementation, ROM/archive commit, asset manifest inclusion, EXE embedding, modification of existing owner archive, screenshot/trace retention, or playable-game claim. |
| Reference Baseline | Current MAME 0.289 requests `jj4b02__ami18cv8-15.u0342` (341 bytes, SHA-1 `8b46f6673ddf11efbc3394ae423ec89d4a1283bf`) and `jj4b01__mmipal16l8bcn.u0341` (279 bytes, SHA-1 `95ce9cfbfb280dfc6a326e378684eff3c6f54701`). S34 established that historical MAME can execute without the first dump. |
| Candidate Proposal | Download a published split-set archive to a temporary external staging path, extract exactly the two named PAL files, verify their size/SHA-1, delete the temporary archive, then run current MAME verification and one bounded headless launch with the existing archive plus the staged PAL directory. |
| Files And ABI Surface | External BYOB assets under `O:\repos.hobby\nxvm-assets\media-myraidenii\raiden2\roms\raiden2`; repository documentation/evidence only. No source, test, manifest, API, asset archive or executable changes. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source policy, with the owner-approved external BYOB acquisition exception recorded above. Inputs remain external and untracked. |
| Verification | Verify exact filenames, sizes and SHA-1; confirm the staging archive is deleted; run current MAME `-verifyroms` and one bounded headless launch against combined external paths; run documentation governance and `git diff --check`. |
| Expected Markers | Both exact hashes match; current MAME no longer reports missing PALs; headless process terminates within budget; Git has no PAL/ROM additions. |
| Asset Needs | Owner-approved external PAL inputs only; the existing archive stays read-only and unmodified. |
| Reporting Requirements | Record source URL, exact hashes, external location, verifier/launch result and cleanup. Do not retain or report protected payload content. |
| Stop Conditions | Hash/size mismatch, extra extracted media, need to modify the owner archive, a request to commit/embed inputs, or evidence that current MAME cannot combine the staged files with the archive. |
| Exit Criteria | Current MAME accepts the two external PALs and completes its bounded black-box launch, or the exact combined-path limitation is documented without a repository workaround. |
| Original Owner Request | Download the PAL into nxvm-assets as BYOB, then compile/connect it. |
| Similar-Issue Sweep | Verify no temporary archive remains, no PAL/ROM is staged under the repository, no existing asset is overwritten and no protected input is added to Git. |

## Archived Packet — M1 T4 S34

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S34 follows accepted S33 P2 `3e708f2af`; one session performs coordinator and executor roles. |
| Admission And Approval | The owner asked to source the missing PAL from MAME and continue. Source policy forbids downloading protected game/PAL data, so this packet uses only an official historical MAME binary as a transient black-box research tool. |
| Objective | Establish whether the existing owner archive can supply a lawful executable black-box baseline without obtaining either missing PAL dump. |
| Non-goals | No PAL/ROM download, archive mutation, source/table/handler import, graphics decoder, renderer/frame, shared change, screenshot/trace retention, playable-game claim or product artifact. |
| Reference Baseline | S33 used current MAME 0.289, whose strict verifier rejected two PAL blobs. The owner archive remains read-only at its recorded identity. |
| Candidate Proposal | Download the official MAME 0.182 Windows binary outside the repository; verify its identity, run `-verifyroms` and one bounded headless launch against the existing archive; retain only neutral status/speed observations. |
| Files And ABI Surface | `docs/myraidenii/{states/CURRENT.md,etc/evidence/}` only. No source, test, manifest, API, asset or executable changes. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source policy. MAME is research-only; protected inputs are read-only and no output payload is retained. |
| Verification | Verify the downloaded file is a PE, record its SHA-256, run official MAME `-verifyroms` and one ten-second headless launch, then remove the external tool directory after retaining neutral evidence. Run documentation governance and `git diff --check`. |
| Expected Markers | Historical verifier calls the set `best available` despite the no-good PAL; headless process terminates within its ten-second budget; no PAL is acquired. |
| Asset Needs | Existing owner-supplied external archive only; no new asset is accepted or stored. |
| Reporting Requirements | Record source, tool hash, commands, neutral outcome, cleanup and precise limit: executable behavior does not establish a decoder/layout contract. |
| Stop Conditions | Any need to download or preserve PAL/ROM data, inspect/copy third-party implementation, retain protected video/audio output, make decode claims from a source table, or modify production code without a separate contract. |
| Exit Criteria | A reproducible lawful black-box baseline or documented failure; archive S34 with the correction and transfer only the still-needed layout/behavior contract to later Video work. |
| Original Owner Request | Search/download missing PAL into external assets and compile/connect it. |
| Similar-Issue Sweep | Check that no tool, temporary configuration, archive output, screenshot, trace or downloaded protected media remains under the repository or external asset root. |

## Archived Packet — M1 T4 S33

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S33 follows accepted S32 P2 `fb7c3f5a4`; one session performs coordinator and executor roles. |
| Admission And Approval | The owner authorized L2 behavior modeling when L3 primary hardware detail is unavailable. This packet permits an external emulator only as a transient black-box reference; it does not authorize source/table/handler import. |
| Objective | Establish whether a lawful, reproducible behavioral-oracle workflow can qualify later graphics layout/decryption work without exposing protected game bytes or copying third-party implementation. |
| Non-goals | No MyRaidenII production/test change, graphics decode, decryption, renderer/frame, shared change, ROM commit, screenshot/trace retention or playable-game claim. |
| Candidate Proposal | Obtain one official upstream emulator binary outside the repository, pin its release identity/hash, run it only against the owner-supplied archive with a bounded controlled launch, and retain only neutral observations/checkpoints. If its behavior cannot produce an independent layout contract, record that exact stop rather than invent a decoder. |
| Files And ABI Surface | `docs/myraidenii/{states/CURRENT.md,etc/evidence/}` only. No source/API/manifest/asset/product artifact change. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source policy. External emulator remains research-only, all protected media read-only, and neither bytes nor derived decode tables are retained. |
| Verification | Verify source identity/hash, bounded-process cleanup, no retained protected outputs, documentation governance and `git diff --check`. No claim that reference execution proves hardware accuracy. |
| Stop Conditions | Need to inspect/copy/provide third-party implementation or ROM bytes, make a decoder claim from a source table, commit the external tool/output, or change production code without a separately reviewed contract. |
| Exit Criteria | A recorded, reproducible black-box workflow with a clear next contract or a documented insufficiency that leaves production unchanged. |
| Original Owner Request | Continue MyRaidenII under the authorized L2 model boundary rather than require unavailable L3 proof. |
| Similar-Issue Sweep | Check output retention, archive write access, process/temporary cleanup, output paths, license/source identity and whether any proposed fact is actually independent behavior rather than a copied source assertion. |

## Archived Packet — M1 T4 S32

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S32 follows accepted S31 P2 `97361495e`; one session performs coordinator and executor roles. |
| Admission And Approval | The owner authorized continued MyRaidenII implementation and L2 models where primary hardware detail is unavailable. This packet changes only MyRaidenII Core/tests/documentation. |
| Objective | Give the existing 2048-word private Video palette one bounded RGB32 interpretation for a later native-frame owner. |
| Non-goals | No graphics ROM decode, tile/sprite layout, decryption, brightness, priority, scroll, frame publication, Machine/Product route, Shared change or playable-game claim. |
| Reference Baseline | S25 admits a complete 2048-word palette image without channel interpretation. Shared accepts 0x00RRGGBB frames, but MyRaidenII has no native frame yet. |
| Candidate Proposal | Add one Video-local checked palette lookup that interprets the stored 15 low bits as L2 RGB555, expands each 5-bit component to 8 bits and returns 0x00RRGGBB. Keep raw palette ownership and failed-load preservation unchanged. |
| Files And ABI Surface | `src/myraidenii/core/{video.c,video.h}`, existing Board smoke, CURRENT and one evidence record. No Shared/public component API, asset, ROM, Product or Machine change. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source-policy rules. The mapping remains L2, one Video owner and no imported source/table/algorithm. |
| Verification | Focused x64/x86 Board smoke with synthetic palette vectors and bounds/argument failure; existing selected-set probe; documentation governance and `git diff --check`. |
| Expected Markers | Zero, individual primary extremes and a mixed synthetic RGB555 word produce exact RGB32 values; invalid index/pointer leaves caller output unchanged; no frame is published. |
| Asset Needs | None. Tests use synthetic palette words only; no external archive access, extraction or content observation. |
| Reporting Requirements | Record the L2 source/limits, exact bit/expansion rule, failed-input behavior, actual diff and verification; state that this does not establish selected-board palette hardware accuracy. |
| Stop Conditions | Need to import a reference layout/table/handler, infer brightness/alpha, touch Shared, create a renderer/frame path, inspect protected media or claim exact board colour. |
| Exit Criteria | One reviewed MyRaidenII P with bounded conversion proof, dual-width focused qualification, evidence and cleanup; otherwise revert the whole interpretation rather than add alternate color paths. |
| Original Owner Request | Continue implementing MyRaidenII under the repository governance and permitted L2 source boundary. |
| Similar-Issue Sweep | Check raw palette load/reset, all color extraction shifts, output alpha, index bounds, caller output preservation and existing RGB32 frame conventions so no duplicate palette converter or format appears. |

## Archived Packet — M1 T4 S31

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S31 follows accepted S30 P2 `0860cdeee`; one session performs coordinator and executor roles. |
| Admission And Approval | The owner authorized continued MyRaidenII implementation and bounded L2 models without source import. This packet changes only MyRaidenII Core and its owned unit tests/documentation. |
| Objective | Establish the one Board-to-Video ingress for the selected set's immutable character, background and sprite image roles, with exact owner-supplied inventory sizes and lifetime rules. |
| Non-goals | No ROM extraction, protected media commit, decoder/decryption algorithm, tile/sprite semantics, frame publication, Machine/Product implementation, Shared change, renderer, or playable-game claim. |
| Reference Baseline | S30 reaches its containment budget with no remaining selected control-window fault. The audited inventory identifies one 128 KiB character image, two 2 MiB background images and four 2 MiB sprite images; Core currently accepts only program lanes. |
| Candidate Proposal | Extend the existing Board creation config with one fixed-role immutable image bundle. Validate a supplied bundle is complete; Video privately copies the roles so the caller may release staging data after creation, matching the existing program-lane lifetime. |
| Files And ABI Surface | `src/myraidenii/core/{board.c,board_interface.h,graphics_interface.h,video.c,video.h}`, existing Core Board smoke, CURRENT and one evidence record. This is an App-owned Core API extension only; no Shared API, asset, external archive or product artifact changes. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source-policy rules. Preserve Core's inward dependencies, one Video asset owner, copied/lifetime-safe public input and `lib_status` failures. |
| Verification | x64/x86 Board smoke for exact input validation and copy/lifetime; existing selected-set probe to prove no program/control regression; documentation governance and `git diff --check`. |
| Expected Markers | Missing, mismatched or partial image roles reject creation without a Board; accepted synthetic role images remain owned by Video after caller staging is changed/released; no frame, decoder or external media is retained. |
| Asset Needs | Existing inventory metadata only. Tests create synthetic in-memory role images and never open, extract, retain or inspect the owner-supplied archive. |
| Reporting Requirements | Record the role/size/lifetime contract, one-owner proof, model grade/limits, actual diff and verification; explicitly state that asset ingress is not graphics decoding or output. |
| Stop Conditions | Need to inspect/copy protected ROM content, infer graphics layout/decryption, add a generic asset registry, create a second state owner, touch Shared, or publish an incomplete frame. |
| Exit Criteria | One reviewed MyRaidenII P with exact role validation and copied-lifetime proof, dual-width focused qualification, evidence and cleanup; otherwise abandon the whole ingress change without placeholder compatibility. |
| Original Owner Request | Continue implementing MyRaidenII under the repository governance and permitted L2 source boundary. |
| Similar-Issue Sweep | Board creation rollback, program/image role validation, Video reset/destroy, caller lifetime, every role's exact size and test fixtures were checked; no raw external asset is retained or another owner is introduced. |

| S | Delivered scope | Receipt |
| --- | --- | --- |
| S1 | Finite Video/Sprite convergence ledger; no invented receiver admitted without a lawful flag-family consumer contract. | P1 `ac0872270`; coordinator review: documentation governance and `git diff --check` passed. |
| S2 | Lawful-source disposition: existing sources cannot establish the sprite-control consumer contract, so code remains correctly blocked. | P1 `f67876e8e`; coordinator review: documentation governance and `git diff --check` passed. |
| S3 | Public FPGA implementation provenance review; it is MAME-derived/GPL and supplies no independent Video/Sprite contract. | P1 `ec71c2227`; coordinator review: documentation governance and `git diff --check` passed. |
| S4 | Owner-authorized L2 sprite-flags latch from reference-observed behavior; no imported implementation or claimed decode/render effect. | P1 `2276bbe2c`; selected-set x64/x86 both advance past the prior flags stop; complete repository units pass 669/669 on x64 and x86. |
| S5 | Selected-set-observed `0x696--0x697` Board-local write-only L2 hole, independently bounded from Video/Sprite state. | P1 `bc67b7bbc`; x64/x86 Board and selected-set probes pass; post-change repository-only units pass 642/642 per width; [L2 evidence](../etc/evidence/t4-s5-selected-control-hole-l2.md). |
| S6 | Selected-set-observed `0x69c--0x69d` Board-local write-only L2 hole, independently bounded from Video/Sprite state. | P1 `e317a10b8` accepted by coordinator review; x64/x86 Board and selected-set probes pass; post-change repository-only units pass 642/642 per width; [L2 evidence](../etc/evidence/t4-s6-selected-control-hole-l2.md). |
| S7 | Selected-set-observed `0x692--0x693` Board-local write-only L2 hole, independently bounded from Video/Sprite state. | P1 `f4ebdcf4f` accepted by coordinator review; x64/x86 Board and selected-set probes pass; post-change repository-only units pass 642/642 per width; [L2 evidence](../etc/evidence/t4-s7-selected-control-hole-l2.md). |
| S8 | Exact selected-set-observed `0x69a--0x69b` Board-local write-only L2 hole. | P1 `fa34720df` accepted by coordinator review; x64/x86 focused Board and selected-set probes advance to the independent next stop `0x690`; [L2 evidence](../etc/evidence/t4-s8-selected-control-hole-l2.md). |
| S9 | Exact selected-set-observed `0x690--0x691` Board-local write-only L2 hole. | P1 `b576fcae5` accepted by coordinator review; x64/x86 focused Board and selected-set probes reach the separate COP-owned `INVALID_STATE` boundary; [L2 evidence](../etc/evidence/t4-s9-selected-control-hole-l2.md). |
| S10 | Bounded Board-owned COP failure attribution for the selected set. | P1 `6a79f1102`; x64/x86 probes both identify command `0x5205`, first failed DMA address `0x6a0`, and existing `INVALID_STATE`; complete repository-only units pass 642/642 on each width. [Evidence](../etc/evidence/t4-s10-cop-failure-attribution.md). |
| S11 | L2 sprite-protection upload aperture from independently observed reference address ownership. | P1 `a9c048fd9`; x64/x86 focused Board and selected-set probes reach the separate Video/CRTC receiver at `0x600`; complete repository units pass 642/642 per width. [Evidence](../etc/evidence/t4-s11-sprite-protection-upload-l2.md). |
| S12 | L2 Video-owned CRTC register aperture from independently observed reference address ownership. | P1 `9042cdd35`; x64/x86 Board and selected-set probes advance from `0x600` to `0x640`; complete repository units pass 642/642 per width. [Evidence](../etc/evidence/t4-s12-crtc-register-aperture-l2.md). |
| S13 | L2 CRTC write-only extension at the exact next selected-set stop. | P1 `22d47c4b0`; P2 `24546816b`; x64/x86 focused Board tests and selected-set probes advance from `0x640` to the next independent stop `0x658`; complete repository-only units pass 669/669 on each width. [Evidence](../etc/evidence/t4-s13-crtc-write-extension-l2.md). |
| S14 | Bounded attribution of the selected set's first unowned control-window access at `0x658`. | P1 `f8fc5f867` accepted by coordinator review: x64/x86 focused Board tests and selected-set probes identify the same one-byte write while preserving `INVALID_STATE`; all 642 repository unit cases pass per width when the known temporary-output-sensitive DAG negative case is executed separately from the other 641 cases. [Evidence](../etc/evidence/t4-s14-control-fallback-attribution.md). |
| S15 | Exact selected-set word-write receiver at `0x658--0x659`, separately owned from CRTC. | P1 `0392b97e2` accepted by coordinator review: x64/x86 focused Board tests and selected-set probes accept only the complete word span, then reach the same independent next byte-write stop at `0x65a`; all 642 repository unit cases pass per width when the temporary-output-sensitive DAG negative case is executed separately from the other 641 cases. [Evidence](../etc/evidence/t4-s15-object-write-sink-l2.md). |
| S16 | Exact selected-set word-write receiver at `0x65a--0x65b`, separately owned from CRTC and S15. | P1 `84f3023fb` accepted by coordinator review: x64/x86 focused Board tests and selected-set probes accept only either exact word span, reject the cross-pair without RAM fallback, then reach the same independent next byte-write stop at `0x65c`; all 642 repository unit cases pass per width when the temporary-output-sensitive DAG negative case is executed separately from the other 641 cases. [Evidence](../etc/evidence/t4-s16-object-write-sink-l2.md). |
| S17 | Exact selected-set word-write receiver at `0x65c--0x65d`, separately owned from CRTC and S15--S16. | P1 `a815a6a0a` accepted by coordinator review: x64/x86 focused Board tests and selected-set probes accept only the three exact word pairs, reject cross-pairs without RAM fallback, then reach the same independent next byte-write stop at `0x65e`; all 642 repository unit cases pass per width when the temporary-output-sensitive DAG negative case is executed separately from the other 641 cases. [Evidence](../etc/evidence/t4-s17-object-write-sink-l2.md). |
| S18 | Source-bounded L2 replacement of the incremental pairs with the complete unowned object write aperture `0x650--0x69f`. | P1 `cdd761ac0`; x64/x86 Board smoke and selected-set probes pass; complete repository unit suites pass 642/642 per width when the temporary-output-sensitive DAG negative case is executed separately. [Evidence](../etc/evidence/t4-s18-object-aperture-l2.md). |
| S19 | Selected-set-observed tile-bank-2 Board register at `0x470--0x471`. | P1 `7258833b8`; x64/x86 Board smoke passes, both selected-set probes advance to `0x6cc`, and the full 642-case unit qualification passes per width. [Evidence](../etc/evidence/t4-s19-tile-bank-2-l2.md). |
| S20 | Selected-set-observed tile-bank-01 latch at `0x6cc` plus adjacent `0x6cd` no-op. | P1 `229b564bb`; P3 `8ad61e876` corrects the overwide latch. x64/x86 Board smoke passes, both selected-set probes advance to `0x6c4`, and the full 642-case unit qualification passes per width. [Evidence](../etc/evidence/t4-s20-tile-bank-01-l2.md). |
| S21 | Selected-set-observed sprite-protection constant write span at `0x6c4--0x6c5`. | P1 `448691874`; x64/x86 Board smoke passes, both selected-set probes advance to `0x6c0`, and the full 642-case unit qualification passes per width. [Evidence](../etc/evidence/t4-s21-sprite-protection-constant-l2.md). |
| S22 | Bounded sprite-protection register group at `0x6c0--0x6c3` and `0x6c6--0x6c7`. | P1 `6d0246f46`; x64/x86 Board smokes and selected-set probes pass; complete repository unit suites pass 642/642 per width. [Evidence](../etc/evidence/t4-s22-sprite-protection-registers-l2.md). |
| S23 | Attribute the selected-set's rejected COP DMA trigger before considering any new DMA mode. | P1 `bcbaeeeac`; x64/x86 probes identify rejected raw mode `0x0015` at `0x6fc`; final complete units pass 642/642 per width. [Evidence](../etc/evidence/t4-s23-cop-dma-trigger-attribution.md). |
| S24 | Record the selected DMA context in the existing COP failure observation. | P1 `bf3e3f396`; x64/x86 probes identify source `0x07c0`, size `0x00ff` and destination `0x0000` for rejected mode `0x0015`; final complete units pass 642/642 per width. [Evidence](../etc/evidence/t4-s24-cop-dma-context-attribution.md). |
| S25 | L2 fixed mode-`0x0015` palette receiver from permitted research and the bounded owner-set descriptor. | P1 `f96557a77`; x64/x86 probes advance to separate mode `0x0014`; complete repository unit suites pass 642/642 per width. [Evidence](../etc/evidence/t4-s25-palette-dma-l2.md). |
| S26 | Capture the selected mode-`0x0014` relative-address setup in the existing COP failure observation. | P1 `0c692e3b5`; x64/x86 probes identify `relative=0`; complete repository unit suites pass 642/642 per width. [Evidence](../etc/evidence/t4-s26-tilemap-dma-context-attribution.md). |
| S27 | L2 fixed mode-`0x0014` tilemap receiver from its selected bounded descriptor. | P1 `63b7c68bc`; x64/x86 Board smokes and owner-set probes advance to an independent Board-control stop; complete repository unit suites pass 642/642 per width. [Evidence](../etc/evidence/t4-s27-tilemap-dma-l2.md). |
| S28 | L2 exact selected-set `0x6ca` write-only no-op. | P1 `d62e0deee`; P2 `7efca76c0`; x64/x86 Board smoke and selected-set probes advance to `0x6d8`; full 642-case unit qualification passes per width. [Evidence](../etc/evidence/t4-s28-control-noop-l2.md). |
| S29 | L2 Sprite-protection X/Y/max-X/source endpoint group. | P1 `6ebe6cd9c`; x64/x86 Board smoke and selected-set probes pass the former `0x6d8` stop and reach `0x762`; full 668-case qualification and the separately run dependency-DAG negative pass per width. [Evidence](../etc/evidence/t4-s29-sprite-protection-latches-l2.md). |
| S30 | L2 Sprite-protection destination read endpoint. | P1 `532ef5a8f`; x64/x86 Board smoke and selected-set probes pass the former `0x762` stop; probes end only at their existing containment budget, and full 668-case qualification plus the separately run dependency-DAG negative pass per width. [Evidence](../etc/evidence/t4-s30-sprite-protection-destination-output-l2.md). |
| S31 | Fixed-role graphics image ingress, lifetime and creation validation. | P1 `197ec9b3c`; x64/x86 Board smoke and selected-set startup probes pass. [Evidence](../etc/evidence/t4-s31-graphics-image-ingress.md). |
| S32 | Bounded Video-owned RGB555 to `0x00RRGGBB` palette lookup. | P1 `822f76ba5`; x64/x86 Board smoke and selected-set startup probes pass. [Evidence](../etc/evidence/t4-s32-palette-rgb555-l2.md). |
| S33 | Recorded the current-MAME verifier limitation without source import or retained media output. | P1 `9b179c83d`; current MAME rejects the supplied archive for PAL blobs. [Evidence](../etc/evidence/t4-s33-black-box-oracle-disposition.md). |
| S34 | Corrected the black-box baseline with historical official MAME; no PAL/ROM was acquired. | P1 `c895eaca0`; the bounded historical run executes the supplied archive with the absent PAL marked no-good-dump. [Evidence](../etc/evidence/t4-s34-historical-mame-oracle.md). |
| S35 | Connected owner-approved external PAL BYOB inputs to current-MAME verification. | P1 `2ea80ac66`; both hashes match and current MAME verifies/runs the combined inputs. [Evidence](../etc/evidence/t4-s35-external-pal-byob.md). |
| S36 | Established a repeatable, neutral current-MAME screen checkpoint. | P1 `ded2d0a81`; two bounded runs agree on 320×240 at frame 600 without retained pixel output. [Evidence](../etc/evidence/t4-s36-mame-screen-checkpoint.md). |
| S41 | Owner-authorized L2 character/background graphics decoder primitive. | P1 `b15bf1768`; x64/x86 focused and MyRaidenII unit groups pass. Both full 643-case repository unit selections complete; the controlled junction-only cleanup case requires the authorized execution context and passes separately on both widths. [Evidence](../etc/evidence/t4-s41-l2-graphics-layout.md). |

## Archived Packet — M1 T4 S30

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S30 follows accepted S29 P2 `6e1b1b5ee`; one session performs coordinator and executor roles. |
| Admission And Approval | The owner authorized continuing MyRaidenII and L2 models derived from external emulator research without source import; this packet changes only MyRaidenII. |
| Objective | Receive exactly the selected program's read pair `0x762--0x763` as a Board-owned L2 Sprite-protection destination-output endpoint, so it no longer falls through the unmapped control window. |
| Non-goals | No Sprite-protection algorithm, decryption, object transfer, new output state, write access, timing claim, rendering/frame effect, Shared/public API, ROM inspection, asset change or playable-game claim. |
| Reference Baseline | S29's dual-width probes next stop at a one-word read `0x762`. Permitted L2 map research identifies the pair as a Sprite-protection destination output. Existing Board route `0x6c6--0x6c7` is the sole private destination latch in the same bounded mechanism. |
| Candidate Proposal | Add one exact read-only replacement route that returns the existing reset-zero destination latch; reuse its current byte/word lane callbacks, retain the `0x6c6` write-only ingress and add no storage or helper. This is a declared L2 mirror, not a claim that all protection semantics are modeled. |
| Files And ABI Surface | MyRaidenII `core/board.c`, existing Board smoke, existing selected-set startup probe, CURRENT and one L2 evidence record. No Shared/public header, media, asset or product artifact change. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source-policy rules. The model is L2 only; one Board state owner, `lib_bool`/LIB_NULL/lib_status and inward dependencies remain required. |
| Verification | Focused x64/x86 Board smoke; x64/x86 selected-set probe; complete repository dual-width unit suites; documentation governance and `git diff --check`. |
| Expected Markers | The exact pair reads the existing destination latch through both byte lanes; writes remain rejected; reset returns zero; both probes pass the former `0x762` boundary without a widened fallback. |
| Asset Needs | Existing owner-supplied `O:\assets\raidenii\raiden2.zip`, read-only; transient program-lane staging only, no content dump, raw trace or screenshot retention. |
| Reporting Requirements | Record L2 basis/limit, single-owner/no-second-state proof, exact access contract, verification result, diff size, next stop and remaining non-goals. |
| Stop Conditions | Any need to infer a protection algorithm, attach a Sprite/Video consumer, add new mutable state or Shared API, accept write access, inspect protected ROM content, diverge by width or retain media. |
| Exit Criteria | One reviewed MyRaidenII P with exact endpoint proof/evidence, dual-width qualification and cleanup; otherwise transfer/refuse the whole endpoint without widening a fallback. |
| Original Owner Request | Continue implementing MyRaidenII under the repository governance and permitted L2 source boundary. |
| Similar-Issue Sweep | Check all existing Sprite-protection queries and routes, byte/word splitting, read/write directions and reset so the new read endpoint neither adds state nor changes existing route ownership. |

## Archived Packet — M1 T4 S29

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S29 follows accepted S28 P3 `f871e1185`; one session performs coordinator and executor roles. |
| Admission And Approval | The owner authorized continuing MyRaidenII and standing L2 models using external emulator research while source-policy restrictions remain in force; this packet changes only MyRaidenII. |
| Objective | Receive the complete selected Sprite-protection endpoint group `0x6d8--0x6df` with its map-established byte/word access directions, so the selected program can pass its current `0x6d8` write without a generic control fallback. |
| Non-goals | No sprite decryption, object transfer, frame/render effect, protection algorithm, register readback beyond the mapped max-X pair, timing claim, Shared/public API, ROM inspection, asset change or playable-game claim. |
| Reference Baseline | S28's dual-width selected-set probes stop at one-byte write `0x6d8`. Permitted L2 map research identifies pairs `0x6d8/0x6da/0x6dc/0x6de` as Sprite-protection X, Y, max-X and source respectively; only max-X is mapped readable. |
| Candidate Proposal | Extend the existing private Board Sprite-protection latch owner with four reset-zero `lib_u16` pairs. Preserve existing Core byte splitting and little-endian lanes; X, Y and source are write-only, max-X is readable/writeable. |
| Files And ABI Surface | MyRaidenII `core/board.c`, existing Board smoke, existing selected-set startup probe, CURRENT and one L2 evidence record. No Shared/public header, media, asset or product artifact change. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source-policy rules. The model is L2 only; `lib_bool`/LIB_NULL/lib_status and inward dependencies remain required. |
| Verification | Focused x64/x86 Board smoke; x64/x86 selected-set probe; complete repository dual-width unit suites; documentation governance and `git diff --check`. |
| Expected Markers | All four pairs accept contained byte/word writes; only max-X read/inspect succeeds; reset clears readable max-X; both probes advance beyond `0x6d8` with no retained protected payload. |
| Asset Needs | Existing owner-supplied `O:\assets\raidenii\raiden2.zip`, read-only; transient program-lane staging only, no content dump, raw trace or screenshot retention. |
| Reporting Requirements | Record source grade, exact access contract, local state/no-effect boundary, verification result, diff size, next stop and remaining non-goals. |
| Stop Conditions | Any need to infer protection/render/decryption semantics, add a generic register framework or Shared API, accept a different access contract, inspect protected ROM content, diverge by width or retain media. |
| Exit Criteria | One reviewed MyRaidenII P with exact endpoint proof/evidence, dual-width qualification and cleanup; otherwise transfer/refuse the whole group without widening a fallback. |
| Original Owner Request | Continue implementing MyRaidenII under the repository governance and permitted L2 source boundary. |
| Similar-Issue Sweep | Check existing Sprite-protection latch fields, byte/word lane handling, query/read/write directions, reset and nearby `0x6ce` flags/`0x6fc` DMA routes so no duplicate state owner or widened control path remains. |

## Archived Packet — M1 T4 S28

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S28 follows accepted S27 P2 `58a038ff4`; one session performs coordinator and executor roles. |
| Admission And Approval | The owner's standing L2 authorization permits this bounded address-owner model; the source policy continues to forbid importing code, tables, algorithms or protected media. |
| Objective | Receive only the selected program's exact one-byte write at `0x6ca` as a write-only no-op. |
| Non-goals | No `0x6cb` ROM bank, readback, retained register state, generic control window, tile/sprite semantics, Shared change, ROM inspection or playable-game claim. |
| Reference Baseline | S27's x64/x86 probes agree on a byte write at `0x6ca`. Permitted L2 map research distinguishes that byte as a no-op from the adjacent `0x6cb` Raiden II program-ROM bank endpoint. |
| Candidate Proposal | Add one exact Board replacement route that reuses the existing selected-control no-op owner; read and inspect remain refused. A wider access is left to the existing Core byte splitter, whose adjacent `0x6cb` lane already has its independent bank owner. |
| Files And ABI Surface | MyRaidenII `core/board.c`, existing Board smoke, startup probe, CURRENT and one evidence record. No Shared/public API, media, asset or product artifact change. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source-policy rules. The address model is L2 only. |
| Verification | Focused x64/x86 Board smoke; x64/x86 selected-set probe; complete repository dual-width unit suites; documentation governance and `git diff --check`. |
| Expected Markers | `0x6ca` write succeeds without state/readback; adjacent `0x6cb` remains owned by its existing bank route; both probes advance to the next independent stop. |
| Stop Conditions | Any need to combine the byte with `0x6cb`, retain behavior, add a generic route, change Shared, inspect ROM payload or diverge by width. |
| Exit Criteria | Focused/full dual-width proof, evidence and one reviewed MyRaidenII P, or refusal/transfer if the boundary cannot stay exact. |
| Original Owner Request | Continue MyRaidenII under permitted L2 evidence without fabricating a full hardware model. |
| Similar-Issue Sweep | Check existing no-op route boundaries, wider-write splitting, sprite-object overlap and reset so this byte cannot silently widen a neighboring owner. |

S28 is closed. The Board receives only the exact selected one-byte `0x6ca`
write as a state-free, write-only no-op. The existing Core byte splitter keeps
the adjacent `0x6cb` bank route independent. The old unsupported-DMA Board
smoke row now uses still-unimplemented `0x0016`: S27 made `0x0014` a legal
tilemap mode, so retaining it as the failure row would test stale behavior.
The dual-width focused Board smokes, selected-set probes and complete 642-case
unit qualifications pass; the dependency-DAG negative case is executed
separately because its probe creates an isolated temporary tree. No Shared
source/API, ROM/media payload or product artifact changed.

## Archived Packet — M1 T4 S23

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S23 follows accepted S22 P2 `c39d84c04`; one session performs coordinator and executor roles. |
| Admission And Approval | Owner's standing approval permits bounded diagnostic and L2 work under the MyRaidenII source policy. The source policy still forbids importing code, tables, algorithms or protected media. |
| Objective | Record the existing COP DMA trigger rejection through the existing bounded failure observation, including its raw mode, so the selected-set probe can attribute `0x6fc` without guessing a receiver. |
| Non-goals | No DMA mode acceptance, Video/Palette implementation, new public API, new failure type, generic trace system, Shared change, ROM inspection, or playable-game claim. |
| Reference Baseline | S22's x64/x86 probe reaches a terminal CPU fault at `0x6fc`; `cop.c` already identifies that exact pair as the DMA trigger and rejects unsupported raw modes. The existing failure summary only records failures after an accepted pending operation. |
| Candidate Proposal | Reuse the existing private COP failure observation: before returning the existing invalid-state result for an unsupported DMA mode, fill its existing operation/value/physical/status fields once. Remove Board's redundant snapshot so its existing getter reads the COP owner directly. Existing reset clears it; the probe already prints the existing status fields. |
| Files And ABI Surface | MyRaidenII `core/cop.c`, `core/board.c`, existing Board smoke, selected-set probe, CURRENT and one evidence record. No Shared component, public interface, product, ROM, asset or external-code change. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source-policy rules. Attribution is not a DMA implementation or a hardware-accuracy claim. |
| Verification | Focused x64/x86 Board test; x64/x86 selected-set probe; complete repository dual-width unit suites; documentation governance and `git diff --check`. |
| Expected Markers | Unsupported mode leaves no pending operation, yields existing invalid-state behavior, records only existing bounded fields, clears on reset and produces identical x64/x86 probe attribution. |
| Asset Needs | Existing owner-supplied `O:\assets\raidenii\raiden2.zip`, read-only; transient startup staging only, with no retained payload or raw trace. |
| Reporting Requirements | Record the raw mode, source boundary, unchanged refusal behavior, next ownership decision and remaining non-goals. |
| Stop Conditions | The selected trigger is not the documented DMA trigger, the diagnostic requires a new public/shared contract, host widths diverge, protected-data exposure, or cleanup failure. |
| Exit Criteria | Focused/full dual-width proof, evidence and one reviewed MyRaidenII P, or refusal/transfer because bounded attribution fails. |
| Original Owner Request | Continue implementing MyRaidenII under the repository governance and permitted L2 source boundary. |
| Similar-Issue Sweep | Check every existing early DMA-trigger refusal, accepted DMA failure, pending-state guard and reset path so attribution does not overwrite an earlier failure or change acceptance. |

S23 is closed. COP is the sole owner of its bounded failure observation, and
Board's existing getter now reads that owner directly. Both selected-set probes
report the same rejected DMA mode `0x0015` at `0x6fc`, while all accepted modes
and the unimplemented Video-DMA modes retain their pre-existing behavior. The
complete x64/x86 unit qualification passes 642/642 per width. No Shared
component, public API, ROM/media payload, product artifact or playable-game
claim changed.

## Archived Packet — M1 T4 S24

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S24 follows S23 P3 `282ba4a29`; one session performs coordinator and executor roles. |
| Admission And Approval | Owner's standing approval permits bounded diagnostic work under the MyRaidenII source policy. No Shared component, external code, protected payload or mode implementation is admitted. |
| Objective | Capture the already-selected DMA context's source, size and destination when the existing trigger rejection occurs, so the selected-set probe can establish a finite descriptor without violating the write-only context contract. |
| Non-goals | No mode acceptance, DMA transfer, Video/Palette implementation, register readback, generic trace, new function/callback, Shared API, ROM inspection, artifact or playable-game claim. |
| Reference Baseline | S23 identifies raw mode `0x0015` at `0x6fc`; its context registers `0x478--0x47d` are intentionally write-only and cannot lawfully be read back through the Board bus. |
| Candidate Proposal | Extend the existing bounded COP failure observation with the three reset-zero `lib_u16` values from the already-selected private DMA context. Populate them only with the first rejected DMA trigger alongside its existing fields; reset clears all fields. The existing Board getter and selected-set probe consume that observation without adding a second diagnostic route. |
| Files And ABI Surface | MyRaidenII `core/cop.h`, `core/cop.c`, existing Board smoke, existing selected-set probe, CURRENT and one evidence record. The existing diagnostic record gains three fields; no Shared component, new function, callback, platform code, ROM, asset or external-code change. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source-policy rules. This is a bounded runtime observation, not a source of hardware timing or palette semantics. |
| Verification | Focused x64/x86 Board smoke; x64/x86 selected-set probe; complete repository dual-width unit suites; documentation governance and `git diff --check`. |
| Expected Markers | Rejected mode preserves existing invalid-state behavior and no pending operation; observation contains the exact selected descriptor on both widths; reset clears it; the write-only bus reads remain refused. |
| Asset Needs | Existing owner-supplied `O:\assets\raidenii\raiden2.zip`, read-only; transient staging only, with no retained payload, raw trace or screenshot. |
| Reporting Requirements | Record descriptor values, source boundary, reset/failure behavior, dual-width probe result and remaining mode-15/Video non-goals. |
| Stop Conditions | Capturing the context requires exposing a write-only bus read, a new generic trace/API, host widths diverge, protected data exposure, or cleanup failure. |
| Exit Criteria | Focused/full dual-width proof, evidence and one reviewed MyRaidenII P, or refusal/transfer if bounded observation cannot be made. |
| Original Owner Request | Continue implementing MyRaidenII under the repository governance and permitted L2 source boundary. |
| Similar-Issue Sweep | Check each existing DMA-trigger refusal, reset, accepted DMA advance failure and Board diagnostic getter so one first-failure owner remains. |

S24 is closed. The existing COP-owned failure record now supplies the selected
DMA context at an immediate rejection without creating bus readback or a second
diagnostic path. Both owner-set probes agree on mode `0x0015`, source `0x07c0`,
size `0x00ff` and destination `0x0000`; those values do not establish palette
semantics or authorize DMA acceptance. The complete x64/x86 unit qualification
passes 642/642 per width. No Shared component, product artifact, ROM/media
payload or playable-game claim changed.

## Archived Packet — M1 T4 S25

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S25 follows S24 P2 `6e322a604`; one session performs coordinator and executor roles. |
| Admission And Approval | The owner permits an L2 model informed by other emulators where L3 material is unavailable. The source policy still forbids importing code, tables, algorithms or protected media. |
| Objective | Admit only the selected mode-`0x0015` fixed palette DMA receiver, using the S24 descriptor as its exact eligibility gate. |
| Non-goals | No RGB conversion, renderer/frame, tile/sprite DMA, brightness mode, generic DMA framework, new Shared API, ROM inspection, artifact or playable-game claim. |
| Reference Baseline | S24 observes mode `0x0015`, source `0x07c0`, size `0x00ff`, destination `0`; permitted L2 research identifies the corresponding Raiden II palette aperture as 2048 16-bit entries at `0x1f000--0x1ffff`. |
| Candidate Proposal | Board constructs the existing private Video object before COP. COP accepts exactly that descriptor at its existing safe point, reads 2048 words through the existing checked Core-memory owner, then atomically passes those raw words to Video. Video retains reset-zero palette words only. Any descriptor or source-read failure retains the old palette. |
| Files And ABI Surface | MyRaidenII `core/cop.h`, `core/cop.c`, `core/video.h`, `core/video.c`, `core/board.c`, existing Board smoke, selected-set probe, CURRENT and one evidence record. No Shared component or external-code import. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source-policy rules. The palette receiver is L2 and does not claim original board timing or colour interpretation. |
| Verification | Focused x64/x86 Board smoke; x64/x86 selected-set probe; complete repository dual-width unit suites; documentation governance and `git diff --check`. |
| Expected Markers | The exact descriptor accepts and publishes all 2048 words atomically; altered source/size/destination refuses without publication; source failure preserves the prior palette; reset clears it; both selected-set probes advance past the former rejection. |
| Asset Needs | Existing owner-supplied `O:\assets\raidenii\raiden2.zip`, read-only; transient startup staging only, with no retained payload, raw trace or screenshot. |
| Reporting Requirements | Record the precise L2 eligibility contract, the source-policy boundary, failure/reset semantics, next probe stop and dual-width proof. |
| Stop Conditions | The implementation requires a generic DMA/Video API, source readback, a Shared change, protected-data exposure, host-width divergence or an unbounded receiver. |
| Exit Criteria | Focused/full dual-width proof, evidence and one reviewed MyRaidenII P, or refusal/transfer if the finite receiver cannot be made at the stated owners. |
| Similar-Issue Sweep | Check existing DMA source failure, pending clearing, reset, supported modes and every mode-`0x0015` descriptor field so no alternate path accepts the palette receiver. |

S25 is closed.  The precise selected descriptor now reaches a private,
reset-zero 2048-word Video palette through the existing COP safe point and
checked Core memory path.  It has no RGB decoder, renderer, public readback or
general DMA rule.  Altered descriptor fields and partial Video input leave the
previous palette intact.  The owner-set probes agree on the next separate
mode-`0x0014` rejection; it is not admitted by this receiver.  Complete
repository unit suites pass 642/642 on each width.  No Shared component,
product artifact, ROM/media payload or playable-game claim changed.

## Archived Packet — M1 T4 S26

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S26 follows S25 P2 `f6269721e`; one session performs coordinator and executor roles. |
| Admission And Approval | The owner's standing approval permits bounded diagnostics and L2 work under the MyRaidenII source policy. No mode implementation, Shared change or protected-data capture is admitted. |
| Objective | Capture the selected rejected mode-`0x0014` DMA context's existing relative-address word alongside the S24 descriptor fields. |
| Non-goals | No tilemap transfer, destination interpretation, renderer, generic trace, new function/callback, Shared API, ROM inspection, artifact or playable-game claim. |
| Reference Baseline | S25 advances to mode `0x0014` with source `0x033f`, size `0x027b` and destination `0xfffe`; existing register `0x476` is part of the selected DMA configuration but has not been retained in the failure observation. |
| Candidate Proposal | Add one reset-zero `lib_u16` field to the existing COP-owned first-failure record. Populate it only at the existing immediate rejected-DMA trigger from the already-selected `0x476` configuration word. Existing Board getter and startup probe consume it. |
| Files And ABI Surface | MyRaidenII `core/cop.h`, `core/cop.c`, existing Board smoke, existing startup probe, CURRENT and one evidence record. The existing internal diagnostic record gains one field; no new function, callback, Shared component, ROM or asset change. |
| Verification | Focused x64/x86 Board smoke; x64/x86 selected-set probe; complete repository dual-width unit suites; documentation governance and `git diff --check`. |
| Expected Markers | Rejected mode remains rejected/no-pending; the captured relative word agrees across widths and reset clears it; the existing mode-`0x0015` palette route remains unchanged. |
| Stop Conditions | Capturing it needs bus readback, a generic trace/API, a Shared change, protected-data exposure or host-width divergence. |
| Exit Criteria | Focused/full dual-width proof, evidence and one reviewed MyRaidenII P, or refusal/transfer if a bounded observation cannot be made. |
| Similar-Issue Sweep | Check each immediate unsupported-DMA rejection, accepted-DMA failure and reset path so the first-failure record remains single-owner and no rejected request gains a pending operation. |

S26 is closed.  The existing COP-owned record now captures the reset-zero
relative-address word at rejected DMA admission without adding readback or a
second diagnostic route.  The selected descriptor has `relative=0` on both
host widths and remains rejected with no pending operation.  No mode behavior,
Shared component, product artifact, ROM/media payload or playable-game claim
changed.

## Archived Packet — M1 T4 S27

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S27 follows S26 P2 `5fbf826b9`; one session performs coordinator and executor roles. |
| Admission And Approval | The owner permits an L2 model based on bounded selected-set observation and permitted external-emulator research. The source policy still forbids importing code, tables, algorithms or protected media. |
| Objective | Admit only the selected mode-`0x0014` fixed private tilemap DMA receiver. |
| Non-goals | No tile format/decode, scroll/bank behavior, renderer/frame, destination generalization, generic DMA framework, new Shared API, ROM inspection, artifact or playable-game claim. |
| Reference Baseline | S26 fixes the descriptor at `source=0x033f`, `size=0x027b`, `destination=0xfffe`, `relative=0`; permitted L2 research identifies a private 0x2800-byte tilemap transfer for mode `0x0014`. |
| Candidate Proposal | COP accepts exactly that descriptor at its existing safe point, reads 0x1400 words at `0x0cfc0` through the existing checked Core-memory owner, then atomically passes them to Video. Video retains reset-zero raw tilemap words only. |
| Files And ABI Surface | MyRaidenII `core/cop.c`, `core/video.h`, `core/video.c`, existing Board smoke, startup probe, CURRENT and one evidence record. No Shared component, new generic interface, ROM or asset change. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source-policy rules. This is L2 and does not claim original tile/pixel semantics or timing. |
| Verification | Focused x64/x86 Board smoke; x64/x86 selected-set probe; complete repository dual-width unit suites; documentation governance and `git diff --check`. |
| Expected Markers | The exact descriptor atomically publishes 0x1400 words; altered fields and partial Video input retain prior data; reset clears it; both probes advance past the prior mode-`0x0014` refusal. |
| Stop Conditions | The implementation needs a generic DMA/Video API, source readback, Shared change, protected-data exposure, host-width divergence or an unbounded receiver. |
| Exit Criteria | Focused/full dual-width proof, evidence and one reviewed MyRaidenII P, or refusal/transfer if the finite receiver cannot be made at the stated owners. |
| Similar-Issue Sweep | Check the existing palette path, mode selection, source failure, reset and pending clearing so neither exact receiver becomes a generic DMA fallback. |

S27 is closed. Its exact raw receiver retains one trigger-time relative value
so a later register write cannot alter an accepted pending transfer, and uses
the existing checked memory route for all source words before one Video commit.
Both widths advance through mode `0x0014` to a new independent Board-control
write at `0x6ca`; complete repository unit qualification passes 642/642 on
each width, documentation governance passes, and transient probe staging was
removed. No decode, frame or generic DMA behavior was admitted.

## Archived Packet — M1 T4 S22

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S22 follows S21 P2 `e007e826d`; one session performs coordinator and executor roles. |
| Admission And Approval | Owner's standing approval permits bounded L2 behavior from external address-map evidence where physical L3 material is unavailable; source policy forbids importing code, tables, algorithms, or protected media. |
| Objective | Establish complete Board ownership for the adjacent sprite-protection register group: reset-zero read/write word latches at `0x6c0--0x6c1` and `0x6c2--0x6c3`, plus a reset-zero write-only word latch at `0x6c6--0x6c7`. |
| Non-goals | No protection algorithm, sprite decryption, destination interpretation, frame effect, generic register framework, public API, Shared change, reference implementation translation, ROM inspection, or playable-game claim. |
| Reference Baseline | S21's x64/x86 probes stop on a byte write at `0x6c0`. The selected Raiden II inherited map gives named read/write ownership to `0x6c0--0x6c3`, write-only ownership to `0x6c6--0x6c7`, and independently assigns the intervening `0x6c4--0x6c5` no-op to S21. |
| Candidate Proposal | Add three exact Board routes and three `lib_u16` reset-zero latches. Byte and contained word lanes use explicit little-endian handling. Readable latches expose stored bytes; the destination latch rejects read/inspect. |
| Files And ABI Surface | MyRaidenII `core/board.c`, existing Board smoke, selected-set probe, CURRENT and one evidence record. No public interface, Shared component, product, ROM, asset, or external-code change. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source-policy rules. Address ownership and local latching are L2 behavior, not a hardware-accuracy claim. |
| Verification | Focused x64/x86 Board test; x64/x86 selected-set startup probe; complete repository dual-width unit suites; documentation governance and `git diff --check`. |
| Expected Markers | Read/write latch lane round-trips and reset are verified; destination writes and reads fail as mapped; S21 no-op remains independent; a wider access keeps Core's established byte-route splitting; both probes report the same next terminal. |
| Asset Needs | Existing owner-supplied `O:\assets\raidenii\raiden2.zip`, read-only; transient startup staging only, with no retained payload or raw trace. |
| Reporting Requirements | Record the endpoint group, L2 limits, lane/reset behavior, next probe stop, dual-width result and remaining non-goals. |
| Stop Conditions | Required protection behavior beyond latching, conflicting endpoint evidence, Shared/public API change, divergent host widths, protected-data exposure, or cleanup failure. |
| Exit Criteria | Focused/full dual-width proof, evidence and one reviewed MyRaidenII P, or refusal/transfer because bounded assumptions fail. |
| Original Owner Request | Continue implementing MyRaidenII under the repository governance and permitted L2 source boundary. |
| Similar-Issue Sweep | Test every group endpoint, byte/word lanes, readable versus write-only distinction, reset, adjacent S21 no-op, wider accesses through Core's byte-route split and duplicate owner paths. |

## Retained Packet — M1 T4 S21

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S21 follows S20 P4 `91c97c436`; one session performs coordinator and executor roles. |
| Admission And Approval | Owner's standing approval permits bounded L2 behavior from external address-map evidence where L3 material is unavailable; source policy forbids importing code, tables, algorithms, or protected media. |
| Objective | Model the selected-set-observed `0x6c4--0x6c5` sprite-protection constant write span as a stateless Board-owned no-op. |
| Non-goals | No readback, constant interpretation, sprite-protection implementation, generic register framework, public API, Shared change, reference implementation translation, ROM inspection, or playable-game claim. |
| Reference Baseline | S20's x64/x86 probes stop on a one-byte write at `0x6c4`. The selected Raiden II inherited map marks `0x6c4--0x6c5` write-only and states that a constant is written alongside the separate `0x6c0` receiver. |
| Candidate Proposal | Add the exact two-byte span to the existing Board no-op owner. It accepts contained writes only, retains no state and leaves reads/inspection on the existing refusal path. |
| Files And ABI Surface | MyRaidenII `core/board.c`, existing Board smoke, selected-set probe, CURRENT and one evidence record. No public interface, Shared component, product, ROM, asset, or external-code change. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source-policy rules. Address ownership is L2 behavior, not a hardware-accuracy claim. |
| Verification | Focused x64/x86 Board test; x64/x86 selected-set startup probe; complete repository dual-width unit suites; documentation governance and `git diff --check`. |
| Expected Markers | Contained byte and word writes are accepted, reads/inspection remain refused, a crossing write is rejected, and both probes report the same next terminal. |
| Asset Needs | Existing owner-supplied `O:\assets\raidenii\raiden2.zip`, read-only; transient startup staging only, with no retained payload or raw trace. |
| Reporting Requirements | Record L2 source boundary, stateless behavior, next probe stop, dual-width result and remaining non-goals. |
| Stop Conditions | Required protection behavior, conflicting endpoint evidence, Shared/public API change, divergent host widths, protected-data exposure, or cleanup failure. |
| Exit Criteria | Focused/full dual-width proof, evidence and one reviewed MyRaidenII P, or refusal/transfer because bounded assumptions fail. |
| Original Owner Request | Continue implementing MyRaidenII under the repository governance and permitted L2 source boundary. |
| Similar-Issue Sweep | Check only the exact pair, byte/word containment, read/inspect refusal, cross-pair behavior and duplicate owner paths. |

S21 is closed.  The stateless constant sink preserves the explicit boundary
between it and the `0x6c0--0x6c3` sprite-protection read/write group.  Both
probes now stop at the same `0x6c0` write; that stateful group remains a
separate ownership obligation.  No Shared component, public API, ROM/media
payload, desktop/RDP validation, or playable-game claim changed.

## Retained Packet — M1 T4 S20

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S20 follows S19 P2 `ee5a5c514`; one session performs coordinator and executor roles. |
| Admission And Approval | Owner's standing approval permits bounded L2 behavior from external address-map evidence where L3 material is unavailable; source policy forbids importing code, tables, algorithms, or protected media. |
| Objective | Model the selected-set-observed two-byte tile-bank-01 write endpoint at `0x6cc--0x6cd` as reset-zero Board state. |
| Non-goals | No readback, renderer, layer selection, palette, generic register framework, public API, Shared change, reference implementation translation, ROM inspection, or playable-game claim. |
| Reference Baseline | S19's x64/x86 probes first stop on a one-byte write at `0x6cc`; the temporary bounded receiver advances both to a one-byte `0x6cd` write. The selected Raiden II reference map assigns the write-only tile-bank-01 endpoint over `0x6cc--0x6cd`; S28 already records this range as a distinct established owner. |
| Candidate Proposal | Add one Board-owned two-byte replacement route, one `lib_u16` reset-zero latch and no read route. The latch retains both admitted byte lanes for a later evidenced Video consumer but has no current frame effect. |
| Files And ABI Surface | MyRaidenII `core/board.c`, existing Board smoke, selected-set probe, CURRENT and one evidence record. No public interface, Shared component, product, ROM, asset, or external-code change. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source-policy rules. Address ownership and retention are L2 behavior, not a hardware-accuracy claim. |
| Verification | Focused x64/x86 Board test; x64/x86 selected-set startup probe; complete repository dual-width unit suites; documentation governance and `git diff --check`. |
| Expected Markers | Byte writes at both lanes and a complete word write succeed, reads/inspection remain refused, nearby `0x6ca` and `0x6ce` retain their existing refusal, and both probes report the same next terminal. |
| Asset Needs | Existing owner-supplied `O:\assets\raidenii\raiden2.zip`, read-only; transient startup staging only, with no retained payload or raw trace. |
| Reporting Requirements | Record L2 source boundary, reset/write-only behavior, next probe stop, dual-width result and remaining non-goals. |
| Stop Conditions | Need for tile/render semantics, conflicting endpoint evidence, Shared/public API change, divergent host widths, protected-data exposure, or cleanup failure. |
| Exit Criteria | Focused/full dual-width proof, evidence and one reviewed MyRaidenII P, or refusal/transfer because bounded assumptions fail. |
| Original Owner Request | Continue implementing MyRaidenII under the repository governance and permitted L2 source boundary. |
| Similar-Issue Sweep | Test the exact byte ownership, read/inspect refusal, adjacent fallback ownership and reset lifecycle; verify no second tile-bank-01 or generic register path exists. |

S20 P2's combined-latch conclusion was reopened by the map recheck.  S20 P3
narrows `0x6cc` to its actual tile-bank latch and retains `0x6cd` as the
separate mapped no-op; the repeated complete dual-width qualification passes.
S20 is closed.  Neither endpoint has readback or a frame effect.  The next
stop remains the unowned `0x6c4` write.  No Shared component, public API,
ROM/media payload, desktop/RDP validation, or playable-game claim changed.

## Retained Packet — M1 T4 S19

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S19 follows coordinator-accepted S18 P1 `cdd761ac0`; one session performs coordinator and executor roles. |
| Admission And Approval | Owner's standing approval permits bounded L2 behavior from external emulator evidence where physical L3 material is unavailable; source policy forbids importing code, tables, algorithms, or protected media. |
| Objective | Model the selected-set-observed Board tile-bank-2 endpoint at `0x470--0x471` as one reset-zero, byte-addressable 16-bit local register. |
| Non-goals | No tile renderer, layer selection, palette, CRTC change, generic register framework, public API, Shared change, reference implementation translation, ROM inspection, or playable-game claim. |
| Reference Baseline | S18 probes reach a one-byte Board read at `0x470` on both widths. The selected Raiden II reference map declares a read/write `0x470--0x471` tile-bank endpoint; earlier internal B07 inventory assigns tile-bank behavior to Board/Video, not COP. |
| Candidate Proposal | Add one Board-owned two-byte replacement route and one `lib_u16` reset-zero field. Reads, inspection and byte/word writes use explicit little-endian lane handling; it retains values locally only for this endpoint. |
| Files And ABI Surface | MyRaidenII `core/board.c`, existing Board smoke, selected-set probe, CURRENT and one evidence record. No public interface, Shared component, product, ROM, asset, or external-code change. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source-policy rules. L2 address/behavior evidence is not a hardware-accuracy claim. |
| Verification | Focused x64/x86 Board test; x64/x86 selected-set startup probe; complete repository dual-width unit suites; documentation governance and `git diff --check`. |
| Expected Markers | Initial read returns zero; byte lanes and complete word round-trip; reset clears value; neighbouring access remains refused; both probes pass `0x470` and report the same next terminal. |
| Asset Needs | Existing owner-supplied `O:\assets\raidenii\raiden2.zip`, read-only; transient startup staging only, with no retained payload or raw trace. |
| Reporting Requirements | Record L2 source boundary, exact lane/reset behavior, next probe stop, dual-width result and remaining non-goals. |
| Stop Conditions | Need for tile/render semantics, conflicting endpoint evidence, Shared/public API change, divergent host widths, protected-data exposure, or cleanup failure. |
| Exit Criteria | Focused/full dual-width proof, evidence and one reviewed MyRaidenII P, or refusal/transfer because bounded assumptions fail. |
| Original Owner Request | Continue implementing MyRaidenII under the repository governance and permitted L2 source boundary. |
| Similar-Issue Sweep | Test full/byte lanes, initial/reset state, read/inspect, neighbours and control-failure ownership; verify no duplicate tile-bank or generic register path exists. |

S19 is closed.  The local L2 register accepts only `0x470--0x471`, preserves
its two byte lanes, and resets to zero without claiming a tile-rendering
effect.  Both selected-set probes advance to the identical next unowned Board
write at `0x6cc`; that endpoint remains a separate ownership obligation.
No Shared component, public API, ROM/media payload, desktop/RDP validation, or
playable-game claim changed.

## Retained Packet — M1 T4 S18

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S18 follows coordinator-accepted S17 P1 `a815a6a0a`; one session performs coordinator and executor roles. |
| Admission And Approval | Owner authorizes behavioral L2 models from external emulator behavior when L3 is unavailable and explicitly asks to avoid inefficient one-address-at-a-time work. The source-policy boundary remains: no implementation, table, algorithm or ROM bytes are imported. |
| Objective | Replace S15--S17's incremental adjacent pairs with one stateless, write-only Board aperture at the complete currently unowned portion of the externally documented OBJ range: `0x650--0x69f`. |
| Reference Baseline | `42b5e6a54`; both widths reach the next byte write at `0x65e`. MAME's address map identifies `0x640--0x6bf` as an OBJ interface but leaves it commented; this repository already has CRTC ownership through `0x64f` and protection-upload ownership from `0x6a0` through `0x6bf`. Thus `0x650--0x69f` is the exact remaining contiguous aperture. |
| Candidate Proposal | One existing Board route covers `0x650--0x69f`; contained writes are accepted as stateless L2 ownership except mixed-width accesses that straddle an existing selected-control receiver. Reads and inspection remain refused, source bytes are not retained, and no object/decryption/rendering semantics are added. Existing CRTC, protection and selected-control receivers retain their ranges and Core's existing byte-splitting behavior. |
| Non-goals | No generic RAM, object register meanings, readback, renderer, decryption, palette behavior, physical claim, Shared change, external source translation, protected-ROM inspection, raw trace retention, desktop qualification or playable-game claim. |
| Files And ABI Surface | MyRaidenII `core/board.c`, existing Board/CRTC/protection tests, selected-set integration probe, CURRENT and one evidence record. No public interface, Shared component, product, ROM, asset or external-code change. |
| Verification | Focused x64/x86 Board route test; x64/x86 selected-set startup probe; complete repository dual-width unit suites after production change; documentation-governance and `git diff --check`. |
| Expected Markers | Writes at both aperture bounds and representative former gaps are accepted; reads/inspection remain refused; CRTC/protection boundary tests continue to own their ranges; both probes pass the previous `0x65e` stop and report one identical next stop. |
| Stop Conditions | Required object behavior beyond write acceptance, evidence contradicting aperture ownership, Shared change, divergent host widths, protected-data exposure or cleanup failure. |
| Exit Criteria | Focused/full dual-width proof, evidence and one reviewed MyRaidenII P, or refusal/transfer because bounded assumptions fail. |
| Similar-Issue Sweep | Removed stale “unmodelled” expectations in the aperture, retained CRTC/protection tests as their own owners, tested lower/upper bounds/read/inspect/reset, and proved no object data persists. |

S18 is closed.  The source-bounded aperture advances both selected-set probes
past the prior `0x65e` stop to the same next unmodeled Board write at `0x470`.
That address remains a separate T4 ownership/evidence obligation.  No
playable-game, desktop/RDP, renderer, sound, or physical-hardware claim is
implied.

## Retained Packet — M1 T4 S17

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S17 follows coordinator-accepted S16 P1 `84f3023fb`; one session performs coordinator and executor roles. |
| Admission And Approval | Owner authorizes a bounded behavioral L2 model from external emulator behavior when L3 material is unavailable. The exact two-byte span is inferred only from existing Core byte/word routing and the first post-S16 selected-set byte write; no external code, table or algorithm is imported. |
| Objective | Establish the smallest write-only receiver for the selected set's adjacent word span at `0x65c--0x65d`, separate from Video CRTC and prior object pairs. |
| Non-goals | No object register semantics, readback, generic RAM, rendering, decryption, palette behavior, physical-hardware claim, Shared change, source translation, protected-ROM inspection, raw trace retention, desktop qualification or playable-game claim. |
| Reference Baseline | `84f3023fb`; both widths accept the two prior word pairs and then report first unowned access as a one-byte write at `0x65c`. FBNeo's CRTC path ends at `0x64f`; MAME's commented OBJ aperture surrounds the area without defining contents or readback. |
| Candidate Proposal | Extend the existing Board-owned object write route over exactly the new adjacent pair and permit that pair only through the existing stateless callback. The route continues to prevent unsupported within-window accesses from falling through to ordinary RAM. |
| Files And ABI Surface | MyRaidenII `core/board.c`, existing Board smoke, selected-set integration probe, CURRENT and one evidence record. No public interface, Shared component, product, ROM, asset or external-code change. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source-policy rules. Address ownership is L2 behavioral context only. |
| Verification | Focused x64/x86 Board route test; x64/x86 selected-set startup probe with transient staging/cleanup; complete repository dual-width unit suites after production change; documentation-governance and `git diff --check`. |
| Expected Markers | Each exact byte lane and word pair are accepted; reads, inspection, cross-pair writes and immediate neighbours remain refused; both probes advance past `0x65d` to the same independent next stop. |
| Asset Needs | Existing owner-supplied `O:\assets\raidenii\raiden2.zip`, read-only; probe stages only transient program lanes and retains no payload, screenshot or raw trace. |
| Reporting Requirements | State L2 limitation; report exact accepted/rejected boundaries, dual-width result, next stop and no playable-game claim. |
| Stop Conditions | Need for access outside the observed span, readback, algorithmic behavior, Shared change, contradictory runtime evidence, divergent host widths, protected-data exposure or cleanup failure. |
| Exit Criteria | Focused/full dual-width proof, evidence and one reviewed MyRaidenII P, or refusal/transfer because bounded assumptions fail. |
| Similar-Issue Sweep | Check `0x65c--0x65d`, immediate neighbours, read/inspect, widths, reset and no overlap with CRTC or prior object pairs; no fallback widening or duplicate Video state. |

S17 is closed. The bounded L2 receiver owns only the three proven pairs and
the selected-set probe's next independent unowned access is a one-byte write
at `0x65e`. M1 T4 remains open; further ownership needs a separately admitted
packet or a new bounded diagnostic strategy.

## Retained Packet — M1 T4 S16

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S16 follows coordinator-accepted S15 P1 `0392b97e2`; one session performs coordinator and executor roles. |
| Admission And Approval | Owner authorizes a bounded behavioral L2 model from external emulator behavior when L3 material is unavailable. The exact two-byte span is inferred only from the existing Core byte-split word route and the first post-S15 selected-set byte write; no external code, table or algorithm is imported. |
| Objective | Establish the smallest write-only receiver for the selected set's next adjacent word span at `0x65a--0x65b`, separate from Video CRTC and S15 ownership. |
| Non-goals | No object register semantics, readback, generic RAM, rendering, decryption, palette behavior, physical-hardware claim, Shared change, source translation, protected-ROM inspection, raw trace retention, desktop qualification or playable-game claim. |
| Reference Baseline | `0392b97e2`; both widths accept `0x658--0x659` and then report the first unowned access as a one-byte write at `0x65a`. FBNeo's CRTC path ends at `0x64f`; MAME's commented OBJ aperture surrounds this region, without defining contents or readback. |
| Candidate Proposal | One stateless Board-owned two-byte write-only L2 sink at exact `0x65a--0x65b`. One Board route owns the four-byte evidence window so an unsupported cross-pair write cannot fall back to ordinary RAM; its write callback accepts only contained byte lanes and the complete word for either exact pair. Reads, inspection, cross-pair writes and neighbours remain refused; no getter or frame effect exists. |
| Files And ABI Surface | MyRaidenII `core/board.c`, existing Board smoke, selected-set integration probe, CURRENT and one evidence record. No public interface, Shared component, product, ROM, asset or external-code change. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source-policy rules. Address ownership is L2 behavioral context only. |
| Verification | Focused x64/x86 Board route test; x64/x86 selected-set startup probe with transient staging/cleanup; complete repository dual-width unit suites after production change; documentation-governance and `git diff --check`. |
| Expected Markers | Exact byte lanes and complete word are accepted; read/inspect, cross-pair writes and immediate neighbours remain refused; both probes advance past `0x65b` to the same independent next stop. |
| Asset Needs | Existing owner-supplied `O:\assets\raidenii\raiden2.zip`, read-only; the probe stages only transient program lanes and retains no payload, screenshot or raw trace. |
| Reporting Requirements | State L2 limitation; report exact accepted/rejected boundaries, dual-width result, next stop and no playable-game claim. |
| Stop Conditions | Need for access outside the observed span, readback, algorithmic behavior, Shared change, contradictory runtime evidence, divergent host widths, protected-data exposure or cleanup failure. |
| Exit Criteria | Focused/full dual-width proof, evidence and one reviewed MyRaidenII P, or refusal/transfer because bounded assumptions fail. |
| Similar-Issue Sweep | Check `0x65a--0x65b`, immediate neighbours, read/inspect, widths, reset and no overlap with CRTC or S15; no fallback widening or duplicate Video state. |

S16 is closed. The bounded L2 receiver owns only the proven adjacent pair and
the selected-set probe's next independent unowned access is a one-byte write
at `0x65c`. M1 T4 remains open; any receiver for that new stop requires a
separately admitted packet.

## Retained Packet — M1 T4 S15

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S15 follows coordinator-accepted S14 P1 `f8fc5f867`; one session performs coordinator and executor roles. |
| Admission And Approval | Owner authorizes an external-emulator behavioral L2 model when physical L3 material is unavailable. This model uses only independently stated address ownership and the selected-set's bounded runtime access; no implementation/source/table import is permitted. |
| Objective | Establish the smallest write-only receiver for the exact selected-set word write at `0x658--0x659`, separate from Video CRTC ownership. |
| Non-goals | No full object/sprite interface, readback, generic RAM, rendering/decryption/palette behavior, physical-hardware claim, Shared change, source translation, protected-ROM inspection, raw trace retention, desktop qualification or playable-game claim. |
| Reference Baseline | `f8fc5f867`; both widths report first fallback write `0x658`, one byte. FBNeo routes only `0x600--0x64f` writes to its CRTC helper; MAME documents a commented OBJ aperture `0x640--0x6bf`. Neither source establishes object contents or readback. |
| Candidate Proposal | One stateless Board-owned two-byte, write-only L2 object sink at exact `0x658--0x659`. It accepts either contained byte lane or the contained word through the existing Core route, refuses reads/inspection/mixed widths/neighbours, exposes no getter and has no frame effect. |
| Files And ABI Surface | MyRaidenII `core/board.c`, existing Board smoke, selected-set integration probe, CURRENT and one evidence record. No public interface, Shared component, product, ROM, asset or external-code change. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source-policy rules. The address ownership is L2 behavioral context only. |
| Verification | Focused x64/x86 Board test through the real Core route; x64/x86 selected-set startup probe with transient staging/cleanup; complete repository dual-width unit suites after production change; documentation-governance and `git diff --check`. |
| Expected Markers | The exact byte lanes and contained word are accepted; read/inspect, mixed accesses and immediate neighbours remain refused; both probes advance past `0x659` to the same independent next stop. |
| Asset Needs | Existing owner-supplied `O:\assets\raidenii\raiden2.zip`, read-only; the probe stages only transient program lanes and retains no payload, screenshot or raw trace. |
| Reporting Requirements | Record runtime write fact and external ownership separately; state the L2 limitation; report exact accepted/rejected boundaries, dual-width results, next stop and no playable-game claim. |
| Stop Conditions | Need for access beyond the observed two-byte word span, readback, algorithmic behavior, Shared change, contradictory runtime evidence, divergent host-width result, protected-data exposure or cleanup failure. |
| Exit Criteria | The exact receiver has focused/full dual-width proof, evidence and one reviewed MyRaidenII P, or it is refused/transferred because the bounded assumptions fail. |
| Original Owner Request | Do not require physical-level material; use other emulator observable behavior as clearly labeled L2 where L3 is unavailable. |
| Similar-Issue Sweep | Check both `0x658--0x659` lanes, immediate neighbours, read/inspect, widths, reset and no overlap with CRTC; ensure no fallback widening or duplicate Video state. |

S15 is closed. The bounded L2 receiver accepts only the observed word span and
the selected-set probe's next independent unowned access is a one-byte write
at `0x65a`. M1 T4 remains open; that next stop requires a separately admitted
packet rather than widening this receiver.

## Retained Packet — M1 T4 S14

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S14 follows coordinator-accepted S13 P2 `24546816b`; one session performs coordinator and executor roles. |
| Admission And Approval | Owner permits a bounded L2 model from external behavior when physical L3 material is unavailable, but requires it to be labeled L2 and not derived from external source implementation. This S admitted only neutral runtime attribution, not a receiver model. |
| Objective | Determine whether the selected set's exact next stop at `0x658` reaches the Board fallback as a read or write and with what byte width, without accepting or altering that access. |
| Non-goals | No receiver, physical control-hardware claim, fallback widening, generic RAM, new executor, Shared change, source translation, protected-ROM inspection, PC/register/payload capture, raw trace retention, desktop qualification or playable-game claim. |
| Candidate Proposal | Board keeps one reset-cleared first-failure observation containing only valid, physical address, byte width and Core access direction. Existing fallback callbacks record it once then return their unchanged `LIB_STATUS_INVALID_STATE`; they do not inspect source/destination data. |
| Verification | Focused x64/x86 Board test through the real Core memory route; x64/x86 selected-set startup probe with transient staging/cleanup; complete repository dual-width unit suites after production change; documentation-governance and `git diff --check`. |
| Exit Criteria | The bounded observation has focused/full dual-width proof, evidence and one reviewed MyRaidenII P, or the access does not reach a lawful Board fallback and is transferred. |

## Retained Packet — M1 T4 S13

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S13 follows accepted S12 P1 `9042cdd35`; one session performs coordinator and executor roles. |
| Admission And Approval | Owner's 2026-10-19 authorization permits an external-emulator behavioral L2 model when physical L3 material is absent. The owner additionally directs use of other emulator behavior as clearly labeled L2 when L3 is unavailable. It permits behavior only; no source, table, ROM, trace or algorithm import. |
| Objective | Establish the smallest Video-owned receiver for the selected set's exact next stop at `0x640`, preserving the existing CRTC state owner without treating the Video area as generic Board RAM. |
| Non-goals | No physical CRTC timing claim, rendering/decryption/palette implementation, generic control RAM, unproven extension readback, new executor, Shared/API change, source translation, protected-ROM inspection, trace retention, desktop qualification or playable-game claim. |
| Reference Baseline | `9042cdd35`; both selected-set probes advance through the S12 CRTC read/write aperture then fault at `0x640`. MAME establishes read/write ownership only through `0x63f`; an independent FBNeo driver routes CRTC writes through `0x64f`. The two sources do not establish extension read behavior. |
| Candidate Proposal | Extend the reset-cleared Video CRTC storage from 64 to 80 bytes. Board retains `0x600--0x63f` read/write behavior and accepts only writes at `0x640--0x64f`; reads/inspection in the extension and all accesses at immediate neighbours remain refused. |
| Files And ABI Surface | MyRaidenII `core/video.[ch]`, `core/board.c`, existing Board smoke, CURRENT and one evidence record. No Shared component, public cross-component ABI, product, ROM, asset or external-code change. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source-policy rules. The address/ownership fact is L2 behavioral context only. |
| Verification | Focused x64/x86 Board test; x64/x86 selected-set startup probe with transient staging/cleanup; complete repository dual-width unit suites after production change; manifest, documentation-governance and `git diff --check`. |
| Expected Markers | The original aperture preserves read/write/reset behavior; the exact extension preserves byte write order/reset, refuses read/inspect and immediate neighbours, and both widths advance past the prior `0x640` stop to one same next independently owned boundary. |
| Asset Needs | Existing owner-supplied `O:\assets\raidenii\raiden2.zip`, read-only; probe stages only transient program lanes and retains no payload, screenshot or raw trace. |
| Reporting Requirements | Record local stop and external ownership separately; state the L2 limitation; report exact accepted/rejected range, dual-width results, next stop and no playable-game claim. |
| Stop Conditions | Required algorithmic behavior, non-byte width semantics that Core cannot lawfully split, contradictory reference/runtime evidence, Shared change, protected-data exposure, divergent host-width result or cleanup failure. |
| Exit Criteria | The bounded Video receiver has focused/full dual-width proof, evidence and one reviewed MyRaidenII P, or a conflicting contract is refused and transferred. |
| Original Owner Request | Do not require physical-level material; use other emulator observable behavior as clearly labeled L2 where L3 is unavailable. |
| Similar-Issue Sweep | Check all `0x600--0x64f` bytes, surrounding `0x5ff`/`0x650`, original versus extension read ownership, reset, and ensure Board has no duplicate Video state or generic fallback acceptance. |

## Retained Packet — M1 T4 S12

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S12 follows accepted S11 P1 `a9c048fd9`; one session performs coordinator and executor roles. |
| Admission And Approval | Owner's 2026-10-19 authorization permits an external-emulator behavioral L2 model when physical L3 material is absent. It permits behavior only; no source, table, ROM, trace or algorithm import. |
| Objective | Establish the smallest Video-owned receiver for the selected set's next exact stop at `0x600`, and prove bounded forward progress without treating the Video area as generic Board RAM. |
| Non-goals | No physical CRTC timing claim, rendering/decryption/palette implementation, generic control RAM, new executor, Shared/API change, source translation, protected-ROM inspection, trace retention, desktop qualification or playable-game claim. |
| Reference Baseline | `a9c048fd9`; both selected-set probes advance through the S11 COP upload aperture and fault at `0x600`. The pinned external reference map assigns `0x600--0x63f` as read/write CRTC ownership. |
| Candidate Proposal | One reset-cleared 64-byte Video CRTC register aperture. It preserves byte reads/writes only within `0x600--0x63f`; Board translates physical addresses to existing Video ownership and retains all neighbouring refusal. |
| Files And ABI Surface | MyRaidenII `core/video.[ch]`, `core/board.c`, existing Board smoke, CURRENT and one evidence record. No Shared component, public cross-component ABI, product, ROM, asset or external-code change. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source-policy rules. The address/ownership fact is L2 behavioral context only. |
| Verification | Focused x64/x86 Board test; x64/x86 selected-set startup probe with transient staging/cleanup; complete repository dual-width unit suites after production change; manifest, documentation-governance and `git diff --check`. |
| Expected Markers | The aperture preserves byte order and reset, accepts only `0x600--0x63f`, rejects immediate neighbours, and both widths advance past the prior `0x600` stop to one same next independently owned boundary. |
| Asset Needs | Existing owner-supplied `O:\assets\raidenii\raiden2.zip`, read-only; probe stages only transient program lanes and retains no payload, screenshot or raw trace. |
| Reporting Requirements | Record local stop and external ownership separately; state the L2 limitation; report exact accepted/rejected range, dual-width results, next stop and no playable-game claim. |
| Stop Conditions | Required algorithmic behavior, non-byte width semantics that Core cannot lawfully split, contradictory reference/runtime evidence, Shared change, protected-data exposure, divergent host-width result or cleanup failure. |
| Exit Criteria | The bounded Video receiver has focused/full dual-width proof, evidence and one reviewed MyRaidenII P, or a conflicting contract is refused and transferred. |
| Original Owner Request | Do not require physical-level material; use other emulator observable behavior as clearly labeled L2 where L3 is unavailable. |
| Similar-Issue Sweep | Check all `0x600--0x63f` bytes, surrounding `0x5ff`/`0x640`, reset/readback ownership, and ensure Board has no duplicate Video state or generic fallback acceptance. |

## Retained Packet — M1 T4 S11

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S11 follows accepted S10 P1 `6a79f1102`; one session performs coordinator and executor roles. |
| Objective | Admit the smallest L2 Board receiver for the selected set's COP `0x5205` destination `0x6a0`, based on independently observed emulator address ownership, and prove bounded forward progress without importing external implementation. |
| Non-goals | No emulation of external sprite-protection algorithms, CRTC/Video behavior, generic control RAM, Shared/API change, protected ROM inspection, trace capture, playable-game claim, or physical-hardware-accuracy claim. |
| Candidate | One reset-cleared, write-only, Board-owned `0x6a0--0x6bf` sprite-protection upload aperture. It accepts only contained writes, rejects reads and inspection, and preserves no public readback contract. |
| Source Policy | MAME address ownership is behavioral L2 context only. No source code, tables, ROM content, payload, PC/register data or external algorithm is imported. |
| Verification | Focused x64/x86 Board smoke; x64/x86 selected-set startup probe with transient staged program lanes; complete repository dual-width unit suites after production change; documentation governance and `git diff --check`. |
| Exit Criteria | The aperture's contained-write/read-refusal contract is covered; both widths advance beyond the prior COP stop to the same independent next receiver; reviewed evidence and one MyRaidenII P are committed/pushed. |
| Stop Conditions | The aperture needs readback, algorithmic behavior, an external source translation, a Shared change, ambiguous width semantics, divergent host-width result, protected-data exposure or staging cleanup failure. |

## Retained Packet — M1 T4 S10

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S10 follows accepted S9 P1 `b576fcae5`; one session performs coordinator and executor roles. |
| Objective | Attribute the selected-set COP `INVALID_STATE` to the existing pending operation and first failing Core DMA address without logging protected payload or inventing a receiver. |
| Non-goals | No COP algorithm/mapping acceptance, generic trace recorder, second execution path, Shared/Core API, ROM dump, full instruction trace, Board fallback route, or playable-game claim. |
| Candidate | One reset-cleared, Board-owned copy of the immediate COP failure observation; a read-only MyRaidenII Board getter for the existing bounded integration probe. COP records only operation discriminator, command/mode value, first failed bus address and existing status. |
| Files And ABI Surface | MyRaidenII Core private COP header/source, Board interface/source, existing Board unit and selected-set integration probe only. No Shared ABI change. |
| Source Policy | The observation is neutral runtime metadata under the existing owner-supplied opt-in probe. It must not print or retain ROM bytes, PCs, register dumps or raw trace. |
| Verification | Focused x64/x86 Core Board smoke and selected-set probe; complete dual-width repository unit suites after production change; documentation governance and `git diff --check`. |
| Exit Criteria | The probe emits a bounded, stable fault attribution on both widths, synthetic failure/reset coverage proves no stale diagnostic state, and one reviewed MyRaidenII P is committed/pushed. |
| Stop Conditions | Observation would require protected payload, ambiguous multiple failures, a Shared change, a new execution path, or contradictory dual-width result. |

## Retained Packet — M1 T4 S9

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S9 follows accepted S8 P1 `fa34720df`; one session performs coordinator and executor roles. |
| Admission And Approval | Owner's 2026-10-19 authorization permits bounded L2 behavioral models from other emulators when L3 material is unavailable. No external code, tables, ROM content or protected trace may be imported. |
| Objective | Use the exact selected-set stop at `0x690`, current reference-map-gap context and actual forward-progress replay to admit only the smallest stateless Board operation that advances the existing selected program. |
| Non-goals | No blanket `0x690--0x69f` acceptance, Video/Sprite/COP state, generic RAM, Shared/Core API change, source translation, ROM retention, physical-hardware claim or playable-game claim. |
| Reference Baseline | `fa34720df`; after exact routes at `0x692`, `0x696`, `0x69a` and `0x69c`, both widths fault at `0x690`. The current reference map leaves the local `0x690--0x69f` interval unassigned between named receivers. |
| Candidate Proposal | Extend the existing selected-control write-only no-state callback only by the exact smallest route if replay advances and local reads, inspection, mixed widths and neighbours remain rejected; otherwise retain/refine refusal. |
| Files And ABI Surface | `src/myraidenii/core/board.c`, its existing Board smoke, CURRENT and S9 evidence only if accepted. No public API or ABI change. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source-policy rules. External behavior is L2 context only. |
| Verification | x64/x86 focused Board test and selected-set probe with existing transient staging/cleanup; complete repository-only dual-width unit suites after production change; documentation governance and `git diff --check`. |
| Expected Markers | Exact two-byte route accepts only byte lanes or contained word writes, rejects reads/inspection/mixed widths/neighbours, and advances both selected-set probes beyond `0x690`; otherwise report retained stop. |
| Asset Needs | Existing owner-supplied `O:\assets\raidenii\raiden2.zip`, read-only; existing probe stages only program lanes transiently and retains no payload, trace or screenshots. |
| Reporting Requirements | Record local stop and reference-map facts separately, L2 limitation, exact accepted/rejected boundaries, dual-width results, next stop and no playable-game claim. |
| Stop Conditions | Contradictory replay, stateful/read access, inaccessible exact width, required new owner/API, protected-data exposure or probe cleanup failure. |
| Exit Criteria | A bounded L2 disposition is complete: accepted Board-only route has focused/full dual-width proof and evidence, or refusal is retained/transferred. One reviewed MyRaidenII P is committed/pushed. |
| Original Owner Request | Do not require the owner to supply physical-level detail when a bounded other-emulator L2 model can make correct progress. |
| Similar-Issue Sweep | Recheck existing selected write-only routes and every immediately adjacent byte for accidental widening, duplicate ownership, readback or generic-RAM fallback. |

## Retained Packet — M1 T4 S8

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S8 follows accepted S7 P1 `f4ebdcf4f`; one session performs coordinator and executor roles. |
| Admission And Approval | Owner's 2026-10-19 authorization permits bounded L2 behavioral models from other emulators when L3 material is unavailable. No external code, tables, ROM content or protected trace may be imported. |
| Objective | Use the exact selected-set stop at `0x69a`, current reference-map-gap context and actual forward-progress replay to admit only the smallest stateless Board operation that advances the existing selected program. |
| Non-goals | No blanket `0x690--0x69f` acceptance, Video/Sprite/COP state, generic RAM, Shared/Core API change, source translation, ROM retention, physical-hardware claim or playable-game claim. |
| Reference Baseline | `f4ebdcf4f`; after exact routes at `0x692`, `0x696` and `0x69c`, both widths fault at `0x69a`. The current reference map leaves the local `0x690--0x69f` interval unassigned between named receivers. |
| Candidate Proposal | Extend the existing selected-control write-only no-state callback only by the exact smallest route if replay advances and local reads, inspection, mixed widths and neighbours remain rejected; otherwise retain/refine refusal. |
| Files And ABI Surface | `src/myraidenii/core/board.c`, its existing Board smoke, CURRENT and S8 evidence only if accepted. No public API or ABI change. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source-policy rules. External behavior is L2 context only. |
| Verification | x64/x86 focused Board test and selected-set probe with existing transient staging/cleanup; complete repository-only dual-width unit suites after production change; documentation governance and `git diff --check`. |
| Expected Markers | Exact two-byte route accepts only byte lanes or contained word writes, rejects reads/inspection/mixed widths/neighbours, and advances both selected-set probes beyond `0x69a`; otherwise report retained stop. |
| Asset Needs | Existing owner-supplied `O:\assets\raidenii\raiden2.zip`, read-only; existing probe stages only program lanes transiently and retains no payload, trace or screenshots. |
| Reporting Requirements | Record local stop and reference-map facts separately, L2 limitation, exact accepted/rejected boundaries, dual-width results, next stop and no playable-game claim. |
| Stop Conditions | Contradictory replay, stateful/read access, inaccessible exact width, required new owner/API, protected-data exposure or probe cleanup failure. |
| Exit Criteria | A bounded L2 disposition is complete: accepted Board-only route has focused/full dual-width proof and evidence, or refusal is retained/transferred. One reviewed MyRaidenII P is committed/pushed. |
| Original Owner Request | Do not require the owner to supply physical-level detail when a bounded other-emulator L2 model can make correct progress. |
| Similar-Issue Sweep | Recheck existing selected write-only routes and every immediately adjacent byte for accidental widening, duplicate ownership, readback or generic-RAM fallback. |

## Retained Packet — M1 T4 S7

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S7 follows accepted S6 P1 `e317a10b8`; one session performs coordinator and executor roles. |
| Admission And Approval | Owner's 2026-10-19 authorization permits bounded L2 behavioral models from other emulators when L3 material is unavailable. No external code, tables, ROM content or protected trace may be imported. |
| Objective | Use the exact selected-set stop at `0x692`, the current reference-map gap and actual forward-progress replay to admit only the smallest stateless Board operation that advances the existing selected program. |
| Non-goals | No blanket `0x690--0x69f` acceptance, Video/Sprite/COP state, generic RAM, Shared/Core API change, source translation, ROM retention, physical-hardware claim or playable-game claim. |
| Reference Baseline | `e317a10b8`; after the exact `0x69c--0x69d` route, both widths fault at `0x692`. The current reference map leaves its local `0x690--0x69f` interval unassigned between named receivers. |
| Candidate Proposal | Extend the existing selected-control write-only no-state callback only by the exact smallest route if the replay advances and local reads, inspection, mixed widths and neighbours remain rejected; otherwise retain/refine the refusal. |
| Files And ABI Surface | `src/myraidenii/core/board.c`, its existing Board smoke, CURRENT and S7 evidence. No public API or ABI change. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source-policy rules. External behavior is L2 context only. |
| Verification | x64/x86 focused Board test and selected-set probe with existing transient staging/cleanup; complete repository-only dual-width unit suites after a production change; documentation governance and `git diff --check`. |
| Expected Markers | Exact two-byte route accepts only byte lanes or contained word writes, rejects reads/inspection/mixed widths/neighbours, and advances both selected-set probes beyond `0x692`; otherwise report the retained stop. |
| Asset Needs | Existing owner-supplied `O:\assets\raidenii\raiden2.zip`, read-only; existing probe stages only program lanes transiently and retains no payload, trace or screenshots. |
| Reporting Requirements | Record local stop and reference-map facts separately, L2 limitation, exact accepted/rejected boundaries, dual-width results, next stop and no playable-game claim. |
| Stop Conditions | Contradictory replay, stateful/read access, inaccessible exact width, required new owner/API, protected-data exposure or probe cleanup failure. |
| Exit Criteria | A bounded L2 disposition is complete: accepted Board-only route has focused/full dual-width proof and evidence, or refusal is retained/transferred. One reviewed MyRaidenII P is committed/pushed. |
| Original Owner Request | Do not require the owner to supply physical-level detail when a bounded other-emulator L2 model can make correct progress. |
| Similar-Issue Sweep | Recheck existing selected write-only routes and every immediately adjacent byte for accidental widening, duplicate ownership, readback or generic-RAM fallback. |

## Retained Packet — M1 T4 S6

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S6 follows accepted S5 P1 `bc67b7bbc`; one session performs coordinator and executor roles. |
| Admission And Approval | Owner's 2026-10-19 authorization permits a bounded L2 model from other emulators where L3 material is unavailable. It permits behavioral facts only, not source/code/table/ROM import. |
| Objective | Use the exact selected-set fault point at `0x69c`, the current reference-map gap and actual forward-progress replay to admit only the smallest state-free Board operation that makes the existing program advance. |
| Non-goals | No blanket `0x690--0x69f` acceptance, Video/Sprite/COP state, generic RAM, reference implementation translation, Shared/Core API change, ROM retention, or physical-hardware-accuracy claim. |
| Reference Baseline | `bc67b7bbc`; dual-width bounded probe passes `0x696` then faults at `0x69c`. Current MAME RAIDEN II map has a gap between its sprite-buffer trigger at `0x68e--0x68f` and next receiver at `0x6a0`, providing L2 map-disposition context only. |
| Candidate Proposal | Extend the existing selected-control write-only no-state callback by one exact two-byte route only if the selected-set replay advances and all local read/inspection/neighbour/boundary tests retain refusal; otherwise retain refusal and record/transfer the result. |
| Files And ABI Surface | `test/myraidenii/integration/selected_set_startup_probe.c` for bounded diagnostic evidence; if proven, `src/myraidenii/core/board.c`, its existing Board smoke, CURRENT and S6 evidence. No public API or ABI change. |
| Applicable Rules | MyRaidenII guide, execution, architecture, coding, documentation and source-policy rules. The precise reference fact is L2; local program behavior is a bounded external integration observation. |
| Verification | x64/x86 selected-set probe with existing transient staging/cleanup; focused Board x64/x86 test; complete repository-only unit suites per width after production change; documentation governance and `git diff --check`. |
| Expected Markers | Both widths stop at the same named address before the change; a state-free exact route accepts only its two byte lanes and retains reads, inspection, mixed widths and neighbours as errors; the probe advances beyond `0x69c` or the refusal is retained with a justified record. |
| Asset Needs | Existing owner-supplied `O:\assets\raidenii\raiden2.zip`, read-only; existing probe stages only program lanes transiently and retains no payload, trace or screenshots. |
| Reporting Requirements | Record the selected-set fault fact separately from the reference-map fact, L2 limitation, exact accepted/rejected range, dual-width results, next stop and no playable-game claim. |
| Stop Conditions | Read/stateful access, access width not representable by existing callback contract, reference contradiction, required new owner/API, protected-data exposure, or failed staging cleanup. |
| Exit Criteria | A bounded L2 disposition resolves the operation; any accepted Board-only route has focused and full dual-width proof/evidence, or an incompatible operation is explicitly retained/refused and transferred. One reviewed MyRaidenII P is committed/pushed. |
| Original Owner Request | Do not require physical-level details from the owner; use other emulator observable behavior as an L2 reference when L3 material is absent. |
| Similar-Issue Sweep | Review each existing selected write-only route and the immediate `0x690--0x69f` neighbours for accidental widening, duplicate ownership, readback or generic-RAM fallback. |

## Retained Packet — M1 T4 S5

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S5 follows accepted S4 P1 `2276bbe2c`; one session performs coordinator and executor roles. |
| Admission And Approval | Owner authorized L2 behavioral models from other emulators when L3 material is unavailable on 2026-10-19; no external source import, translation, algorithm/table reuse or protected ROM retention is authorized. |
| Objective | Establish the exact selected-set-observed `0x696--0x697` V30 control write as a Board-local, write-only, no-state L2 hole, then prove that the real program advances past it on both host widths. |
| Non-goals | No Video/Sprite register, RAM, decoder, decryption, palette, renderer, Shared API, Core CPU change, generic control-window acceptance, or claim of original-hardware accuracy. |
| Reference Baseline | `2276bbe2c`; the bounded selected-set probe reaches a terminal fault with code `0x696` only after the completed `0x6ce--0x6cf` sprite-flags route. Current MAME RAIDEN II map is an L2 behavioral reference and does not assign this span a device receiver. |
| Candidate Proposal | Add one two-byte replacement route to the existing selected-control write-only hole owner; retain explicit rejection for all reads, inspection, adjacent bytes and wider mixed accesses. |
| Files And ABI Surface | `src/myraidenii/core/board.c`, its existing Board unit smoke, S5 evidence and CURRENT only. No public API or ABI changes. Expected production +5--10/-0--3; test +12--24/-0--4; refine after actual change review. |
| Applicable Rules | MyRaidenII guide, architecture/coding/source policy, execution and documentation rules; source policy permits L2 behavioral contracts but prohibits external implementation import. |
| Verification | Focused x64/x86 Board smoke; bounded selected-set probe on x64/x86 with transient staged lanes and cleanup; documentation governance, manifest if applicable and `git diff --check`; complete repository units per width before S closure. |
| Expected Markers | Synthetic actual Board writes accept only the exact two bytes and preserve no readback; selected set advances beyond `0x696`; all unrelated control addresses still reject. |
| Asset Needs | Existing owner-supplied `O:\assets\raidenii\raiden2.zip`, read-only and staged transiently by the existing probe; no bytes, screenshots or traces retained. |
| Reporting Requirements | Record exact route, L2 limitation, reference distinction, actual diff, synthetic/selected-set results, cleanup and any next stop without claiming playable output. |
| Stop Conditions | A read, stateful consumer, width other than the bounded word, contradictory reference behavior, required Shared/API change, protected-source exposure or probe cleanup failure. |
| Exit Criteria | Exact Board-only route, tests and evidence are complete; dual-width complete units pass; actual change review and one MyRaidenII P are committed/pushed. |
| Original Owner Request | When no physical-level evidence is available, use other simulators as L2 references rather than blocking progress; do not require the owner to supply more detailed materials. |
| Similar-Issue Sweep | Recheck the complete `0x400--0x7ff` control route table for accidental widening, duplicate device ownership, readback or generic-RAM fallback; classify every hit. |

M1 T3 remains open after accepted S1--S25:

| S | Delivered scope | Receipt |
| --- | --- | --- |
| S1 | B01--B09 board/COP contract and source-grade ledger | `e83f186a8`, `86c634958` |
| S2 | V30 construction, static program lanes, dynamic bank and board RAM/control routes | `dd2611e1b`, `9f941ae69`, `f2c30a1e6` |
| S3 | Main IRQ acknowledgement, active-low inputs and main/sound latches | `4d340ce41`, `d7b2c32b1` |
| S4 | Bounded owner-ROM COP/DMA address observation and receiving ledger | `da6d9d3a5`, `77110cf04` |
| S5 | Local COP configuration/register boundary and explicit unmodelled command failure | `e284fb386`, `5079476c4` |
| S6 | Bounded command/result/DMA receiver ledger and cleanup | `38e778ea8`, `e84ce8020` |
| S7 | COP evidence-grade decision and Video-owner transfer | `4cb7776f1` |
| S8 | Video-owned L2 raster/VBlank receiver, actual IRQ/reset proof and test build wiring | `34e22a100` |
| S9 | Finite initial COP upload/trigger inventory and first strict rejection | `456cde1fb` |
| S10 | Uploaded transfer family and approved Shared callback-fault publication repair; eight receiving releases | `f2ca9ae01`, `990ff6eda`, `af1649f60` |
| S11 | L2 position/velocity updates and common COP memory transport; explicit 2a05 non-eligible disposition | `b977bfa48` |
| S12 | Complete nine-ID geometry contract, result/config gaps and minimal Types review request; no code changes | `87a1ac547` |
| S13 | Complete four-ID L2 collision/result receiver, bus/failure/latch/reset proof and S12 status-prefix correction | `b3098766d` |
| S14 | Complete decimal register family, byte/mode/reset/guest proof, exact old failure-stage preservation and safe pointer offsets | `c28a162d5` |
| S15 | Complete COP-owned register inventory, explicit unmodelled routes and shared stored DMA-mode readback | `fb5e14f7b` |
| S16 | Complete low-control operational refusal, preserved valid/mixed routes and isolated actual-COP failure matrices | `acc9cc10c` |
| S17 | Independent L2 angle-approach memory effect, target/step configuration and true-byte Core DMA transport | `b3a7cc0a6` |
| S18 | Approved optional Types math declaration vocabulary, existing runtime contract and unchanged product linkage | `9b41a82ca` |
| S19 | Complete nine-ID L2 numeric/result chain, common dispatch status, bounded arithmetic and failure-prefix proof | `02a72979a` |
| S20 | Whole residual COP/RNG/DMA contract, existing live timeline capability and cohesive implementation receivers | `d20d65ee0` |
| S21 | Complete nominal L2 RNG aliases, existing timeline observation and actual Board/guest/reset proof | `bede72c2a` |
| S22 | Complete mode-banked DMA/copy/fill/sort receiver, one pending owner, and actual Board proof | `52fc45e98` |
| S23 | Complete Video-DMA/palette/brightness receiving-evidence gate; no invented implementation | `c48f9e02c` |
| S24 | Complete COP 2a05 offset-add receiver and bounded actual-bus proof | `7d37760f7` |
| S25 | Bounded selected-set startup diagnostic and explicit Board/COP-owner transfer | P1 `c75a016d0`; P2 state receipt; P3 owner correction |
| S26 | Selected-set `0x43e` write-only Board hole and bounded next-stop transfer | P1 `42845208d` |
| S27 | Exact `0x6c8--0x6c9` selected write-only span and bounded Sprite/Video transfer | P1 `f792feffa` |
| S28 | Sprite-flags receiver boundary and transfer to queued Video work | P1 `b2a2d00d5`; P2 closure receipt |
| S29 | Complete refusal/evidence boundary for `1905`, `9100` and `9900` | P1 `a5904833b`; P2 closure receipt |

The owner approved advancing Video's prerequisite inside T3 and delegates
routine same-T S planning. B04 now has an actual modeled periodic receiver;
full Video rendering and DMA qualification remain queued work. The next S
receives B07/B08 COP operation contracts/evidence and real-set startup proof.
It may not copy reference code, fabricate results or silently close unresolved
COP obligations. No further approval is needed for ordinary same-T ordering.

S26 admits the selected-set-observed `0x43e--0x43f` write-only hole as a
Board-local L2 route. The previous CPU fault is a correct V30 16-bit
`MOV moffs, AX` write, while the pinned reference map assigns no receiver to
that word. The route stores no state and has no readback; all other unknown
control addresses remain explicitly refused. The same two-width probe now
passes that point and reaches a new `0x6c8` write. That later address is not
absorbed by S26 and remains the next Board/Video map-evidence receiver.

Sequential coordinator actual-change review
accepts S10's Shared P1 f2ca9ae01, NXVM receiving P2 990ff6eda and MyRaidenII
P3 af1649f60. Both full unit suites pass687/687; selected PC receiving
integrations pass14/14 per width. Eight current0564 PC EXEs are rebuilt,
deployed and hash/PE verified; MyNES and protected inputs remain unchanged.
The [S10 review](../etc/evidence/t3-s10-cop-transfer-review.md) archives its
packet, code delta, incomplete first x64 attempt, final proof and cleanup;
[results](../etc/evidence/t3-s10-results.json) retain exact release identities.
Build now contains only the persistent .ccache/.ninja caches.

The complete S10 batch is upload/register admission plus the explicit L2
5205/5a05/f205 transfer family, not a generic programmable COP. Other B07/B08
numeric/geometry/collision/unresolved-trigger/DMA-sort operations and genuine
selected-set startup remain T3 obligations. S9's initialization observation
is not a new S10 ROM run. No complete external suite, desktop/RDP, physical
timing or playable game is claimed. T3 and M1 remain open; ordinary same-T S
planning remains owner-delegated.

Sequential coordinator actual-change review accepts S11 P1 b977bfa48.
0205/0905 now have explicit L2 macro effects and41 new actual guest/failure
cases in the existing executable;2a05 still rejects because its write width
and neighbouring-word effects are not established. Its same-T B07 semantic
reconciliation receiver is mandatory before T3 can close. The original
three-ID batch is not misreported as wholly implemented.

Both final full suites pass687/687 with no failures/skips (x64 106.12s,
x86 126.27s). Ten EXEs and protected inputs remain unchanged; only local
COP source/test and MyRaidenII records changed. Build contains only caches,
with no owned temporary output left. The [S11 record](../etc/evidence/t3-s11-cop-state-update.md)
retains the complete packet, source identities, actual delta, qualification
and explicit unverified boundaries. S11 is closed; its execution qualification remains unchanged by S12.
Other B07/B08 numeric/geometric/collision/DMA/result and real-set startup
obligations remain open. S9's22 uploaded IDs are not the full game's universe.

S12 research/design is accepted after coordinator actual-change review of
P1 87a1ac547. The [complete geometric receiver](../etc/evidence/t3-s12-cop-geometry-contract.md)
archives all nine forms, shared result/configuration dependencies, L2
limitations, independent oracles, exact future file/API estimate and packet.
No production/test/build/manifest or artifact changes occurred. Ten release
identities and protected inputs remain unchanged; document checks pass and
build contains only persistent caches. No new unit or game run is claimed.

The S12 proposal requested explicit owner approval for four optional Lib Types
mathematical names across six Shared files. That specific approval has now
been received and is being executed by S18; general product approval was not
used as a substitute. No workaround, raw CRT consumer access or private FPU
dependency is introduced. The next numerical receiver must consume this complete
numeric/result ledger and its evidence choices, not one first-failing opcode.
T3 and the full playable MyRaidenII goal remain open;
2a05, numeric/unresolved/DMA/result and genuine startup retain their original
required receivers. S12 is a complete contract audit, not implemented geometry.

Sequential coordinator actual-change review accepts S13 P1 b3098766d.
The complete four-ID collision batch now has explicit L2 position/bounds
capture and copied read-only results through the existing Core bus/service
path. The [S13 record](../etc/evidence/t3-s13-cop-collision.md) retains the
original packet, containment revision, actual delta, independent test matrix,
source/test/log hashes and qualification limits. It also corrects the S12
numeric entry-status contract without implementing geometry.

Final complete units pass687/687 per width (x64 146.13s; x86 148.06s);
the earlier503/687 deadline attempt is not counted as passing. Shared, all
other Apps, ten releases and protected inputs remain unchanged. All owned
processes are terminal and build contains only persistent caches. S13 is
closed; T3 and playable MyRaidenII remain open.
The remaining numeric/2a05/unresolved/DMA/result/startup batches keep their
mandatory receiving requirements; the accepted S14 decimal receipt follows.

Sequential coordinator actual-change review accepts S14 P1 c28a162d5.
The full decimal input/mode/result family now has an explicit original L2
implementation, independent literal oracles and actual V30/provider proof.
The [S14 record](../etc/evidence/t3-s14-cop-decimal.md) retains its packet,
failure-stage reconciliation, pointer review, actual delta and exact hashes.

Final frozen-source full units pass687/687 per width (x64 109.49s,
x86 129.18s). Shared, other products, ten release identities and protected
inputs remain unchanged. Owned processes are terminal and no disposable
task root exists; build contains only persistent caches. S14 is closed with
no active S packet at its acceptance. T3 and the complete playable MyRaidenII goal remain open;
numeric/math authorization,2a05, unresolved operations, remaining COP/DMA
decode and selected-set startup retain their required receivers.

Sequential coordinator actual-change review accepts S15 P1 fb5e14f7b.
The [S15 record](../etc/evidence/t3-s15-cop-register-inventory.md) archives the
complete map ownership inventory, packet, failure-stage reconciliation,
source/test/log hashes, actual delta and verification limits. Five missing
COP groups no longer succeed as RAM; unimplemented algorithms still fail
explicitly. The existing DMA mode shares stored GPR readback, with no new
state, public API or execution path.

Both final complete unit suites pass687/687 (x64 119.11s; x86 126.84s).
Shared, other Apps, ten releases and protected inputs remain unchanged.
Owned processes are terminal and build contains only persistent caches.
S15 is closed with no active packet. T3, M1 and playable MyRaidenII remain
open; the full low-control window, numeric/2a05/DMA/unresolved operations
and genuine startup retain their mandatory receivers. The independent
Shared complete-span query discrepancy is recorded in TODO, not repaired.

Sequential coordinator actual-change review accepts S16 P1 acc9cc10c.
The [S16 record](../etc/evidence/t3-s16-board-control-window.md) archives its
packet, one fallback-route design, exact retained13/25/14-row COP matrices,
actual Board CPU transfer/fault proof, frozen identities and qualification.
Every400h--7ffh byte has a device disposition; unknown data accesses no
longer succeed as hidden RAM. Existing valid multiword devices and real
boundary prefixes retain their original Core-owned transfer behavior.

Final strict cached builds and full units pass687/687 per width (x64
122.30s; x86 133.90s). Shared, other Apps, ten releases and protected inputs
remain unchanged. Owned processes are terminal; build contains only caches.
S16 is closed with no active packet. T3/M1 remain open: the Shared wide-query
classification debt, numeric/math authorization,2a05, remaining algorithms
and real-set startup retain their original receiving obligations. No desktop,
RDP, integration-suite or playable-game qualification is inferred.

Sequential coordinator actual-change review accepts S17 P1 b3a7cc0a6.
The [S17 record](../etc/evidence/t3-s17-cop-angle-approach.md) archives the
whole nine-ID dependency disposition, packet, independent L2 circular model,
actual53 command cases, byte/failure-prefix proof and exact run identities.
6200's memory effect and target/step capability now use the existing COP
store, admission and single Core DMA/safe-point path without new state/API.

Strict cached builds pass both widths. Full units pass687/687: x64 117.73s
before one final fixture row, followed by the final whole affected-unit
1/1 pass; x86 final687/687,129.64s includes that row. Evidence does not claim
the earlier x64 full run included it; unchanged686 binaries retain their
full proof. Shared, ten releases and protected inputs remain unchanged.
Owned work is terminal and build contains only persistent caches.
S17 is closed with no active packet; T3/M1 remain open. Common arithmetic
status/results, eight other numeric IDs, the explicit Shared math approval,
2a05/RNG/DMA/unresolved forms and genuine startup retain their full receivers.
There is still no playable MyRaidenII or desktop/RDP qualification.

Sequential coordinator actual-change review accepts S18 Shared P1 9b41a82ca.
The [S18 record](../etc/evidence/t3-s18-types-math.md) retains explicit owner
approval, the six-file scope, archived packet, exact aliases/link ownership,
actual delta and dual-width log identities. The additive optional header
provides no algorithm/state and leaves every existing interface unchanged.

Final full repository units pass687/687 per width, zero failures/skips
(x64 113.60s; x86 126.35s). Strict cached builds and subsequent no-work
incremental builds pass. Ten existing releases,33 tracked asset guards and
six media-role hashes remain unchanged; no existing product includes this
header and no release rebuild is required. Owned runs have terminated and
build contains only persistent caches. S18 is closed with no active packet.
The complete nine-ID numerical/result receiver is now unblocked by the math
vocabulary prerequisite, but remains unimplemented/unqualified as recorded
in S12/S17. T3/M1,2a05/RNG/DMA/unresolved forms and genuine startup remain
open; no desktop/RDP, external integration or playable-game pass is claimed.

S19 executor delivery completes the full nine-ID L2 numeric/result receiver.
The [S19 record](../etc/evidence/t3-s19-cop-numeric.md) retains all dispositions,
actual-bus/failure proof, the x87 truncation correction, frozen identities and
actual production+136/-3, test+333/-2 delta. Final complete units pass687/687
per width (x64 116.99s; x86 130.43s), zero failures/skips. Shared, ten release
identities and protected inputs remain unchanged. Sequential coordinator
actual-change review accepts P1 02a72979a against the complete nine-member
ledger and packet. The common register/dispatch/DMA path, all old literal
matrices and exact frozen source/test identities remain intact. S19 is closed
with no active packet and no owned temporary work left. T3/M1 remain open:
2a05, remaining unidentified/unresolved forms, RNG, sorting/DMA and genuine
selected-set startup retain their mandatory receivers. No rendering, sound,
external integration, desktop/RDP or playable-game qualification is claimed.

S20 executor delivery reconciles the complete residual COP/RNG/DMA batch in
the [residual contract](../etc/evidence/t3-s20-residual-contract.md). Existing
Core timeline observation removes the suspected RNG Shared-interface gap;
mode-banked DMA, Video latches, both2a05 outputs and unknown macro semantics
remain distinct required receivers. No source/test/artifact changes or new
dynamic qualification occurred. Sequential coordinator actual git show
review accepts d20d65ee0's four-file209-line record against the complete
residual ledger. Source/test delta remains zero and S19's exact687/687 dual
qualification is retained, not rerun or relabelled. Ten releases and all
protected guards are unchanged; no temporary work exists. S20 is closed with
no active packet. Ordinary local RNG implementation is unblocked without a
Shared edit; DMA, Video and residual/startup obligations keep T3/M1 open.

S21 executor delivery completes the mapped RNG family using existing Core
timeline observation, with no new state/API or Shared changes. The
[S21 record](../etc/evidence/t3-s21-cop-rng.md) archives the packet, explicit
L2 limit, actual production+18/-1 and test+121/-4 delta, independent raster
oracles and runtime guest MMIO. Final full units pass687/687 per width,
zero failures/skips (x64 387.69s; x86 171.12s). Strict cached and no-work
incremental builds pass. Ten existing releases and all protected guards are
unchanged; no affected existing EXE requires rebuilding. Owned runs are
terminal and build contains only persistent caches. Sequential coordinator
actual git show review accepts bede72c2a's six-file+298/-6 delivery against
the full RNG contract, archived packet and frozen qualification identities.
S21 is closed with no active packet. DMA, Video latches, remaining macros
and real selected-set startup keep T3/M1 open.

Sequential coordinator actual-change review accepts S22 P1 `52fc45e98`.
The COP has one raw-mode-selected 512-bank setup owner and replaces its prior
single pending-command fields with one discriminated pending operation; no
second queue, Core DMA route, public interface, Shared component, ROM, or
artifact path was added. The existing Board executable proves byte lanes,
bank isolation, invalid admission, copy/fill units, overlap, stable signed
sort, unknown sort ordering and reset. Final complete cached units pass
687/687 per width, and protected release/media hashes match after restoring
two unrelated MyNES EXEs rewritten by an aggregate build. The S22 evidence
archives exact hashes, L2 limits and remaining receiver boundaries. S22 is
closed with no active packet; T3/M1 remain open for Video-owned DMA, residual
macros and real selected-set startup.

Sequential coordinator actual-change review accepts S23 P1 `c48f9e02c`.
The receiver-3 evidence gate covers all ten modes (`14`, `15`, `80`--`87`),
their common registers and their private Video ownership without source import,
ROM inspection, special-case address, API or code change.  It correctly blocks
implementation until selected-set descriptor and palette/brightness oracles
exist; no unqualified mode is reported as a no-op or a RAM copy.  Documentation
governance and `git diff --check` pass.  S23 is closed with no active packet;
T3/M1 remain open for residual macros and genuine selected-set startup, while
the queued Video task owns later rendering/DMA qualification.

Sequential coordinator actual-change review accepts S24 after the final frozen
two-width qualification.  `2a05` now uses the one existing uploaded-command,
pending-operation and checked Core DMA route: for each slot it reads the
unsigned delta at `B1+1e+4i`, then writes widened sums at `B0+6+4i` and
`B0+1e+4i` in that order.  The existing Board executable proves all three
slots, carry, input/output ordering, pending/reset, source failure and late
DMA-lane prefix behavior.  Final full repository-only units pass687/687 per
width (x64 106.24s; x86 125.78s).  No Shared/public ABI, external source,
ROM/media, artifact, integration, desktop/RDP or playable-game claim changed.
The [S24 record](../etc/evidence/t3-s24-cop-offset-add.md) retains the
complete bounded model and frozen hashes.  S24 is closed.  T3/M1 remains open
for unresolved `1905`/`9100`/`9900`, Video qualification and genuine
selected-set startup.

S25's [bounded selected-set diagnostic](../etc/evidence/t3-s25-selected-set-startup-diagnostic.md)
uses the verified external program lanes without retaining protected payload.
It stops identically on x64 and x86 at a CPU-internal fault caused by a write
to `0x43e`, before a COP command checkpoint. A bounded Debug stack observation
confirms V30's existing 16-bit `MOV moffs, AX` branch, not a 32-bit CPU path.
This address is not established as a Board/COP register, so S25 does not widen
a route or weaken the explicit control window. Its owner is an open
MyRaidenII Board/COP mapping prerequisite; T3 remains open for that and its
existing COP/Video/startup receivers.

S26 accepts only the narrow selected-set `0x43e--0x43f` write-only Board
route. The existing Board executable proves that it does not supply readback
or weaken any other control-window rejection. The two-width selected-set probe
then advances from the prior stop by two instructions to the same new `0x6c8`
write on both widths. That later address is explicitly not absorbed: its
Board/Video receiver remains open, along with the existing COP, Video, and
startup requirements. See the [S26 record](../etc/evidence/t3-s26-control-write-hole.md).

S27 accepts only the independently evidenced `0x6c8--0x6c9` write-only span;
the real V30 word store reaches its two byte lanes and retains no state or
readback. Both selected-set probes next stop at the named `0x6ce--0x6cf`
Sprite/Video flags receiver. The [S27 record](../etc/evidence/t3-s27-selected-control-noop.md)
retains its exact boundary, cleanup and final 687/687 dual-width qualification.
S27 is closed; T3/M1 remain open for the receiver and genuine selected-set
startup, without any playable-game claim.

Sequential coordinator review accepts S28's source-policy-bounded disposition:
the selected write at `0x6ce--0x6cf` has a named Sprite/Video owner but no
independently qualified state, reset, decryption or frame contract. The
existing `0x6cb` program-bank receiver remains correct despite the older
reference-file comment naming `0x6c9`: the same fixed Raiden II map explicitly
maps the bank byte at `0x6cb` and leaves `0x6c8--0x6c9` unassigned. S28
therefore leaves S27's narrow no-op intact, imports no reference algorithm,
and transfers the flags receiver to the queued Video task. Documentation-only
checks pass; no source, test, ABI, ROM/media or executable changed. S28 is
closed with no active packet. T3/M1 remains open for Video, sound, Product and
genuine selected-set startup qualification.

Sequential coordinator review accepts S29's complete three-form disposition.
The local classifier remains the only command admission/advance path; all three
forms still reject before creating a pending operation or result side effect.
No original hardware evidence was found to establish their operand/output,
reset or failure contracts, and source-policy rules prohibit deriving those
contracts from reference comments. The blockers remain T3-owned and are not
misrepresented as aliases, no-ops, Video or Sound work. Documentation-only
checks pass; no source, test, ABI, ROM/media or executable changed. S29 is
closed with no active packet. T3/M1 remains open for the three forms, Video,
sound, Product and genuine selected-set startup qualification.
