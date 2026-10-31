# M5 T582 S34 — Active: remaining PC110 usability costs

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T582 S34 after S33. |
| Admission And Approval | Owner authorized automatic numeric S progression and preserving timing and behavior while improving PC110 usability. |
| Objective | Identify the next substantial, evidence-backed CPU, memory or device cost after S33 and design a safe improvement. |
| Non-goals | No guest-work skipping, timing downgrade, second executor, host clock substitution or owner-media mutation. |
| Reference Baseline | S27–S33 profiling and [S33 receiver evidence](../etc/evidence/t582-s33-prefix.md). |
| Candidate Proposal | [T582 proposal](../proposals/m5-pc110-startup-clock-performance.md), S34. |
| Files And ABI Surface | Initial audit limited to NXVM evidence and task-owned scratch. Before production edits, document the specific owner, interfaces, candidate and verification in this packet. |
| Applicable Rules | NXVM guide, Execution, Documentation, Architecture, Coding and source policy. |
| Verification | Reuse prior samples before running focused probes; preserve guest timing, event ordering, faults, observations and all CPU profiles. Required full dual-width receiver applies to Shared edits. |
| Expected Markers | A prioritized viable design and measured workload limits, or evidence explaining why a proposed improvement is not safe or worthwhile. |
| Asset Needs | Existing private task-owned firmware and media copies only; preserve owner assets. |
| Reporting Requirements | Explain actual observed costs, candidate size and tradeoffs, tests and remaining interactive usability gaps without claiming an unsupported speedup. |
| Stop Conditions | Do not lower timing accuracy, skip guest work or treat matching PC/ticks or incomplete frames as proof of equivalence. |
| Exit Criteria | Qualified improvement or evidence-backed next candidate; keep T582 open until actual usability and owner acceptance. |
| Original Owner Request | Improve PC110 performance to interactive usability while preserving guest behavior. |
| Similar-Issue Sweep | Reconcile existing CPU timing, preview, memory, board and device experiments before introducing another probe. |

### Admitted PIC priority-scan consolidation

S34 additionally admits consolidation of the duplicate master/slave PIC
selection loops in `src/x86-nxvm/chips/pic8259/pic.c`. The existing master
loop remains the single scan; slave requests bypass only the existing
master-specific cascade rules. No interface, state, interrupt ordering,
guest time or other component is changed. The scratch source reduces 28
lines net. Review the actual diff and qualify the existing PIC priority,
rotation, masking, cascade and board tests on both widths before the cached
full Shared receiver gate and fourteen product artifacts. Record host-noise
limits separately from semantic evidence; do not claim a product speedup or
PC110 usability without direct evidence.

The PIC consolidation and dual-width receiver have completed; see
[PIC evidence](../etc/evidence/t582-s34-pic-scan.md). S34 remains active to
identify a substantial route toward the still-unmet usability objective.

### Admitted memory route tier consolidation

The three duplicate query/failure loops over the already-matched device list
in `src/x86-core/machine/memory.c` may use one tier-ordered loop. Preserve
raw and selected-alias precedence, replacement/ordinary/fallback order,
registration order, observational queries, failure notification and RAM
fallback; no interface, persistent state or alternate route is added. Review
and run existing overlap, alias, terminal-failure, memory and cache tests on
both widths, then run the cached full Shared receiver and check all fourteen
product artifacts. Fixed-work and direct-call comparisons are supporting
evidence only; they do not establish interactive PC110 usability.

The memory tier consolidation and dual-width receiver have completed; see
[memory evidence](../etc/evidence/t582-s34-memory-tiers.md). S34 remains
active because the PC110 usability objective is not yet established.

# M5 T582 S33 — Closed: prefix classifier consolidation

Shared `c1b6f95b4` and MyRaidenII `e51572b51` deliver the bounded candidate
and receiver artifacts. [Evidence](../etc/evidence/t582-s33-prefix.md)
records both-width qualification and performance limitations. The PC110
usability goal remains open.

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T582 S33 after S32. |
| Admission And Approval | Owner authorized automatic numeric S progression and code/algorithm simplification without material regression or timing downgrade. |
| Objective | Move the existing single CPU-private prefix classifier into its private header for inlining across the execution and timing translation units. |
| Non-goals | No new decoder, public API, persistent state, timing change, guest-work skipping or owner-media mutation. |
| Reference Baseline | Shared `b2c17ba68`; S32 dual-width fixed-work candidate and source-caller audit. |
| Candidate Proposal | [T582 proposal](../proposals/m5-pc110-startup-clock-performance.md), S33; [S32 evidence](../etc/evidence/t582-s32-execution-cost.md). |
| Files And ABI Surface | X86 Core private CPU instruction source/header and manifest; existing relevant tests if necessary, fourteen receiver artifacts and NXVM evidence. No public interface change. |
| Applicable Rules | NXVM guide, Execution, Documentation, Architecture, Coding and source policy. |
| Verification | Preserve all prefix classification and profiles; dual-width CPU, V30, fault, timing and preview tests, actual diff and generated-code size review, representative PC110 comparison, full cached dual-width receiver and artifact gates. |
| Expected Markers | One classifier and no unsupported performance or usability claim. |
| Asset Needs | Existing private task-owned firmware and media copies for performance probes; preserve owner assets. |
| Reporting Requirements | Actual diff, artifact size, measured limits, tests, and unresolved PC110 usability. |
| Stop Conditions | Do not change guest timing, observable behavior, CPU profile rules, public APIs or owner media. |
| Exit Criteria | Qualified bounded improvement or documented rejection, with full receiver and documentation obligations where production changes. |
| Original Owner Request | Improve PC110 performance to interactive usability while preserving guest behavior. |
| Similar-Issue Sweep | Check all classifier references, supported CPU profiles, preview, fault and V30 paths. |

# M5 T582 S32 — Closed: dominant PC110 execution cost

[S32 evidence](../etc/evidence/t582-s32-execution-cost.md) records the
deadline cost and rejected candidates, and the bounded dual-width prefix
classifier candidate. It does not establish playable PC110 performance.

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T582 S32 after S31. |
| Admission And Approval | Owner authorized automatic numeric S progression and requested a prioritized explanation of Machine overhead. |
| Objective | Identify and prioritize the major remaining cost in the actual PC110 execution chain, including the Machine versus isolated CPU difference. |
| Non-goals | No guest-work skipping, timing downgrade, second executor, host clock substitution or owner-media mutation. |
| Reference Baseline | S27 performance envelope, S20 execution attribution, S28 execution-chain design and S31 memory evidence. |
| Candidate Proposal | [T582 proposal](../proposals/m5-pc110-startup-clock-performance.md), S32. |
| Files And ABI Surface | NXVM evidence and task-owned profiling scratch; no production changes or public interface changes in this audit. |
| Applicable Rules | NXVM guide, Execution, Documentation, Architecture, Coding and source policy. |
| Verification | Trace actual execution and observation paths, distinguish nested from exclusive costs, and use bounded representative measurements only to resolve actionable gaps. |
| Expected Markers | Evidence-ranked cost breakdown and a concrete viable candidate with owner, estimated change and validation boundaries. |
| Asset Needs | Private task-owned copies of external firmware and media only if needed; preserve owner assets. |
| Reporting Requirements | Separate measured facts and hypotheses, identify measurement limitations and do not claim PC110 usability. |
| Stop Conditions | Do not modify production behavior or infer equivalence solely from matching instruction counts or PC. |
| Exit Criteria | Reviewable prioritized design for S33 or documented evidence that another measured hypothesis is required. |
| Original Owner Request | Improve PC110 performance to interactive usability while preserving guest behavior. |
| Similar-Issue Sweep | Reconcile previous profiling and rejected candidates before introducing another probe. |

# M5 T582 S31 — Closed: remaining workload bottlenecks

Shared `b2c17ba68` and MyRaidenII `8c9509b95` qualify the bounded memory
deduplication and receiver artifacts. [Evidence](../etc/evidence/t582-s31-memory.md)
records both-width tests, artifacts and the lack of a proven product speedup.
The T remains open for the PC110 usability objective.

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T582 S31 after S30. |
| Admission And Approval | Owner authorized automatic S progression and optimizations with no material regression or timing downgrade. |
| Objective | Re-evaluate actual PC110 workload bottlenecks and implement justified reductions in memory or other remaining paths. |
| Non-goals | No guest work skipping, second executor, host clock substitution, timing downgrade or owner-media mutation. |
| Reference Baseline | S27–S30 performance and receiver evidence and T582 investigation index. |
| Candidate Proposal | [T582 proposal](../proposals/m5-pc110-startup-clock-performance.md), S31. |
| Files And ABI Surface | Relevant Shared components, PC110 adaptation and existing tests only where justified by measurement and dependency boundaries; manifests, receiver artifacts and NXVM evidence. |
| Applicable Rules | NXVM guide, Execution, Documentation, Architecture, Coding and source policy. |
| Verification | Preserve mappings, invalidation, faults, side effects and time/event ordering; targeted tests and required cached dual-width full Shared receiver gate after Shared production changes. |
| Expected Markers | Evidence-backed improvement or explicit negative result; report actual workload and limits. |
| Asset Needs | Private task-owned copies of existing external firmware and media if required. |
| Reporting Requirements | Actual diff, component ownership, measured results, unchanged behavior and remaining usability work. |
| Stop Conditions | Do not claim equivalence from matching PC/tick checkpoints alone, or skip device or input semantics. |
| Exit Criteria | Qualified changes or prioritized evidence-backed follow-up; add numeric S work before final receiving if actual usability remains unmet. |
| Original Owner Request | Improve PC110 performance until usable, retaining correct guest behavior. |
| Similar-Issue Sweep | Compare prior CPU, memory, scheduler and display candidates before repeating a probe. |
# M5 T582 S30 — Closed: guest-time and device advancement

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T582 S30 after S29. |
| Admission And Approval | Owner authorized automatic numeric S progression and optimizations without timing-accuracy downgrade. |
| Objective | Find and qualify a justified reduction in redundant guest-time or device-advancement work. |
| Non-goals | No guest event skipping, host-clock substitution, second executor, owner-media mutation or product-specific timing special case. |
| Reference Baseline | S27 performance envelope, S28 chain evidence and S29 receiver gate. |
| Candidate Proposal | [T582 proposal](../proposals/m5-pc110-startup-clock-performance.md), S30. |
| Files And ABI Surface | X86 Core Machine and X86 NXVM board scheduling only as justified by evidence, their existing tests and manifests, receiver artifacts and NXVM evidence. No unreviewed API expansion. |
| Applicable Rules | NXVM guide, Execution, Documentation, Architecture, Coding and source policy. |
| Verification | Preserve deadline, asynchronous input, exception, device, reset, debug and before/internal/after ordering; targeted dual-width tests and required cached full Shared receiver gate after production changes. |
| Expected Markers | Measured or demonstrable reduction with same clock and device behavior, or documented negative candidate result. |
| Asset Needs | Task-owned private copies of external media if required. |
| Reporting Requirements | Actual diff, results, limitations and no unsupported PC110 usability claim. |
| Stop Conditions | No accuracy downgrade or assumption that a stable deadline implies no intervening mutation. |
| Exit Criteria | Qualified optimization or evidence-backed rejection; T582 remains open until usability and owner acceptance. |
| Original Owner Request | Improve PC110 performance while preserving guest execution. |
| Similar-Issue Sweep | Reuse previous deadline, clock, DMA, interrupt and board observations before designing new work. |
# M5 T582 S29 — Closed: bounded CPU shift simplification

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T582 S29 after S28. |
| Admission And Approval | Owner authorized automatic numeric S progression and source/algorithm reductions without material regression; no timing downgrade. |
| Objective | Replace duplicated iterative SHL/SHR/SAR execution with one bounded implementation preserving every existing profile's results and observations. |
| Non-goals | No guest instruction skipping, separate executor, timing change, additional public interface, owner-media mutation or premature usability claim. |
| Reference Baseline | S28 execution-chain evidence and accepted S26 rotate differential. |
| Candidate Proposal | [T582 proposal](../proposals/m5-pc110-startup-clock-performance.md), S29; [design](../etc/evidence/t582-s28-execution-chain.md#shift-execution-candidate-for-s29). |
| Files And ABI Surface | Shared X86 Core CPU instructions, existing relevant CPU tests and affected manifests; required receiver artifacts and NXVM evidence. No public ABI extension. |
| Applicable Rules | NXVM guide, Execution, Documentation, Architecture, Coding and source policy. |
| Verification | Compare old and new actual CPU executions across profiles, widths, all counts, flags, timing and observations; existing targeted tests, cached dual-width builds and full Shared receiver unit/integration gates, fourteen EXE checks, matched PC110 work. |
| Expected Markers | Single production shift path, fewer source lines and bounded computation with unchanged guest-visible behavior and documented measured limits. |
| Asset Needs | Task-owned copies of existing external firmware and media; never mutate owner assets. |
| Reporting Requirements | Actual diff and all results; do not equate a local simplification with usable PC110 performance. |
| Stop Conditions | No timing downgrade, invalid C shifts, changed undefined-flag behavior or reduced test assertions. |
| Exit Criteria | Qualified bounded implementation or evidence-backed rejection, plus required receiver and documentation obligations. T582 stays open until actual product acceptance. |
| Original Owner Request | Improve PC110 performance and code/algorithm quality without materially worsening other properties. |
| Similar-Issue Sweep | Compare all three directions, profiles, register/memory operations, boundary counts and error paths against original implementation. |
# M5 T582 S28 — Closed: end-to-end execution-chain design

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T582 S28 after S27. |
| Admission And Approval | Owner authorized continued performance work and automatic S sequencing; timing accuracy downgrades require approval. |
| Objective | Trace and design justified simplifications across CPU execution, retirement, memory and board advancement using the S27 baseline. |
| Non-goals | No second executor, skipped guest work, host-clock substitution, accuracy downgrade or owner-media modification. |
| Reference Baseline | S27 evidence and T582 investigation index. |
| Candidate Proposal | [T582 proposal](../proposals/m5-pc110-startup-clock-performance.md), S28. |
| Files And ABI Surface | Task documentation and task-owned scratch for the design audit; no production changes in this design stage. |
| Applicable Rules | NXVM guide, Execution, Documentation, Architecture, Coding and source policy. |
| Verification | Trace actual call sites and compare existing measurements; use bounded probes only for a specific unresolved hypothesis. |
| Expected Markers | Reviewable candidate owners, exact ordering and invalidation boundaries, estimates and verification plans. |
| Asset Needs | Existing private task-owned media only if a justified probe requires it. |
| Reporting Requirements | Separate evidence from hypothesis and report rejected or uncertain candidates. |
| Stop Conditions | Do not modify runtime behavior during the design stage or lower timing accuracy. |
| Exit Criteria | Concrete justified designs or documented unsafe candidates sufficient to execute S29 and S30. |
| Original Owner Request | Improve PC110 performance to usability with a single correct execution chain. |
| Similar-Issue Sweep | Review CPU, memory, board advancement and prior candidate results without repeating settled experiments. |
# M5 T582 S27 — Closed: comparable workload and performance envelope

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T582 S27 after accepted S26. |
| Admission And Approval | Owner authorized continued PC110 performance work, automatic numeric S sequencing and the S27 workload-baseline plan; timing accuracy downgrades require explicit approval. |
| Objective | Establish a traceable, comparable x86/x64 performance baseline and prioritize the end-to-end execution-chain work without repeating already settled experiments. |
| Non-goals | No production change, host-clock substitution, guest-work skipping, second executor, timing downgrade, owner-media mutation or playability claim from an indirect probe. |
| Reference Baseline | Accepted S26 through `602292076`, closure `010a4b067`, and the T582 investigation index and existing fixed-work and product-path evidence. |
| Candidate Proposal | [T582 proposal](../proposals/m5-pc110-startup-clock-performance.md), S27 and following. |
| Files And ABI Surface | NXVM task documentation and task-owned scratch probes only; no Shared or App production API, executable, configuration or owner media changes. |
| Applicable Rules | NXVM guide, Execution, Documentation, Architecture, Coding and source policy. |
| Verification | Audit source/measurement provenance and workload identity; report guest instructions, ticks, host elapsed, display/input method, width and framebuffer completion where available. Run only new bounded probes required to fill an actionable gap; verify documentation and actual diff. |
| Expected Markers | A prioritized cost/measurement table explicitly distinguishes matched work, approximate workloads, x86/x64 evidence, host variation and unmeasured or unsupported comparisons. |
| Asset Needs | Existing task-owned private copies of PC110 firmware, CMOS and configured disk media; no external media content is committed. |
| Reporting Requirements | Provide baseline values, measurement limitations, candidate priorities and a clear statement that the PC110 usability goal remains open. |
| Stop Conditions | Do not modify guest timing, runtime code or published products during this audit; do not treat matching PC/ticks as complete-state proof. |
| Exit Criteria | Reviewable baseline and prioritized evidence table sufficient to enter S28, with unavailable measurements and limitations explicitly recorded. |
| Original Owner Request | Improve PC110 performance to usability while preserving guest behavior; compare the entire instruction-to-device chain with the reference emulator. |
| Similar-Issue Sweep | Cross-check earlier startup, Game, World Clock, synthetic-loop, CPU timing and board-advance observations for duplicated probes and incompatible workloads. |

# M5 T582 S26 — Closed: single-executor throughput design audit

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T582 S26 after S25 audit. |
| Admission And Approval | Owner authorized continued PC110 performance work and autonomous numeric S sequencing; timing accuracy downgrades require owner consent. |
| Objective | Qualify the compact lexical-preview copy, consolidate duplicate timing-input and lexical-prefix scans, simplify parity and conditional-branch timing calculations, eliminate duplicate timing-result publication, simplify ordinary rotate execution and remove redundant PC110 video-adapter aperture checks without changing guest behavior or timing; retain the larger PC110 usability goal. |
| Non-goals | No hidden accuracy downgrade, guest-work skipping, host-time substitution, second executor or user media mutation. |
| Reference Baseline | Shared `8dd8d06d0`, NXVM `65c15ab69`; [S25 comparison](../etc/evidence/t582-s25-execution-comparison.md). |
| Candidate Proposal | [T582 proposal](../proposals/m5-pc110-startup-clock-performance.md), S26. |
| Files And ABI Surface | Shared X86 Core CPU preview, timing-input, timing selection and flag implementations, their private declarations and manifest; existing CPU timing, preview, arithmetic, bus and Machine retirement-observation tests and their manifest; MyPC110 video adapter and existing route tests, plus NXVM evidence. All seven App receiver artifacts remain under the Shared gate. No public interface or other Shared production package change. |
| Applicable Rules | NXVM guide, Execution, Documentation, Architecture, Coding and source policy. |
| Verification | Audit preview's read and failure paths and omitted fields, including 486 retirement observation across asynchronous entry; verify the unified timing and lexical prefix scans preserve flags and opcode positions for all profiles and malformed input, including prefixed lexeme decoding and real timing previews; verify all 256 parity inputs and all 16 branch conditions across flag combinations through actual CPU instructions. Verify ROL/ROR count, carry, overflow, undefined flags and widths across CPU profiles with real CPU differential and existing tests. Verify the video adapter retains null, zero-length, invalid-range and observed-read semantics using the existing chip and Machine paths. Run dual-width affected tests, manifest and dependency checks; cached 14-App builds and full unit/integration receiving gate apply to Shared changes. Retain fixed-work and actual Game evidence without claiming equal complete machine state. |
| Expected Markers | The existing lexical preview remains the sole path, reads no omitted access log or dispatch table, and all receiver gates are honestly reported. |
| Asset Needs | Existing external reference checkouts and private copied CMOS/media if needed. |
| Reporting Requirements | Behavioral/timing consequences, scope, verification cost, and no claim of real-time usability without evidence. |
| Stop Conditions | Do not change existing timing accuracy or clock contract without the owner's explicit consent. |
| Exit Criteria | Fully qualified bounded improvement or rejection with evidence; T582 remains open until actual usability and full product qualification are achieved. |
| Original Owner Request | Improve PC110 performance to usability while preserving guest behavior. |
| Similar-Issue Sweep | Compare CPU, board timing, RTC, interrupt and all Shared receiver obligations. |

S26 bounded preview-copy implementation is Shared `a794bfcdd`, with PC
receiver artifacts `7e3bd57c8` and MyRaidenII artifacts `b76c5c71e`.
The [evidence](../etc/evidence/t582-s26-compact-preview.md) records both-width
qualification and measured limits. This is not PC110 usability acceptance;
S26 is accepted as a bounded batch; T582 remains open because the larger
throughput and usability objective is unresolved.

An exploratory [same-host PC110-EMU run](../etc/evidence/t582-s26-pc110-emu-measurement.md)
has now executed the owner-supplied BIOS and a private master-disk copy. Its
instruction count, media configuration, guest state and service shortcuts
differ from NXVM, so the recorded duration is not an equivalent speedup ratio.
An isolated per-read refresh-status experiment did not reach the baseline
checkpoint and remained at the firmware's CLI/HLT return address while guest
time advanced; it is not admitted as a fix.
An independent synthetic-loop comparison now shows a large per-instruction
difference even without the real firmware shortcuts; the isolated CPU fixture
and neutral Machine have different buses and observation contracts. This is
an engineering lead, not a complete-state or gameplay qualification.

The owner approved selecting a small, clearly deduplicating algorithm even
without a large measured speedup. The timing-input collector's two scans are
therefore admitted for equivalence review and consolidation; the mixed
measurements are not presented as a proven performance gain. The snapshot-copy
probe remains exploratory and is not admitted as a production change.
The prefix-scan implementation is Shared `38b16a3b3`; its dual-width unit,
integration and receiving results are recorded in the same comparison
evidence. It is a bounded code reduction, not PC110 usability acceptance.
The owner's net-code-reduction rule also admits the earlier parity-folding
candidate for CPU-path and full receiver qualification. The low byte is fixed
width; both the old and new operations are O(1), and mixed earlier timings do
not prove a runtime speedup.
The parity simplification is Shared `4a869137c`; its full dual-width receiving
and the initially stale test-binary failures are detailed in
[parity evidence](../etc/evidence/t582-s26-parity-audit.md). PC110 usability
and the larger performance objective remain unverified and open.

The timing selector's redundant output parameter and duplicate write have also
been eliminated; [the evidence](../etc/evidence/t582-s26-timing-publication.md)
records both-width receiving. This net-smaller internal change preserves the
existing result and does not by itself establish a performance improvement.

The proposed reuse of the 486 timing terms' prefix count was tested and
withdrawn: [the comparison](../etc/evidence/t582-s26-486-term-prefix.md) found
two slower x86 samples and substantial direct-test caller churn alongside the
faster x64 samples. This is not proof of a stable regression or an improvement.

The condition calculation now reads only the flags required by the selected
branch, with no new state or execution path. [The evidence](../etc/evidence/t582-s26-branch-condition.md)
records exhaustive flag comparisons, dual-width receiving and the limits of
the synthetic performance comparison; it does not establish playable speed.

The next-instruction preview remains required by completed-observation and
asynchronous-entry contracts. An intentional observation ablation and an
existing-test extension covering 486 are recorded in [the preview comparison](../etc/evidence/t582-s26-preview-observation-bound.md).
The ablation is not a production proposal and does not establish playable speed.

The PC110 display adapter's redundant aperture checks were removed without
changing the chip's access validation or the adapter's invalid-input status;
[the evidence](../etc/evidence/t582-s26-video-adapter.md) records both-width
tests and a real Game-frame probe. This does not establish playable speed.

The ordinary ROL/ROR helpers now use constant-work width-limited rotation
instead of per-bit loops, retaining existing profile counts and flags. The
[qualification](../etc/evidence/t582-s26-rotate-constant.md) records both-width
CPU differential, unit and integration results, the corrected manifest failure,
receiver artifacts and a Game-frame probe. It does not establish playable speed.

Small-copy callsite attribution identified high-frequency firmware reads and
four-byte RAM writes. A bounded write specialization showed mixed two-width
results and was not adopted; [the evidence](../etc/evidence/t582-s26-small-copy-attribution.md)
records its workload and limitations. A further two-byte firmware-copy
specialization showed no x64 benefit and was also not adopted.

S25 compared local external PC110 references: one uses host-derived RTC and BIOS shortcuts, the other describes QEMU translated execution and fixed per-instruction timing. These are not proven equivalent to NXVM's current timing contract; [evidence](../etc/evidence/t582-s25-execution-comparison.md). No production, public API or EXE changed; the S19 unit gate remains the last complete repository-only run on both widths.

# M5 T582 S25 — Closed: PC110 execution-model comparison

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T582 S25 after S24 audit. |
| Admission And Approval | Owner authorized reference to external emulators, L1/L2 modeling and autonomous numeric S sequencing. |
| Objective | Compare the current PC110 execution and idle paths against external PC110-EMU and PC110-QEMU references to identify a justified performance direction. |
| Non-goals | No imported third-party code, protected firmware commits, fabricated responses, user media mutation or unverified runtime claim. |
| Reference Baseline | Shared `8dd8d06d0`, NXVM `65c15ab69`; [S24 evidence](../etc/evidence/t582-s24-asmi-audit.md). |
| Candidate Proposal | [T582 proposal](../proposals/m5-pc110-startup-clock-performance.md), S25. |
| Files And ABI Surface | Read-only external source and task-owned evidence/probes; changes require precise scope and receiving policy. |
| Applicable Rules | NXVM guide, Execution, Documentation, Architecture, Coding and source policy. |
| Verification | Contrast actual execution, firmware services, guest clocks and I/O with existing measured NXVM behavior; label static versus dynamic evidence and L1/L2 confidence. |
| Expected Markers | Concrete evidence of transferable architecture or behavior and semantic costs, or documented negative evidence. |
| Asset Needs | Existing read-only reference checkouts and private copied CMOS/media as needed. |
| Reporting Requirements | Provenance, estimated implementation/verification work, observable differences and remaining limitations. |
| Stop Conditions | No attribution of a result to an external emulator without inspecting its actual implementation or running it. |
| Exit Criteria | Traceable comparison or fully qualified improvement; T582 remains open pending usability qualification. |
| Original Owner Request | Improve PC110 performance to usability while preserving guest behavior. |
| Similar-Issue Sweep | Compare idle, interrupt, timing and device paths and the existing S19–S24 evidence. |

S24 found that repeated World Clock ASMI requests are APM CPU Idle calls; one complete trace follows a firmware immediate-return branch after reading SCAMP state that matches the published live dump. The evidence does not support suppressing the requests or changing the state by guess; [evidence](../etc/evidence/t582-s24-asmi-audit.md). No production, public API or EXE changed; the S19 dual-width unit gate remains the last complete repository-only run. Usability remains unresolved.

# M5 T582 S24 — Closed: World Clock ASMI service audit

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T582 S24 after S23 audit. |
| Admission And Approval | Owner authorized continued PC110 performance work, L1/L2 models and autonomous numeric S sequencing. |
| Objective | Determine whether repeated World Clock ASMI requests represent legitimate firmware service or waiting for an unimplemented device response. |
| Non-goals | No ignored SMI, invented successful result, skipped guest work, duplicate executor or owner media mutation. |
| Reference Baseline | Shared `8dd8d06d0`, NXVM `65c15ab69`; [S19 evidence](../etc/evidence/t582-s19-steady-profile.md), T580 S48/S90 service evidence. |
| Candidate Proposal | [T582 proposal](../proposals/m5-pc110-startup-clock-performance.md), S24. |
| Files And ABI Surface | Task-owned probes first; change owner determined by evidence and authorized scope. |
| Applicable Rules | NXVM guide, Execution, Documentation, Architecture, Coding and source policy. |
| Verification | Capture actual service identifiers, frequency, return values and relevant guest activity using private media and existing single execution path. |
| Expected Markers | Distinguish normal request traffic from a reproducible missing service, or record negative evidence. |
| Asset Needs | Existing external firmware and private copied CMOS/media only. |
| Reporting Requirements | Source confidence, measured workload, correctness and desktop limitations. |
| Stop Conditions | Do not conflate counts with a fault or substitute guessed firmware return values. |
| Exit Criteria | Traceable negative result or fully qualified correction; T582 stays open pending actual usability qualification. |
| Original Owner Request | Improve PC110 performance to usability while preserving guest behavior. |
| Similar-Issue Sweep | Compare startup and World Clock ASMI calls and existing T580 service records. |

S23 measured instruction preparation/dispatch and rejected a variable-copy candidate without consistent cross-width improvement; [evidence](../etc/evidence/t582-s23-dispatch-audit.md). No production, public API or EXE changed; the S19 dual-width unit gate remains the last complete repository-only run. The real World Clock performance gap remains unresolved.

# M5 T582 S23 — Closed: instruction dispatch cost audit

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T582 S23 after S22 audit. |
| Admission And Approval | Owner authorized continued PC110 performance work and autonomous numeric S sequencing. |
| Objective | Attribute the CPU instruction execution cost identified in S20 and investigate a measured semantics-preserving optimization or a negative result. |
| Non-goals | No omitted fault/interrupt/device behavior, host-clock substitution, second executor, unproven JIT or user media mutation. |
| Reference Baseline | Shared `8dd8d06d0`, NXVM `65c15ab69`; [S20 evidence](../etc/evidence/t582-s20-execution-cost.md). |
| Candidate Proposal | [T582 proposal](../proposals/m5-pc110-startup-clock-performance.md), S23. |
| Files And ABI Surface | Task-owned probes first; production changes only upon correctness and performance evidence and full Shared receiving policy. |
| Applicable Rules | NXVM guide, Execution, Documentation, Architecture, Coding and source policy. |
| Verification | Short fixed-work probes first; product-path tests only when justified. Preserve all CPU profiles, timing, error and observation semantics. |
| Expected Markers | Actual guest checkpoint, cost attribution and a measured candidate or negative evidence. |
| Asset Needs | Existing external firmware and private copied CMOS/media only. |
| Reporting Requirements | Variance, actual diffs, qualification and remaining limitations. |
| Stop Conditions | Do not invent a duplicate decoder, replay path or persistent state. |
| Exit Criteria | Traceable measured audit or fully qualified improvement; T582 remains open pending usability qualification. |
| Original Owner Request | Improve PC110 performance to usability while preserving guest behavior. |
| Similar-Issue Sweep | Inspect instruction preparation, dispatch, decoded operands and existing profiling evidence before proposing implementation. |

S22 rejected a fixed-size cache-copy candidate and a partial LTO experiment due to absent or inconsistent benefit; [evidence](../etc/evidence/t582-s22-cache-copy-audit.md). No production, public API or EXE changed; the S19 unit gate remains the last complete repository-only unit run. The World Clock performance gap remains unresolved.

# M5 T582 S22 — Closed: CPU cache-hit copy audit

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T582 S22 after S21 audit. |
| Admission And Approval | Owner authorized continued performance work and autonomous numeric S sequencing. |
| Objective | Assess actual CPU cache-hit copy costs and test a minimal semantics-preserving candidate, or record negative evidence. |
| Non-goals | No new cache, altered guest timing, skipped accesses, undefined alignment or aliasing behavior, or user media mutation. |
| Reference Baseline | Shared `8dd8d06d0`, NXVM `65c15ab69`; [S15 sampling](../etc/evidence/t582-s15-cpu-line-audit.md). |
| Candidate Proposal | [T582 proposal](../proposals/m5-pc110-startup-clock-performance.md), S22. |
| Files And ABI Surface | Task-owned probes first; production scope only with measured correctness and full Shared receiving policy. |
| Applicable Rules | NXVM guide, Execution, Documentation, Architecture, Coding and source policy. |
| Verification | Short fixed-work comparisons first, dual width where warranted; retain full access width, cache and timing semantics. |
| Expected Markers | Actual guest checkpoint and candidate gain or negative evidence. |
| Asset Needs | Existing external firmware and private copied CMOS/media only. |
| Reporting Requirements | Runtime variance, actual diffs and receiver qualification. |
| Stop Conditions | Do not introduce a duplicate memory path, unaligned typed access, new state or claim performance from one short pair. |
| Exit Criteria | Traceable negative result or fully tested and delivered improvement; T582 stays open until usability qualification. |
| Original Owner Request | Improve PC110 performance to usability while preserving guest behavior. |
| Similar-Issue Sweep | Inspect relevant cache reads/writes and existing sample evidence, avoiding unmeasured generalization. |

S21 compared the same prefix classifier made inline in a scratch timing model; the result did not consistently improve both widths and was rejected. See [evidence](../etc/evidence/t582-s21-timing-input-audit.md). No production, public API or EXE changed; the S19 unit gate remains the last complete repository-only unit run.

# M5 T582 S21 — Closed: instruction timing input reuse audit

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T582 S21 after S20 audit. |
| Admission And Approval | Owner authorized continued performance work and autonomous numeric S sequencing. |
| Objective | Assess repeated decoded-input work within the existing instruction timing owner and measure whether a correctness-preserving simplification improves throughput. |
| Non-goals | No removed timing facts, skipped guest work, duplicate executor, host-time substitution, unsupported global cache or owner media mutation. |
| Reference Baseline | Shared `8dd8d06d0`, NXVM `65c15ab69`; [S20 evidence](../etc/evidence/t582-s20-execution-cost.md). |
| Candidate Proposal | [T582 proposal](../proposals/m5-pc110-startup-clock-performance.md), S21. |
| Files And ABI Surface | Task-owned probes first; production scope limited to a justified timing-owner change and existing tests, subject to Shared receiving policy. |
| Applicable Rules | NXVM guide, Execution, Documentation, Architecture, Coding and source policy. |
| Verification | Short fixed-work dual-width comparisons first; only a promising and semantically valid candidate merits a long product-path comparison. Preserve all profile, prefix, failure, event and timing observations. |
| Expected Markers | Actual guest checkpoint, output/timing equivalence and candidate gain or negative evidence. |
| Asset Needs | Existing external firmware and private copied CMOS/media only. |
| Reporting Requirements | Runtime variance, source confidence, actual diffs, receiver gates and interactive limitations. |
| Stop Conditions | No second decoder, new cross-instruction stale state, false precision, or claim of performance from microbenchmarks alone. |
| Exit Criteria | Traceable negative result or fully tested and delivered improvement; T582 stays open until usability qualification. |
| Original Owner Request | Improve PC110 performance to usability while preserving guest behavior. |
| Similar-Issue Sweep | Check existing timing helpers and previous failed candidates before adding abstractions. |

S20 sampled World Clock SMM, ordinary CPU refresh and timing selection; [evidence](../etc/evidence/t582-s20-execution-cost.md). It found no justified SMM shortcut, and no production, public API or EXE changed. The preceding S19 repository-only unit gate passed 712/712 on each architecture. Realtime usability remains unproven.

# M5 T582 S20 — Closed: SMM execution and memory-service cost audit

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T582 S20 after S19 audit. |
| Admission And Approval | Owner authorized continued PC110 performance work and autonomous numeric S sequencing. |
| Objective | Attribute SMM and normal execution costs in World Clock and investigate a measured, semantics-preserving optimization or document negative evidence. |
| Non-goals | No skipped firmware service, bypassed bus behavior, host-time substitution, second executor or user media mutation. |
| Reference Baseline | S18 Shared `8dd8d06d0`, NXVM `65c15ab69`; [S19 evidence](../etc/evidence/t582-s19-steady-profile.md). |
| Candidate Proposal | [T582 proposal](../proposals/m5-pc110-startup-clock-performance.md), S20. |
| Files And ABI Surface | Task-owned probes and NXVM evidence/status; production scope requires measured correctness evidence and existing authorization. |
| Applicable Rules | NXVM guide, Execution, Documentation, Architecture, Coding and source policy. |
| Verification | Hidden, bounded product-path measurements with private media; preserve physical access ordering, side effects, fault and timing semantics. |
| Expected Markers | Explain measured hot owners and demonstrate an improvement or negative result. |
| Asset Needs | Existing external firmware and private copied CMOS/media only. |
| Reporting Requirements | State sampling limitations, model confidence, actual diffs and interactive gaps. |
| Stop Conditions | No ignored SMI, skipped firmware work, unsupported cache eligibility or treating sampled instruction pointers as inclusive time. |
| Exit Criteria | Traceable measured audit or fully qualified change; T582 stays open until actual usability qualification. |
| Original Owner Request | Improve PC110 performance to usability while preserving guest behavior. |
| Similar-Issue Sweep | Review SMM and ordinary execution, shared bus and other receiver behavior. |

S19 recorded World Clock workload, SMM entry and instruction attribution, and a rejected SMRAM cache candidate; [evidence](../etc/evidence/t582-s19-steady-profile.md). All 712 repository-only unit tests passed on each architecture. No production, public API or EXE changed; realtime usability remains unproven.

# M5 T582 S19 — Closed: PC110 steady-state performance audit

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T582 S19 after S18 delivery. |
| Admission And Approval | Owner authorized continued PC110 performance work, appropriate L1/L2 models and autonomous numeric S sequencing. |
| Objective | Identify and quantify the remaining dominant cost during actual Personaware/World Clock steady operation after the S18 cache improvement, and identify a safe next measurable optimization. |
| Non-goals | No production or public API change in this audit, guest-work skipping, second executor, host-time substitution, user media mutation or unsupported realtime claim. |
| Reference Baseline | Shared `8dd8d06d0`, NXVM `65c15ab69`, S18 [evidence](../etc/evidence/t582-s18-cache-design.md). |
| Candidate Proposal | [T582 proposal](../proposals/m5-pc110-startup-clock-performance.md), S19. |
| Files And ABI Surface | Task-owned probes and NXVM evidence/status only; no production or ABI change. |
| Applicable Rules | NXVM guide, Execution, Documentation, Architecture, Coding and source policy; preserve CPU, device, timing and interaction contracts. |
| Verification | Use existing hidden product-path probes and private copied inputs; distinguish startup from steady state and identify the actual worker and hot owners before changing code. |
| Expected Markers | Bounded guest clock, instruction and frame observations, actual worker attribution, and a measured candidate or negative evidence. |
| Asset Needs | Existing external firmware and private copied CMOS/media only. |
| Reporting Requirements | Sampling overhead, current PC110 user-visible limits, source confidence and any noninteractive gaps. |
| Stop Conditions | No assumption that an IP sample is an inclusive cost or that a boot frame establishes realtime operation. |
| Exit Criteria | Traceable steady-state evidence and measured correctness-preserving opportunity or negative result; repository-only unit suite and governance check; T582 remains open until user-facing usability is established. |
| Original Owner Request | Improve PC110 performance to usability while preserving guest timing and correctness. |
| Similar-Issue Sweep | Compare early, late and idle workloads and identify whether the same bottleneck applies to other CPU profiles. |

# M5 T582 S18 — Closed: PC110 shadow cache design and measurement

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T582 S18 after S17 audit. |
| Admission And Approval | Owner authorized continued PC110 performance work, L1/L2 models where hardware detail is absent, and autonomous numeric S sequencing. |
| Objective | Design and measure a generic 486 provider-cache contract that can express the PC110 chipset's dynamic cacheability without corrupting read/write mappings or guest timing. Admit production work only if correctness and meaningful sustained benefit are demonstrated. |
| Non-goals | No PC110 address special case in shared Core, host-time substitution, skipped guest work, second executor, owner media mutation or unverified usability claim. |
| Reference Baseline | S17 [cacheability evidence](../etc/evidence/t582-s17-pc110-cacheability.md), S14 Shared `e92409c90`. |
| Candidate Proposal | [T582 proposal](../proposals/m5-pc110-startup-clock-performance.md), S18. |
| Files And ABI Surface | X86 Core memory provider cache-probe and VL82C420 selected-index interfaces, MyPC110 firmware/config wiring, and their existing tests/manifests; Lib and Emulator unchanged. |
| Applicable Rules | NXVM guide, Execution, Documentation, Architecture, Coding and source policy; documented L1/L2 distinction and unchanged owner boundaries. |
| Verification | Compare actual PC110 early and late behavior; exercise cache enable/disable, dynamic remapping, asymmetric reads/writes, fetch/data, reset, DMA/snoop and failure paths; cached dual-width full-repository unit/integration and fourteen receiver artifacts before delivery. |
| Expected Markers | Source-backed per-block gating, a single generic route owner, correct invalidation and write behavior, and measured PC110 gain or negative evidence. |
| Asset Needs | Existing external firmware and private copied CMOS/media only. |
| Reporting Requirements | Explicit model confidence, expected and actual diffs, full Shared receiving gate if code changes, and desktop limitations. |
| Stop Conditions | Do not enable an incoherent provider cache or falsely call inference a vendor-documented 420 formula. Do not add a second execution or memory path. |
| Exit Criteria | A tested, measured and fully delivered repair, or a documented negative result with repository-only unit and governance gates; T582 remains open until usability qualification. |
| Original Owner Request | Improve PC110 performance to usability while preserving guest timing and correctness. |
| Similar-Issue Sweep | Check other X86 Core provider routes, aliases and receiver applications; never assume all mapped devices are safe to cache. |


S18 delivered Shared `8dd8d06d0`, NXVM `65c15ab69` and MyRaidenII receiver `4e563df94`; [evidence and measured limits](../etc/evidence/t582-s18-cache-design.md) and [14-artifact inventory](../etc/evidence/t582-s18-artifacts.json) document the dual-width receiving gate. The L1 PC110 provider model improves one measured startup pair but does not make the virtual clock realtime; T582 remains open.

# M5 T582 S17 — Closed: PC110 memory cacheability audit

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T582 S17 after S16 audit. |
| Admission And Approval | Owner authorized continued PC110 performance analysis and autonomous numeric S sequencing. |
| Objective | Determine whether PC110 firmware and shadow-memory execution spends substantial time on non-cacheable provider access, and whether an accurate, single-owner model can safely improve it. |
| Non-goals | No production or public API changes in this audit, no invented hardware cacheability, no guest-work skipping, second executor, owner media modification or unverified usability claim. |
| Reference Baseline | S14 Shared `e92409c90`; S13 cache-control and sampling evidence and S16 execution audit. |
| Candidate Proposal | [T582 proposal](../proposals/m5-pc110-startup-clock-performance.md), S17. |
| Files And ABI Surface | Task-owned probes and NXVM evidence/status only; no production or public ABI changes. |
| Applicable Rules | NXVM guide, Execution, Documentation, Architecture, Coding and source policy; preserve guest timing, invalidation, memory side effects and source confidence. |
| Verification | Inspect actual PC110 provider, 486 cache and decode/invalidation owners; measure actual accesses and cache eligibility using hidden bounded private-media probes before considering a candidate. |
| Expected Markers | Access distribution, relevant control and decode changes, correctness requirements and bounded performance impact or negative evidence. |
| Asset Needs | Existing external firmware and private copied CMOS/media only. |
| Reporting Requirements | Separate observed facts from inferred hardware behavior; identify any contract or timing change and confidence level. |
| Stop Conditions | Do not assume dynamic device memory is plain RAM, bypass callbacks or turn a timing-model change into an unnoticed speed shortcut. |
| Exit Criteria | Traceable evidence and a correctness-preserving measured candidate or concrete negative evidence; repository-only unit suite and governance check; T582 remains open pending usability qualification. |
| Original Owner Request | Improve PC110 performance to usability while preserving guest timing and correctness. |
| Similar-Issue Sweep | Check firmware Flash, shadow RAM, address decode, cache invalidation and the other Core receivers before proposing shared changes. |


S17 identified substantial PC110 shadow-provider traffic, actual early/late ECED cache-control transitions and a read/write mapping asymmetry whose CPU cache implications require source-backed modeling; see [evidence](../etc/evidence/t582-s17-pc110-cacheability.md). The repository-only unit suite passed 712/712 on both x64 and x86; NXVM documentation governance passed. No production, public API or EXE changed. PC110 usability remains unproven.

# M5 T582 S16 — Closed: CPU execution observation cost audit

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T582 S16 after S15 audit. |
| Admission And Approval | Owner authorized continued PC110 performance work and autonomous numeric S sequencing. |
| Objective | Measure the CPU execution and instruction-observation work on the actual PC110 worker and identify a correctness-preserving optimization with a meaningful sustained product benefit, or record negative evidence. |
| Non-goals | No production change in this audit, guest-work skipping, extra executor, host-clock substitution, mutation of owner inputs, or usability claim without proof. |
| Reference Baseline | S14 Shared `e92409c90`, S15 evidence [CPU line audit](../etc/evidence/t582-s15-cpu-line-audit.md). |
| Candidate Proposal | [T582 proposal](../proposals/m5-pc110-startup-clock-performance.md), S16. |
| Files And ABI Surface | Task-owned probes and NXVM evidence/status only; no production or public ABI changes. |
| Applicable Rules | NXVM guide, Execution, Documentation, Architecture, Coding and source policy; preserve instruction, timing, Debug, event and observation contracts. |
| Verification | Hidden bounded fixed-work and PC110 product-path comparisons with private copied media; identify exact workload and state invariants before proposing any optimization. |
| Expected Markers | Evidence of dominant costs, candidate outcome and limits on both CPU widths where applicable. |
| Asset Needs | Existing external firmware and private copied CMOS/media only. |
| Reporting Requirements | Sample bias, semantic boundaries, measured gain or negative result and next action; do not claim realtime usability. |
| Stop Conditions | Do not change production code, create a second execution path, skip guest work, or treat profiling samples as inclusive cost. |
| Exit Criteria | Traceable audit and a measured correctness-preserving candidate or concrete negative evidence; complete repository-only unit suite and governance check; T582 remains open pending usability qualification. |
| Original Owner Request | Improve PC110 performance to usability while preserving guest timing and correctness. |
| Similar-Issue Sweep | Examine zeroing/copy, instruction observation, exception and asynchronous entry, Debug and timing ownership rather than generalizing one local result. |


S16 audited instruction-observation, rollback and timing paths, and rejected scratch candidates that did not establish cross-width or sustained benefit; see [evidence](../etc/evidence/t582-s16-execution-audit.md). The full repository-only unit suite passed 712/712 x64 and 712/712 x86. No production, API or EXE changed; PC110 usability remains unproven.

# M5 T582 S15 — Closed: CPU instruction-line hotspot audit

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T582 S15 after S14 delivery. |
| Admission And Approval | Owner authorized continued PC110 performance work, Shared Core analysis and autonomous numeric S sequencing. |
| Objective | Identify the dominant inlined CPU execution work in actual PC110 startup and later workload, and produce a correctness-preserving next optimization design or negative evidence. |
| Non-goals | No Shared or App implementation changes, second executor, guest-work skipping, host-clock substitution, owner media mutation or unverified usability claim. |
| Reference Baseline | S14 Shared `e92409c90`, published through `bc6c13166`; [S14 evidence](../etc/evidence/t582-s14-memory-routing.md) and [S13 CPU evidence](../etc/evidence/t582-s13-cpu-profile.md). |
| Candidate Proposal | [T582 proposal](../proposals/m5-pc110-startup-clock-performance.md), S15. |
| Files And ABI Surface | Task-owned build probes and NXVM evidence/status only; no production or public ABI changes. |
| Applicable Rules | NXVM guide, Execution, Documentation, Architecture, Coding and source policy; one owner and unchanged timing/observation semantics. |
| Verification | Hidden background PC110 production-path sampling with debug line information, private copied inputs, actual worker identification, early/late windows and bounded runtime; compare potential design against existing timing, exception, device and Debug contracts. |
| Expected Markers | Source-line samples, module and thread attribution, guest progress, overhead and a specific evidence-based next action. |
| Asset Needs | Existing external firmware and private copied CMOS/media only. |
| Reporting Requirements | Attribution limits, sampling overhead, concrete code boundaries, rejected designs and remaining noninteractive limitations. |
| Stop Conditions | No machine semantics change, skipped guest work or claim that a sampled line is inclusive CPU cost; do not infer realtime usability. |
| Exit Criteria | Traceable hotspot evidence and a measured correctness-preserving candidate or concrete negative evidence; T582 remains open until usability is proven. |
| Original Owner Request | Improve PC110 performance to usability while preserving guest timing and correctness. |
| Similar-Issue Sweep | Compare early startup and late Personaware phases; distinguish worker from waiting threads and debug/preview paths. |


S15 completed its sampling and rejected three scratch-only candidates for lack of sustained product benefit; [evidence](../etc/evidence/t582-s15-cpu-line-audit.md) records their limits. The complete repository-only unit suite passed 712/712 on each architecture; NXVM documentation governance passed. No production code, public ABI or EXE changed. T582 remains open; PC110 usability remains unproven.

# M5 T582 S14 — Closed: local memory-route candidate

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T582 S14 after the S13 measured audit. |
| Admission And Approval | Owner authorized ongoing PC110 performance repair, Shared X86 Core work and autonomous numeric S sequencing; all seven App receivers are in scope. |
| Objective | Reduce repeated physical-provider traversal by collecting matching registered provider indices once per route evaluation and retaining the existing query priority and order. |
| Non-goals | No persistent cache, second routing path, guest-time substitution, extra execution owner, owner media mutation, or unproven usability claim. |
| Reference Baseline | Shared 2003fef34 and [S13 measured evidence](../etc/evidence/t582-s13-scheduler-observation.md). |
| Candidate Proposal | [T582 proposal](../proposals/m5-pc110-startup-clock-performance.md), S14. |
| Files And ABI Surface | Shared src/x86-core/machine/memory.c, existing test/x86-core machine registration/route tests and manifests; NXVM evidence and status; fourteen App artifacts. No public API or persistent state change. |
| Applicable Rules | NXVM guide, Execution, Documentation, Architecture, Coding, NXVM layout and source policy; queries remain observational, single owner and existing provider limit. |
| Verification | Dual-width targeted route tests including provider capacity, overlapping priorities, decline/failure order, raw/reset and CPU-only routes and aliases; cached full dual-width builds, unit and integration tests and fourteen published EXEs; preserve owner configuration/media. |
| Expected Markers | Same accepted route, callback order, failure status and guest ticks; fixed-work x64/x86 and actual PC110 product-path measurements; all fourteen artifact identities and full-test disposition. |
| Asset Needs | Existing external firmware with private copied CMOS/media, never owner inputs. |
| Reporting Requirements | Actual per-scope diff and review, test and artifact evidence, measured limits and desktop/device gaps. |
| Stop Conditions | Do not expose or retain indices outside one synchronous resolution; preserve raw and alias precedence, query ordering, observational contract, all 64 slots and failures. Reject if correctness or measured value cannot be shown. |
| Exit Criteria | Candidate and actual diff reviewed, full Shared receiving gate complete and pushed; report bounded improvement without declaring T582 or PC110 usability complete. |
| Original Owner Request | Improve PC110 performance to usability while preserving timing, correctness and one execution owner. |
| Similar-Issue Sweep | Test zero/64 providers, overlapping same- and mixed-tier windows, declined/failed queries, raw and CPU-only routes, dynamic alias/A20 changes and both access directions. |

S14 Shared implementation is `e92409c90`; [qualification and artifact evidence](../etc/evidence/t582-s14-memory-routing.md) records dual-width tests and receiving artifacts. This is a bounded performance improvement; T582 remains open because PC110 usability is not established. Product-scoped receiver commits and qualification record were published through `cfacbaed5`. S14 is closed; the remaining task is PC110 usability.

# M5 T582 S13 — Closed: sustained execution bottleneck audit

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T582 S13 after S12 delivery. |
| Admission And Approval | Owner authorized continued performance repair and autonomous numeric S sequencing. |
| Objective | Identify a larger correctness-preserving performance opportunity for the actual PC110 startup and Personaware workload. |
| Non-goals | No Shared or App implementation changes, host-time substitution, guest work skipping, second executor, owner media mutation or unverified usability claim. |
| Reference Baseline | Shared 2003fef34, NXVM 2f2795611, MyRaidenII f7543ec23; [S12 evidence](../etc/evidence/t582-s12-preview-borrow.md). |
| Candidate Proposal | [T582 proposal](../proposals/m5-pc110-startup-clock-performance.md), S13. |
| Files And ABI Surface | Task-owned probes and NXVM evidence/status only; no ABI change. |
| Applicable Rules | NXVM guide, Execution, Documentation, Architecture, Coding, architecture/layout and source policy. |
| Verification | Compare early startup and later Personaware workload using existing fixed-work and sampled guest-clock observations; isolate proposed changes and compare both architectures before admitting a repair. |
| Expected Markers | Actual guest ticks, retired instructions, selected hotspots, bounds on measurement and explicit candidate correctness constraints. |
| Asset Needs | Existing firmware and isolated copied CMOS/media only. |
| Reporting Requirements | Evidence, measured overhead, estimated scope, retained timing/debug semantics and untested interactive boundaries. |
| Stop Conditions | Do not claim that nominal tick rate or a limited fixed-work improvement proves usable PC110 performance. |
| Exit Criteria | A measured, correctness-preserving next design or concrete negative evidence and continued investigation; T remains open pending actual usability. |
| Original Owner Request | Improve PC110 performance until usable while preserving timing and correctness. |
| Similar-Issue Sweep | Compare both host widths, startup versus later workload, and guest-time costs versus host waiting. |

S13 measured the bounded local provider-list candidate in [scheduler evidence](../etc/evidence/t582-s13-scheduler-observation.md), including dual-width fixed-work and x64 product-path observations. [CPU evidence](../etc/evidence/t582-s13-cpu-profile.md) records other investigated candidates. S13 closes as an audit; it did not modify Shared or publish executables. S14 receives the measured route candidate. PC110 usability remains unproven.

# M5 T582 S12 — Closed: bounded lexical-preview optimization

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T582 S12 after the S11 observation and qualification transfer. |
| Admission And Approval | Owner authorized continued performance repair, Shared X86 Core updates and autonomous numeric S sequencing; all seven App receivers are in scope for qualification. |
| Objective | Remove the measured redundant 486 cache/TLB copy from target-lexeme preview while retaining the original CPU-owned read state and guest semantics. |
| Non-goals | No host-time substitution, second executor, skipped guest work, global state, diagnostics-pointer reuse, mirrored cache, public API change, owner media mutation, or claim that this alone achieves usability. |
| Reference Baseline | a2ee593e2 and [S11 measurements](../etc/evidence/t582-s11-performance-observation.md). |
| Candidate Proposal | [T582 performance proposal](../proposals/m5-pc110-startup-clock-performance.md), S12; S11 final receiving qualification is transferred to S13 or the next suitable S. |
| Files And ABI Surface | Shared src/x86-core/cpu private implementation and corresponding test/x86-core fixtures and manifests; NXVM evidence; receiving binaries for NXVM, My5160, My5170, MyDeskPro386, MyPC110, MyRaidenII and MyNES x86/x64. No public ABI change. |
| Applicable Rules | NXVM guide, Execution, Documentation, Architecture, Coding, architecture/layout and source policy; single CPU owner, nonreentrant callbacks and temporary read-only borrow. |
| Verification | Existing x86/x64 cache, TLB, debug and preview tests plus minimal borrow/lifetime regression; cached full dual-architecture builds, unit and integration qualification and fourteen receiving artifacts for Shared changes; matched PC110 fixed-work and representative guest progress. |
| Expected Markers | Equal architectural, preview, cache/TLB and failure observations; no mutation through borrowed cache; measured performance reported with uncertainty. |
| Asset Needs | Existing external firmware and isolated copied CMOS/media; no owner input modification. |
| Reporting Requirements | Actual per-scope diff, retained owner, cache/preview invariants, measured x86/x64 results, fourteen binary identities, full-test results and any unverified desktop/device boundaries. |
| Stop Conditions | Do not use diagnostic context as storage; do not borrow from mutable foreign threads, expose private state, retain a borrow beyond the synchronous preview, or change debug patch preflight. If equivalence cannot be established, abandon the production change. |
| Exit Criteria | Actual-change review and all required qualification complete; no claim that T582 is complete until usable PC110 performance is directly established and accepted. |
| Original Owner Request | Improve PC110 performance to usability while preserving guest timing, correctness and a single execution owner. |
| Similar-Issue Sweep | Lexical preview versus debug patch preview, nested/reentrant callbacks, TLB hits and misses, cache hits/misses, SMM, exceptions, self-modifying code and all supported CPU profiles. |

S12 implementation and receiving evidence are recorded in
[S12](../etc/evidence/t582-s12-preview-borrow.md). Actual Shared source and
test changes, product artifact identities and qualification were reviewed;
Shared `2003fef34`, NXVM `2f2795611` and MyRaidenII `f7543ec23` are the
product-scoped deliveries. The measured improvement is bounded and the
original usable-performance goal remains open.

# M5 T582 S11 — Deferred: PC110 performance receiving qualification

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T582 S11 after S10 delivery. |
| Admission And Approval | Owner authorized continuous performance work and autonomous numeric S sequencing. |
| Objective | Measure the delivered PC110 runtime against retained baseline evidence and qualify the current receiving state. |
| Non-goals | No Shared or product implementation change, host-time shortcut, owner media mutation or unverified usability claim. |
| Reference Baseline | T582 S1/S6 observations and S10 delivery d021cd925. |
| Candidate Proposal | [T582 performance proposal](../proposals/m5-pc110-startup-clock-performance.md), S11. |
| Files And ABI Surface | Task-owned probes and NXVM evidence/status only; no ABI change. |
| Applicable Rules | NXVM guide, Execution, Documentation, Architecture, Coding, architecture/layout and source policy. |
| Verification | Compare representative startup and clock evidence, x64/x86 and display variants where supported; reuse the just-completed cached full qualification and product identities. |
| Expected Markers | Explicit measured guest progress, RTC/clock evidence, display mode coverage and limitations. |
| Asset Needs | Existing firmware and private media/CMOS copies, never owner inputs. |
| Reporting Requirements | Sources, timings, measurement uncertainty, precise verification and untested interactive boundaries. |
| Stop Conditions | Do not imply real-time performance, correct gameplay or matching UX without direct evidence. |
| Exit Criteria | Receiving qualification evidence and truthful assessment; T remains open pending owner acceptance and all original objectives. |
| Original Owner Request | Improve PC110 performance until usable, preserving timing and correctness. |
| Similar-Issue Sweep | Compare both host widths, DSTN/TFT, guest clock and actual product-path observations. |
# M5 T582 S10 — Closed: PIT deadline aggregation

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T582 S10 after S9 audit. |
| Admission And Approval | Owner authorized performance repair and autonomous numeric S sequencing. |
| Objective | Eliminate redundant clock conversions when selecting the earliest deadline across one PIT's three channels. |
| Non-goals | No guest timing or PIT behavior change, extra state or API, Lib/Emulator change, or owner input mutation. |
| Reference Baseline | Shared 67807dbed and [S9 evidence](../etc/evidence/t582-s9-execution-observation.md). |
| Candidate Proposal | [T582 proposal](../proposals/m5-pc110-startup-clock-performance.md), S9 measured candidate and synthetic boundary comparison. |
| Files And ABI Surface | X86-NXVM board-base/board_deadline.c, existing PIT/board regression as needed, corresponding manifests; approximately +8/-4 production lines. |
| Applicable Rules | NXVM guide, Execution, Documentation, Architecture, Coding, architecture/layout and source policy. |
| Verification | Existing PIT/deadline tests; zero, maximum, overflow, phase and multiple-channel equivalence; cached full dual-width unit/integration and fourteen EXEs for Shared changes. |
| Expected Markers | Same minimum deadline and immediate semantics, measured bounded performance improvement. |
| Asset Needs | Existing firmware and isolated copied CMOS/media. |
| Reporting Requirements | Actual diff, test results, artifact hashes and interactive limitations. |
| Stop Conditions | Never lose UINT64_MAX or immediate events; do not mask conversion errors or change per-PIT clock domain. |
| Exit Criteria | Actual Shared diff reviewed; x64/x86 cached qualification, fourteen receiving artifacts and selected-set MyRaidenII checks passed. Shared 96eab1131, NXVM f89764af4 and MyRaidenII ebb203dce were pushed. T582 remains open until PC110 usability is verified. |
| Original Owner Request | Improve PC110 performance and eliminate redundancy until usable. |
| Similar-Issue Sweep | Both primary and auxiliary PIT domains, invalid channels, conversion overflow and previous nearer deadline. |

# M5 T582 S9 — Closed: execution observation and scheduling audit

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T582 S9 after S8 delivery and review. |
| Admission And Approval | Owner authorized continued performance work and autonomous numeric S sequencing. |
| Objective | Measure per-instruction CPU observation and scheduler costs and identify a larger equivalent optimization for PC110. |
| Non-goals | No skipped instructions, fault rollback, timing or debug semantics; no new mirrored state, guest-clock substitution, public API or owner media changes. |
| Reference Baseline | Shared 67807dbed, NXVM 5dd25ead4, MyRaidenII f507c8b97; S7 sampled execution profile. |
| Candidate Proposal | [T582 proposal](../proposals/m5-pc110-startup-clock-performance.md) and [S7 evidence](../etc/evidence/t582-s7-execution-profile.md). |
| Files And ABI Surface | Initially read-only X86 Core CPU and Machine; task-owned probes in build/t582. Record design and estimate before any implementation. |
| Applicable Rules | NXVM guide, Execution, Documentation, Architecture, Coding, NXVM architecture/layout and source policy. |
| Verification | Matched Release work, exact guest ticks/PC and observation/debug/interrupt regression. Shared changes require 14 EXEs and full dual-width unit/integration tests. |
| Expected Markers | Evidence-backed major cost and useful equivalent repair or documented negative result. |
| Asset Needs | Existing firmware and isolated copied CMOS/media. |
| Reporting Requirements | Measurement overhead, actual diff, architectural semantics, qualification and realtime limits. |
| Stop Conditions | Preserve completed instruction observation across interrupt entry and all debug, trace, retirement and timing consumers. |
| Exit Criteria | Bounded observation and PIT deadline candidates recorded in S9 evidence; the PIT candidate proceeds to S10 after boundary checks. T582 remains open. |
| Original Owner Request | Improve PC110 performance until usable, maintaining correctness and simplicity. |
| Similar-Issue Sweep | Examine instruction observation, interrupt entry, retirement/preview, scheduler deadlines and no-consumer paths. |

# M5 T582 S8 — Closed: keyboard idle deadline repair

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T582 S8 after S7 measured audit. |
| Admission And Approval | Owner authorized continuous performance repair and automatic numeric S sequencing. |
| Objective | Eliminate redundant keyboard repeat deadline queries when the existing signal proves no repeat is active. |
| Non-goals | No altered input, repeat, BAT or guest timing semantics; no new state, public API, Lib/Emulator change or user-media mutation. |
| Reference Baseline | Shared 27ad35459, NXVM 6b01ffd3e; [S7 evidence](../etc/evidence/t582-s7-execution-profile.md). |
| Candidate Proposal | [T582 plan](../proposals/m5-pc110-startup-clock-performance.md) and measured S7 KBC prototype. |
| Files And ABI Surface | X86-NXVM board-at/kbc.c, existing keyboard/KBC tests if necessary, corresponding manifests; estimated +2/-1 production lines; no ABI change. |
| Applicable Rules | NXVM guide, Execution, Documentation, Architecture, Coding, architecture/layout and source policy. |
| Verification | Existing keyboard/KBC tests for inactive/active repeat, scanning, BAT and FIFO; cached full unit/integration x64/x86 and fourteen receiver artifacts for Shared changes. |
| Expected Markers | Same guest timing and input behavior with measured bounded startup work reduction. |
| Asset Needs | Existing approved firmware, private CMOS/media copies and overlays. |
| Reporting Requirements | Actual source/test diff, qualification, artifact hashes, measurement uncertainty and playability limits. |
| Stop Conditions | No reduction of repeat correctness or ignored keyboard startup transitions. |
| Exit Criteria | Coordinator reviewed Shared 67807dbed, NXVM 5dd25ead4 and MyRaidenII f507c8b97; required delivery passed. T582 remains open until usability is proven. |
| Original Owner Request | Improve PC110 performance and eliminate redundant calculations until usable. |
| Similar-Issue Sweep | Repeat signal/deadline behavior across enabled, disabled, empty, active and pending BAT/FIFO paths. |

# M5 T582 S7 — Closed: current PC110 execution profile

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T582 S7 after S6 evidence delivery and coordinator review. |
| Admission And Approval | Owner authorized continuous PC110 performance repair and autonomous numeric S sequencing. |
| Objective | Identify the largest current early and late execution costs after landed DMA and memory optimizations. |
| Non-goals | No production code, guest timing, media or product artifact changes in this audit. |
| Reference Baseline | Shared 27ad35459; current PC 0.5.0582; S6 evidence 347742699. |
| Candidate Proposal | [T582 plan](../proposals/m5-pc110-startup-clock-performance.md); [S6 evidence](../etc/evidence/t582-s6-display-observation.md). |
| Files And ABI Surface | Task-owned profiling and NXVM evidence only; examine actual X86 Core, X86-NXVM and App paths. No API change. |
| Applicable Rules | NXVM guide, Execution, Documentation, Architecture, Coding and source policy. |
| Verification | Bounded current-build guest samples with private CMOS/media; distinguish actual worker samples, instrumentation limitations and task ownership. |
| Expected Markers | Ranked actual hot functions, reproducible workload, a measured next candidate or explicit negative result. |
| Asset Needs | Existing approved firmware, task-owned copied CMOS and overlay media; preserve owner inputs. |
| Reporting Requirements | Source identity, sampling windows, incomplete sample limitations, stop budget and concrete next design. |
| Stop Conditions | Do not infer CPU-time percentages from mixed-thread samples or mutate guest behavior. |
| Exit Criteria | Post-S4 sample and bounded PIC/KBC probes are recorded in S7 evidence; PIC result is inconclusive and KBC candidate advances to S8. PC110 playability remains unqualified. |
| Original Owner Request | Optimize PC110 performance and eliminate redundancy until usable. |
| Similar-Issue Sweep | Compare CPU, scheduler, memory, DMA, video and host pacing against post-S4 baseline. |

# M5 T582 S6 — Closed: PC110 display observation design and proof

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T582 S6 after S5 measured audit. |
| Admission And Approval | Owner authorized continued PC110 performance work and automatic numeric S sequencing. |
| Objective | Prove and implement a single-owner display observation that avoids unnecessary PC110 frame capture while preserving visible behavior. |
| Non-goals | No Lib/Emulator change, incomplete snapshots, skipped cursor phases, guest-time substitution, second display path or owner-media mutation. |
| Reference Baseline | Shared 27ad35459 and NXVM 6b01ffd3e; S5 read-only display probe. |
| Candidate Proposal | [S5 measured evidence and provisional design](../etc/evidence/t582-s5-cpu-timing-performance.md); [T582 proposal](../proposals/m5-pc110-startup-clock-performance.md). |
| Files And ABI Surface | Design candidate extends existing X86-NXVM provider binding and F65535 observation plus PC110 binding, estimated +75/-15 production and +70 test lines. No Lib/Emulator. Prototype and verify before implementation. |
| Applicable Rules | NXVM guide, Execution, Documentation, Architecture, Coding, NXVM architecture/layout and source policy. |
| Verification | Force, first frame, unchanged frame, cursor blink, reset, wrap and error paths; matched guest performance; cached dual-width full unit/integration and fourteen EXEs for Shared changes. |
| Expected Markers | Preserve complete snapshot and guest display semantics; fewer captures and meaningful observed guest performance. |
| Asset Needs | Existing firmware, isolated copied CMOS/media and overlays; do not modify owner inputs. |
| Reporting Requirements | Exact interface and diff, measurements, tests, artifact hashes and native/RDP limits. |
| Stop Conditions | Do not use standard VADP generation to suppress F65535 changes; never acknowledge failed capture or skip guest cursor phase. |
| Exit Criteria | Coordinator reviewed pushed 347742699: the exploratory bypass has only 2.7% one-pair x64 product progress, no proven early-work gain, and lacks force semantics; no Shared change was landed. T582 playability stays open. |
| Original Owner Request | Optimize PC110 hotspots and redundant calculations until usable. |
| Similar-Issue Sweep | Standard VADP and custom F65535, lifecycle/reset, invalid/forced capture and all cursor states. |

# M5 T582 S5 — Closed: CPU timing computation audit

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T582 S5 after S4 delivery and coordinator review. |
| Admission And Approval | Owner authorized continued PC110 performance work and autonomous numeric S sequencing. |
| Objective | Measure repeated CPU instruction timing and prefix decoding work on representative PC110 execution and identify a safe, useful optimization. |
| Non-goals | No guest timing formula changes, skipped instructions, persistent mirrored decode state, new public API, Lib/Emulator changes or owner-input mutation. |
| Reference Baseline | Shared 27ad35459, NXVM 6b01ffd3e, MyRaidenII 7636e914b. |
| Candidate Proposal | [T582 performance proposal](../proposals/m5-pc110-startup-clock-performance.md); [S3 CPU candidate](../etc/evidence/t582-s3-execution-performance.md). |
| Files And ABI Surface | Initially read-only X86 Core CPU timing and decoder; task-owned probes in build/t582. Record concrete design and diff estimate before edits. |
| Applicable Rules | NXVM guide, Execution, Documentation, Architecture, Coding, NXVM architecture/layout and source policy. |
| Verification | Measure matched Release fixed-work on both widths, preserve timing results and Intel/V30 semantics; Shared edits require cached full dual-width unit/integration and fourteen artifacts. |
| Expected Markers | Reproducible hotspot evidence and either a proven useful equivalent repair or a negative result and next owner. |
| Asset Needs | Existing firmware and task-owned copied CMOS/media with overlays; preserve owner inputs. |
| Reporting Requirements | Counts, measurement overhead, actual diff, coverage and remaining interactive/RDP limits. |
| Stop Conditions | Do not conflate prefix semantics across profiles, runtime/preview or emulation modes; no timing accuracy downgrade. |
| Exit Criteria | CPU prefix candidate produced conflicting x64/x86 results and was not landed; audit identified repeated unchanged F65535 captures. Evidence recorded; T582 remains open pending actual playability. |
| Original Owner Request | Improve PC110 performance by eliminating evidenced redundant work until usable. |
| Similar-Issue Sweep | Examine shared prefix count callers, nested timing calculations and other CPU profiles without duplicating test suites. |

# M5 T582 S4 — Closed: DMA arbitration performance audit

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T582 S4 after S3 delivery and coordinator review. |
| Admission And Approval | Owner authorized continued PC110 performance work and autonomous S sequencing. |
| Objective | Measure board DMA arbitration in the actual PC110 workload and determine whether a safe idle fast path produces a useful improvement. |
| Non-goals | No guest timing changes, skipped transfers or cascade updates, new persistent state, public API, Lib/Emulator changes or user-media mutations. |
| Reference Baseline | Shared 1f8a4d477, NXVM f0907772f, MyRaidenII 825ac6b82; PC 0.5.0582. |
| Candidate Proposal | [T582 performance proposal](../proposals/m5-pc110-startup-clock-performance.md) and [S3 evidence](../etc/evidence/t582-s3-execution-performance.md). |
| Files And ABI Surface | Measured candidate: X86-NXVM board-base/dma_bus.c and existing DMA channel regression, plus respective manifests; approximately +8 production lines. No new ABI or state. [Evidence](../etc/evidence/t582-s4-dma-performance.md). |
| Applicable Rules | NXVM guide, Execution, Documentation, Architecture, Coding, NXVM architecture/layout and source policy. |
| Verification | Matched Release fixed-work comparison on x64/x86; cascade, active channels, request transitions and transfer regression. Any Shared change requires cached 14-EXE build and full dual-width unit/integration qualification. |
| Expected Markers | Equivalent guest ticks and transfer behavior, measured host-time change, and a justified keep-or-reject decision. |
| Asset Needs | Existing approved firmware and isolated private CMOS/media; preserve owner files. |
| Reporting Requirements | Report actual hotspot evidence, measurement limits, estimated and actual diff, verification and remaining playability gap. |
| Stop Conditions | Do not infer idle from the gated pending-request query; do not skip cascade propagation, active primary transfers, callbacks or guest timing. |
| Exit Criteria | Coordinator reviewed pushed Shared 27ad35459, NXVM 6b01ffd3e and MyRaidenII 7636e914b; full qualification passed. T582 remains open until playability is proven. |
| Original Owner Request | Improve PC110 performance and eliminate redundant work until usable. |
| Similar-Issue Sweep | Review single/dual DMA controller, primary/secondary active/request, disabled/masked cascade, and request-changing callbacks. |

# M5 T582 S3 — Closed: execution and memory-route costs

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T582 S3 after S2 closure 840094bdc. |
| Admission And Approval | Owner authorized continued performance work and automatic numeric S sequencing; S2 proves a substantial whole-product gap remains. |
| Objective | Measure CPU and physical-memory routing costs, remove evidenced redundant work at its owner, and demonstrate equivalent results with improved throughput. |
| Non-goals | No guest-time reduction, skipped device effects, firmware-address shortcut, second executor, speculative public API, Lib/Emulator change or owner-input mutation. |
| Reference Baseline | Shared d8a94dc61, receiver delivery 7c7a98a13/0d48682cf, S2 closure 840094bdc; PC 0.5.0582. |
| Candidate Proposal | [T582 plan](../proposals/m5-pc110-startup-clock-performance.md); [S1 audit](../etc/evidence/t582-s1-performance-audit.md); [S2 measured delivery](../etc/evidence/t582-s2-planar-performance.md). |
| Files And ABI Surface | Measured design admitted: X86 Core machine/memory.c and machine.c, existing memory-device-registration test and two manifests; [design and fixed-work evidence](../etc/evidence/t582-s3-execution-performance.md). Estimated production +30/-8, test about +35. No new ABI, persistent state, Lib/Emulator change or guest timing change. |
| Applicable Rules | NXVM guide, CONTRIBUTING, Execution, Documentation, Architecture and Coding; NXVM architecture/layout, UX/source policy; current proposal. |
| Verification | Fixed-work or matched-checkpoint baseline/candidate measurements, route-boundary semantics and existing regression reuse. Any Shared implementation must receive cached full x86/x64 unit/integration tests and all fourteen EXEs before closure. |
| Expected Markers | Named measured cost; unchanged guest results, address routing and timing; meaningful improvement rather than a source-only smell claim. |
| Asset Needs | Existing approved ROM and private CMOS/media copies; preserve all owner files. |
| Reporting Requirements | Counts, measurement overhead, actual diff, discarded candidates, delivered source/artifact identities and native/RDP limits. |
| Stop Conditions | No unproven fast path across narrow providers, A20, aliases, parity or observers; no private live cross-thread reads or false full-qualification claim. |
| Exit Criteria | Implementation and actual-diff review complete; matched Release 30-million-instruction runs reduce host time 4.57% x64 and 6.61% x86 at identical guest ticks/PC. Full dual-width unit/integration qualification and 14-artifact publication complete. Shared 1f8a4d477, NXVM f0907772f and MyRaidenII 825ac6b82 are pushed; T582 playability remains open. |
| Original Owner Request | Optimize PC110 hotspots and redundant calculations until usable; keep Shared receiving qualification complete. |
| Similar-Issue Sweep | Read/write/fetch/cache queries, raw/reset and decoded routes, CPU profiles, timing-result consumers and all relevant boundary regressions. |

# M5 T582 S2 — Closed: equivalent F65535 planar expansion

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T582 S2 after S1 coordinator audit closure. |
| Admission And Approval | Owner admitted performance repair and automatic S sequencing; S1 actual source review and dual-width fixed-work evidence select this bounded implementation. |
| Objective | Reduce redundant F65535 planar pixel work without changing any guest or snapshot result; measure product impact and qualify receivers. |
| Non-goals | No timing downgrade, new cache/state/API, incomplete snapshot, firmware shortcut, Lib/Emulator change or user file edit; no whole-product performance claim from a microbenchmark. |
| Reference Baseline | 990266ae0 S1 closure; production baseline 59ad6e78b, products 0.5.0581. |
| Candidate Proposal | [T582 plan](../proposals/m5-pc110-startup-clock-performance.md), [S1 evidence and exact design](../etc/evidence/t582-s1-performance-audit.md); [S2 implementation and qualification](../etc/evidence/t582-s2-planar-performance.md). |
| Files And ABI Surface | Shared: F65535 planar loop, existing chip test, source/test manifests; necessary PC version/preset synchronization; root Console test fixture restores detached standard handles and verifies child exit status after a reproduced x86 qualification crash. NXVM/MyNES/MyRaidenII: required receiver EXEs and own delivery records. No public ABI change. |
| Applicable Rules | NXVM guide, CONTRIBUTING, Execution, Documentation, Architecture, Coding; NXVM architecture/layout, UX/source policy; S1 actual-change design. |
| Verification | Existing chip regression plus complete-snapshot comparison on padded stride/nonzero start; cached x64/x86 Release builds and publication of all seven Apps (14 EXEs); full-repository unit/integration tests on both widths; manifest, dependency, artifact and documentation gates. Record desktop/device limitations honestly. |
| Expected Markers | Identical output with fewer per-pixel calculations; measured fixed-work improvement, actual guest path intact, all required deployed hashes and PE widths. |
| Asset Needs | Existing approved external ROM/media; isolated measurement copies only. No new inputs or modified owner INI/CMOS/media. |
| Reporting Requirements | Per-owner actual diff, timings and their scope, tests, 14 artifacts, failures/unverified gates and remaining T582 performance gaps. |
| Stop Conditions | No changed pixel/flag/palette/stride semantics, artificial guest speedup, false test pass or omitted receiver. Resolve in-scope defects; report genuine unavailable external prerequisites. |
| Exit Criteria | Coordinator closed S2 after actual-diff review, complete dual-width qualification and 14-artifact delivery: Shared d8a94dc61, NXVM 7c7a98a13, MyRaidenII 0d48682cf, all pushed. Local improvement is proven; T582 remains open for whole-product playability and further proven hotspots. |
| Original Owner Request | Optimize PC110 performance and redundant work; any Shared update requires cached 14-EXE publication and cached full tests. |
| Similar-Issue Sweep | Text/cursor path unchanged; all planar dimensions, plane bits, palette/start/stride and first/repeated capture; check other receiver coupling and existing test overlap. |

# M5 T582 S1 — Closed: measured planar repair admitted next

| Field | Required record |
| --- | --- |
| Identifier Mode | New — M5 T582 S1; next numeric NXVM task after accepted T581. |
| Admission And Approval | Owner closed T581 and explicitly selected performance as the next task on 2026-10-29; former queue position 14 is promoted. |
| Objective | Establish representative current PC110 startup and steady-execution performance, locate evidenced costs, and design the smallest correct repair. |
| Non-goals | No production optimization before evidence, guest-time substitution, timing downgrade, firmware shortcut, unrelated App change or owner media/configuration edit. |
| Reference Baseline | Implementation 59ad6e78b, closure f07c61150, released PC version 0.5.0581; retain T580 S89 and T581 observations with their limits. |
| Candidate Proposal | [Performance proposal](../proposals/m5-pc110-startup-clock-performance.md); S1 baseline/audit, S2 measured repair, final S qualification. |
| Files And ABI Surface | S1 changes NXVM governance/evidence and only necessary task-owned probes; read App, X86-NXVM, X86 Core and neutral runtime paths. No production/public ABI or Shared source/test change is admitted by S1. |
| Applicable Rules | NXVM guide; CONTRIBUTING; shared Execution, Documentation, Architecture and Coding; NXVM architecture, source layout, UX, roadmap and source/research policy; prior S89 and S90 evidence. |
| Verification | Reuse stable cached builds; bounded sequential dual-width measurements with fixed private inputs and existing safe observation APIs; distinguish host time, Core ticks, RTC, PIT/DOS and frame/guest checkpoints. Documentation gate and actual audit review. Any later admitted Shared update triggers cached Release publication of all seven Apps x two architectures (14 EXEs) and cached full-repository unit/integration tests on both widths, per the current Execution rule and owner instruction. |
| Expected Markers | Reproducible workload and timings, owner-attributed bottleneck evidence, unchanged input hashes, justified repair plan and diff estimate without guessed gains. |
| Asset Needs | Existing external admitted BIOS/media; private configured CMOS and overlay/copied disks. No new downloads, ROM changes or owner input writes. |
| Reporting Requirements | Report measured versus inferred results, sampling overhead, phase boundaries, proposed owner/files/diff and unverified desktop/RDP behavior. |
| Stop Conditions | Do not turn failed/unfinished sampling into success, use unsafe live private reads, mutate user inputs, or implement unreviewed Shared contracts. Unproved hotspot means more bounded investigation, not a fabricated fix. |
| Exit Criteria | Baseline and actual hot-path audit support a concrete repair design or accurately identify the remaining measurement gap; S1 does not claim the performance problem fixed. |
| Original Owner Request | Close T581 and admit the next priority T to address performance. |
| Similar-Issue Sweep | Compare DSTN/TFT, x86/x64, early firmware/late guest, Machine versus UI, pacing versus execution/device work, and prior rejected optimization candidates. |

# M5 T581 — Closed after owner acceptance

| Field | Required record |
| --- | --- |
| Identifier Mode | New — M5 T581 S1. |
| Admission And Approval | Owner admitted the first queued task on 2026-10-29 and authorized autonomous execution. |
| Objective | Determine and implement a safe stopped-state PC110 display/firmware selection and one executable per architecture. |
| Non-goals | No runtime switching while running or paused, no user configuration or media destruction, no unnecessary changes to unrelated Apps or shared Lib/Emulator. |
| Reference Baseline | T580 S90 accepted at d2ac7cf0b; existing four PC110 artifacts and fixed variant bindings. |
| Candidate Proposal | [PC110 selectable display](../proposals/m5-pc110-display-variant-selection.md); [S1 design evidence](../etc/evidence/t581-s1-vdisplay-design.md) records media, worker and firmware ownership; implement the smallest correct route. |
| Files And ABI Surface | NXVM and Shared targets: PC110 App and tests; X86-NXVM profile pre-reset, CMOS restore and Product context/extension hooks with tests/manifests. Lib/Emulator are unchanged. Rebuild all five PC products at their common 0.5.0581 version; MyNES/MyRaidenII are not receivers. |
| Applicable Rules | NXVM guide, execution, architecture, coding, documentation, UX and source/research policy. |
| Verification | Cached dual-width builds and ten deployed PC EXEs; 712/712 units per width; nine targeted integrations and three real-firmware integrations per width; three other PC entry receivers per width; actual DSTN-to-TFT guest boots reach Personaware on both widths. Native Window/RDP remains unverified; see S1 evidence. |
| Expected Markers | DSTN default; vdisplay selects matching firmware and display only when stopped; two binaries with both BYOB firmware inputs. |
| Asset Needs | Existing external BIOS and font inputs; isolated media and CMOS copies; preserve owner files. |
| Reporting Requirements | Report actual owner boundaries, failure/rollback behavior, diff, tests, artifact identity, and desktop/RDP limitations. |
| Stop Conditions | No live firmware mutation, partial selection, fake fallback, unintended media writes, or unrelated product changes. |
| Exit Criteria | Owner accepted S1 and closed T581 on 2026-10-29. Shared 8031ee3e3 and NXVM 59ad6e78b are pushed; all delivery criteria accepted with the recorded native desktop/RDP and empty-CMOS boundaries. |
| Original Owner Request | Default DSTN, vdisplay=dstn/tft while stopped, and only x86/x64 executables. |
| Similar-Issue Sweep | Compare both variants, lifecycle transitions, firmware/CMOS ownership, and all existing product extension callers. |
# M5 T580 — Closed by owner direction

| Task | Retained progress |
| --- | --- |
| T580 | Owner closed T580 on 2026-10-29, transferring its five remaining qualification areas to the end of the NXVM queue. S90 was accepted. Shared `0904691e5` and NXVM `ce0353490` deliver the F65535 cursor and PC110 L1 startup compatibility repairs. Both architectures passed 713 unit cases and targeted firmware/product integrations; configured TFT and derived DSTN reached Personaware without input. [Evidence](../etc/evidence/t580-s90-acceptance-failure.md) preserves model and test limits. Empty DSTN CMOS requires initial configuration; startup throughput and native desktop/RDP qualification are transferred without a full-qualification claim. Earlier S records below are historical. |

# M5 T580 S89 — Accepted: PC110 guest clock-rate investigation

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T580 S89. |
| Admission And Approval | Owner requested autonomous completion of PC110; S88 retained the observed slow guest clock for investigation. |
| Objective | Identify whether the observed slow World Clock originates in RTC, PIT/guest timing, CPU throughput or presentation, using bounded evidence. |
| Non-goals | No unapproved Shared changes, host clock substitution, user-media writes or foreground execution. |
| Reference Baseline | S87 released-product screenshots; S88 RTC persistence and clock ownership evidence. |
| Candidate Proposal | Use existing firmware, product and machine observation paths with task-owned data; measure guest time against monotonic elapsed host time and identify the first supported cause before proposing changes. |
| Files And ABI Surface | PC110 App, existing diagnostics/tests and NXVM evidence; Shared implementation or interface changes require established authorization. |
| Applicable Rules | NXVM guide, execution, architecture, coding, documentation and source/research policies. |
| Verification | Bounded actual-machine timing observation and targeted dual-width regressions for any admitted fix; retain baseline and limits. |
| Expected Markers | Measured guest-versus-host progression, named owner, and justified correction or documented boundary. |
| Asset Needs | Existing embedded firmware and task-owned configuration/media copies; preserve original inputs. |
| Reporting Requirements | Distinguish RTC register values, DOS/PIT time, guest displayed time, host time and CPU throughput. |
| Stop Conditions | No fabricated guest state, unapproved Shared changes or unexplained claims of exact hardware timing. |
| Exit Criteria | Observed clock behavior explained and a necessary correction verified, or an evidenced narrow receiver retained without claiming full timing qualification. |
| Original Owner Request | Complete a usable PC110 product without excessive or redundant test runs. |
| Similar-Issue Sweep | Compare stopped and running guest behavior and both PC110 variants without imposing PC110 policy on other profiles. |

The [S89 evidence](../etc/evidence/t580-s89-clock-rate.md) measures both
Machine runners, reuses the late-guest trace and rejects two isolated
optimization candidates. No production performance change or actual World
Clock root cause is established. Late-guest timing and a measured safe
performance correction remain a T580 receiver; the T is not complete.

# M5 T580 S88 — Accepted: PC110 clock and configuration persistence

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T580 S88. |
| Admission And Approval | Owner requested autonomous completion of T580 and approved RTC calendar persistence in X86-NXVM and MyPC110 without changing Lib or compensating host-off time. |
| Objective | Preserve PC110 guest RTC calendar and configuration across normal product stop and process restart. |
| Non-goals | No Lib change, host-off elapsed-time compensation, owner media edits, foreground execution, invented hardware behavior or unapproved Shared changes. |
| Reference Baseline | S72 RTC persistence boundary; S83 product lifecycle; S87 four release guest observations. |
| Candidate Proposal | Trace actual App persistence and RTC ownership; reproduce any concrete gap with isolated state, then design and fix only the established owner. |
| Files And ABI Surface | PC110 App and tests; X86-NXVM RTC, Board, Machine and tests with the approved RTC persistence boundary; no Lib or Emulator changes. |
| Applicable Rules | NXVM guide, execution, architecture, coding, documentation and source/research policies. |
| Verification | Actual saved and restored guest-visible RTC/configuration across process restart, with targeted dual-width tests for any fix. |
| Expected Markers | Evidence-backed persistence contract, reproduced behavior and an appropriate correction or explicit source boundary. |
| Asset Needs | Existing firmware and task-owned configuration/media copies; preserve original inputs. |
| Reporting Requirements | Distinguish configuration RAM, RTC calendar, elapsed host-off behavior and unverified hardware details. |
| Stop Conditions | No guest-state fabrication, user-file edits or unsupported source-grade claims. |
| Exit Criteria | RTC/configuration behavior correctly implemented and verified for the admitted contract, or an evidenced narrow receiver for continuation. |
| Original Owner Request | Complete PC110 implementation and verify usable products. |
| Similar-Issue Sweep | Compare TFT/DSTN and existing shared RTC behavior without imposing PC110 policy on other machines. |

S87 established interactive Personaware on all four published EXEs with
TFT-generated isolated CMOS; its limits are recorded in
[S87 evidence](../etc/evidence/t580-s87-input-path.md). S72 records that
configuration RAM and RTC calendar currently have different persistence.
The [S88 investigation](../etc/evidence/t580-s88-clock-persistence.md)
records the calendar persistence fix, dual-width tests and real Product Debug
port round trip. The owner explicitly excluded elapsed host-off time. Real
guest Setup/date-setting and the observed slow World Clock remain separate
qualification work; S88 does not claim them complete.

# M5 T580 S87 — Accepted: PC110 product guest startup continuation

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T580 S87. |
| Admission And Approval | Owner requested autonomous completion of T580 and continuation of PC110 work. |
| Objective | Trace the released PC110 product from DOS driver startup to an interactive guest or a precisely evidenced defect. |
| Non-goals | No firmware patching, forced Guest state, owner media modification, foreground tasks or unapproved Shared changes. |
| Reference Baseline | S79 real-firmware Guest diagnostic; S86 actual product Setup, F12 and DOS boot observations. |
| Candidate Proposal | Compare Guest input and execution at the product driver boundary with the established real-firmware diagnostic using bounded, isolated background observations; correct only demonstrated defects. |
| Files And ABI Surface | PC110 App, existing tests and task evidence; Shared changes require established approval. |
| Applicable Rules | NXVM guide, execution, architecture, coding, documentation and source/research policies. |
| Verification | Real product route, isolated guest media, input and frame evidence, affected dual-width checks for any production change. |
| Expected Markers | Guest progress to interactive input or a reproducible and localized failure boundary. |
| Asset Needs | Existing approved external firmware and task-owned media copies; preserve owner assets. |
| Reporting Requirements | Distinguish native input delivery, guest consumption, product screen, isolated diagnostic and unresolved limits. |
| Stop Conditions | No fabricated guest state, protected media change or Shared API change without its applicable approval. |
| Exit Criteria | Observed interactive guest or an evidenced narrowly scoped implementation receiver with required checks. |
| Original Owner Request | Complete PC110 implementation and verify usable released products. |
| Similar-Issue Sweep | Compare product and diagnostic input paths and original/TFT variants where evidence warrants. |

S86 established real TFT PC DOS startup but not an interactive desktop. Its
final Guest diagnostic and screenshot are recorded in [S86 evidence](../etc/evidence/t580-s86-product-observation.md).

Five product-path observations and their limits are recorded in
[S87 evidence](../etc/evidence/t580-s87-input-path.md). The fifth reached the
Personaware launcher through Emulator queue calibration; Tab and Enter posted
to the actual Window were followed by World Clock opening. The x64 observation
used scratch instrumentation and did not test native mouse capture or qualify
the released EXE independently. Its owned background processes have exited.
No firmware, user media or Shared source was changed.

The sixth observation ended before F12 because its monitor-prompt handshake
timed out despite a visible pause notification. The seventh reached
Personaware and saved a 128-byte guest CMOS through orderly exit. The eighth
used the byte-identical published TFT x64 EXE with a copy of that CMOS and
reached Personaware; normal Window Tab/Enter was followed by World Clock.
The direct-write task-owned master image changed during that run. Original
inputs and user configuration remained untouched. The observer was stopped
after the successful checkpoint, not counted as a full-budget test.

The ninth and tenth release observations reached Personaware and opened World
Clock through Window keyboard input: TFT x86 and derived-DSTN x64. Both used
fresh task-owned copies of the TFT Guest-generated CMOS and media. The DSTN
result is a cross-variant saved-configuration experiment, not a pristine DSTN
cold-setup proof. Both were deliberately stopped after the checkpoint and
are not full-budget registered integration results.

The eleventh observation reached Personaware and opened World Clock through
Window keyboard input on the published derived-DSTN x86 EXE. It used a copy
of the TFT Guest-generated CMOS, so it is not a pristine DSTN cold-setup
proof. It was deliberately stopped after the checkpoint, not counted as a
full-budget integration pass. Existing capture-mock, PC110 construction and
inking units passed 3/3 per architecture. Native mouse capture remains a
separate proof. The owned background processes have exited.

# M5 T580 S86 — Accepted: PC110 product boot and Setup observation

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T580 S86. |
| Admission And Approval | Owner accepted S85 and requested continued PC110 work. |
| Objective | Trace actual PC110 product execution toward Setup and determine whether its displayed colours match guest framebuffer data. |
| Non-goals | No foreground processes, owner media or configuration edits, firmware patching, or unapproved Shared changes. |
| Reference Baseline | S57 palette evidence, S78/S79 firmware boot, S83 lifecycle, S84/S85 product Window evidence. |
| Candidate Proposal | Use bounded background product and guest-path probes to trace the existing firmware, device and display path; repair confirmed in-scope defects. |
| Files And ABI Surface | PC110 App and tests, task evidence; shared changes require established approval. |
| Applicable Rules | NXVM guide, execution, architecture, coding, documentation and source/research policies. |
| Verification | Actual product path, isolated guest state, matching framebuffer and Window observations, applicable dual-width tests. |
| Expected Markers | Identifiable startup progression, Setup display and guest colour evidence or a precisely localized defect. |
| Asset Needs | Existing approved external ROM and task-owned temporary configuration/media; preserve owner inputs. |
| Reporting Requirements | Separate actual guest, frame, Window, RDP and desktop evidence and unresolved gaps. |
| Stop Conditions | No forced guest state, protected media modification or unsupported precision claims. |
| Exit Criteria | Confirmed product behaviour and colour path with required verification, or an evidenced narrow receiver for continuation. |
| Original Owner Request | Complete PC110 implementation and investigate unexpected blue Setup/Error pages. |
| Similar-Issue Sweep | Compare original and TFT variants, guest output and shared display boundaries. |

An initial isolated TFT x64 observation ended on a black Window while the
firmware was still progressing. [S86 evidence](../etc/evidence/t580-s86-product-observation.md)
records the longer observation. A scratch-instrumented TFT product reached the
guest 640x480 ERROR
and date/time pages in the actual RDP Window. Its background pixel matches the
earlier pink guest-frame value; normal F12 and Enter input progressed the guest.
The released derived-DSTN product also reached both pages through ordinary
F12 and Enter input. The connected RDP client showed pink, matching the Guest
frame. A released TFT product subsequently displayed PC DOS driver startup;
its final driver screen and Guest state transfer to S87. Full boot and
physical-panel qualification are not claimed.

# M5 T580 S85 — Accepted: initial Window scale correction

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

# M5 T580 — Closed by owner direction after S73

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

The [proposal](../history/M5-T580-pc110-qualification-proposal.md) and
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
