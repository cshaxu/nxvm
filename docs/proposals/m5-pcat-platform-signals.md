# M5 PC/AT Platform Signals: PIT, PPI, RTC, And NMI

Complete the selected PC/AT platform-signal graph while preserving the T346
deterministic timeline owner. This is a mechanism package, not a collection of
convenient port patches: every adopted signal must have one manual contract,
one state owner, an explicit producer/consumer path, a reset/finalize rule, and
a focused proof. RTC events remain IRQ8; no task may reinterpret them as NMI.

## Planned subtasks

| Subtask | Objective | Boundary |
| --- | --- | --- |
| S1 | Build the 8254/MC146818/IBM-PC/AT manual-to-source-to-proof signal ledger and decide which PPI/speaker and NMI wires have a PC/AT contract and a receiver. | Audit only; no behavior claim or source import. |
| S2 | Reconcile the retained 8254 counters, gates, channel-0 IRQ0, and any reproduced channel-2/port-61 signal behavior through one output/lifecycle owner. | No host audio or sampled waveform. |
| S3 | Reconcile MC146818 calendar, periodic/update/alarm, register-C acknowledgement, IRQ8, CMOS index/mask adapter, and reset/finalize semantics. | No host wall clock, firmware rewrite, or RTC-to-NMI shortcut. |
| S4 | Implement only the selected IBM-PC/AT parity or I/O-channel-check NMI producer if S1 establishes its wiring and a deterministic probe; otherwise record an evidence-backed transfer. Close the platform-signal graph. | No generic CPU-delivery redesign, arbitrary board wiring, or synthetic source. |

## Required method and exit standard

Use the Intel 8254 contract, Motorola MC146818A contract, and IBM PC/AT
technical reference as primary behavior sources. Bochs and PCjs may guide
questions only; their source is never imported or treated as specification.
Sweep every port registration, timeline advance, output callback, IRQ/NMI
assert/deassert, reset, finalize, and consumer. A form is complete only when
the ledger supplies its manual requirement, owner, proof, and retained or
transferred limitation.

The task may transfer an electrical, host, or unselected-board behavior only
to a named Queue/TODO receiver with an admission condition. It may not hide a
missing PPI/speaker or NMI producer behind a generic "L3" label. Task closure
requires a stable deterministic-L3 state graph for every selected signal and
all relevant current-gate, governance, and source-boundary proof.
