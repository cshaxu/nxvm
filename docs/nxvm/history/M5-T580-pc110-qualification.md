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

## S49 accepted integrated-controller receiver

Coordinator actual-change review accepts `53ef8f1c1`: one immutable App
topology adds the standard-compatible integrated FDC, IRQ6 and DMA2 with
source-checked SMC fixed status inputs, no external drive/media and no Shared
change. Both real TFT BIOS widths complete DOR reset, actual IRQ wait return
and four reset SENSE results while retaining the earlier timer/SMI proof.
The final App units, existing Board FDC regressions, four preview builds/PE
widths, boundaries and documentation checks pass. Exact +73/-9 source/test
lines, rejected drafts, source grades, hashes and safe cleanup are recorded
in [S49 evidence](../etc/evidence/t580-s49-integrated-fdc.md). S49 closes;
T580 retains lower Flash/Setup/boot/guest/live product qualification.

### S49 retained pre-acceptance packet

+## M5 T580 S49 — Integrated FDC reset and IRQ receiver

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T580 S49, next unused S after accepted S48. |
| Admission And Approval | Owner's standing autonomous PC110 implementation authorization, reaffirmed 2026-10-25; reuse existing inward Board/chip contracts. No Lib/Emulator or other Shared implementation is admitted. |
| Objective | Complete the S48-identified integrated-controller receiver: App-owned FDC port/IRQ6/DMA2 wiring, reset completion, guest acknowledgement and empty-drive behavior, then classify the real unchanged-BIOS consumer. |
| Non-goals | No inserted media, fabricated IRQ/completion flag, BIOS interception, hardware drive-presence lie, new controller/API/queue, lower Flash implementation, owner INI/ROM modification or premature PC110 publication. |
| Reference Baseline | Clean pushed 6c474e957; [S48 route evidence](../etc/evidence/t580-s48-post-initialization-route.md), existing standard-compatible FDC/PIC/DMA contracts and external SMC U22 hardware evidence. |
| Candidate Proposal | [PC110 qualification](../proposals/m5-pc110-qualification.md); consume integrated-FDC reset/IRQ/empty-drive batch of the [ledger](../etc/evidence/t580-s1-pc110-qualification-ledger.md). |
| Files And ABI Surface | App pc110_plan.c and existing App composition/integration tests plus NXVM evidence/history/current. Estimate production +25–55/-0–5 and tests +20–60/-0–15; no public ABI change. Reuse fixture; no new test target. |
| Applicable Rules | Architecture one controller/IRQ/time/media owner and App-only board composition; Coding existing contracts/Types, no duplicate controller or framework; Execution finite semantic coverage, background cache reuse, exact review/count and cleanup; source policy read-only external references/immutable BIOS; Documentation one active packet. |
| Verification | Stable x64/x86 Ninja/ccache builds; existing App unit verifies the actual production driver ingress without media, existing Board tests cover controller reset/SENSE behavior, and unchanged-BIOS consumer proves DOR/IRQ/guest acknowledgement wiring. Run affected dependency/document gates and build both fixed PC110 previews; existing published PC/MyNES/MyRaidenII artifacts are unchanged unless actual linked production inputs change. |
| Expected Markers | FDC MSR and DOR route present without media; reset release produces real IRQ6 and guest confirmation; no host memory mutation. Real BIOS crosses its FDC reset wait or yields an exact source-qualified remaining controller predicate, not ceiling-as-success. |
| Asset Needs | Existing immutable TFT BIOS/font and external hardware/reference records. No new protected inputs or changes to owner media/INI. |
| Reporting Requirements | Initial design/estimate, findings if the existing interface cannot express the receiver, exact source grades, actual production/test counts, dual-width results and remaining product boundaries. |
| Stop Conditions | Stop on an unexpressible boundary or contradictory hardware source before adding a workaround. Each firmware run at most 32 million instructions or 180 seconds, output 64 KiB; task-owned build/t580-s49-fdc at most 4 MiB, archive required facts/hashes then remove after owned process exit. |
| Exit Criteria | Complete admitted receiver or a consolidated exact non-acceptance/design correction; no partial checkpoint counts as full repair. Actual diff reviewed, applicable tests/gates pass, previews built, owner inputs preserved, commit/push and coordinator acceptance. T580 remains open for Flash/Setup/boot/guest/live product qualification. |
| Original Owner Request | Implement PC110 autonomously through compile/test/release governance and complete T580 without synthetic firmware behavior. |
| Similar-Issue Sweep | Controller absence versus external drive/media absence; IRQ assertion/deassertion/PIC acknowledgement, reset timing, DMA binding and configuration endpoints; both composition paths and cold versus processor-only reset. |

## S50 accepted lower-Flash receiver

Coordinator actual-change review accepts implementation `d3180e0f8`.
The review reads all changed production, test and governance files, checks the
packet against the original PC110 request and confirms that the one App latch
feeds only the existing external E/F decoder. The model explicitly separates
shadow, high reset alias, cold reset and processor-only reset. No Shared,
owner asset, host loader or published product is changed.

Both widths have passing evidence for every one of the 712 repository unit
cases, with initial static timeouts and unchanged failed-item reruns separately
reported. The actual BIOS timer/SMI/FDC integrations, existing App fixtures,
four preview PE identities, dependency/document checks and clean incremental
builds pass. The actual source/test delta is +74/-0, net +74. Seven temporary
logs (250,232 bytes) are archived by facts/hashes then safely removed; the
temporary sampler has no retained diff. See
[S50 evidence](../etc/evidence/t580-s50-lower-flash-selection.md).

S50 closes the functional lower-bank receiver, not actual Setup bulk-copy,
loader or boot qualification. The normal sample ends in the SMM checksum
loop without INT 19h; that is not treated as a deadlock or product success.
T580 remains open under its unchanged ledger. The next coherent investigation
must reconcile normal cold POST versus the later power/suspend route and
its real predicates, rather than add a firmware-address bypass.

### S50 retained pre-acceptance packet

## M5 T580 S50 — Guest-owned lower Flash selection

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T580 S50 after accepted S49. |
| Admission And Approval | Owner's standing autonomous PC110 implementation authorization; App-local existing port/memory contracts only. No Shared implementation change admitted. |
| Objective | Complete the S48-identified port-98 lower/upper Flash toggle, independent shadow selection, guest copy/restore and reset-retention receiver; reconcile actual normal firmware progression after S49. |
| Non-goals | No host decompressor, synthetic loader/service/jump, lower-ROM aliases in C/D, writable Flash, owner INI/ROM/media changes, new API/clock/queue or premature publication. |
| Reference Baseline | Clean pushed d347ce8b6; [S48 route evidence](../etc/evidence/t580-s48-post-initialization-route.md), [S49 FDC proof](../etc/evidence/t580-s49-integrated-fdc.md), immutable admitted TFT image and external hardware references. |
| Candidate Proposal | [PC110 qualification](../proposals/m5-pc110-qualification.md), lower Flash/copy/restore batch in its [ledger](../etc/evidence/t580-s1-pc110-qualification-ledger.md). |
| Files And ABI Surface | App firmware/configuration owner and existing firmware/configuration tests plus existing real BIOS harness; NXVM evidence/current/history. Estimate production +15–35/-0–5 and tests +25–70/-0–15. No public Shared ABI or new test executable. |
| Applicable Rules | Architecture one register/decoder owner and unchanged guest execution; Coding existing Types and fixtures, no loader bypass; Execution finite semantic coverage, stable cached graphs, bounded background proof and cleanup; source policy immutable external inputs; Documentation single packet and truthful unresolved gates. |
| Verification | Both widths: complete repository-only unit suite, upper/lower external Flash reads, shadow independence, restore, full versus processor-only reset; actual unchanged BIOS receiver or exact semantic disposition. Rebuild both fixed PC110 previews, inspect PE widths; run affected boundaries/document gates. Published other Apps remain unchanged for App-only inputs. |
| Expected Markers | Guest control toggles select the original lower two banks and restore upper banks without mutating Flash/shadow. Actual firmware copy/checksum/loader entry is separate from complete Setup/boot qualification; no retirement-ceiling success. |
| Asset Needs | Existing hash-pinned external TFT BIOS/font and source records only. No new guest media or owner configuration changes. |
| Reporting Requirements | State the L2 decoder/reset assumptions, actual source/test delta, normal firmware milestones, dual-width results and every remaining product/source gate. |
| Stop Conditions | Reconcile contradictory decoder evidence before implementation; no speculative fallback. Each firmware observation at most 32 million instructions or 180 seconds, output 64 KiB; verification phase at most 2400 seconds. build/t580-s50-flash is S-owned, at most 4 MiB, remove after copied results/hashes and process exit. |
| Exit Criteria | Whole admitted toggle/copy/restore/reset batch has direct proof or a consolidated exact unresolved contract; actual-diff review, applicable tests/gates, previews, commit/push and coordinator acceptance. T580 retains all unqualified Setup/boot/guest/live product members. |
| Original Owner Request | Continue complete PC110 implementation autonomously through normal governance without firmware fabrication. |
| Similar-Issue Sweep | Both low external E/F banks, high reset alias, internal shadow reads/writes, decoder cache invalidation, cold and processor-only reset, and all known port-98 callers. |

Executor delivery: the App-owned receiver and existing fixture extensions are
implemented; [S50 evidence](../etc/evidence/t580-s50-lower-flash-selection.md)
records +74/-0 source/test lines, passing evidence for all 712 units per
width (initial static-check timeouts and unchanged reruns explicitly retained),
dual-width actual-BIOS prefix regressions and four verified preview identities.
Temporary code/logs are removed; published Apps and owner inputs are unchanged.
Coordinator actual-change acceptance is still required. The real Setup bulk
copy/loader and remaining T580 product gates are not qualified by this receiver.

## S51 accepted cold-power diagnosis

Coordinator actual-change review accepts audit `bae92d9e1`. The complete
retained diff is NXVM documentation only: source/test/artifact +0/-0, no
Shared or owner-input mutation. Both widths directly establish the first
cold FB clear and later zero-valued suspend guard; x64 completes the finite
checksum and reaches firmware-requested HLT. The evidence distinguishes
successful observations from x86 timeouts and a rejected second-watchpoint
edit, and does not promote any of them to boot/product success.

The original harness is restored/rebuilt, both App unit sets pass 11/11,
unchanged previews retain their S50 hashes, and the complete unchanged unit
corpus retains S50 qualification. Dependency/document/whitespace gates pass.
Nine exited-process logs (12,472 bytes) are fact/hash archived and safely
removed. The packet's original request, coverage ledger, source grades and
all remaining product gates are reviewed together; no source/bit behavior is
invented to make the BIOS skip suspend.

S51 closes as a diagnosis and coherent receiver-design disposition, not a
production power fix. T580 remains open. The next admission must settle
SL-compatible hardware status, suspend/wake and retention as one contract
across FB/B8/FF and the existing mode-22h protocol before selecting repair.

### S51 retained pre-acceptance packet

## M5 T580 S51 — Post-FDC cold POST and suspend-route reconciliation

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T580 S51, next unused S after accepted S50. |
| Admission And Approval | Owner's standing autonomous PC110 implementation request, including evidence-based App fixes without further permission; no Shared implementation change admitted here. |
| Objective | Reconcile the complete post-FDC normal POST versus power/suspend handoff: actual SMM request/dispatcher, saved state, source/predicate, checksum/exit and subsequent normal boot route. Implement only an App-owned receiver demonstrated missing by that evidence, after revising this packet with its concrete contract. |
| Non-goals | No firmware interception, invented wake/service/guest flag, host decompressor, ROM/INI/media mutation, second clock/queue, Shared API change, physical-accuracy claim or premature publication. A sampled checksum instruction is not a deadlock. |
| Reference Baseline | Clean pushed 267b67a0b; accepted [S50 receiver](../etc/evidence/t580-s50-lower-flash-selection.md), S49 real FDC proof, unchanged admitted TFT image and pinned read-only hardware/reference sources. |
| Candidate Proposal | [PC110 qualification](../proposals/m5-pc110-qualification.md), consuming the complete post-initialization cold/power handoff and boot-route batch of its [ledger](../etc/evidence/t580-s1-pc110-qualification-ledger.md). |
| Files And ABI Surface | First audit uses the existing actual-BIOS harness temporarily, estimated +100–180/-0–15 then removed; retained App production/test change remains zero until a concrete owner/protocol and revised estimate are recorded. Existing execute watchpoint brackets the already-qualified prefix, with copied retirement state only after the actual guard; no guest instruction or timeline is skipped. Evidence/current/history only if no repair is justified. No new public interface or test executable. |
| Applicable Rules | Architecture one state/clock/execution owner and inward dependencies; Coding existing copied observations/fixtures, no BIOS-specific production branch or parallel implementation; Execution finite semantic batch, cached background observations and exact cleanup/review; source policy immutable external inputs and labelled L2 references; Documentation one active packet and unchanged T predicate. |
| Verification | Actual guest CPU execution after established timer/FDC prefixes; bounded copied register/I/O snapshots at real dispatcher and power entry/exit, classification of normal cold versus resume choices, source cross-check. Corroborate a meaningful endpoint on x86. Remove temporary sampler and rebuild original harness; retain S50 complete unit qualification for unchanged code, rerun complete dual-width units if retained production/tests change. Run applicable boundaries/docs and affected preview builds. |
| Expected Markers | Exact first suspend request/source/predicate and its caller, checksum completion/exit or actual INT 19h; distinguish firmware-requested power behavior from missing host input or an instruction ceiling. No successful-boot claim from the earlier FDC prefix. |
| Asset Needs | Existing hash-pinned TFT BIOS/font and external PC110 references only. No new protected binary or owner configuration/media change. |
| Reporting Requirements | State the actual cause or unresolved contract, source/model grade, justified owner and minimal design before repair; actual retained/removed code delta, dual-width proof and remaining product gates. |
| Stop Conditions | Each observation at most 33 million instructions or 180 seconds, 64 KiB output; verification phase at most 2400 seconds. The initial 32M sample proves 28,165 of 32,768 checksum iterations, so the additional 1M contains that known finite completion and power exit; it is not a repair or success criterion. build/t580-s51-handoff is S-owned, at most 4 MiB; preserve facts/hashes and clean after owned process exit. Stop speculative repair on contradictory sources or a needed Shared contract; continue independent evidence work. |
| Exit Criteria | Whole handoff batch has an evidenced diagnosis and owner/design disposition, with any admitted repair proved; no first-PC patch or invented return. Temporary code removed, actual diff reviewed, applicable verification/cleanup, commit/push and coordinator acceptance. T580 remains open for every unqualified product/guest member. |
| Original Owner Request | Continue PC110 autonomously through correct implementation and governance; use labelled reference-based L2 where original hardware sources are unavailable, without synthetic firmware shortcuts. |
| Similar-Issue Sweep | Cold versus processor-only reset, SMM source/acknowledge/rearm and saved context, BIOS cold/resume branches, EC/planar readback, checksum/HLT/return and all identified normal boot/Setup callers. |

Executor audit delivery: [S51 evidence](../etc/evidence/t580-s51-cold-power-handoff.md)
records both actual cold FB clears and later suspend guards, x64's complete
checksum/power exit, rejected diagnostic attempts and the required coherent
power/status/wake contract. No justified production repair is selected yet.
All temporary code/logs are removed; source/test/artifact delta is zero.
Both restored App unit sets pass 11/11; the unchanged complete unit corpus
retains S50 qualification. Coordinator actual-change acceptance is pending.

## S52 executor delivery — RTC address space and power disposition

Shared `3ca0a166f` extends the existing RTC/Board descriptor with classic-zero
versus compatible-128 register selection. The App selects 128 and existing
fixtures cover address isolation, reset retention, defaults and NMI separation.
The two actual CPU I/O probes change from AA/AA to 55/AA without a second
CMOS receiver. Total retained source/tests are +110/-41; no Lib/Emulator,
X86 Core or other App private implementation changes.

Both complete unit sets pass 712/712, the original timer/SMI/FDC integration
passes and eight receiving PC products rebuild with their actual INI identity
checks. The unchanged BIOS still requests suspend; no guest boot or PC110
product qualification is claimed. Four previews stay unpublished. The
[complete evidence](../etc/evidence/t580-s52-power-contract.md) retains the
source/contract comparison, actual hashes, remaining whole power receiver
and safe cleanup of 24 files/27,077,174 bytes. Coordinator actual-diff
acceptance is pending.

## S52 coordinator actual-change acceptance

The coordinator reviews the actual pushed Shared and NXVM source/test diffs,
config defaults and callers, both manifests, the 712/712 results per width,
actual before/after probes, restored BIOS regressions, all eight receiving
artifacts and the explicit unqualified power/product members. The one RTC
store, construction-selected extent, existing Board route and App declaration
are accepted; no duplicate path, outward dependency, warning exemption,
forced BIOS state or hidden product publication is introduced.

S52 closes as a bounded capacity correction and complete power-member
disposition. The power implementation itself is not accepted or marked done.
FB/status, the actual physical wake transaction, retention/reset and firmware
restoration remain together in T580's original receiver. This does not close
T580, narrow its original universe or claim guest boot. Both implementation
P commits are pushed, scratch is removed, and owner inputs/other Apps remain
unchanged.

### S52 retained pre-acceptance packet

## M5 T580 S52 — SL power/status and suspend/wake contract

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — next unused S52 after accepted S51. |
| Admission And Approval | Standing owner authorization to complete PC110 autonomously, including approved X86 NXVM chip/Board changes. Coordinator review admits the reproduced 64/128-register RTC/Board receiver below, with Shared and NXVM targets; Lib, Emulator and other Apps' private code are excluded. Any further Shared boundary requires concrete review before implementation. |
| Objective | Reconcile cold FB status, B8 resume cause, FF suspend control and existing mode-22 protocol as a complete power/retention/wake contract, using actual firmware and source-labelled SL references; select and prove only a justified receiver. |
| Non-goals | No forced status bit, synthetic wake, BIOS redirect, host service/decompressor, second clock/state owner, protected input mutation or premature product publication. |
| Reference Baseline | Clean pushed 013b58329; accepted S50 receiver and S51 actual cold-power observations. |
| Candidate Proposal | [PC110 qualification](../proposals/m5-pc110-qualification.md) and its unchanged complete product ledger. |
| Files And ABI Surface | Existing X86 NXVM RTC config gains a normalized extended-register selection; existing Board config carries it, retaining default 64-register decode. One RTC-owned array serves the selected 64/128 space. PC110 selects 128. Existing RTC, Board and App fixtures cover isolation/retention and real guest I/O. Estimated production +25–45/-8–15 and tests +50–90/-15–25, plus mechanical default initializer adaptation and manifests; no new function, executor, state manager or test executable. Temporary existing-harness samplers are removed. |
| Applicable Rules | Product guide Task Reading Set; shared Execution/Architecture/Coding/Documentation; concrete architecture/layout; source policy. One guest executor, state owner and clock; immutable external ROM; references are not implementation imports. |
| Verification | Both widths prove the pre-fix 3B/7B alias and post-fix isolation through actual PC110 guest I/O; verify classic decode, complete selected RAM retention, IRQ/NMI behavior and configuration rejection. Run complete dual-width repository units, real BIOS prefix/power comparison, manifests/boundaries/docs, affected eight PC artifact builds and four previews. No PC110 release or full boot claim from a fixed register receiver. |
| Expected Markers | A coherent evidenced cold/suspend/wake/retention contract or precise unresolved boundary, never an instruction ceiling or arbitrary forced normal-boot branch counted as success. |
| Asset Needs | Existing pinned TFT BIOS/font, Intel SL manuals and read-only PC110 references. No new protected image acquisition. |
| Reporting Requirements | Sources and confidence, owner/design and exclusions, actual deltas, verification limits and remaining boot/product gates. |
| Stop Conditions | build/t580-s52-power is S-owned scratch, maximum 32 MiB; the admitted Shared capacity repair expands the original 2400 s research budget to 4800 s for full dual-width qualification and eight receiving artifacts. Individual guest observation remains maximum 33M instructions/300 s/64 KiB output. Remove copied research output after facts/hashes are archived and owned processes exit. Contradictory source or a further Shared extension stops speculative implementation, not independent research. |
| Exit Criteria | Whole admitted power-contract batch has a reconciled design/disposition and any justified repair proof; actual diff review, applicable verification, cleanup, commit/push and coordinator acceptance. T580 remains open for all original guest/product gates. |
| Original Owner Request | Continue PC110 through correct, minimal implementation using explicit L2 references when physical/original evidence is unavailable. |
| Similar-Issue Sweep | Cold/processor reset, config/status ownership, all FB/B8/FF/mode-22 callers, SMM/HLT entry, real wake sources and retained RAM/CMOS/SMRAM; no generic HLT or unrelated product alteration. |

Coordinator design review: real CPU IN/OUT probes confirm 3B/7B aliasing on
both widths. Board fixes the address at six bits and RTC holds only 64 bytes,
while the actual BIOS and both pinned reference models use the upper space.
The admitted L2 correction preserves all old Board defaults and has one
register owner. It does not assume that fixing NVRAM also fixes FB/wake.

Executor delivery: Shared `3ca0a166f` implements the selected RTC/Board
capacity receiver; the NXVM delivery selects it and refreshes the eight PC
EXEs. Both widths prove guest-IN/OUT isolation and pass all 712 unit cases,
the original timer/SMI/FDC integration and four deployed INI identities.
Both repaired BIOS observations reach the same genuine suspend endpoint,
not boot. The [evidence](../etc/evidence/t580-s52-power-contract.md) records
actual +110/-41 source/test lines, four unpublished previews, exact artifact
hashes and the complete power-contract disposition. All 24 task-temporary
files are safely removed; owner inputs and other Apps remain unchanged.
Coordinator actual-change review/acceptance is pending. T580's physical
wake/status/retention, Setup/guest and full product gates remain open; neither
the NVRAM repair nor unit qualification is a completed PC110 product.

## S53 implementation — complete bounded power receiver

After accepted S52, the coordinator admits the complete EC/power/status/wake/
retention/restoration batch on d206cddfa. Original controller records support
one App reply/RAM receiver; SL references and actual BIOS support the named
L2 suspend/explicit-button/retained-reset/SMI transaction. No firmware change,
forced status, automatic wake, host service, new clock or executor is used.

Shared `85a65c3a8` adds the Chip receiver and optional copied Machine key
binding. The NXVM implementation supplies PC110's F12 binding and EC/reset
integration, extends existing owner fixtures and the sole real-BIOS harness,
and refreshes all eight receiving PC EXEs. Both widths prove all 512 retained
CPU bytes and actual BIOS RSM. Registered power CTest passes at 186.98 s x64
and 248.64 s x86. All 712 distinct units per width have passing results;
initial static timeouts and unchanged-budget serial corrections are retained,
not relabeled. Four actual INI identities pass per width, owner INI hashes
stay unchanged, and the four PC110 previews remain unpublished.

The [S53 evidence](../etc/evidence/t580-s53-power-receiver.md) records sources,
actual C/H +540/-43 (net +497), public/state ownership, artifact hashes,
failed observers and verification limits. The official cleaner removes 28
owned temporary files/3,011,103 bytes after every controller exits; stable
Ninja/ccache trees remain. Lib, Emulator, Debugger, IBM-PC Product, MyNES and
MyRaidenII source/test trees remain unchanged. Coordinator review is pending.
This is not live panel/input/RDP, Setup, full guest or PC110 publication
qualification; all original T580 product predicates remain open.

## S53 coordinator actual-change acceptance

The coordinator switches roles after executor delivery and reviews the pushed
Shared `85a65c3a8` and NXVM `a8561555e` diffs, ownership, copied callback
lifetime, existing callers, manifests, whole unit outcomes including failed
attempts, registered actual-BIOS power proof, eight artifact hashes and INI
retention. The complete bounded EC/power transaction is accepted. There is no
new executor, queue, clock, BIOS-specific production branch, Lib/Emulator
change or hidden publication. C/H +540/-43 and CMake +6/-1 are justified by
the missing receiver and its existing-fixture/real-consumer regressions.

S53 closes; T580 remains open for its unchanged complete product universe.
The pre-acceptance packet and earlier status notes are preserved in
[retained status](M5-T580-retained-status.md), with only relocated Queue links
repaired. Current is reduced to the one latest T580 progress record so old
packets cannot be mistaken for concurrent active work. Scratch is absent,
the stable caches remain, and all implementation/receiving P commits are pushed.

## S54 implementation — ordinary POST/bootstrap handoff

Shared P1 `48fe7a544` is pushed. The admitted post-restoration route inventory
exposes a repeated #NM at the
original BIOS's FNINIT. S54 corrects the CPU-owned 486SX absent-FPU class
and missing successful-ESC L2 cost allocation, without a BIOS-PC branch,
numeric FPU, ROM modification, new interface or Lib/Emulator change.
The source conflict and explicit compatibility-model limits remain in the
[S54 evidence](../etc/evidence/t580-s54-post-restoration-route.md).

Both widths execute the original retained wake/RSM and ordinary POST to
the real INT19 handler, at the same 56,679,591 instructions, four resets,
four exceptions and identical 512 retained bytes. Registered boot tests pass
in 224.16 s x64 and 302.96 s x86 under the evidenced 360 s complete-work
budget; the earlier 300 s failures are not reclassified. FDC/timer/ASMI and
all 712 distinct repository units per width have passing evidence, including
the documented initial x64 executor assertion failure and corrected retest.

Eight PC and two explicitly receiving MyRaidenII artifacts are rebuilt and
verified; all eight PC INI identity integrations pass. Four PC110 previews
stay unpublished, MyNES and owner INI contents stay unchanged. Actual C/H
delta is +342/-32 (net +310), with no new production helper/state/API. The
official cleaner removes 41 owned logs/399,400 bytes after task exit;
the stable caches remain. Executor delivery awaits coordinator acceptance.
Setup, actual storage/guest, display/live input, lifecycle and PC110
publication retain their original unqualified ledger rows.

## S54 coordinator actual-change acceptance

Coordinator review accepts Shared `48fe7a544`, NXVM `d7fd63518` and
MyRaidenII receiving `47a4c9092` after reading their actual diffs, unchanged
public/ownership boundaries, source grades, finite route dispositions,
complete unit outcomes, real-BIOS proof and deployed identities. The earlier
failed attempts, timing-model limits and remaining original gates remain
explicit. Both manifests, both affected documentation gates and whitespace
checks pass. All receiving artifacts match the recorded SHA-256/PE/version;
owner INI and MyNES remain byte-identical. No task-owned native work or
scratch remains. S54 closes, not T580. The pre-acceptance packet moves to
retained history and Current holds only compact latest accepted progress.

## S55 executor delivery - Loader, keyboard ordering and remaining route

The actual selected-medium investigation is extended through the unchanged
BIOS's own compressed Setup recovery. Both host widths compare every one
of its 232,185 decoded bytes before first Setup execution, consume ordinary
Enter input and remove the observed 00301 keyboard error after selecting the
existing timed KBC response in the PC110 App. The poll-dependent candidate
is explicitly retired because it cannot release an IRQ-only response.
The final nominal 100 us model is L2, sourced from an external controller
service cadence; no Shared implementation, public API or second clock changes.

Both widths end at the same firmware software-reset request after two input
stages: 144,097,172 instructions, four resets, four known exceptions and no
Core fault. The absent 92h reset route is identified; its correct Board-owned
A20/reset receiver, two-slot storage, PCIC, VGA/panel and original product
gates stay open at their named T580 owners. This is the complete observed
failure-class disposition allowed by the S55 packet, not an ATA, DOS or
configuration-save qualification. Detailed sources, failed attempts, full
results and regression ownership remain in [S55 evidence](../etc/evidence/t580-s55-ata-guest-route.md).

All 712 repository units per width pass; both registered firmware integrations
pass with unchanged budgets, including boot handoff at 236.77 s x64 and
352.18 s x86. Four unpublished previews are rebuilt and inventoried; all ten
S54 published identities, MyNES, owner INI and external inputs remain unchanged.
Actual C/H delta is +310/-23, net +287, across six paths, with production
only +6/-0 in three App configuration files. Three test-local helpers retain
actual loader/sector, transcript and frame evidence in the existing harness.
No new production state/helper/interface or test framework is introduced.
The official cleaner removes 128 owned files/14,332,369 bytes after verified
task exit and protected-file hash review. Executor requests coordinator
actual-change acceptance; S55 and T580 are not closed by this delivery.

## S55 coordinator actual-change acceptance

Coordinator reads the actual `6964e8280` source/test and documentation diff,
reviews the retired poll candidate against IRQ-only delivery, the two App
construction paths, bounded actual-BIOS evidence and complete remaining-class
disposition. It accepts the finite loader and keyboard receiver, not ATA/DOS,
configuration save, physical latency, panel/DSTN or whole-T qualification.
The exact pre-acceptance packet is preserved in retained status. Both full
unit suites, unchanged-budget registered integrations, input hashes, four
unpublished preview identities and ten unchanged published identities are
verified. No owned process or scratch remains; Shared, public API, owner
inputs and unrelated Apps are unchanged. S55 closes after this governance
P; T580 stays open for its explicitly retained receivers.

## S56 executor delivery - System-control A20 and processor reset

The single existing Board route now owns independent keyboard/fast A20
inputs, their effective OR, system-control readback, reset edge/rearm and
cold-reset initialization. One public config Boolean admits this route on
PC110 without a PC/AT facade; no Lib/Emulator or CPU/Core change is needed.
Both unchanged original BIOS replays service Setup's reset request and reach
FFFF:FFF0 after 144,097,170 instructions/five resets with no new Core fault.
This does not prove configuration persistence or post-restart guest boot.

The [S56 record](../etc/evidence/t580-s56-system-control.md) retains actual
changes, sources, failed test setups and exact verification limits. C/H
delta is +162/-34, net +128; the affected static check is +9/-1. All 712 units
per width, eight receiving INI identities, eight profile boots, four additional
integrations per width and specialized checks pass. Eight PC EXEs are deployed
and four PC110 previews remain unpublished in the
[inventory](../etc/evidence/t580-s56-artifacts.json). Owner inputs, MyNES and
MyRaidenII are unchanged. All 98 scratch files/7,013,740 bytes are removed
with the official hash-reviewed cleaner; both caches remain. Executor
delivery awaits coordinator actual-change acceptance; T580 remains open.

## S56 coordinator actual-change acceptance

Coordinator reads the actual Shared `dde5da6de` and NXVM `88124376c` diffs,
including their source/test/manifest changes, root gate, receiving artifacts
and evidence. It accepts the complete bounded A20/system-control/reset
receiver after both route variants, both physical inputs, edge/rearm,
retention/cold reset and real unchanged-BIOS reset proof. The original
product gates remain open; the pink-background feedback is retained at
F65535 rather than treated as a host inversion. All scoped checks, input
hashes, artifacts and task-exit/cleanup are verified. The exact packet moves
to retained history; S56 closes with no active packet. T580 remains open.

## S57 executor delivery - F65535 planar and programmed colour

The existing F65535 owns four CPU read latches, read comparison and all four
write modes, map/address masks, Attribute/DAC/PEL resolution and fixed text
glyph slots/A-B selection. App forwards Core's existing inspection flag
through the changed read signature; no new public function, enum, renderer,
clock or worker. Actual review removes an unsupported mode-3 ROP exception
and reproduces/fixes an x64 aperture-length overflow with existing Types.
C/H delta is +319/-65, net +254, with production +124/-64.

The [S57 record](../etc/evidence/t580-s57-f65535-planar-colour.md) retains
every candidate/review boundary and exact final verification. Both host
widths draw byte-identical ERROR/date-time frames with BE9E96 background,
then complete the observed fifth-reset request. Full final-tree 712 units
per width, strict/types/dependency/manifest gates, eight receiving INI
identities and scoped firmware regressions pass. Normal-path replays are
explicitly distinguished from final preflight-only verification; there is
no broad desktop, RDP, panel, normal Setup or selected-guest claim.

The [inventory](../etc/evidence/t580-s57-artifacts.json) verifies eight
published PC receiver relinks (only timestamp/checksum changes) and four
unpublished rebuilt PC110 previews. Owner INI/ROM/font/media, MyNES and
MyRaidenII are unchanged. All 90 owned scratch files/13,302,526 bytes are
removed with the hash-reviewed cleaner; both caches remain. Executor
delivery requests actual-change coordinator acceptance; T580 stays open.

## S57 coordinator actual-change acceptance

Coordinator reads Shared `7cca77dcf` and NXVM `2326fa3c0`, including final
source/test/manifest diffs, caller migration, review corrections, all failed
attempts, the normal-domain equivalence rationale for the last range guard,
actual-frame/log proof and twelve artifact identities. It accepts the named
standard planar/colour/inspection/range receiver after exact final-tree
units/gates and scoped current firmware regression. It does not replace
the original normal Setup/guest/panel/DSTN/live-product predicates with the
restored colour or a reset vector. No Lib/Emulator or CPU/Core change,
external-input mutation, other-App pollution or new rendering path occurs.
All owned work exits and scratch is removed. The packet moves to retained
history; S57 closes with a clean workspace after this governance P. T580
remains open for its original complete receiver universe.

## S58 executor delivery — RB5C396 empty-controller receiver

S58 adds the one source-labelled, empty two-slot RB5C396 receiver at the
actual TFT-BIOS 3E0h/3E1h index/data route. `x86-nxvm` owns the controller's
index, two socket register banks, documented IDs, read-only fields and
board-provided socket input levels. `app-mypc110` owns only the existing
byte-port adapter and reset wiring. Reset clears controller-local state
without fabricating a card transition or changing physical socket inputs.

The implementation is deliberately not a PC Card, storage, IRQ/window,
PCMCIA-ATA, DOS, Setup, panel or product-release claim. The source record
and explicit limits are in [S58 evidence](../etc/evidence/t580-s58-pcic-receiver.md).

After the final production reset wiring, both host widths pass all 714
registered unit routes. The existing real-firmware FDC-reset and boot-handoff
integrations also pass on x64 (42.24 s and 225.02 s) and x86 (54.59 s and
306.85 s). Those integrations show no regression in their named firmware
checkpoints; they do not prove that the retained BIOS UI consumed the new
PCIC receiver. Commits `0b0e1f42c` and `1c357a14f` are pushed. No PC110
artifact is published, per the admitted packet; T580 remains open for its
unchanged qualification universe.

## S59 executor delivery — ATA two-device construction route

S59 makes the existing two-slot fixed-disk configuration real without adding
a second storage path. Construction now declares the physical HDC slot count;
Machine allocates, binds, resets and inserts every declared slot through its
existing registry route. MyPC110 declares two slots and connects the already
reserved secondary media ID to ATA slave selection. The existing ATA chip's
four stale master-only guards now consume its existing Drive/Head unit route
for slave CHS/LBA/IDENTIFY operations. Existing one-device PC plans declare
one slot, and fixed geometry remains master-only.

Both widths pass the focused Machine, Board HDC and MyPC110 units plus X86-NXVM
source/test manifest gates. Existing unchanged-firmware FDC-reset and
boot-handoff checkpoints pass x64 in 50.33/270.45 s and x86 in 57.00/314.03 s.
Those checks are strictly non-regression evidence; they do not qualify a
SunDisk/CF, PC Card storage, Setup persistence, guest boot, panel or release.
See [S59 evidence](../etc/evidence/t580-s59-ata-two-device-route.md).

After a fresh full Ninja relink, the complete repository-only unit suite also
passes 714/714 on both x64 (212.88 s) and x86 (73.32 s).  A pre-relink run
briefly exposed a stale `vm-hdc-port-smoke` executable; rebuilding the unit
graph and rerunning it made the same source pass, so no assertion or source
was weakened.  This closes S59's admitted construction route only.  T580's
unqualified firmware/media/product domains remain open.

## S60 executor delivery — PC110 primary ATA geometry

S60 consumes the existing master-only exact-geometry construction contract in
the MyPC110 plan.  The locally inspected Open-Source-PC110 live-dump record
provides the L2 fact for the soldered SunDisk primary: 123 cylinders, two
heads and 32 sectors per track (4,030,464 bytes).  No controller, media route,
CMOS seed, slave geometry, guest behavior or public interface changes.

The existing MyPC110 construction test now proves a disposable exact-size file
reaches the real Machine/Board ATA master route with 123 cylinders, while a
one-sector file is rejected by the existing exact-size guard.  It does not
claim either file is authentic media.  Both widths pass the full 14-test
MyPC110 labelled suite, including unchanged real-firmware FDC-reset and
boot-handoff regressions: x64 in 45.91/231.73 seconds and x86 in 53.51/318.12
seconds.  After a fresh full Ninja relink, all repository-only units pass
714/714 on x64 (61.36 seconds) and x86 (59.03 seconds).  No PC110 artifact is
published.  Authentic SunDisk/CF inputs, BIOS storage consumption,
CMOS/Setup persistence, PCIC mapping, selected guest and release qualification
remain open.

## S61 executor disposition — extended-CMOS seed prerequisite

S61 audited the existing 64-byte `0Eh`–`3Fh` Machine seed against the Board's
matching 50-default capacity and the documented PC110 extended CMOS range.
The source record demonstrates real persisted values in `40h`–`7Fh`, but the
locally pinned research checkout and external PC110 asset profiles do not
contain the cited exact 128-byte capture.  No source code was expanded with
invented bytes.  The required external, hash-recorded source payload and the
single-route follow-up are retained in the S61 evidence; normal Setup
persistence remains open.

The existing Easy-Setup diagnostic's comparison oracle was deliberately
scratch-only and has been removed, so it cannot be reused as a configuration
seed or current runtime proof.  S61 makes no production change.

S61 also rejects the locally available PC110-QEMU and PC110-EMU disk images as
SunDisk substitutes: their capacities/partition geometry do not fit the
source-recorded 7,872-sector primary.  No truncation or media conversion is
admitted.

The reported blue Easy-Setup/ERROR background is retained as a configuration
and pre-entry-initialization qualification gap, not solved by reversing RGB in
F65535.  The existing RGB path produces S57's `BE9E96` rose value unchanged.
PC110-QEMU's direct-loader palette overwrite and PC110-EMU's presentation
overrides are L2 diagnostic comparisons only and are not imported: both would
modify or reinterpret guest presentation rather than model the missing source
input.  A current unmodified-BIOS frame plus the exact extended CMOS/default
capture is required before a receiver can be admitted.

## S62 executor disposition — unmodified Setup palette review

S62 confirms that S57's direct, unchanged-TFT-BIOS ERROR/date-time capture is
still the exact current F65535 → Machine → Window source route.  On both host
widths its 640×480 ERROR frame has the same recorded SHA-256 and dominant
`BE9E96` rose background.  No second temporary diagnostic is retained merely
to repeat that result after S57's comparison-only scratch oracle was removed.

The user-reported blue frame is therefore not evidence for a generic RGB/BGR
repair, guest-image patch or host palette override.  Main Setup/persistence,
extended-CMOS source capture, panel identity and desktop/RDP validation remain
unqualified and are retained as separate T580 work.  See the S62 review for
the exact source/diff proof.

## S63 executor delivery — Font-ROM aperture receiver

S63 completes the externally observed Font-ROM register and aperture receiver:
the 7-bit bank, unused and segment readbacks, bit-0 enable state, disabled
`DE000h` open-bus reads and immutable selected 8 KiB bank are all handled by
one App-local object with one port provider and one existing memory route.
The source supports only post-initialization live behavior, so this delivery
does not claim a power-on reset value or invent writable scratch behavior for
`1161h`. Full repository unit qualification and the unpublished preview
relinks are recorded in the [S63 evidence](../etc/evidence/t580-s63-font-window.md).
No Shared component, payload, asset, firmware, INI or PC110 release artifact
changed. DOS/V consumption, panel output, configuration persistence, guest
boot, PCIC/media behavior and product qualification remain open.

## S64 executor delivery — 8042/TrackPoint reset receiver

S64 adds one board-selected 8042 reset command-byte fact to the existing
configuration family. MyPC110 now starts from the observed `65h` state, so AUX
reports are rejected until the guest sends `A8h`; the normal `F2h`, `E9h` and
`F4h` standard PS/2 path is unchanged. Generic unconfigured 8042 behavior
retains its existing `43h` default. The receiver is source-labelled L2 and
does not model U75 firmware, physical pointer behavior or any new input path.
See [S64 evidence](../etc/evidence/t580-s64-kbc-trackpoint-reset.md).

## S64 coordinator acceptance

Coordinator review accepts implementation `40be5c4f2`. The changed controller
has one optional board-selected reset byte, preserves the unconfigured `43h`
path, and derives AUX admission from the command byte rather than introducing
a second mouse route. The direct generic and public PC110 proofs cover the
observed `65h` state and standard PS/2 sequence on both widths. Both 714-test
unit suites pass; the complete 755-test host runs have one pre-existing
external `native_midi_output` setup failure because this host exposes no MIDI
output endpoint. That non-PC110 integration is recorded, not treated as a
passing gate or a reason to alter Lib.

All eight current 0.5.0580 receiving-PC artifacts were rebuilt and PE-width
checked. MyPC110 previews remain unpublished. T580 remains open for the
separate qualification domains listed in the proposal and S1 ledger.

## S65 executor delivery — F65535 panel-table audit

S65 traced the existing F65535 XR index/data route during the selected,
unchanged TFT BIOS through ordinary power restoration, POST and `INT 19h`.
Neither that complete route nor the bounded C000 option-ROM route issued an
XR write. The existing standard-VGA palette path therefore remains the only
evidenced colour owner; no RGB/BGR swap, panel interpretation or production
display change is justified. The temporary diagnostic change was removed.
See [S65 evidence](../etc/evidence/t580-s65-f65535-panel-table-audit.md).

## S65 coordinator acceptance

Coordinator review accepts `aba6329a8`: it contains only the active-packet
transition, one evidence record and the historical disposition. The temporary
diagnostic path was removed before commit; no production, shared interface,
firmware, media or artifact changed. The existing x64 FDC real-firmware
integration passes after the restored harness. T580 remains open for the
separate Setup, guest/storage, PCIC, panel, input/lifecycle and publication
receivers already named by its convergence ledger.

## S66 executor delivery — real Easy-Setup handoff and first frame

S66 reuses the existing real-firmware diagnostic without changing product
code.  A scratch-only oracle derived from the selected immutable TFT BIOS
confirms on x64 and x86 that the guest itself produces all 232,185 Setup bytes,
retires the original `9000:009E -> 5000:0000` transfer, publishes a 640×480
graphics frame, consumes two ordinary Enter make/break pairs, and reaches its
subsequent real reset.  Both widths have identical frame hashes and the known
four cold-prefix exceptions only.  The derived oracle, frames and logs are
removed after their hashes are recorded.  This is not a normal F1-selection,
persistence, guest-boot, physical-panel, desktop or release claim.  See
[S66 evidence](../etc/evidence/t580-s66-real-easysetup-handoff.md).

## S67 executor delivery — extended CMOS-seed receiver

S67 replaces the earlier no-payload disposition with a source-backed,
external-only 128-byte live CMOS capture recovered from the retained
Open-Source-PC110 historical evidence.  The sole existing Machine seed path
now preserves either a 64-byte classic image or the explicitly selected
128-byte extended image; it applies only configuration registers `0Eh` onward
to the Board and rejects extended data on a classic decode.  Core retains sole
ownership of live calendar/status state, and the original DSTN build does not
inherit the TFT capture.

Both x64 and x86 unit proofs cover accepted sizes, rejected malformed sizes,
the classic-decode rejection, and real Board delivery at `0Eh`, `3Fh`, `40h`
and `7Fh`.  The unchanged selected TFT BIOS reaches the same Easy-Setup
handoff, 640×480 frame, two ordinary Enter transactions and fifth reset on
both widths.  This closes the seed receiver only: it does not establish
universal CMOS power-on state, durable Setup persistence, authentic media
boot, PCIC behavior, panel identity, DSTN behavior, desktop use, or PC110
publication.  See [S67 evidence](../etc/evidence/t580-s67-pc110-extended-cmos-seed.md).

## S67 coordinator acceptance

Actual-change review accepts `9fdfc29d8` for its bounded S67 contract. The
implementation has one seed route, accepts only 64 or 128 bytes, rejects an
extended seed on a classic board, and leaves the raw capture outside Git. The
actual source/test paths match the packet; there is no Lib, Emulator, x86-core,
x86-debug, x86-ibmpc, ROM or runtime-selector change. The non-green Model40
x86 integration is an explicitly retained, pre-existing qualification item and
does not invalidate the focused seed proof or establish a PC110 release claim.

The S67 receiving sweep has one explicit non-green result: the pre-existing
x86 Model40 floppy boot matrix misses its established 300,000 ms guest
terminal budget on two runs and remains unfinished at 330,000 ms.  The
observed FDC progress varies by run.  This is retained as the existing
Model40 x86 throughput qualification concern; it is neither attributed to the
PC110 seed route nor counted as a passing integration result.

## S68 executor delivery — Easy-Setup input and reset

The unchanged TFT firmware was replayed on both host widths with an existing
diagnostic extended only to observe keyboard and RTC port activity. Both
runs returned zero: the ERROR screen (`00173`, `00163`) precedes the first
Enter, the date/time editor precedes the second, and the guest consumes both
make/break pairs before RTC writes and a processor reset. The current build
trees' unit suites pass 669/669 per width. No product or Shared code and no
released executable changed. [S68 evidence](../etc/evidence/t580-s68-easysetup-input-reset.md)
records the actual register observation and the outstanding error-log,
post-reset persistence and main-menu boundaries.

## S68 coordinator acceptance

Actual-change review accepts `8d3ca6ccc` for the diagnostic packet. The sole
code change is the existing MyPC110 test harness; observation begins at the
real handoff and does not supply firmware data or alter injected input. Both
widths and the unit suites passed, and task scratch was removed after hash
review. The 128-entry trace cap and reset-before-persistence boundary remain
explicit in the evidence. T580 remains open.

## S69 executor delivery and coordinator review

The selected TFT BIOS itself compares its captured CMOS configuration sum
`056D` with `0391` and selects error `173`; its invalid-time path selects
`163`. The same checksum checkpoint succeeds as an observation on both host
widths. An x64 real Setup replay subsequently observes the firmware write a
matching `032E` checksum before CPU reset. Both host widths then reproduce
the distinct `102` timer-test failure after the reset; the pending IRQ0 is a
new PIT edge after PIC initialization, not a request surviving it. The
model-specific receiver and process-lifetime CMOS persistence remain open.

The actual change is a bounded extension of the existing MyPC110 integration
diagnostic plus task evidence; no production or Shared component, external
media, INI or published executable changes. The source-backed seed is not
rewritten. Both architectures build the diagnostic and pass the short
checksum observation; the post-reset boot remains a documented failure.
No full repository or desktop qualification is claimed. See
[S69 evidence](../etc/evidence/t580-s69-post-error-investigation.md).

## S70 executor delivery — post-Setup timer and IRQ

Shared implementation `3d692e367` makes the existing 8259 input owner
retract an unacknowledged request when
its input falls in edge mode, consistent with the original Intel request-latch
contract; no PC110-specific interrupt shortcut is introduced. On x64 the
unchanged TFT firmware passes the previously observed `102` branch and later
reaches a second power wait, not a completed post-Setup boot. The full
X86-NXVM unit selection passes 233/233 on each host width; existing test
stimuli now hold IRQ inputs through acknowledgement or pending-state checks.

Both host widths pass the registered real-firmware FDC-reset and boot-handoff
checkpoints. The original x86 360-second boot-handoff timeout is retained in
the evidence; the same executable subsequently reached genuine F12, RSM and
INT 19h in approximately 719 seconds, and the registered rerun passes in
745.24 seconds under an x86-specific 1080-second budget. All four PC product
profiles build and pass INI identity and floppy-boot receiving tests on both
widths. The initial x86 DeskPro 300-second failure and same-host old-PIC
control also remain recorded; its measured 357-second direct success led to
a 480-second x86-only guest bound and a passing 370.32-second registered
rerun. No guest timing or x64 budget changed.

Eight affected PC artifacts were rebuilt with their existing identity and
checked for PE width; see [S70 inventory](../etc/evidence/t580-s70-artifacts.json).
The counted tracked implementation delta, excluding manifests, documentation
and binaries, is one production file +3/-1 and 54 test/config files +130/-73.
Lib and Emulator are unchanged. This is bounded PIC and receiving evidence,
not full-tree, desktop, Setup persistence, post-Setup boot or PC110 product
qualification. Sources, hashes and unresolved results are in
[S70 evidence](../etc/evidence/t580-s70-reset-interrupt-investigation.md).

## S70 coordinator acceptance

Actual-change review accepts Shared `3d692e367` and NXVM `3a1c703a0` for
the bounded PIC/receiver batch. The original 300- and 360-second failures
remain recorded; measured longer x86 executions and final registered CTests
pass under the documented x86-only budgets. The eight PC artifacts and both
test manifests match the evidence. Lib and Emulator were not changed. S70
closes without asserting complete PC110 boot or full-repository qualification;
the second post-Setup power wait and the remaining T580 ledger stay open.

## S71 corrective qualification finding

After S70 acceptance, inspection of the actual x86 Ninja command showed its
Release cache had empty `CMAKE_C_FLAGS_RELEASE`; CPU objects and all four
S70 x86 PC artifacts were consequently built without compiler optimization.
The x86 functional results above are valid only for that unoptimized graph,
and the S70 x86 artifacts do not satisfy the required optimized-Release
contract. S71 corrects and requalifies those artifacts and receiving tests.
The x64 graph used `-O3 -DNDEBUG` and is unaffected. The exact origin of the
empty cached value has not been established. See
[S71 evidence](../etc/evidence/t580-s71-postsetup-power.md).

## S71 executor delivery — real post-Setup resume and optimized x86

Both widths completed the unchanged TFT firmware's Setup-save reset, second
physical F12 wake, retained CPU-state restoration and INT 19h handoff. This
is not proof of a booted guest. The stable x86 Release tree was corrected to
`-O3 -DNDEBUG`; 669 x86 units, four PC profile receivers and both existing
PC110 firmware integration cases passed. The four x86 PC executables were
rebuilt; their hashes and the unchanged x64 identities are in the
[artifact inventory](../etc/evidence/t580-s71-artifacts.json). The original
DeskPro 300-second limit passed after optimization. The x86 PC110 handoff
case passed under its former 1080-second limit in 355.89 seconds, and its
new 600-second registration was verified but not rerun. The actual tracked
build/test delta is +55/-23 lines across three files; no production C, Lib or
Emulator source changed. Details and remaining qualification boundaries are
in the [S71 evidence](../etc/evidence/t580-s71-postsetup-power.md).

## S71 coordinator acceptance

Actual-change review accepts `f23baaa30` for the scoped firmware handoff and
x86 Release correction. The tested 1080-second registration and final
600-second registration remain distinguished in the evidence; no completed
guest boot or desktop qualification is claimed. The owned background jobs
exited, and 63 reviewed scratch files totaling 6,245,660 bytes were removed.
T580 remains open for its other qualification domains.

## S72 executor delivery — PC110 CMOS configuration persistence

The PC110 App saves and restores each variant's executable-adjacent CMOS
configuration file. The guest RTC remains the live owner; only the quiescent
configuration RAM is captured, after successful Product binding and before
Machine destruction. Empty files are initial sentinels; malformed nonempty
files and I/O failures report errors. The existing Storage truncate/write
operation is not crash-atomic, and RTC calendar/off-time behavior is outside
this delivery. The two real TFT firmware Setup/save runs and independent BIOS
checksum replays passed on both host widths. Affected X86-NXVM and PC110
tests, actual Product file-handling tests, the PC identity receivers, eight
rebuilt PC artifacts and the specialized gates passed as detailed in the
[S72 evidence](../etc/evidence/t580-s72-cmos-persistence.md) and
[artifact inventory](../etc/evidence/t580-s72-artifacts.json). PC110 guest
boot, RTC clock persistence, DSTN/display qualification and product release
remain open; Lib and Emulator are unchanged.
