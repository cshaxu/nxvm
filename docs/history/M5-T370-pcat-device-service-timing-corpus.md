# M5 T370: PC/AT Device Service-Timing Corpus

## Task Record

T370 follows the closed T369 logical bus-stage package. It owns the
source-backed command/service timing corpus for every selected IBM PC/AT 5170
Model-339 baseline device that makes externally visible progress. It neither
claims a physical clock nor closes the 5170 L3 baseline.

## Active Progress

### S1: Selected device-service owner and source ledger

S1 establishes the exact per-device command, state, DRQ/IRQ, acknowledgement,
reset/cancellation, clock-domain and consumer inventory before a timing value
or state-model repair is admitted.

S1's [owner ledger](../etc/evidence/t370-s1-pcat-device-service-owner-ledger.md)
accepts no timing value. It assigns the four bounded device groups required
before later implementation work and retains all physical-clock conversions as
explicit transfers.

S1 is accepted at `a3df36dd`. S2 is limited to PIT, PIC and RTC/CMOS
command/event-domain reconciliation; it may not select a time conversion until
the source, project clock and consumer boundary all agree.

### S2: PIT/PIC/RTC domain reconciliation

S2 retains the existing controller lifecycle and event ordering, but confirms
that the descriptor ratios and RTC `ticks_per_second` lack a source-labelled
physical conversion. The [reconciliation](../etc/evidence/t370-s2-pit-pic-rtc-domain-reconciliation.md)
therefore transfers oscillator/INTA propagation and gives S3 the bounded
dual-8237A/FDC controller-domain decision.

S2 is accepted after `199e0ed2` restores the documentation gate. Its retained
logical controller proof does not upgrade any project ratio to physical time.

### S3: Dual-8237A/FDC domain reconciliation

S3 confirms the single FDC DRQ/DMA2/IRQ6 lifecycle and records that uPD765
timers have no source-labelled mapping to project elapsed ticks. Its
[reconciliation](../etc/evidence/t370-s3-dma-fdc-domain-reconciliation.md)
therefore transfers FDC physical service values and assigns S4 the 8042/
keyboard command/FIFO/IRQ domain.

S3 is accepted at `3d5cd542`; its terminal and reset proof is logical only and
does not convert any FDC action into a device-duration claim.

### S4: 8042/keyboard domain reconciliation

S4 retains one KBC command/FIFO/IRQ/reset owner and confirms that the selected
Model-339 descriptor deliberately supplies zero response and typematic delay
fields.  The [reconciliation](../etc/evidence/t370-s4-kbc-keyboard-domain-reconciliation.md)
therefore preserves protocol/lifecycle proof without converting KBC project
ticks into controller or keyboard duration.  S5 receives planar-parity NMI,
CGA/VADP visible-state/raster work and the task-level transfer audit.

S4 is accepted at `12e77141`. Its retained zero-delay configuration is an
explicit non-admission of physical controller or keyboard duration, not a
completed 5170 timing result.

### S5: planar parity, CGA and task transfer audit

S5 reconciles the selected planar-parity NMI and CGA/VADP lifecycle/topology
owners without assigning device duration.  Its [transfer audit](../etc/evidence/t370-s5-planar-cga-transfer-audit.md)
completes all four T370 device groups and transfers physical clocks, bus phase,
controller microstates and final model readiness to the ordered phase and 5170
audit candidates.

S5 is awaiting closure governance. Its device-group audit is complete, but
this task remains open until the queue and final closure record are verified.

T370 is closed after S5 governance: its retained proposal is archived beside this record, and physical-duration transfers advance to T371. This does not make the 5170 Model-339 L3-ready.
