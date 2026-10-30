# MyPC110: startup performance and guest clocks

Transfer from T580 by owner direction on 2026-10-29. Diagnose the long startup and slow World Clock using actual product, guest clock, RTC and timer observations. Determine the correct owner and implement a measured, minimal repair without replacing guest time with an unexamined host-time shortcut. Preserve the owner's exclusion of elapsed host-off compensation. Distinguish L1/L2 models from manual-defined facts.

Acceptance requires a before/after representative startup measurement, guest clock and RTC evidence, both host architectures, appropriate regression and product artifacts. Do not claim hardware accuracy or root cause from a single observation. See [S89](../etc/evidence/t580-s89-clock-rate.md), [S88](../etc/evidence/t580-s88-clock-persistence.md) and [S90](../etc/evidence/t580-s90-acceptance-failure.md).

## Admission and sequence

The owner selected performance as the next task on 2026-10-29, superseding
its former queue position 14. Admit M5 T582 after accepted T581. Baseline:
`59ad6e78b` implementation and `f07c61150` closure, PC products 0.5.0581.
The primary workload is the unified MyPC110 product with configured private
CMOS and disk copies; DSTN is the default, TFT is the comparison variant.
Empty-CMOS first setup is a different workload and remains separately queued.

- **S1: representative baseline and bottleneck audit.** Reuse existing probes
  and diagnostics. Correlate monotonic host time, retired instructions, Core
  ticks, RTC and PIT/DOS observations through existing safe observation paths.
  Separate POST, DOS/Personaware startup and steady guest operation; distinguish
  Machine-only from actual product/UI overhead. Compare both host widths.
  Inspect the hot execution/device/display paths and measure promising areas
  before proposing changes. Exit with an evidenced owner, reproducible baseline,
  bounded repair design and estimated diff, or an explicit unresolved finding;
  test success alone is not performance evidence. No production change in S1.
- **S2: measured owner-local repair.** Admit the concrete S1 design, preserve
  guest timing and device ordering, compare the same baseline/candidate workload,
  retain only demonstrated improvements and add the minimum relevant regression.
  Split independent mechanisms into further numeric S tasks if evidence requires.
- **S3: remaining execution costs.** S2's x64 integrated pair still advances
  only about 25 guest seconds in 300 host seconds. Audit CPU refresh, repeated
  timing-result reads, and physical-memory route/line qualification using
  task-owned counters and fixed-work comparisons. The earlier RAM candidate
  rejected every configuration with any alias; establish which actual accesses
  caused that rejection before revisiting it. Preserve narrow device windows,
  raw/reset routes, A20, aliases, parity, observers and guest timing. Do not
  remove checks merely because they repeat syntactically. Implement only an
  evidenced equivalent improvement; otherwise retain the negative evidence
  and select the next measured owner. No new public interface is presumed.
- **S4: board DMA arbitration audit.** Measure the idle and active paths in the actual PC110 workload; preserve cascade and callbacks, and only retain a demonstrated equivalent improvement.
- **S5: CPU timing computation audit.** Measure repeated prefix and timing work; preserve all profiles and timing semantics.
- **S6: display observation.** Prove correct provider-owned observation and avoid unchanged PC110 capture without losing cursor updates.
- **S7: current execution profile.** Re-sample the post-S4 guest and select the next evidence-backed hotspot.
- **S8: keyboard idle deadline.** Eliminate the measured redundant inactive-repeat query while preserving the existing input contract.
- **S9: execution observation and scheduling audit.** Measure per-instruction observation and scheduling costs with full correctness boundaries.
- **S10: PIT deadline aggregation.** Select each PIT domain's nearest channel before converting to source ticks.
- **S11: receiving qualification and delivery.** Reconcile startup and steady
  guest/clock observations on both widths and variants, run affected receiver
  regressions, rebuild/deploy affected products, review actual diff and publish.
  Close only after owner acceptance. Renumber this final S if another measured
  implementation batch is required. Every Shared batch independently meets the
  fourteen-artifact and full-test receiving gate below.

- **S12: lexical-preview cache/TLB borrow.** The S11 fixed-work and late
  comparisons identified a bounded candidate. Replace only the 486 preview's
  private cache/TLB copy with a synchronous immutable borrow from the CPU owner;
  preserve cache visibility, faults, debugger and all profiles. Do not reuse the
  diagnostic pointer used by the temporary probe. Run the Shared receiving gate
  and report actual guest throughput; continue the T if usability remains unmet.
- **S13: sustained execution bottleneck audit.** Investigate the remaining
  order-of-magnitude PC110 slowdown using actual startup and later workload;
  only admit measured, correct improvements.
- **S14: local memory-route provider selection.** Preserve raw and selected-alias
  precedence, collect matching provider indices once locally, and evaluate each
  query in its existing tier and registration order. Preserve the 64-provider
  limit, declines, failures, CPU-only routes and dynamic address decoding. The
  [S13 measurement](../etc/evidence/t582-s13-scheduler-observation.md) is a
  bounded opportunity, not a usability claim. Qualify all Shared receivers.
- **S15: CPU instruction-line hotspot audit.** Attribute the large inlined CPU
  execution bucket to source lines on the actual worker during early and late
  PC110 execution, then evaluate a concrete semantics-preserving candidate.
- **S16: CPU execution observation cost audit.** Measure per-instruction execution,
  observation and timing handling on the actual worker; pursue only a measured,
  semantics-preserving candidate.
- **S17: PC110 memory cacheability audit.** Measure the existing firmware and
  shadow route under the 486 cache policy, including decode, invalidation,
  device side effects and timing; no production change without evidence.
- **S18: PC110 shadow cache design and measurement.** Investigate a generic
  provider capability with dynamic chip-controlled cacheability and coherent
  write/invalidation semantics; retain only a correct and measurable result.
- **S19: PC110 steady-state performance audit.** Reassess the post-S18
  Personaware/World Clock path and identify remaining measured costs.
- **S20: SMM execution and memory-service cost audit.** Attribute World Clock SMM
  and normal execution costs, examine existing bus and device contracts, and
  measure a semantics-preserving candidate or document a negative result.
- **S21: instruction timing input reuse audit.** Test whether repeated decoding
  inside the existing timing owner can be shared without changing observable
  timing, prefix or fault behavior; use short fixed-work probes before any
  long product run and reject unmeasured or insufficient candidates.
- **S22: CPU cache-hit copy audit.** Measure fixed-width copy overhead in the
  existing cache-hit route using short comparisons, and admit only a simple
  semantically equivalent improvement with sustained product-path evidence.
- **S23: instruction dispatch cost audit.** Use bounded fixed-work probes to
  identify preparation and dispatch costs before selecting any optimization.
- **S24: World Clock ASMI service audit.** Capture actual firmware service
  requests and return behavior during steady operation; distinguish legitimate
  guest activity from a missing device response before considering changes.
- **S25: PC110 execution-model comparison.** Compare existing external PC110
  emulator execution and idle behavior with the measured NXVM path, and identify
  a minimal correctness-preserving performance direction or concrete limits.
- **S26: single-executor throughput design audit.** Identify a viable path to
  substantial speedup while preserving the current timing and observation
  contracts; make any accuracy or RTC tradeoff explicit and unapproved. A
  measured compact preview-copy candidate is a bounded implementation batch,
  not evidence that substantial speedup or usability has been achieved.
- **S27 or later: receiving qualification.** S11's original whole-product and
  clock qualification remains pending after S26 and any later measured batches.

## Constraints and evidence

No host-time substitution, nominal clock reduction, skipped side-effecting
instructions/ports, firmware-address performance special cases, second executor
or hidden acceleration path. Preserve the existing admitted L1 firmware model;
its presence does not authorize further shortcuts. No host-off compensation.
No permanent cache or state is justified without an actual measured bottleneck.
Read-only profiling may inspect Shared; production Shared scope is determined
by evidence and existing authorization, not assumed from this admission.

Use the existing x86/x64 Ninja and project ccache trees. Run long measurements
hidden/in the background, sequentially for comparable throughput; reuse the
accepted T581 frames and baseline hashes rather than repeating full boots merely
for screenshots. Task scratch is `build/t582`, owned by this task until evidence
is archived. Each run must have a stated time/work budget; start with short
bounded samples, use a boot-length budget only for actual boot qualification.
Do not change owner configuration/media or discard unrelated work. S1 governance
and audit alone do not refresh product version strings or EXEs.

T580 S89's KBC and division candidates did not demonstrate gains: do not repeat
them without new evidence. T581's final frames establish boot success, not an
exact first-Personaware timestamp; extract phase timing only where observations
support it. Sampling overhead and native Window/RDP limitations must be reported.

## Shared receiving gate

Owner reaffirmed the current Execution rule on 2026-10-29: any admitted Shared
update requires cached Release builds and publication of all seven Apps on
x64 and x86 (14 EXEs): NXVM, My5160, My5170, MyDeskPro386, MyPC110,
MyRaidenII and MyNES. Run cached full-repository unit and integration tests
on both architectures, not only the modified package's tests. Reuse the stable
Ninja graphs and ccache; do not discard valid intermediates. Record actual
passes, failures and unavailable desktop/device gates separately; incomplete
verification cannot be reported as a complete pass. Verify deployed identities,
PE widths and hashes, and commit changed artifacts in their product-scoped P
commits. Receiving product versions and user configuration remain unchanged
unless separately admitted. S1's documentation/read-only audit does not itself
trigger a production rebuild. Execution remains the publication authority.
