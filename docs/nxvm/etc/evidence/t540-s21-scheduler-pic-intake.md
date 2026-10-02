# M5 T540 S21 Scheduler And PIC Source Intake

S21 is a source-only owner audit. The originally prospective combined
deadline/advance/PIC code move is too large for one reviewable boundary:
`machine_scheduler.c` is 436 lines, `cpu_bus.c` is 332 lines, and the first
also contains the only guest-tick publication path. No source, ABI, timing
value, profile, asset, INI, test or executable changed in S21.

## Exact current owners and callers

- `core_machine_publish_elapsed_ticks` is the single Core time-publication
  entry. Its callers are CPU retirement in `machine.c`, explicit paused time,
  next-deadline progression and bounded L1 compatibility progression. It
  updates the Core timeline before dispatching device effects. It must not
  migrate into an IBM-PC board component or be duplicated there.
- `core_machine_capture_time_observation_private` currently merges generic
  timeline and FPU completion with IBM-PC PIT/auxiliary PIT, RTC, DMA, FDC,
  HDC, KBC, PIC, XT keyboard and D4 refresh observations. The board inputs
  must cross as copied next-due/immediate/L1-blocking values. A missing or
  unsourced device deadline remains explicitly blocking; a synthetic
  `now + 1` must not be sold as L3.
- `core_machine_arbitration_advance` currently owns DMA HOLD and wait
  windows, refresh HOLD, PIT advancement and PIC refresh;
  `core_machine_readiness_advance` follows with FDC/HDC/FPU/RTC;
  `core_machine_peripheral_advance` follows with XT keyboard/KBC, PIC and
  VADP. The order is causal: a readiness effect may become PIC-eligible only
  at the following due tick. FPU progression and CPU prefetch reservation
  remain neutral-Core concerns even when adjacent to board effects.
- `core_machine_cpu_bus.interrupt_pending` calls
  `core_machine_pic_scan_interrupt` directly. Its `acknowledge_interrupt`
  begins a Core CPU transaction, calls `core_machine_pic_get_interrupt`
  (master, then slave on cascade), records the vector and commits. The
  PIC selection/IRR-to-ISR effect belongs to the board service; Core keeps
  transaction and CPU vector ordering. Direct PIC unit tests may continue
  to exercise the PIC board API; only the CPU bus bridge changes.
- `core_machine_transaction_trace` in `cpu_bus.c` invalidates CPU-side
  locality on acknowledged DMA HOLD under a named Generic-AT policy. This
  is not a generic PIC rule and cannot silently be copied into neutral
  `x86/core`. S24 must explicitly locate the policy owner while preserving
  the current trace and prefetch behavior.

The finite receiver sequence is S22 copied board deadlines, S23 ordered
board advance phases, S24 PIC pending/INTA and HOLD locality, S25 mixed
plan/reset lifetime, S26 physical neutral Core move, then S27 onward for
proven common/AT/XT board mechanisms. Each code receiver deletes its old
caller in the same P; none installs a second scheduler or generic event bus.

## Regression anchors

- Deadline and causal order: Core time, explicit-time, rational-clock,
  timing-checkpoint, timeline, scheduler, PIT divider/IRQ0, RTC storage,
  FDC/HDC due, KBC cadence and XT keyboard tests.
- Arbitration and trace: DMA channel/RTC authority, D4 refresh HOLD,
  competition, prefetch locality, transaction lifecycle and external-time
  trace tests.
- PIC handoff: PIC phase, IRQ lifecycle, cascade/OCW3, CPU interrupt
  delivery and INTA trace tests.
- Every code receiver runs full repository-only x64/x86 units, specialized
  gates and one external boot checkpoint per four profiles and width, with
  eight optimized products. S21 itself changed documentation only; source
  verification starts from the accepted S20 **469/469** unit and **8/8**
  external boot baseline.

This audit neither changes a controller evidence grade nor authorizes a
hardware timing reinterpretation. A contract expansion discovered during
S22-S24 must be recorded before implementation rather than patched around.
