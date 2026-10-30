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
- Initialization and prefetch refill attribution: [S23 observations](t582-s23-dispatch-audit.md#s26-boundary-and-prefetch-attribution).
- CPU LTO, optimization level and PGO: [S11](t582-s11-performance-observation.md), [S13](t582-s13-cpu-profile.md), [S22](t582-s22-cache-copy-audit.md). The S26 [partial LTO](t582-s26-cpu-lto-audit.md), [optimization-level](t582-s26-optimization-level.md) and [PGO](t582-s26-pgo-audit.md) experiments overlap earlier work; differences in compilation scope do not establish an independent product benefit.
- Deadline caching, clock conversion, DMA and PIC shortcuts: [S13](t582-s13-scheduler-observation.md), [post-cache mutation boundaries](t582-s26-post-cache-deadline.md), [S26 single-tick](t582-s26-single-tick-audit.md), [unconfigured PIT clock](t582-s26-unused-pit-audit.md), and [PIC selection](t582-s26-pic-select-audit.md). The latter checks interrupt selection, distinct from S13's empty-deadline candidate.
- Individual board-advance sampled costs: [S13 observations](t582-s13-scheduler-observation.md#s26-board-advance-attribution).
- Clock advance narrow-division candidate: [S11 observations](t582-s11-performance-observation.md#subsequent-narrow-division-candidate).
- Instruction preview and copies: [S11](t582-s11-performance-observation.md), [S22](t582-s22-cache-copy-audit.md), [S26 preview copy](t582-s26-preview-audit.md) and [preview count](t582-s26-preview-cost.md).
- Measured compact preview candidate and receiving status: [S26 compact preview](t582-s26-compact-preview.md).
- CPU parity folding: [S26 parity audit](t582-s26-parity-audit.md).
- Branch timing condition simplification: [S26 comparison](t582-s26-branch-condition.md).
- PC110 display adapter aperture validation: [S26 check](t582-s26-video-adapter.md).
- Duplicate one-byte opcode-record copy: [S26 copy audit](t582-s26-opcode-copy.md).
- Memory read-modify-write precheck frequency: [Personaware Game](t582-s26-personaware-game.md#read-modify-write-precheck-frequency).
- Game drawing guest-PC samples and instruction-byte attribution:
  [Personaware Game](t582-s26-personaware-game.md#game-drawing-workload).
- Direct primary-opcode dispatch: [S26 dispatch probe](t582-s26-direct-dispatch.md).
- Memory-route one-pass and SMRAM cache candidates: [S15](t582-s15-cpu-line-audit.md) and [S19](t582-s19-steady-profile.md).
- 486 cache lookup and line-proof frequency: [S26 cache query](t582-s26-cache-query-cost.md).
- Ordinary cache access and guest-visible test-register publication:
  [S26 cache query](t582-s26-cache-query-cost.md#ordinary-access-test-register-publication-audit).
- Absent-FPU scheduler call removal: [S26 FPU probe](t582-s26-fpu-scheduler-probe.md).
- Single-owner timing-result write: [S26 timing probe](t582-s26-timing-owner-probe.md).
- 486 timing-term prefix reuse with preserved direct tests: [S26 comparison](t582-s26-486-term-prefix.md).
- Observation-preview ablation and 486 asynchronous regression:
  [S26 comparison](t582-s26-preview-observation-bound.md).
- Same-host PC110-EMU reference run: [S26 comparison](t582-s26-pc110-emu-measurement.md).
- Same synthetic loop on both engines and isolated CPU: [S26 comparison](t582-s26-pc110-emu-measurement.md#same-synthetic-instruction-loop).
- Instruction-entry snapshot copy attribution and GCC string-operation
  experiment: [S26 comparison](t582-s26-pc110-emu-measurement.md#instruction-boundary-copy-attribution).
- PC110 port read upper-bound probe: a task-only candidate removed the second
  wired-OR provider traversal entirely, which is invalid for general machines.
  PC110 uses ATA rather than the registered Compaq wired-OR route. At the
  identical 10,000,000-instruction / 79,964,294-tick / `0003dbc5` checkpoint,
  x64 baseline/candidate took 21,262/22,946 ms; reverse candidate/baseline took
  22,315/21,064 ms. These sequential runs do not prove general equivalence or
  isolate host variation, but do not justify adding a provider count or index.
  No production route or receiver changed.

## Direction and evidence limits

The measured World Clock workload advanced approximately six guest seconds in 60 host seconds ([S19](t582-s19-steady-profile.md)). A separate instrumented sample attributed roughly 65% of Machine-run intervals to CPU refresh and the rest to Machine-side work, scheduling and instrumentation; roughly 29% of sampled refresh intervals was timing selection and observation ([S20](t582-s20-execution-cost.md)). These are approximate inclusive measurements from a specific workload, not exact production percentages.

The configured external-media inventory and the distinction between clock and interactive acceptance are recorded in the [workload audit](t582-s26-workload-audit.md).

The [Game click observation](t582-s26-personaware-game.md#single-click-response-observation)
establishes one successful adjacent-pair action through direct Machine input,
but not whole-game qualification or native Window/RDP input.

Even a hypothetical removal of all measured timing-selection/observation cost would therefore leave most of the measured execution work intact; it cannot by itself explain an order-of-magnitude improvement. The next useful design should investigate the complete retirement-to-device-advance path and its observation boundaries, with representative guest workloads and correctness evidence, rather than assuming isolated instruction micro-optimizations can meet the goal. Guest instructions, device side effects, clocks and asynchronous observation must remain correct under the current packet. No alternative execution path or scheduling optimization is authorized merely by this index.
