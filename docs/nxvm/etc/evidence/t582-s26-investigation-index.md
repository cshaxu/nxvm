# T582 S26 — throughput investigation index

The task's remaining goal is usable PC110 execution, not a count of isolated optimizations. Read this index and the linked evidence before repeating a probe; the active packet remains the task authority.

## Landed changes

- Planar display work: [S2](t582-s2-planar-performance.md).
- CPU timing reuse and plain-RAM routing: [S3](t582-s3-execution-performance.md).
- DMA arbitration, idle keyboard and PIT deadlines: [S4](t582-s4-dma-performance.md), [S8](t582-s8-keyboard-deadline.md), [S10](t582-s10-pit-deadline.md).
- Lexical-preview cache/TLB borrow: [S12](t582-s12-preview-borrow.md).
- Memory-provider route selection: [S14](t582-s14-memory-routing.md), Shared commit `e92409c90`.
- PC110 firmware provider cache: [S18](t582-s18-cache-design.md), Shared commit `8dd8d06d0` and PC110 commit `65c15ab69`.

These are bounded improvements; none establishes real-time performance.

## Already evaluated without adoption

- CPU dispatch and repeated prefix classification: [S11](t582-s11-performance-observation.md), [S13](t582-s13-scheduler-observation.md), [S15](t582-s15-cpu-line-audit.md), [S16](t582-s16-execution-audit.md), [S21](t582-s21-timing-input-audit.md), [S23](t582-s23-dispatch-audit.md). S26's [execution-prefix experiment](t582-s26-execution-prefix-audit.md) repeated S13's per-byte candidate; it should not have been run again without first checking that record.
- 486 short-branch outcome inferred from clocks: [S39 comparison](t582-s39-execution-convergence.md#reusing-486-short-branch-outcome-through-timing-metadata); rejected as fragile and without consistent speedup.
- Initialization and prefetch refill attribution: [S23 observations](t582-s23-dispatch-audit.md#s26-boundary-and-prefetch-attribution).
- CPU LTO, optimization level and PGO: [S11](t582-s11-performance-observation.md), [S13](t582-s13-cpu-profile.md), [S22](t582-s22-cache-copy-audit.md). The S26 [partial LTO](t582-s26-cpu-lto-audit.md), [optimization-level](t582-s26-optimization-level.md) and [PGO](t582-s26-pgo-audit.md) experiments overlap earlier work; differences in compilation scope do not establish an independent product benefit.
- CPU host tuning: [S39 single-pair exploration](t582-s39-execution-convergence.md#cpu-host-tuning-exploration).
- Wider CPU and Machine LTO and Game frame-condition audit:
  [S26 comparison](t582-s26-core-lto.md).
- Fat LTO with native test links and archive-extraction alternative:
  [S34 comparison](t582-s34-fat-lto.md).
- Instruction-timing reset and 486 zero-access candidates:
  [S34 comparison](t582-s34-current-profile.md#instruction-timing-fact-reset).
- Deadline-observation removal upper bound: [S28 experiment](t582-s28-execution-chain.md#deadline-observation-upper-bound).
- Deadline caching, clock conversion, DMA and PIC shortcuts: [S13](t582-s13-scheduler-observation.md), [post-cache mutation boundaries](t582-s26-post-cache-deadline.md), [S26 single-tick](t582-s26-single-tick-audit.md), [unconfigured PIT clock](t582-s26-unused-pit-audit.md), and [PIC selection](t582-s26-pic-select-audit.md). The latter checks interrupt selection, distinct from S13's empty-deadline candidate.
- Repeated refresh request observations in arbitration and the attachment
  contract boundary: [S34 analysis and cost sample](t582-s34-current-profile.md#refresh-request-observation-candidate),
  [S39 startup upper bound](t582-s39-execution-convergence.md#refresh-request-observations-at-the-arbitration-boundary).
- Individual board-advance sampled costs: [S13 observations](t582-s13-scheduler-observation.md#s26-board-advance-attribution).
- Clock advance narrow-division candidate: [S11 observations](t582-s11-performance-observation.md#subsequent-narrow-division-candidate).
- Zero-or-one clock-conversion candidate: [S39 comparison](t582-s39-execution-convergence.md#zero-or-one-clock-conversion-candidate).
- PIT-only zero-tick call guard: [S39 probe](t582-s39-execution-convergence.md#pit-only-zero-tick-call-guard).
- PIC source-mask expression: [S39 probe](t582-s39-execution-convergence.md#pic-source-mask-expression-probe).
- Idle floppy head-unload scan: [S39 dual-width probe](t582-s39-execution-convergence.md#idle-floppy-head-unload-guard).
- Instruction preview and copies: [S11](t582-s11-performance-observation.md), [S22](t582-s22-cache-copy-audit.md), [S26 preview copy](t582-s26-preview-audit.md) and [preview count](t582-s26-preview-cost.md).
- Measured compact preview candidate and receiving status: [S26 compact preview](t582-s26-compact-preview.md).
- CPU parity folding: [S26 parity audit](t582-s26-parity-audit.md).
- Disabled instruction-breakpoint scan: [S39 comparison](t582-s39-execution-convergence.md#disabled-instruction-breakpoint-scan).
- Keyboard signal predicate: [compiled check](t582-s39-execution-convergence.md#keyboard-signal-snapshot-compiled-check).
- Flush-pending guard split: [S39 experiment](t582-s39-execution-convergence.md#flush-pending-guard-split-experiment).
- Port transaction value setter inlining: [S39 probe](t582-s39-execution-convergence.md#port-transaction-value-setter-inlining-probe).
- Empty timeline advancement: [S39 probe](t582-s39-execution-convergence.md#empty-timeline-advance-upper-bound-probe).
- 486 decode-prefix single-scan: [S39 experiment](t582-s39-execution-convergence.md#486-decode-prefix-single-scan-experiment).
- Branch timing condition simplification: [S26 comparison](t582-s26-branch-condition.md).
- Deferred timing-branch next-byte lookup: [S26 comparison](t582-s26-branch-next-byte.md).
- PC110 display adapter aperture validation: [S26 check](t582-s26-video-adapter.md).
- PC110 firmware E/F route-query fast path: [S13 query probe](t582-s13-scheduler-observation.md#pc110-firmware-route-query-probe).
  It previously duplicated query validation and did not improve either width;
  do not repeat it without a materially different workload or implementation.
- Duplicate one-byte opcode-record copy: [S26 copy audit](t582-s26-opcode-copy.md).
- Early startup instruction and STOS write cost: [S39 sampling](t582-s39-execution-convergence.md#early-start-instruction-and-stos-attribution).
- Bus-write component sampling and combined span proof: [S39 probe](t582-s39-execution-convergence.md#startup-bus-write-components-and-combined-span-proof).
- Coarse empty-provider-page index upper bound: [S39 comparison](t582-s39-execution-convergence.md#coarse-empty-provider-page-upper-bound).
- Memory-route end arithmetic consolidation: [S39 qualification](t582-s39-execution-convergence.md#memory-route-end-arithmetic-consolidation).
- Write-buffer empty/drain observation candidates: [S39 comparison](t582-s39-execution-convergence.md#write-buffer-preparation-and-duplicate-observation).
- Memory read-modify-write precheck frequency: [Personaware Game](t582-s26-personaware-game.md#read-modify-write-precheck-frequency).
- Game drawing guest-PC samples and instruction-byte attribution:
  [Personaware Game](t582-s26-personaware-game.md#game-drawing-workload).
- Direct primary-opcode dispatch: [S26 dispatch probe](t582-s26-direct-dispatch.md).
- Memory-route one-pass and SMRAM cache candidates: [S15](t582-s15-cpu-line-audit.md) and [S19](t582-s19-steady-profile.md).
- Combined raw/decoded scan on current routing: [S39 current comparison](t582-s39-execution-convergence.md#combined-raw-and-decoded-provider-scan-on-current-routing).
- Replacement-query placement and two-provider population: [S39 measurement](t582-s39-execution-convergence.md#replacement-provider-scan-population).
- Memory-route address-index upper bound: [S39 measurement](t582-s39-execution-convergence.md#memory-route-address-index-upper-bound).
- 486 cache lookup and line-proof frequency: [S26 cache query](t582-s26-cache-query-cost.md).
- Ordinary cache access and guest-visible test-register publication:
  [S26 cache query](t582-s26-cache-query-cost.md#ordinary-access-test-register-publication-audit).
- Absent-FPU scheduler call removal: [S26 FPU probe](t582-s26-fpu-scheduler-probe.md).
- Correct-layout 486SX absent-FPU guard: [S39 comparison](t582-s39-execution-convergence.md#486sx-absent-fpu-scheduler-candidate).
- Retirement trace disabled-path guard: [S26 trace audit](t582-s26-trace-audit.md).
- Single-owner timing-result write: [S26 timing probe](t582-s26-timing-owner-probe.md).
- 486 timing-term prefix reuse with preserved direct tests: [S26 comparison](t582-s26-486-term-prefix.md).
- Deferring complex-opcode timing fields: [S39 dual-width comparison](t582-s39-execution-convergence.md#deferred-complex-instruction-timing-fields).
- Deferring operand-cache timing fields: [S39 dual-width comparison](t582-s39-execution-convergence.md#deferred-operand-cache-timing-fields).
- Zero-field REP operand timing: [S39 sample and dual-width probe](t582-s39-execution-convergence.md#startup-operand-cache-timing-populations).
- Split nominal timing implementation: [S39 scratch comparison](t582-s39-execution-convergence.md#split-nominal-timing-scratch-candidate).
- 486 access-penalty algebra: [S39 dual-width exploration](t582-s39-execution-convergence.md#486-access-penalty-algebra-candidate).
- 486 PUSH/POP dependency guard: [S34 comparison](t582-s34-current-profile.md#486-decode-dependency-classification); previous-opcode scan guard: [S35 comparison](t582-s35-cost-attribution.md#previous-opcode-scan-experiment). Neither established a cross-width benefit.
- 486 timing-profile branch reorder: [S39 comparison](t582-s39-execution-convergence.md#486-timing-profile-selection-order); neither width showed consistent ordering benefits.
- World Clock sampling of individual 486 timing terms:
  [S26 measurement](t582-s26-486-term-sampling.md).
- Current Game drawing host-symbol samples and repeated zero-access experiment:
  [S39 observation](t582-s39-execution-convergence.md#current-game-host-instruction-samples).
- PC110 Turbo runner quantum comparison and Standard-mode probe correction:
  [S39 comparison](t582-s39-execution-convergence.md#runner-rendezvous-quantum).
- PC110 absent-floppy CMOS versus the original private seed, including a
  complete-menu comparison: [S39 evidence](t582-s39-execution-convergence.md#current-production-cmos-comparison).
- BIOS HLT-wait upper bound and full-menu result:
  [S39 exploration](t582-s39-execution-convergence.md#firmware-hlt-wait-upper-bound-exploration).
- Early-boot Machine-return sampling bias, including actual versus sampled
  `00038000` endpoint counts: [S39 measurement](t582-s39-execution-convergence.md#machine-return-endpoint-sampling-bias).
- Prefix-stripped opcode samples through 100 million startup instructions:
  [S39 middle-phase sample](t582-s39-execution-convergence.md#startup-middle-phase-instruction-samples).
- Forced-inline conditional-jump helper upper bound:
  [S39 comparison](t582-s39-execution-convergence.md#conditional-branch-compiler-upper-bound-probe).
- Scheduler-only pacing-field suppression prototype:
  [S39 comparison](t582-s39-execution-convergence.md#scheduler-only-time-observation-fields).
- Per-address port bucket lookup upper bound and admitted owner-local candidate:
  [S39 comparison](t582-s39-execution-convergence.md#port-bucket-lookup-upper-bound).
- Timing-capture call sites and invalid pending-retirement ablation:
  [S39 measurement](t582-s39-execution-convergence.md#retirement-timing-capture-and-second-loop).
- Existing CPU refresh returning timing to Machine:
  [S39 comparison](t582-s39-execution-convergence.md#returning-timing-with-cpu-refresh).
- Retained-prefetch lexical preview count and fast-path comparison:
  [S39 comparison](t582-s39-execution-convergence.md#lexical-preview-using-an-already-retained-prefetch).
- Contiguous instruction-timing counter reset:
  [S39 comparison](t582-s39-execution-convergence.md#contiguous-instruction-timing-reset).
- GCC CPU no-inline-functions policy:
  [S39 comparison](t582-s39-execution-convergence.md#cpu-compilation-inlining-policy).
- Incremental lexical preview scan: [S28 experiment](t582-s28-execution-chain.md#incremental-lexical-preview-scan).
- Observation-preview ablation and 486 asynchronous regression:
  [S26 comparison](t582-s26-preview-observation-bound.md).
- Lexical decoder prefix collection: [S26 comparison](t582-s26-lexeme-one-pass.md).
- Same-host PC110-EMU reference run: [S26 comparison](t582-s26-pc110-emu-measurement.md).
- Firmware `FF353h` SMI requests and a traced APM CPU-idle service:
  [S24 audit](t582-s24-asmi-audit.md). The S39 Game source counter revisits
  that address without classifying Game request registers; consult S24 first.
- Same synthetic loop on both engines and isolated CPU: [S26 comparison](t582-s26-pc110-emu-measurement.md#same-synthetic-instruction-loop).
- Instruction-entry snapshot copy attribution and GCC string-operation
  experiment: [S26 comparison](t582-s26-pc110-emu-measurement.md#instruction-boundary-copy-attribution).
- Sampled full-size snapshot-copy cost on both widths:
  [S39 measurement](t582-s39-execution-convergence.md#full-size-cpu-snapshot-copy-sampling).
- Small-copy callsite attribution and four-byte RAM-write candidate:
  [S26 comparison](t582-s26-small-copy-attribution.md).
- SMM save/restore transfer count and repeated cache-way lookup boundary:
  [S26 audit](t582-s26-smm-cache-lookup.md).
- Post-retirement asynchronous-entry scope ablation:
  [S26 upper bound](t582-s26-event-scope-upper.md).
- Generic timing-input metadata collection ablation:
  [S26 upper bound](t582-s26-formula-input-upper.md).
- Register-reference read destination check:
  [S26 comparison](t582-s26-reference-read.md).
- ModR/M register reread and width-sensitive reuse:
  [S26 comparison](t582-s26-register-reread.md).
- Constant-work ordinary rotation:
  [S26 implementation and qualification](t582-s26-rotate-constant.md).
- PC110 port read upper-bound probe: a task-only candidate removed the second
  wired-OR provider traversal entirely, which is invalid for general machines.
  PC110 uses ATA rather than the registered Compaq wired-OR route. At the
  identical 10,000,000-instruction / 79,964,294-tick / `0003dbc5` checkpoint,
  x64 baseline/candidate took 21,262/22,946 ms; reverse candidate/baseline took
  22,315/21,064 ms. These sequential runs do not prove general equivalence or
  isolate host variation, but do not justify adding a provider count or index.
  No production route or receiver changed.

## Direction and evidence limits

The current dual-width menu observations are recorded with the
[FDC result and x86 follow-up](t582-s39-execution-convergence.md#current-x86-personaware-menu-observation).
They remain multi-minute startup measurements, not desktop acceptance.

The measured World Clock workload advanced approximately six guest seconds in 60 host seconds ([S19](t582-s19-steady-profile.md)). A separate instrumented sample attributed roughly 65% of Machine-run intervals to CPU refresh and the rest to Machine-side work, scheduling and instrumentation; roughly 29% of sampled refresh intervals was timing selection and observation ([S20](t582-s20-execution-cost.md)). These are approximate inclusive measurements from a specific workload, not exact production percentages.

The configured external-media inventory and the distinction between clock and interactive acceptance are recorded in the [workload audit](t582-s26-workload-audit.md).

The [Game click observation](t582-s26-personaware-game.md#single-click-response-observation)
establishes one successful adjacent-pair action through direct Machine input,
but not whole-game qualification or native Window/RDP input.

Even a hypothetical removal of all measured timing-selection/observation cost would therefore leave most of the measured execution work intact; it cannot by itself explain an order-of-magnitude improvement. The next useful design should investigate the complete retirement-to-device-advance path and its observation boundaries, with representative guest workloads and correctness evidence, rather than assuming isolated instruction micro-optimizations can meet the goal. Guest instructions, device side effects, clocks and asynchronous observation must remain correct under the current packet. No alternative execution path or scheduling optimization is authorized merely by this index.
