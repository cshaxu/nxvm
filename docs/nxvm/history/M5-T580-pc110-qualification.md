# M5 T580 — MyPC110 product and guest qualification

## Admission

T580 consumes the existing [MyPC110 qualification proposal](../proposals/m5-pc110-qualification.md)
after T579's bounded real-firmware source gate.  Its end state is four
build-fixed artifacts: original-DSTN x86/x64 and upgraded-TFT x86/x64.  The
task may only claim those artifacts runnable after their specific firmware,
panel, machine, lifecycle, configuration and guest proof gates have passed.

S1 is an evidence-led convergence audit.  It must enumerate every qualification
member and place each in a bounded successor, a source/asset-gated transfer or
an evidenced rejection.  It cannot turn the existing partial firmware trace
into a product acceptance or add a guessed fallback.

## S43--S44: actual timer calibration receiver

S43's sampled execution identifies a deliberate BIOS error halt and a rejected
PIT/refresh delta, superseding the unsampled delay-throughput interpretation.
S44 implementation `1f30224c3` corrects only the App's clock configuration and
existing firmware harness. Both widths report timer pass with delta 1668;
current unit coverage is 712 cases per width, with x64 timeout reruns explicitly
recorded. The code/build-registration delta is +100/-119, net -19.

The [S44 evidence](../etc/evidence/t580-s44-pit-refresh-clock.md) and
[original qualification ledger](../etc/evidence/t580-s1-pc110-qualification-ledger.md)
retain source grades, retained manual views and open product requirements.
Preview construction is verified; complete POST, display, guest and release
qualification remain open. No PC110 product is published by this delivery.

## S45: post-timer reset and firmware service wait

The bounded x64 continuation passes three processor-reset boundaries
and reaches the memory-flag wait calling the 15EE/AX=5380/BH=84 service. It does
not reach a boot interrupt within 200M instructions. No production or test
change remains after removing the sampler. [S45 evidence](../etc/evidence/t580-s45-post-timer-reset.md)
records the actual copied observations and the complete next receiver batch.

## S46: SMI/SMRAM mechanism audit

The [S46 audit](../etc/evidence/t580-s46-smi-smram-receiver-audit.md)
proves that the existing CPU can take SMI but the App has not prepared the
handler's active memory view. It rejects isolated pin, cold-constant and
C-shadow candidate changes, all of which were withdrawn. The next coherent
repair boundary is the complete board cold-decode/SMRAM/SMI mechanism, not a
host BIOS service or another instruction-specific patch.

## S47: port-width prerequisite and configuration receiver

Owner-approved Shared P1 `d603e81e7` preserves actual write widths through the
existing port callback; read callbacks remain unchanged. The App receiver
implements the source-labelled L2 SL-compatible byte/word unlock and reset
protocol, verified by actual CPU OUT instructions. Both widths retain timer
calibration success, and all 712 current unit cases have passing evidence
with explicitly recorded reruns. Eight receiving PC artifacts are refreshed
at 0580; MyRaidenII receives a separate same-version artifact refresh.

The [S47 evidence](../etc/evidence/t580-s47-cold-initialization-contract.md)
records the actual delta, cache repair, test qualifications and artifact hashes.
The complete SMRAM/SMIACT/initializer/trigger receiver remains open; this
delivery does not establish full POST, DOS or a publishable PC110 product.

The subsequent S47 shadow receiver implements the per-16-KiB C--F access
fields and source-labelled E/F Flash view, retiring App index/write-state
mirrors, prefilled guest shadow and the F9 hard-gate hypothesis. Both widths
execute the unchanged initializer, handler copy and return; enhanced existing
timer integration passes with delta 166E after one processor reset. The
[S47 evidence](../etc/evidence/t580-s47-cold-initialization-contract.md)
separates this direct proof from still-open SMRAM isolation, SMIACT forwarding,
trigger/RSM and boot-only Flash selection. S47 and T580 remain active.

The final S47 continuation uses Shared `794380f69` for CPU/DMA-qualified
memory routing, bounded cache-view invalidation, existing SMIACT forwarding,
chip ASMI and the missing F65535 DAC status read. The App owns the separate
SMRAM contents and hardware decode. Full BIOS caller reconciliation rejects
the draft's bit-7 trigger and establishes source mask 20 from the initializer
that registers the vendor handler and enables ASMI. The actual BIOS executes
that service, acknowledges it and returns through RSM on both host widths;
timer delta is 166C. All 712 repository unit cases pass per width, all four
receiving PC INI identities pass and the affected eight PC/two MyRaidenII
artifacts rebuild. Lib, Emulator and MyNES stay unchanged. Evidence and actual
source/test counts are in the [S47 record](../etc/evidence/t580-s47-cold-initialization-contract.md).
Coordinator actual-change review accepts Shared `794380f69`, NXVM `782f09ebb`
and MyRaidenII `5ed407b4b`. S47 is closed for the complete cold-initialization
receiver. This is not complete POST, guest or PC110 product qualification;
T580 remains open with the unchanged final product predicate.

The following is its retained pre-acceptance packet, not an active task or a
second current authority. The acceptance statement above supersedes its
in-progress review wording.

### S47 retained pre-acceptance packet

Approved write-width prerequisite and App configuration receiver are implemented;
Shared P1 is `d603e81e7`. Both widths have passing evidence for all 712 unit
cases (including explicitly recorded timeout/cache reruns), unchanged BIOS timer
calibration and four deployed PC INI identities. Eight PC EXEs are refreshed
at 0580 and the Core-linked MyRaidenII pair at 0007. See the
[S47 verification and remaining contract boundary](../etc/evidence/t580-s47-cold-initialization-contract.md).
The active-view receiver is implemented in Shared P8 `794380f69` and its
App continuation. Both widths pass all 712 repository unit cases, actual BIOS
ASMI/vendor-handler/RSM/timer and four receiving PC INI identities. Eight PC
and two Core-linked MyRaidenII artifacts are refreshed. S47 awaits final
coordinator acceptance; T580's later POST/guest/product rows remain open.
No PC110 boot or product release is claimed.

Coordinator actual-change review accepts the bounded write-width prerequisite
and App protocol delivery: Shared P1 `d603e81e7`, NXVM P2 `2fe38e9af` and
receiving MyRaidenII P3 `98b49cfde` are pushed. Review confirms one existing
callback path, unchanged reads/error propagation, mechanical legacy adapters,
actual CPU unlock/lock/reset tests and unchanged Lib/Emulator/owner inputs.
The receiver evidence names the remaining CPU-only SMRAM and SMIACT contract
gaps explicitly; this acceptance does not close S47's complete cold-memory
batch or T580. No extra desktop/guest qualification is inferred from unit or
INI identity tests.

The receiver now implements one register-qualified C--F shadow decoder instead
of prefilled C/F buffers and a shadow-write Boolean. The unchanged BIOS reaches
its initializer, copies the 38000 handler and returns on both host widths.
The existing timer integration also requires the copy and initialization
markers; both widths pass. The previously assumed F9 hard gate is retired as
an unproven L2 model contradicted by the actual caller sequence. The
[S47 record](../etc/evidence/t580-s47-cold-initialization-contract.md)
preserves both the superseded observation and the new proof. The active-view
continuation now implements CPU-only SMRAM, SMIACT and ASMI acknowledgement/
rearm, with actual vendor-handler and RSM proof on both host widths. Final
regression/publication review is in progress; no boot qualification is implied.

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T580 S47, after accepted S46. |
| Admission And Approval | Owner's continuing autonomous PC110 implementation direction, followed by explicit approval of the existing port-callback access-width extension. Apply its narrower write-only form. No change to Lib/Emulator or protected inputs is admitted. |
| Objective | Reconcile the entire cold ROM/shadow/SMRAM initialization route and its board controls, then repair the established App-owned mechanism rather than repeating isolated SMI pulses or initial-value changes. |
| Non-goals | No firmware PC hook, injected handler, fabricated BIOS result, host-written wait flag, replacement CPU path, second clock or claim that a sampled prefix qualifies boot. |
| Reference Baseline | 98889bc39; unchanged external TFT BIOS, S44 clock proof, S45 wait and S46 rejected hypotheses. |
| Candidate Proposal | MyPC110 qualification; consume the complete board/memory/real-firmware receiver batch in the existing qualification ledger. |
| Files And ABI Surface | Targets Shared, NXVM and receiving MyRaidenII artifacts, delivered in separate P commits: add access_bytes only to the existing X86 Core port write callback, propagate real width or split-lane width 1, migrate X86 NXVM/DeskPro/PC110 production and test callers, update matching manifests; PC110 firmware/configuration/plan and existing tests remain the cold-initialization receiver. Refresh four PC pairs at current T580 version, synchronizing the stale family version header; MyRaidenII retains 0007. MyNES has no Core link and remains untouched. Lib/Emulator stay unchanged. No second callback path or inferred width from value. |
| Applicable Rules | One ROM/RAM decode owner and real CPU path; complete-batch source reconciliation; L2 inference labelled separately from exact source facts; no protected source import. |
| Verification | Inspect full firmware initialization and all current App controls before modification. Existing Core tests prove unsplit 1/2/4-byte and split one-byte write callbacks, unchanged values and errors. Run affected Core/Board/App tests in both stable Ninja trees and rebuild every affected admitted PC artifact; MyNES/MyRaidenII inputs are reviewed and unaffected consumers need no manufactured artifact. The grouped C/E/control/copy observation is retained as evidence. Proven PC110 repairs require focused dual-width App regressions and unchanged-BIOS replay. |
| Expected Markers | Actual initialization entry/return or the complete rejecting predicates, all relevant decode-control writes and copied destination state; for repair, actual handler fetch and RSM return through the existing CPU path. |
| Asset Needs | Existing immutable embedded TFT BIOS/font and separately labelled DSTN-derived input; preserve external masters and owner mypc110.ini. |
| Reporting Requirements | Whole-batch design and estimated source/test delta before retained changes; actual source grades, review, verification and remaining qualification rows. No per-instruction mini-S or new permanent framework. [S47 observation and approved width contract](../etc/evidence/t580-s47-cold-initialization-contract.md) records the grouped observation and write-only migration estimate +100--180/-60--110 source/test lines. |
| Stop Conditions | Any guessed control-register side effect or required unapproved Shared contract stops that implementation. Diagnostics stop at the semantic boundary, 200M instructions, 600 seconds or 64 KiB output. Own and confirm exit of each background process. |
| Exit Criteria | A complete, source-labelled receiver repair with its actual-path proof, or an exact reconciled contract gap without speculative production code; temporary diagnostics removed, evidence retained and applicable gates run. |
| Original Owner Request | Implement PC110 through usable, verified fixed products under the established build/test/publication rules. |
| Similar-Issue Sweep | Audit C/E/F ROM and shadow sources, both constructor/reset paths, protected bootstrap aliases, SMRAM initialization/active decode, reset persistence, trigger and clear/rearm as one mechanism. |

### S47 decoder implementation brief

Under the owner's continuing X86 Core/chip and App implementation authorization,
replace the App's prefabricated C and always-writable F paths with one C--F
shadow allocation and register-qualified per-16-KiB access. The complete cold
decoder batch includes external C/D absence, source-recorded E/F Flash banks,
C/E/F access fields, high reset Flash alias, full-reset decode and CPU-only-reset
preservation. External Flash stays immutable; the guest performs copies. Use
the existing route owner and one side-effect-free indexed-register read in the
chip; delete App index/write-state mirrors and the old write-notification API.
Existing chip/App fixtures cover all four access choices, boundaries, actual
instruction/data reads and reset. Preserve the separately open SMRAM/SMIACT and
trigger receiver rather than claim the whole S repaired from a decoder unit.
Estimated source delta +150--230/-110--170, existing tests +90--150/-25--60;
Lib/Emulator and owner inputs remain out of scope. Firmware-path evidence and
source grades must determine retention, not just passing synthetic tests.

The first decoder replay exposes an in-scope protocol conflict: the real
F3B72/F3BE9 return closes the formerly assumed F9 gate, then its caller invokes
F3C52 with ordinary EC/ED accesses and no FB. The old model returns FF and
discards the required writes. Reconcile the complete caller/return sequence
against the always-accessible reference models; retire that unproven hard gate
and its Boolean as L2 compatibility, rather than add a BIOS-address exception.
S30's historical evidence remains intact and is explicitly superseded for this
mechanism. The diagnostic is stopped once this conflict is proved, not called
a failed firmware qualification or allowed to consume its whole time budget.

The implemented decoder uses Shared P6 `86976f86d` for the observational chip
read. Both stable graphs rebuild the affected App units and TFT/DSTN previews;
all 712 current unit cases pass on each host width. The existing real-BIOS
timer integration passes with copy/initializer markers and delta 166E on both
widths. Actual C/H delta against `24bfb9ce9` is +313/-216, net +97 across 12
paths; production is net +3 and the rest is existing-test coverage. No admitted
published App links the changed VL82C420 library; earlier S47 artifacts remain
current and PC110 previews remain unpublished. The next complete receiver is
CPU-only SMRAM visibility, Board SMIACT forwarding and qualified source/return,
not an unconditional SMI pulse or a host-written service result.

### S47 active-view design reconciliation

Before retaining the SMRAM/source receiver, collect the unchanged BIOS's complete
OMDCR/unit-selection, Block 2 source/enable/rearm and software-port sequence up
to initialized service dispatch in one bounded observation. The existing App
integration harness owns this temporary sampler (600 s, 200M retirements,
64 KiB output); remove it after preserving source-labelled conclusions.
Original SL pages 169--170 require CPU-only SMRAM while DMA still accesses
ordinary RAM. Compare extending the existing route-mode admission with widening
every device callback; prefer the narrower single resolver, no extra clock or
public parallel callback. No speculative trigger or Shared memory contract is
implemented until that whole decode/source design is reconciled.

The grouped retirement stream identifies another existing coupling defect:
the initializer's ordinary CPU-lock word write (22h=0101) also relocks Block 2
in the App. Subsequent firmware Block 2 reads return FF and writes disappear,
despite no corresponding indexed-bank lock. Original SL's separate CPU and
82360 configuration mechanisms plus the complete caller sequence reject this
coupling. Remove those two cross-lock calls, retain the CPU's own lock/status
and the chip's explicit/reset protocol, and revise the existing route assertions
to prove independence. Estimated retained App source -2 lines, existing tests
+10--20/-5--10; no Shared change. Reconcile B6 global enable, B7 source, FA
rearm and OMDCR together before implementing any SMI trigger.

The corrected indexed sequence now reaches B6=90 after initialization (rather
than discarded writes/FF reads), while the earlier 15EE request runs with B6=0.
Original SL pages 59/84 identify ASMI at index 84/85 and B5 bit 2; the unchanged
firmware registers its vendor ASMI service for mask 20 in the same initializer.
Reconcile these as an explicit L2 ASMI receiver:
chip owns B5/B6 enable, B7 pending/acknowledgement and FA rearm; App wires its
configured software port to the existing Core SMI input, never a BIOS-PC gate.

Under standing X86 Core/chip/Board implementation authorization, the cohesive
active-memory receiver may add one CPU-only overlay mode to the existing Core
route enum, carry CPU/DMA admission privately through the same resolver, and
forward the existing SMIACT output through Board's profile binding. Keep public
device callback signatures unchanged. One bounded decode-change notification
at Core/CPU invalidates stale cache/prefetch state; reuse the cache-range owner,
without honoring a snoop-disable bit for a changed memory view. The App's one
firmware allocation includes the distinct 64 KiB SMRAM view, controlled by
IBU-selected 302 bit 3 or physical SMIACT. Full reset clears transient view and
signal state; CPU-only reset preserves guest-prepared contents. Lib/Emulator
remain untouched. Estimated source +220--330/-60--110, existing tests
+110--180/-20--45. Every affected admitted PC/MyRaidenII artifact must rebuild;
no PC110 release or S47 closure without actual BIOS SMI/RSM and regressions.

The rejected bit-7 trigger trial reaches the prepared power handler, reads
B7=80 and acknowledges it with 7F. Its guest video-state save exposes one concrete missing
F65535 DAC operation: read 3C7 is registered but returns UNSUPPORTED. Sweep all
four DAC ports at that chip, add the ordinary read/write-mode status latch
(3 in read mode, 0 in write mode), and extend its existing protocol fixture.
Estimated extra source +5--8 and tests +8--15; no API, renderer or BIOS-address
exception. The SMM path is not accepted until the real handler returns.

Whole-caller review rejects the draft's B7=80 ASMI assignment: the unchanged
initializer at Flash 2 B3FF registers its vendor service with source mask 20,
then programs 84/85 and B5 bit 2 in the same function. Mask 80 instead reaches
the suspend path ending in FF=82/HLT. Use the source-qualified L2 bit-5 request,
preserve W0C acknowledgement and FA rearm, and require actual service/return
observations before accepting the receiver. The rejected bit-7 trial is not
a product or firmware defect and does not justify bypassing the guest HLT.

Both widths now pass the enhanced unchanged-BIOS integration: initialized copy,
natural ASMI, vendor service entry, RSM and timer delta 166C. Whole-repository
unit regression and receiving artifacts are still under verification. Task-local
verification logs, if needed, belong only to `build/t580-s47-verify` (S47 owner,
2400-second phase budget, 8 MiB); preserve results/hashes in evidence and remove
them after the associated native processes exit. The PDF scratch directory was
removed after source hashes and the externally archived original were recorded.

## S48 accepted route audit

Coordinator actual-change review accepts `368a765f2`: the complete route batch
separates working staged resets and later ASMI/vendor/RSM from absent FDC
reset/IRQ wiring and independent lower Flash selection. Both actual firmware
widths remain before INT 19h. The temporary sampler is removed, restored App
units pass 11/11 per width, documentation gates pass, and task logs are safely
removed after their facts/hashes are recorded. No production/test, manifest,
artifact or owner-input change remains. S48 closes as an audit, not PC110
qualification; T580 continues with the existing FDC receiver first.

### S48 retained pre-acceptance packet

+## M5 T580 S48 — Post-initialization POST and boot-route reconciliation

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T580 S48, next unused S after accepted S47. |
| Admission And Approval | Owner-directed autonomous PC110 implementation; existing instruction to continue through T closure, reaffirmed on 2026-10-25. No new Shared implementation is admitted by this diagnostic packet. |
| Objective | Reconcile the complete post-initialization batch in the qualification ledger: staged processor resets, firmware services, Flash view selection and actual boot-route entry. Identify coherent receiver requirements before further implementation. |
| Non-goals | No firmware interception, synthetic service, guest flag/register forcing, ROM/media/INI mutation, product publication or Lib/Emulator change. An instruction/wall budget is not POST success. |
| Reference Baseline | Clean pushed 9fb5d7d33; accepted S47 cold receiver and actual vendor ASMI/RSM proof. |
| Candidate Proposal | [PC110 qualification](../proposals/m5-pc110-qualification.md), consuming the post-initialization and boot-route batch of its [ledger](../etc/evidence/t580-s1-pc110-qualification-ledger.md). |
| Files And ABI Surface | Existing App integration harness may receive a temporary bounded sampler; NXVM evidence/current/history only retained for audit. Estimated temporary test +30–80/-0 lines, removed after evidence; production and public ABI +0/-0. |
| Applicable Rules | Execution coverage convergence, bounded background diagnostics and actual-change review; Architecture one execution/time/data owner; Coding reuse existing fixture and no duplicate production path; source policy external immutable firmware and read-only references; Documentation single active packet. |
| Verification | Reuse original x64 Ninja/ccache tree and actual embedded TFT BIOS. Continue normal execution through reset boundaries; record semantic checkpoints, bounded instruction/I/O snapshots and terminal disposition. Corroborate a meaningful new checkpoint on x86; run documentation and whitespace gates for retained documentation-only delivery. |
| Expected Markers | Actual timer pass and vendor return followed by classified reset/service/Flash activity; INT 19h, boot-block transfer or an exact bounded non-success disposition. No ceiling-as-success result. |
| Asset Needs | Existing admitted external TFT BIOS/font, pinned PC110-EMU/PC110-QEMU/Open-Source-PC110 references. No new protected input or owner configuration change. |
| Reporting Requirements | Initial plan/estimate, observed progress and final exact proof limits; distinguish whole batch inventory from one repaired prefix and preserve sources, output hashes and remaining receiver. |
| Stop Conditions | Each owned observation at most 200 million instructions or 600 seconds and 64 KiB output; stop on terminal fault/CLI-HLT or actual boot-route milestone. Temporary directory build/t580-s48-route, this S owner, at most 4 MiB; archive required facts then remove it after process exit. |
| Exit Criteria | Complete affected route inventory, actual unchanged-firmware observations, explicit accepted/unqualified dispositions and a coherent next implementation design; temporary sampler removed, actual diff reviewed, applicable gates pass, commit/push and coordinator acceptance. This S cannot qualify the product merely by reaching farther. |
| Original Owner Request | Continue the complete PC110 T autonomously using existing external reference/firmware assets and normal compile/test/release governance. |
| Similar-Issue Sweep | Compare cold versus processor-only reset, C/E/F/SMRAM views, configured ASMI dispatch and all identified firmware boot-entry callers; retain separate storage/panel/input/product obligations. |
