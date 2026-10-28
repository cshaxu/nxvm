# M5 T580 S85 — Delivered for owner review: Window scale and shared corpus review

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T580 S85. |
| Admission And Approval | Owner accepted S84, approved the Shared Lib correction and requested focused review of snap restore, test fixture ownership, timeout budgets and cross-repository synchronization before acceptance. |
| Objective | Prevent native minimum Window width from becoming the initial automatic frame scale. |
| Non-goals | No new public API, second window state machine or recovery framework; do not change guest video semantics, owner media or configuration. |
| Reference Baseline | S84 same-run PC110 Window trace and DeskPro EGA readiness record. |
| Candidate Proposal | Limit the initial automatic scale to natural frame size while retaining fit-down and subsequent manual scaling. |
| Files And ABI Surface | Lib Window implementation, existing Lib Window test, both manifests, NXVM/PC110, MyNES and MyRaidenII receiving artifacts and task evidence; no ABI changes. |
| Applicable Rules | NXVM guide and execution, architecture, coding, documentation and UI rules. |
| Verification | Dual-width cached full build and full registered tests, targeted size regression and actual PC110 Window observation, release and artifact verification. |
| Expected Markers | 8x1 forced to 120x1 followed by 640x400 does not create 9600x6000; fit-down and manual scaling remain correct. |
| Asset Needs | Existing external firmware and task-owned temporary probes; preserve owner inputs. |
| Reporting Requirements | Actual diff, builds, tests, artifacts, desktop limits and pushed commits; await owner validation. |
| Stop Conditions | Do not modify owner files or claim unexecuted checks. |
| Exit Criteria | Reviewed fix, complete requested verification and published artifacts; owner validation required. |
| Original Owner Request | Fix PC110 Window startup and continue its product qualification. |
| Similar-Issue Sweep | Check initial fit-down, user resizing, PC110 and DeskPro, without guest-specific host rules. |

S86 is reserved to resume S84 PC110 product and Setup colour observation after
S85 delivery and owner acceptance; only one S is active at a time.

The first draft passed the dual-width focused test but the real PC110 product
still reached 9600x6000: a later WM_SIZE reset the scale. The owner approved
the explicit-operation contract: WM_SIZE updates actual geometry but cannot
itself select a scale; an actual WM_SIZING interaction commits its result on
WM_EXITSIZEMOVE. Existing explicit natural-size and maximize operations retain
their paths. Snapped-frame adaptation uses the current snapped region without
replacing the scale for ordinary windows. Focused tests pass on both widths.
The P1 production revision passed 759/759 registered non-MIDI tests per
width; the strengthened snap assertion was separately rerun on both widths.
Lib production and test manifests pass on both widths; both build trees report
no work on an incremental rebuild.
The P2 PC110 TFT x64 deployed binary was observed through an isolated
background copy: before the probe resized it, its RDP client was 640x400.
The probe's later debug handshake failed, so this is not a boot or setup-colour
qualification. The [S85 evidence](../etc/evidence/t580-s85-window-scale.md)
records product scope and verification limits.

The owner subsequently identified an untested snap-restore-to-next-frame
contract. A controlled existing fixture reproduced the loss of restored
scale; the current correction treats the native snap-to-restored state
transition as a distinct event. Both focused architecture tests pass; native
drag message ordering remains unverified. Earlier P1/P2 are superseded by the
corrective Shared and receiving-artifact commits.

Both earlier full suites passed 759/760; the native MIDI output test failed
in each architecture and in independent single-test reruns. A direct WinMM
probe enumerated one MIDI device but midiOutOpen returned result 1. After the
owner connected RDP, the native MIDI integration test passed separately on
both x86 and x64. The initial full runs and individual later passes are distinct
evidence; they are not a single uninterrupted 760-test passing run. Owner
inputs were preserved.

The later complete review run passed 760/760 on each width with RDP connected.
It used the earlier configured timeout budgets. After the owner selected 60
seconds, all three affected checks passed again on each width after
reconfiguration. Physical snap/drag and cold-cache timing remain unverified.

# M5 T580 S84 — Accepted: PC110 deployed Window observation

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T580 S84. |
| Admission And Approval | Owner authorized autonomous T580 completion and requested investigation of actual PC110 setup colours. |
| Objective | Observe a released PC110 executable's actual Window output and compare its guest setup colours with recorded guest framebuffer evidence. |
| Non-goals | No foreground process, user INI or media changes, firmware modification, Shared production change or RDP claim without RDP evidence. |
| Reference Baseline | S57 guest-frame colours; S78/S79 firmware diagnostics; S80 deployed binaries and S83 product lifecycle. |
| Candidate Proposal | Run the existing product Window probe in a hidden background process using a task-owned copy and configuration; capture and inspect the client image, including an isolated copy of the already reviewed guest media. |
| Files And ABI Surface | NXVM task evidence and temporary probe inputs only unless an actual product defect is demonstrated. |
| Applicable Rules | NXVM Task Reading Set, execution, architecture, coding, documentation, UI and source/research policy. |
| Verification | Report the actual executable, firmware, client dimensions, process status and captured pixels; distinguish desktop session and RDP. |
| Expected Markers | Real Window frame and measurable colours, or an explicit bounded failure rather than an inferred result. |
| Asset Needs | Existing deployed EXE and previously reviewed external media; task-owned temporary INI, CMOS and disk copies with overlay access. Preserve all owner files. |
| Reporting Requirements | Include reproducible observation, capture limitations, unverified firmware variants and remaining task gaps. |
| Stop Conditions | Do not force guest state, expose a foreground process, modify user media or infer colours from an unrelated window. |
| Exit Criteria | Actual deployed Window evidence is reviewed and recorded, or a concrete unresolved receiver is identified for continued work. |
| Original Owner Request | Implement PC110 and investigate the unexpectedly blue ERROR/Easy Setup pages. |
| Similar-Issue Sweep | Compare the result against the separately captured firmware framebuffer and both fixed variants. |

The deployed TFT x64 Window was observed in an RDP session at 9600x6000;
the screenshot was black and does not prove the reported setup colour. A
task-local instrumented product build now traces the initial 8x1 frame,
native 120x1 client, stored 120/8 scale and subsequent 640x400 frame resized
to 9600x6000 in one run. See [S84 evidence](../etc/evidence/t580-s84-product-window.md).
A candidate Shared Lib correction requires the owner's Shared-code review;
no Shared implementation has been changed and S84 remains active.
The original F65535 reference also identifies an independent Screen Off
snapshot gap; it is not yet tied to the observed window-size failure and
requires a separate validated display behavior decision.
The actual TFT x64 product's Window was subsequently resized by the
background probe for observation. It showed a black 640x400 client with a
short white cursor after 240 seconds; the paused debugger reported
`3800:1333 HLT` with interrupts disabled. The owner of that no-media guest
state remains unconfirmed; see S84 evidence.
The address matches S51's firmware suspend point. A later task-local F12
host-message probe did not establish delivery or a CPU continuation because
its pause/query handshake failed; this remains unverified, not a product-failure
claim. The current product README also omits the App's F12 resume binding.
With the probe handshake corrected, a fresh timed F12 run reported a different
CPU location, F000:E98A, after the input and paused successfully. It did not
sample the location immediately before input and did not produce the pink
page; F12 delivery and guest continuation are not yet fully attributed.
A further product run did sample the prior 3800:1333 HLT, resumed through the
normal monitor, posted F12 and later observed F000:C852. The screen was still
black with a cursor. This is product-path continuation evidence, not a
complete causal trace or colour/guest-media qualification.
The real TFT x64 product also accepted an isolated, overlay-backed test master
and remained controllable, but its timed F12 run still showed a black Window
and did not establish a guest disk read or DOS/Setup. The source disk hashes
were unchanged; see S84 evidence.
The external BIOS identifies the post-F12 location as a candidate 3F4h FDC
status-poll loop, consistent with the captured DX and opcode. That initial
single sample did not distinguish forward progress from a stuck wait; runtime
shadow bytes were not captured.
In a further real-product run, the first post-F12 sample was F000:C852 and a
second sample after six seconds of normal resume was 3000:8000. This proves
the guest reached another instruction location, but does not by itself prove
that the FDC loop completed; see S84 evidence.

# M5 T580 S83 — Accepted: PC110 released-product lifecycle

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T580 S83. |
| Admission And Approval | Owner authorized autonomous PC110 completion and required real product integration. |
| Objective | Cover lifecycle commands through both actual PC110 product executables, complementing the lower-level real-firmware diagnostic. |
| Non-goals | No new production API, extra execution path, Shared changes, guest patching or desktop/RDP claim. |
| Reference Baseline | S80 product process test covers reset and CMOS; S82 verifies firmware via the lower-level diagnostic. |
| Candidate Proposal | Extend the existing isolated product process test with start, pause, resume, reset, stop and exit using the existing monitor contract. |
| Files And ABI Surface | PC110 App integration test and task evidence; no production ABI changes. |
| Applicable Rules | NXVM Task Reading Set, execution, architecture, coding and documentation rules. |
| Verification | Both variants and widths in real product processes, complete repository unit suite and documentation gate. |
| Expected Markers | Correct ordered lifecycle completion texts from actual products and clean process exit. |
| Asset Needs | Existing compiled EXEs and task-owned temporary INI/CMOS; preserve owner inputs. |
| Reporting Requirements | Distinguish product lifecycle, low-level guest, actual desktop and RDP evidence. |
| Stop Conditions | Do not lower assertions, add a private API or claim a guest frame from a monitor message. |
| Exit Criteria | Both variant product lifecycle tests and the required complete unit suites pass on both widths. |
| Original Owner Request | Complete PC110 implementations with meaningful product tests and clean delivery. |
| Similar-Issue Sweep | Review whether existing diagnostics are being mistaken for product execution. |

Both DSTN and TFT actual-product process lifecycle tests passed on x86 and
x64. The complete registered unit suite passed 713/713 on each width. This
test-only change did not rebuild product EXEs or modify Shared. The tests do
not claim an observed PC110 desktop or RDP guest screen; see
[S83 evidence](../etc/evidence/t580-s83-product-lifecycle.md).

# M5 T580 S82 — Accepted: repository integration qualification

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T580 S82. |
| Admission And Approval | Owner authorized autonomous T580 completion and full cached unit/integration validation. |
| Objective | Run registered repository and owner-managed external integrations on x86 and x64, and resolve evidenced PC110 failures. |
| Non-goals | No Shared Lib/Emulator changes, user-media modifications, hidden test skips or unsupported desktop qualification claims. |
| Reference Baseline | S81 full unit qualification, S78–S80 real PC110 guest and product artifacts. |
| Candidate Proposal | Build integration targets in the existing cached graphs and run registered integration cases, recording failures and owners. |
| Files And ABI Surface | Task evidence and only the owner of an evidenced failure. |
| Applicable Rules | NXVM Task Reading Set, execution, architecture, coding and documentation rules. |
| Verification | Both-width integration suites, affected regression tests, artifact identity and documentation gates. |
| Expected Markers | Actual test inventory and pass/fail/skip results; no partial-suite completion claim. |
| Asset Needs | Existing authorized external media with task-owned writable copies; preserve masters, INI and snapshots. |
| Reporting Requirements | Distinguish background, desktop and RDP evidence and report unresolved qualification. |
| Stop Conditions | Do not suppress test failures, mutate owner media or bypass component boundaries. |
| Exit Criteria | Required integration results and remaining qualification limits are recorded and submitted. |
| Original Owner Request | Complete the PC110 task with valid releases and tests. |
| Similar-Issue Sweep | Identify stale private diagnostic coupling and missing integration registration. |

The four PC profile-specific integration matrices and the PC110 guest and
artifact cases passed on both widths; the five registered desktop integration
tests passed on each. The x64 default-profile non-desktop suite failed twice on
native MIDI output when parallel, then passed 42/42 in a serial run; x86 passed
42/42. Independent WinMM probes also returned an external error. That intermittent
failure's cause remains unknown. See [S82 evidence](../etc/evidence/t580-s82-repository-qualification.md).
The T580 diagnostic boundary and actual PC110 desktop/RDP experience remain
open; these results do not alone close T580.

# M5 T580 S81 — Accepted: remove duplicate PC110 test composition

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T580 S81. |
| Admission And Approval | Owner authorized autonomous completion and explicitly requested a single production path and nonredundant tests. |
| Objective | Remove the PC110 App's test-only duplicate base construction while preserving meaningful board, input and display coverage at the actual composition or owning chip. |
| Non-goals | No new production APIs, raw Core getters, second machine path, Lib/Emulator edits or loss of demonstrated timing/latch checks. |
| Reference Baseline | S78 evidence identifies pc110_machine.c as a second construction used only by base/display tests; machine_construction_smoke uses the actual plan and driver. |
| Candidate Proposal | Inventory every retired assertion, use existing X86-NXVM chip/board tests where they already own it, migrate PC110-specific wiring checks through the real driver, then remove duplicate production code and tests. |
| Files And ABI Surface | PC110 App/test/CMake and NXVM task evidence only; any genuinely missing Shared coverage is identified before changing its owner. |
| Applicable Rules | NXVM Task Reading Set, execution, architecture, coding and documentation rules. |
| Verification | Cached x86/x64 builds, complete repository unit suite, relevant chip/board tests, product composition and artifact identity, documentation and boundary gates. |
| Expected Markers | One PC110 construction owner; no retired base-composition references; an assertion-by-assertion disposition and no coverage loss. |
| Asset Needs | Existing synthetic test fixtures; preserve user firmware, media, configuration and released assets. |
| Reporting Requirements | Report deleted and retained paths, per-assertion coverage, actual diff and any limitations. |
| Stop Conditions | Do not make public interfaces or change guest behavior solely to simplify testing; do not delete unique checks without transferring them. |
| Exit Criteria | Actual production composition and appropriate chip tests cover the former behaviors; duplicate code and test registration removed; complete repository unit suite passes on both widths. |
| Original Owner Request | Implement a usable PC110 with clean architecture, relevant tests, released binaries and a clean worktree. |
| Similar-Issue Sweep | Inspect remaining PC110-only tests for other duplicate composition and private implementation dependencies. |

NXVM P1 `1a5b4cd6a` removes the test-only construction and its two tests,
transfers PC110 wiring checks to the real product construction, and retains
controller and latch tests at their owning components. Both widths passed 11
PC110 unit tests, six artifact/CMOS integration tests and four controller and
display owner tests. The four rebuilt EXEs passed their firmware identity
checks. See [S81 evidence](../etc/evidence/t580-s81-composition-inventory.md).
P2 prematurely marked S81 accepted before the required full unit suite. The
reopened S81 completed the missing suite: 668 registered non-MyNES and 45
MyNES unit tests passed for each of x86 and x64, using the same cached build
trees. No MyNES product executable was rebuilt for this test.
The real-firmware diagnostic's private Machine boundary and full-repository
and desktop qualification remain open; this does not close T580.

# M5 T580 S80 — Accepted: PC110 artifact build and deployment

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T580 S80. |
| Admission And Approval | The owner authorized autonomous completion and explicitly requested four fixed PC110 binaries containing external BYOB firmware. |
| Objective | Replace PC110-only previews with one build/deploy path for four named x86/x64 DSTN/TFT executables, retaining fixed firmware identities and the adjacent INI. |
| Non-goals | No runtime firmware selection, ROM/media commit, user INI rewrite, duplicate production entry, Lib/Emulator change or premature T580 completion claim. |
| Reference Baseline | S78 and S79 verified both firmware variants through real Setup, DOS and guest input; PC110 currently has only build-tree previews. |
| Candidate Proposal | Use the existing product artifact helper and firmware libraries, retain one target per fixed variant, update consumers and owner-local release checks. |
| Files And ABI Surface | NXVM App/test/CMake/docs/assets and the Shared repository ignore policy; no public API or Shared production component changes. |
| Applicable Rules | NXVM Task Reading Set, execution, architecture, coding, documentation and source/research policy, including the owner's explicit embedded-EXE authorization. |
| Verification | Cached x86/x64 build, four PE widths, exact names and embedded variant identities, product/INI integration, affected units, artifact and documentation gates. |
| Expected Markers | Only mypc110_x86.exe, mypc110_x64.exe, mypc110_tft_x86.exe and mypc110_tft_x64.exe, with fixed BIOS/font hashes and unchanged INI. |
| Asset Needs | Existing validated external PC110 firmware and generated build-only embedding sources; preserve all external media and INI. |
| Reporting Requirements | Record exact artifacts/hashes, firmware provenance, tests and unverified desktop/RDP behavior; separate Shared and NXVM commits. |
| Stop Conditions | Do not publish raw firmware, change firmware inputs, overwrite the owner INI, or claim the full T accepted from build success alone. |
| Exit Criteria | Four correctly named, deployed and tracked binaries and their fixed firmware pairing are verified and committed with the build/test/document changes. |
| Original Owner Request | Implement both PC110 variants and publish four owner-named EXEs, with tests, commits and push. |
| Similar-Issue Sweep | Remove obsolete preview target references, inspect release-root ownership and avoid a second build or deployment path. |

Shared P1 `da7c5a97d` limits tracking to the four owner-selected EXE names;
NXVM P2 `6657cb7dc` supplies their formal build/deployment, artifacts,
tests and [release evidence](../etc/evidence/t580-s80-pc110-release.md).
Both-width product/CMOS/reset, positive and swapped-firmware negative tests,
PE checks, artifact-root and documentation gates passed. The owner INI and
CMOS files were not modified. This accepts S80, not the full T580: the
test-only duplicate composition, diagnostic private boundary, full-repository
qualification and desktop/RDP verification remain open.

# M5 T580 S79 — Accepted: PC110 DSTN guest qualification

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T580 S79. |
| Admission And Approval | The owner authorized autonomous PC110 S tasks and requested original and TFT variants on both host widths. |
| Objective | Verify the derived DSTN firmware's own Setup, DOS boot and guest input on x86 and x64, and repair an evidenced machine-owned defect if found. |
| Non-goals | No guest-state injection, ROM mutation, fabricated success, unrelated product changes or physical-accuracy claim. |
| Reference Baseline | S78 accepted TFT guest input and both-width DSTN power/resume handoff; neither proves DSTN Setup or DOS. |
| Candidate Proposal | Reuse the existing real-firmware guest diagnostic linked to the DSTN firmware; identify any true variant-specific divergence before changing its owner. |
| Files And ABI Surface | PC110 App/tests and NXVM task evidence; Shared X86-Core/X86-NXVM only for demonstrated defects under existing authorization, no Lib/Emulator changes. |
| Applicable Rules | NXVM Task Reading Set, execution, architecture, coding and source/research policy. |
| Verification | Both-width real DSTN firmware and guest checkpoints, affected tests and artifacts if code changes, manifests and documentation gates. |
| Expected Markers | Actual DSTN Setup execution, firmware boot handoff, DOS loader, guest input and resulting frame, with statuses and Core faults recorded. |
| Asset Needs | Existing reviewed derived DSTN ROM, task-owned DOS media overlay and Setup reference; preserve external masters. |
| Reporting Requirements | Separate derived-ROM provenance, actual guest results and unavailable desktop/RDP checks. |
| Stop Conditions | Do not force TFT behavior onto DSTN, patch the guest, suppress a real fault or claim a host session was tested when absent. |
| Exit Criteria | Both architectures reach and independently demonstrate the admitted original-panel guest path, or the exact failing owner is identified and repaired within this S. |
| Original Owner Request | Implement PC110 with fixed original and TFT binaries and complete build, tests, release and submission. |
| Similar-Issue Sweep | Review variant firmware selection and test oracle ownership without a second product execution path. |

Both host widths reached the derived DSTN firmware's actual Setup entry and
matched the reviewed Setup image, completed the DOS loader, all four inking
calibration points and isolated PS/2 input, and produced the same launcher
frame without a Core fault. Both diagnostic processes exited zero. The
[S79 evidence](../etc/evidence/t580-s79-dstn-guest.md) records the derived
firmware provenance and the limits of this background guest verification.
PC110 release targets, production EXE qualification, test-boundary cleanup,
desktop/RDP checks and the T580 closure gate remain open.

# M5 T580 S78 — Accepted: PC110 controller restoration and pointing device

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T580 S78. |
| Admission And Approval | The owner authorized autonomous PC110 S tasks and previously approved the bounded Shared/Core changes needed by the product. |
| Objective | Identify and implement the missing controller restoration event and demonstrate the genuine PC110 BIOS and guest pointing-device path. |
| Non-goals | No host BIOS interception, guest-memory injection, manufactured driver success, external media changes, Lib/Emulator changes, or physical-timing claim without a source. |
| Reference Baseline | Accepted S77 records real C201h failure, the SMM lock, two real firmware callbacks and the 0Ah/9Bh and 11h mailbox sequence. Its 10h reply did not resolve the failure. |
| Candidate Proposal | Follow the original controller image, firmware and source-labelled references to identify the receiver and event; implement one functional L2 model at its correct owner, then verify the actual guest. |
| Files And ABI Surface | PC110 App and tests, or X86-NXVM/Core only if the evidence requires their existing boundary; no new public API merely for diagnostics. |
| Applicable Rules | NXVM Task Reading Set, execution, architecture, coding and source/research policy. |
| Verification | Affected x86/x64 unit and real-firmware regressions, actual guest driver and input path, plus required affected artifacts and manifest/gate checks. Long work is bounded and backgrounded. |
| Expected Markers | Original firmware clears its lock; BIOS mouse requests succeed; actual guest input and final frame are independently observed; no Core fault. |
| Asset Needs | Reviewed external BIOS, Setup oracle and task-owned disk overlays; preserve their originals. |
| Reporting Requirements | Separate modeled controller facts, real guest outcomes, failure cases, both widths and unexecuted desktop/RDP validation. |
| Stop Conditions | Do not patch guest state, substitute another device, fabricate a BIOS result or claim physical timing from an L2 model. |
| Exit Criteria | The real firmware clears its own lock and guest pointing-device initialization and input are verified, with failures and cleanup honestly accounted for. |
| Original Owner Request | Implement PC110 and both firmware variants through build, test, release and submission, with faithful input and display. |
| Similar-Issue Sweep | Check controller reply ownership, reset/retention and device paths without introducing duplicate lifecycle or input queues. |

An L1 temporary mailbox timing experiment let the original x64 firmware
clear its own SMM lock and reach handoff, but does not establish the event's
physical timing, data or actual guest mouse behavior. Its code was removed
after launch. See the [S78 investigation](../etc/evidence/t580-s78-controller-restoration.md).
A real-guest x64 probe using the temporary hypothesis checked the DOS mouse
driver's actual BIOS request and return.
The App candidate uses a separately defined L1 delay model;
both-width PC110 unit suites, firmware resume and FDC/reset tests pass. The
PC110 product is not yet released. The initial x64 DOS probe
used the earlier experimental binary, so its result must not be described
as direct validation of the revised implementation.
That experimental probe returned C201h successfully but then recorded C206h
failures and a Core fault. A subsequent bounded x64 probe of the candidate
observed the relevant BIOS and 8042 calls.
The follow-up confirmed that C206h fails because the mouse rejected E6h;
Shared P1 `d93729439` corrects that command and NXVM P2 `2fc16aff3` refreshes
the eight existing PC artifacts. Both commits are pushed. The current
candidate passed both-width PC110 units and the full X86-NXVM test group,
and both-width real TFT DOS/inking runs reached the same launcher image.
NXVM P3 `0886b2211` records the independently tested controller model and is
pushed. Both-width PS/2-only guest input probes succeeded and their matching
moved-pointer frame is the same as the host-input run. DSTN power/resume and
boot handoff also passed on both widths. NXVM P4 `e9a14aae5` retains the
diagnostic and evidence. The reviewer accepts S78 against its scoped exit;
complete DSTN guest qualification, final PC110 releases and T580 remain open.

# M5 T580 S77 — Accepted: PC110 inking device and input boundary

Coordinator review accepts pushed Shared P1 `2f853d2db` and NXVM P2
`b597189bd` for the inking receiver and the independently sourced 10h
completion. Both-width targeted units and firmware checkpoints are recorded
in S77 evidence. The optional long guest diagnostic exits 1 on its inherited
VM86 exception predicate and is not called a passing test. Mouse C201h,
the duplicate test-only construction, private diagnostic boundary, DSTN DOS,
desktop presentation and final PC110 release qualification remain open in
T580; S78 investigates the mouse rather than declaring T580 complete.

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T580 S77. |
| Admission And Approval | Owner authorized autonomous PC110 implementation and S76 was accepted at `7fa26dc31`. |
| Objective | Investigate and implement the PC110 inking-device boundary needed by the genuine driver and calibration flow, using documented hardware or labelled L2 reference evidence. |
| Non-goals | No host BIOS interception, guest-memory injection, external media modification, fake calibration success, or Lib/Emulator changes without an independently justified contract. |
| Baseline | [S76 evidence](../etc/evidence/t580-s76-status-port.md) records real PC DOS driver output and its first inking-calibration request; input consumption is not yet proven. |
| Candidate Proposal | Map existing Machine input, port and IRQ owners against the independently documented inking protocol; implement a single bounded hardware receiver and its input adapter, then verify the real guest path. |
| Files And ABI Surface | PC110 App and appropriate X86-NXVM/Core receiver and tests if warranted; public contract extensions only under existing owner authorization. |
| Applicable Rules | NXVM Task Reading Set, execution, architecture, coding and source/research policy. |
| Verification | Deterministic protocol tests, affected dual-width builds and real-firmware/driver receiver as evidence warrants; hidden bounded background work in existing Ninja trees. |
| Asset Needs | Reuse the reviewed task-owned test master and Setup oracle as read-only inputs with overlays; preserve external originals. |
| Reporting Requirements | Distinguish protocol, host input, actual calibration, pointing-device driver, DOS startup and desktop validation. |
| Stop Conditions | Do not invent the digitizer protocol, substitute a PS/2 mouse for inking, modify guest software or claim unobserved hardware precision. |
| Exit Criteria | The documented inking interface and real driver calibration are demonstrated, or a specific next unimplemented owner is located and addressed. |
| Original Owner Request | Complete PC110 implementation, original and TFT variants, and four published executables. |
| Similar-Issue Sweep | Keep PS/2 relative pointing and inking samples independent and audit both routes for real caller ownership. |

The implementation covers the documented inking receiver and the separately
source-supported EC 10h completion. Shared P1 `2f853d2db` supplies the
generic input/IRQ attachment and is pushed. The App implementation, tests,
records and affected PC artifacts form the NXVM implementation P; coordinator
review must retain the negative mouse result and the optional diagnostic's
exit 1 rather than treating either as a passing result.

The protocol, input/IRQ implementation and its dual-width unit verification are
recorded in [S77 evidence](../etc/evidence/t580-s77-inking-boundary.md).
The real TFT driver has accepted all four calibration points and reached the
same Personaware launcher frame on x86 and x64; the older power diagnostic
still returns failure on its unchanged exception-count predicate. Mouse-driver
investigation and desktop presentation remain outstanding.
The current mouse follow-up records the actual C201h SMI and EC mailbox
instructions in a bounded background guest replay. A proposed FFh mailbox
reply was rejected and removed after an isolated trace showed that it did
not explain the observed failure; see S77 evidence for the distinction.
The subsequent real trace confirms the guest's SMM lock branch. The selected
controller firmware's 10h handler emits an AAh completion, now modeled by
the App mailbox; all 13 PC110 units, real-firmware FDC/reset and boot-handoff
tests pass on both host architectures. The separate DSTN firmware's FDC/reset
and boot-handoff diagnostics passed on both widths. The candidate's real x64
guest mouse trace still returns C201h failure with the SMM EC lock set; the
10h completion is therefore not a demonstrated fix for that failure. A
shorter real-firmware handoff probe confirmed that the lock is already 01h
before DOS starts; power restoration is the next investigation boundary.
The follow-up observed the mailbox transition from available at suspend to
empty before RSM while the lock remained set. A bounded real-firmware trace
identified 3E5DDh as the consumer; the observed restoration did not execute
the known lock-clear points. A shorter resume-only trace confirmed both
original callbacks are invoked: the first sends 0Ah/9Bh and 11h, while the
second fails to observe the input needed to clear the lock. The controller
event remains under investigation; temporary observers were removed.

# M5 T580 S76 — Accepted: PC110 operating-system progression

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T580 S76. |
| Admission And Approval | Owner authorized autonomous PC110 implementation and S75 was accepted at `57bae0c18`. |
| Objective | Follow the real TFT BIOS and validated loader beyond its first instruction, capture actual system output and execution state, and identify and fix any evidenced next owner defect. |
| Non-goals | No firmware interception, injected guest state, external image writes, Shared Lib/Emulator changes, or unsupported claim that a bounded run reached a DOS prompt. |
| Baseline | [S75 evidence](../etc/evidence/t580-s75-system-loader.md) establishes matching initial loader execution on both host widths. |
| Candidate Proposal | Continue the existing real-firmware diagnostic after loader handoff for a finite instruction budget, capture the actual guest display and machine stop reason, then use evidence to select any further repair. |
| Files And ABI Surface | Existing PC110 diagnostic and responsible App or X86-NXVM/Core code only if the observed behavior requires a repair. |
| Applicable Rules | NXVM Task Reading Set, execution, architecture, coding and source/research policy. |
| Verification | Hidden bounded background execution using stable Ninja trees; x64 first, matching x86 and affected regressions when justified. |
| Asset Needs | S73 task-owned derived test disk and Setup oracle, read-only inputs and guest overlays; retain external originals unmodified. |
| Reporting Requirements | Distinguish actual guest screen, loader progression, DOS prompt, display and desktop results; record exact bounds, hashes and remaining gaps. |
| Stop Conditions | Do not inject guest code, mutate external images, or treat reaching an instruction budget as operating-system success. |
| Exit Criteria | A real DOS boot checkpoint is independently demonstrated or a specific evidenced next failing owner is repaired and validated. |
| Original Owner Request | Complete PC110 implementation, original and TFT variants, and four published executables. |
| Similar-Issue Sweep | Check the actual responsible loader, disk and display routes without speculative special cases. |

The former F65535 status-port wait is corrected by Shared `e3bb44fc1`;
targeted dual-width controller and real-firmware FDC checks pass. The x64
post-loader replay passed that wait but later accumulated exceptions and
entered another loop; a bounded follow-up captures its exception context.
See [S76 evidence](../etc/evidence/t580-s76-status-port.md). At this checkpoint
S76 was still active and no DOS or PC110 release qualification had been claimed.

The subsequent context capture found an empty VM86 INT 1Ch IDT gate, so the
observed IDT-related #GP is consistent with the CPU check. The later loop
reads the BIOS timer byte at 0000:046Ch rather than polling the 02F8h port;
the 50-million-instruction run ended after only three ticks while the guest
waits for more than nineteen. A longer real-guest replay passed that wait
but then showed a sustained exception storm without a new display frame;
its responsible path remains under investigation. DOS startup is not proven.

The storm sample is VM86 `INT 16h` with AH=01h, a keyboard availability
query. A bounded probe sent one Enter through the existing Machine driver at
an earlier query and inspected display registers after execution. No new
frame was produced before the subsequent scanout change.

The input probe's Enter was sent at an earlier query, so its outcome does not
prove that the later wait consumed input. Its captured CRTC and graphics
registers identify a 640-by-475, 128-byte-stride planar configuration that
the former fixed mode-12 scanout rejected. The bounded planar receiver now
produces identical real-firmware PC DOS startup frames on both widths, and
the affected component and PC110 units pass. All eight existing PC receiver
artifacts were rebuilt and their product/INI identity checks pass. See the
S76 evidence for exact observations and the unresolved DOS-input and
desktop-display boundaries.

The later input probe reached PC DOS driver messages and the inking driver's
first calibration request; because elapsed execution and one key both changed,
it does not prove input consumption. The supplied disk invokes INKDRV and the
local PC110 references describe an independent IRQ-driven digitizer. Inking,
mouse-driver behavior, final DOS/Personaware operation, both physical panel
variants, Window colour and product publication remain open under T580.

# M5 T580 S75 — Accepted: PC110 system-loader handoff

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T580 S75. |
| Admission And Approval | Owner authorized autonomous PC110 implementation and S74 was accepted at `d9b4cf893`. |
| Objective | Follow the genuine selected TFT BIOS and disk boot chain through loading and executing the initial DOS system loader; independently compare guest-loaded bytes with the documented test disk. |
| Non-goals | No BIOS or disk interception, synthetic guest-memory injection, ROM modification, external media writes, Lib/Emulator change, or claim that loader entry proves DOS startup. |
| Baseline | [S74 evidence](../etc/evidence/t580-s74-partition-boot.md) proves both host widths reach the actual partition sector and x64 reaches the selected disk's system-file comparison. |
| Candidate Proposal | Extend the existing bounded diagnostic to observe the first system loader at its disk-defined handoff, with independently read expected bytes; investigate and repair only any evidenced receiver defect. |
| Files And ABI Surface | Existing MyPC110 diagnostic, responsible App or X86-NXVM/Core source and associated tests only when justified. |
| Applicable Rules | NXVM Task Reading Set, execution, architecture, coding and source/research policy. |
| Verification | Hidden bounded background execution using stable x64/x86 Ninja trees, independent disk evidence, affected regressions and manifests according to actual changes. |
| Asset Needs | Reuse the S73 task-owned reviewed test master and Setup oracle as read-only inputs with overlays; retain their external originals untouched. |
| Reporting Requirements | Distinguish partition entry, loader entry, DOS prompt and desktop; record input and output hashes, actual tests and remaining gaps. |
| Stop Conditions | No guest boot-code injection, external image mutation or unsupported physical-accuracy claim. |
| Exit Criteria | Real loader handoff proven at both widths or the specific next failure identified and repaired with relevant verification. |
| Original Owner Request | Complete PC110 implementation, both panel variants and four released executables. |
| Similar-Issue Sweep | Check disk geometry and shared receiver callers without speculative product-specific changes. |

S75 delivery: the unchanged TFT BIOS and selected disk loaded and executed
the independently checked 1,536-byte system loader on both host widths.
Both widths built, passed all 12 MyPC110 units and rejected missing media.
See [S75 evidence](../etc/evidence/t580-s75-system-loader.md); only the App
diagnostic and task records changed, with no production or release binary
changes. DOS prompt, original panel, Window colour and PC110 release remain
open under T580.

# M5 T580 S74 — Accepted: PC110 partition boot receiver

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T580 S74. |
| Admission And Approval | Owner authorized autonomous PC110 implementation, source-labelled L2 references, and S73 closed at `a916220f9`. |
| Objective | Continue the genuine TFT BIOS handoff through the actual MBR and its active partition boot sector, identify and repair any evidenced device, board or CPU receiver defect, and retain the remaining DOS/Personaware path for T580. |
| Non-goals | No host BIOS/disk interception, ROM patch, synthetic guest memory, external media writes, Lib/Emulator modification, or claim that reaching the partition sector proves DOS startup. |
| Baseline | [S73 evidence](../etc/evidence/t580-s73-pc110-ata-slave.md): dual-width real BIOS to matching primary MBR, validated ATA Recalibrate, and identified task-local geometry-adjusted test image. |
| Candidate Proposal | Extend the existing diagnostic after its verified MBR entry, follow the guest's ordinary INT 13h and the second 7C00h handoff, and independently compare the selected partition sector. Then observe the next finite DOS execution checkpoint and repair only a demonstrated existing owner. |
| Files And ABI Surface | Existing MyPC110 diagnostics, affected App or X86-NXVM/Core implementation and tests only if evidence requires; no new public contract without the existing owner authorization. |
| Applicable Rules | NXVM Task Reading Set, execution, architecture, coding and source/research policy. |
| Verification | Hidden, finite background work in stable Ninja/ccache trees; x64/x86 direct guest and independent disk evidence; affected receiver regressions, manifests and artifacts as required by actual changes. |
| Asset Needs | Reuse S73's reviewed task-local test image and Setup oracle for this immediate continuation, each as a read-only input with overlay. Preserve and hash the source external image; remove task-local inputs after the receiver no longer needs them. |
| Reporting Requirements | Distinguish MBR, partition boot sector, DOS and desktop results; record source/test delta, provenance, budget, output hashes, failures and retained qualification gaps. |
| Stop Conditions | Never alter external master media or inject guest boot code; do not treat a timeout or sector handoff as OS success. |
| Exit Criteria | Actual MBR-to-partition transfer is proven on both widths, or a specific next failing owner is established and repaired within this bounded receiver; record subsequent DOS checkpoint honestly. |
| Original Owner Request | Complete actual PC110 implementation, both hardware variants and four published executables. |
| Similar-Issue Sweep | Check boot read geometry and shared ATA callers without changing other machines speculatively. |

S74 delivery: the independently compared MBR and active partition boot sector
were executed under the real TFT BIOS on both host widths. The x64 follow-up
also reached the selected disk's successful system-file-name check. Both
widths built, MyPC110 units passed 12/12 each, and missing-media preflight
failed correctly on each. See [S74 evidence](../etc/evidence/t580-s74-partition-boot.md)
for exact binary boundaries, hashes, provenance and remaining limitations.
Only the App diagnostic and task documentation changed; no production source
or released product binary changed. DOS, DSTN, Window colour and PC110
publication remain open under T580.

# M5 T580 — Open after S73

S73 is accepted: Shared `43f87360a` implements ATA Recalibrate and NXVM
`2056af519` records real-firmware dual-width BIOS-to-MBR handoff, receiving
regressions and eight rebuilt PC artifacts. The accepted pair uses an
explicitly documented, resized test master and the original read-only slave;
this is not DOS or desktop qualification. Further boot, RTC, display and
PC110 release work remains open. See [S73 evidence](../etc/evidence/t580-s73-pc110-ata-slave.md).

S72 Shared `54a2518de` and NXVM `9c0a31a54` are accepted for configuration
RAM persistence. Real TFT Setup/save and separate original-BIOS checksum
replays pass on both host widths; Product file handling is exercised by both
variants. The affected units, PC identity receivers, eight rebuilt PC EXEs
and specialized gates are recorded in the
[S72 evidence](../etc/evidence/t580-s72-cmos-persistence.md). The existing
Storage operation is not crash-atomic, and RTC calendar/off-time persistence
remains unqualified. T580 stays open for real guest boot, both panel models,
desktop behavior and PC110 publication.

S71 implementation `f23baaa30` is accepted for the bounded real-firmware
post-Setup power path and optimized x86 Release correction. Both widths reach
the real INT 19h handoff after the second wake; 669 x86 units and the affected
PC receivers pass, and the four optimized x86 PC artifacts are published.
The [S71 evidence](../etc/evidence/t580-s71-postsetup-power.md) preserves the
exact timeout and remaining qualification boundaries. T580 remains open.

S70 is accepted: Shared `3d692e367` corrects the 8259 input lifetime and
NXVM `3a1c703a0` records dual-width receiver evidence and eight rebuilt PC
artifacts. X86-NXVM units pass 233/233 per width, both real-firmware FDC and
boot-handoff checkpoints pass per width, and all eight PC profile identity
and boot receivers pass under their final measured budgets. Earlier timeouts
and the old-PIC control remain documented, not relabelled as passes.

The unchanged TFT BIOS passed the previously observed `102` branch and the
subsequent power wait on both widths in S71. RTC calendar persistence, main
Setup, actual guest/media boot, PCIC, panel identity and color, DSTN, desktop
behavior and PC110 product publication remain unqualified. See
[S70 evidence](../etc/evidence/t580-s70-reset-interrupt-investigation.md).

## Verified Baseline

S65 is accepted at `aba6329a8`. It traced the selected TFT BIOS through its
ordinary recovery, POST and `INT 19h` route and found no F65535 XR index/data
write. The bounded C000 replay also found none. The existing standard VGA
palette route remains the sole evidenced colour owner; the reported blue frame
does not authorize an RGB/BGR or panel-table patch. See
[S65 evidence](../etc/evidence/t580-s65-f65535-panel-table-audit.md).

## Verified Baseline

Both host widths run the unchanged TFT BIOS's complete Setup loader and
ordinary input/reset transaction. ERROR and subsequent date/time frames are
byte-identical across host widths with programmed BE9E96 background.
Both reach the fifth real reset at FFFF:FFF0 after 144,110,532 instructions,
with four known exceptions and no Core fault. Normal main Setup,
configuration persistence and selected guest boot remain unqualified.

All 712 exact final-tree units per width and specialized gates pass.
Main-feature real FDC/boot regressions pass at 40.54/215.85 s x64 and
53.38/321.83 s x86; after the final preflight-only range guard, rebuilt
firmware FDC regressions pass at 43.99/63.34 s. The
[S57 record](../etc/evidence/t580-s57-f65535-planar-colour.md) distinguishes
these exact source boundaries, the mathematical normal-domain equivalence,
failed candidates, source grades and unverified domains. It does not
relabel pre-guard executable bytes as the final build.

Eight PC receiving builds and adjacent INI identities pass; their published
relinks change only PE timestamp/checksum. Four PC110 previews are rebuilt
but unpublished. The [inventory](../etc/evidence/t580-s57-artifacts.json)
records actual hashes, PE widths and embedded 0.5.0580. Lib, Emulator,
X86 Core/Debug/IBMPC, MyNES/MyRaidenII, owner INI and external ROM/font/media
are unchanged. All 90 owned scratch files/13,302,526 bytes are removed after
task-exit/hash review; stable Ninja/ccache remain.

S58 adds the source-labelled empty two-slot RB5C396 controller at the actual
3E0h/3E1h route, with no card, storage, IRQ/window or guest claim. Final-tree
unit qualification is 714/714 on each width. Existing real-firmware
FDC-reset/boot-handoff integrations pass x64 in 42.24/225.02 s and x86 in
54.59/306.85 s; they remain bounded non-regression evidence, not PCIC
consumption or product qualification. See the [S58 record](../etc/evidence/t580-s58-pcic-receiver.md)
and [task history](../history/M5-T580-pc110-qualification.md).

S59 completes the declared ATA two-device construction route.  After a fresh
Ninja relink of every unit target, the complete repository-only unit suite
passes **714/714** on both x64 and x86 (212.88 s and 73.32 s respectively).
That qualification includes the focused ATA slave construction proofs and
the existing real-firmware FDC-reset/boot-handoff non-regressions recorded in
the S59 evidence.  It does not qualify authentic SunDisk/CF inputs, BIOS
storage consumption, PCIC storage, Setup persistence, selected-guest boot,
panel behavior, desktop use, or product publication.

S60 selects the source-recorded primary SunDisk geometry through the existing
master-only Machine construction route: **123x2x32**.  Its disposable
exact-size/short-size proof and full MyPC110 labelled regression pass on x64
and x86; the unchanged real-firmware FDC-reset/boot-handoff checks pass in
45.91/231.73 seconds x64 and 53.51/318.12 seconds x86.  After fresh relinks,
the full repository-only unit suite passes **714/714** on x64 (61.36 seconds)
and x86 (59.03 seconds).  This does not qualify real SunDisk/CF media, BIOS
storage consumption, Setup persistence, PCIC storage, selected guest or a
PC110 release; no PC110 executable is published.  See the
[S60 evidence](../etc/evidence/t580-s60-pc110-master-geometry.md).

## Remaining T580 Qualification

The [proposal](../proposals/m5-pc110-qualification.md) and
[convergence ledger](../etc/evidence/t580-s1-pc110-qualification-ledger.md)
retain normal Setup/configuration persistence and post-reset progression,
per-slot SunDisk/CF storage and selected guest, PCIC/card handling, display
enable/blanking and panel/TFT/DSTN identity, live input/lifecycle, desktop and
four-artifact publication. Restored ERROR/date-time colours close only the
named standard planar/colour receiver.

Next admission must consume a coherent remaining receiver with actual source
and direct proof; do not seed successful CMOS, hook BIOS PCs, force guest
flags or introduce host display swaps. Source-labelled functional L2 is
permitted; physical-unit measurement is not a prerequisite to model work.
No remaining original domain is removed or called complete.

[Task history](../history/M5-T580-pc110-qualification.md) records acceptance.
[Retained status](../history/M5-T580-retained-status.md) preserves prior
packets as history, not concurrent active work.
[Queue](QUEUE.md) retains unrelated ordered candidates.

## S64 acceptance

Implementation `40be5c4f2` is accepted after actual-change review. The
optional reset byte stays in the existing board-to-KBC configuration route;
the generic `43h` default is preserved, and PC110's `65h` disables AUX until
the guest sends the ordinary `A8h` command. All 714 unit tests pass on both
widths; complete 755-test host runs retain only the unrelated
`lib.integration/native_midi_output` host-endpoint failure. Eight existing PC
artifacts are rebuilt and PE-width checked; four MyPC110 previews stay
unpublished. The active packet will be replaced only when the next bounded
T580 receiver is admitted.
