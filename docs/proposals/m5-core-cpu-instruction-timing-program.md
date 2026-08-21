# M5 Core CPU Instruction Timing Programs

## Purpose

Migrate the retained 8086, 80186, 80286 and 80386DX instruction-timing ledgers
onto the Core timing-plan path. Replace coarse scalar or ad-hoc surcharge
selection with neutral instruction-form/context timing programs that express
documented constants, formulas and deterministic bounded ranges.

## Shared Admission Baseline

This candidate applies the
[Core specification-driven timing design](../etc/architecture/specification-driven-l3-timing.md)
through the plan boundary and consumes the earliest CPU-program batch:
`CPU-EXEC`, `CPU-EXCEPT`, and `CPU-FPU` from the
[T433 S6 Core L3 admission ledger](../etc/evidence/t433-s6-core-l3-admission-feasibility-ledger.md).
The [T433 S7 source-sufficiency ledger](../etc/evidence/t433-s7-core-source-sufficiency-ledger.md)
distinguishes a missing Core form selector from a board/transaction input; this
candidate may not turn either into a profile-local algorithm or guessed value.
It owns instruction-form and delivery/FPU rule admission, not board
availability, prefetch production, INTA waveform or profile-local algorithms.
Those retain their ledger-assigned receivers and explicit L2 status until
their own contract is implemented.

## Evidence-First Completion Method

This candidate follows the binding [Td S121 CPU timing governance record](../etc/evidence/td-s121-cpu-timing-evidence-first-governance.md). It is a three-phase task, in this order:

1. freeze a complete 8086/80186 successful-retirement evidence and model ledger before inspecting current timing values;
2. audit the current decoder, timing selectors, retirement publisher, accounting and tests against that ledger, then publish one finite repair plan; and
3. implement declared ledger batches, remove obsolete timing paths, and perform a final full-ledger closure audit.

For an Intel range-only row, a version-pinned, same-CPU reference model that conforms to manual semantics and bounds is a labelled L2 model, never an L3 formula. A manual/reference conflict is resolved in favour of manual semantics while retaining only separable reference timing mechanics. If no range exists, the ledger follows the same reference, corrected-model and project-model ladder; only a row for which no compliant explainable model can be established is an explicit L1 exception. A midpoint is the final bounded L2 fallback, not a hidden default; no successful row may retain an anonymous one-tick result.

## Required Scope

Consume the reconciled CPU rows and retained T357/T359/T360/T361/T362/T363
ledgers. For each admitted form/context, select timing before the sole Core
retirement publication point and retain architectural fault, interrupt,
prefix, string, branch, operand/address-size and mode semantics. A range or
source gap must first receive the Td S121 evidence/model disposition. An
insufficiently supported row is explicit L1 only when that ladder exhausts;
it may not be hidden in a generic fallback.

The four existing CPU profiles are the complete universe for this candidate.
Later CPU profiles require a later Core admission.

## Dependencies

Consumes the Core timing-plan boundary and its CPU conformance rows. It
precedes CPU-to-board transaction timing because instruction-internal time and
external completion must have distinct owners.

## Implementation Ownership And S Decomposition

The current instruction scalar selectors reside in `machine.c` because that
file coordinates decoded execution and retirement. They are nevertheless CPU
instruction timing, not board timing. T435 moves their rule selection into a
new Core-private `src/core/machine/cpu_timing.c` subsystem with a private
same-directory header where needed. `machine.c` retains only execution
coordination, the one call to that subsystem, and the sole retirement
publication. `cpu_instructions.c` retains decode and semantic execution; it
may publish decoder-owned timing inputs, but must not acquire independent
manual-clock policy. `cpu.c` and `cpu_interface.h` remain CPU-profile identity
and feature-gate owners, not timing-table owners.

T435 owns successful-retirement instruction constants, formulae and explicit
architectural inputs. It must not own READY/HOLD/BUSLOCK arbitration, prefetch
availability, memory/I/O wait states, DMA competition, INTA waveforms, device
response/BUSY duration, or board physical cycles. Those remain the separately
admitted transaction/device/board timing receivers.

| S | scope and owned output | exact exit criterion |
| --- | --- | --- |
| S3 / B0 | Create `cpu_timing.c`; materialize canonical manifest keys; accept decoder-owned inputs; emit shared result records; route every successful CPU timing request through one publisher | all manifests generate canonical records; one `machine.c` timing/publication seam; no board/transaction rule enters CPU timing |
| S4 / 8086 | Implement 8086 L3 plus 16 named L2:G3 keys, including EA, odd word, prefixes, repeat and Group-3 partitions | all 649 keys conform; manual bounds/origin/unallocated proof; old selector removed |
| S5 / 80186 | Implement exact/formula keys and 25 declared midpoint L2 keys, including 80186-only forms and legal contexts | all 602 keys conform; no endpoint/constrained legacy arithmetic path remains |
| S6 / 80286 | Implement all L3 forms including real/PM, privilege/gate/task, EA/odd word, next-byte and repeat inputs | all 812 keys conform; no successful fallback remains |
| S7 / 80386DX | Implement all L3 forms including size, VM86/PM, system forms, early-out multiply and r/m/repeat contexts | all 1,232 keys conform; formula/path inputs are published and checked |
| S8 / B4 | Cross-profile result consumption, obsolete-selector removal and closure audit | zero nonconforming key across four manifests; focused and cross-profile regressions pass |

Each CPU S verifies decode/semantic execution and fault boundaries as a
non-regression condition. It does not reopen T401 instruction semantics unless
a timing-input observation proves a concrete semantic defect; that requires a
separate corrective admission.

## Evidence And Completion Standard

Require source-to-program mapping for every frozen CPU row, focused timing and
fault/retirement regressions for every rule kind, cross-profile non-regression,
and a closure audit showing every row is L3, labelled L2, explicit L1, not
applicable or unsupported. No profile obtains an unlabelled timing default.

## Non-goals And Stop Conditions

Do not reopen instruction semantic correctness already accepted by T401, add
80486/P5, conflate memory/I/O waits with instruction cost, infer undocumented
physical timing, or perform L4 work. Transfer a source conflict to the master
ledger receiver rather than guessing a cost.
