# T450 S4 PIC 8259A Current-Code Gap Audit

## Scope

This audit preserves the `PIC-R1`--`PIC-T5` source universe frozen in T450
S3. `Conforming` means current code implements the stated logical behavior for
the selected IBM AT two-PIC personality; it does not convert an S3 L2 or L4
row into an L3 timing claim. `Receiver` names the queued Core PIC phase
contract candidate unless the source already excludes the row from L3.

## Row Disposition

| S3 ID | Current owner and evidence | Disposition | Receiver or boundary |
| --- | --- | --- | --- |
| PIC-R1 | `pic.c` initialization, `core_machine_pic_reset`, IRR/ISR/IMR state; `core_machine_pic_irq_lifecycle_smoke.c` and `core_machine_pic_lifecycle_s4_smoke.c`. | Conforming. | None. |
| PIC-R2 | `core_machine_pic_begin_initialization` and `io_write_00x0`; `core_machine_pic_command_priority_smoke.c` initializes both controllers. | Conforming for x86 initialization; explicit reinitialization test is missing. | PIC candidate: add ICW1 reinitialization regression. |
| PIC-R3 | `io_write_00x1` records ICW2--ICW4 and respects ICW1's IC3/IC4 sequence; command-priority smoke covers normal AT values and AEOI/SFNM. | Partial: stored configuration is conforming for selected AT, but generic single/cascade topology is not consumed. | PIC candidate: bind ICW3/SNGL configuration to selection or explicitly constrain the public personality. |
| PIC-R4 | `io_write_00x0`, `io_write_00x1`, `core_machine_pic_eoi_service`; command-priority and OCW3 smokes cover mask, EOI/rotation, poll/read and special mask. | Conforming. | None. |
| PIC-R5 | `io_read_00x0` and `io_read_00x1`; `core_machine_pic_ocw3_smoke.c` covers IRR, ISR, poll acknowledgement and IMR paths. | Conforming. | None. |
| PIC-F1 | Source-reference counts in `core_machine_pic_irq_source_assert/deassert`; level refresh in `core_machine_pic_refresh`; lifecycle smoke covers master and slave edge/level reassertion. | Conforming. | None. |
| PIC-F2 | `core_machine_pic_priority_rank`, `core_machine_pic_request_can_interrupt` and `GetRegTopId`; command-priority smoke covers nesting and rotation. | Conforming. | None. |
| PIC-F3 | `RespondINTR`, OCW2 cases in `io_write_00x0`; command-priority smoke covers specific/non-specific EOI, rotation and AEOI. | Conforming for logical EOI. | S3's electrical acknowledge point remains L4. |
| PIC-F4 | `core_machine_pic_request_can_interrupt` and OCW3 ESMM/SMM path; `core_machine_pic_ocw3_smoke.c` covers special-mask behavior. | Conforming. | None. |
| PIC-F5 | Poll branch in `io_read_00x0`; `core_machine_pic_ocw3_smoke.c` covers status, acknowledge and AEOI poll. | Conforming. | None. |
| PIC-F6 | `core_machine_pic_select` and `core_machine_pic_get_interrupt` hard-code cascade on master IRQ2 and ignore the programmed ICW3 map; selected IBM AT configuration uses that route. | Wrong-value for generic 8259A cascade; conforming only for selected AT topology. | PIC candidate: one selected-topology contract or ICW3-driven selection; do not add a parallel PIC path. |
| PIC-F7 | Port registration in `core_machine_pic_initialize` fixes 0020h/0021h and 00A0h/00A1h; IRQ14 cascade lifecycle is covered by command-priority and lifecycle smokes. | Conforming for selected AT topology. | None. |
| PIC-F8 | `core_machine_pic_scan_interrupt`, `peek_interrupt`, `get_interrupt` deliver a vector directly to the CPU executor; CPU integration callers are `cpu_instructions.c` and lifecycle smokes. | Missing-input for a source-selected command-to-visible/CPU-delivery phase contract, not a missing two-pulse waveform implementation. | PIC candidate after the Core transaction/arbitration owner selects a logical delivery contract; waveform remains L4. |
| PIC-T1 | No PIC clock-domain field or advance loop exists; PIC changes only on source or port operation. | Conforming. | None. |
| PIC-T2 | `core_machine_pic_refresh` supplies deterministic ordering but is invoked by the current machine scheduler without an admitted PIC command-to-visible timing plan. | L2/unallocated timing fact. | PIC candidate: declare only a sourced logical phase; no inferred delay. |
| PIC-T3 | No code claims data-sheet electrical setup/hold/propagation values. | Correctly excluded (L4). | No receiver. |
| PIC-T4 | `core_machine_pic_initialize` owns the four AT PIC ports and master/slave route; Core owns this state. | Conforming for selected AT topology. | Generic topology gap is PIC-F6. |
| PIC-T5 | T433's selected visibility/delivery requirement is not a registered plan input; no test proves a declared phase contract. | Missing-input and missing-test, not a chip-state defect. | PIC candidate. |

## Exhaustive Findings

The only production capability gap inside the PIC source universe is the
hard-coded IRQ2 cascade assumption despite ICW3/SNGL being accepted and stored.
It has one future receiver: the queued PIC phase-contract candidate. The
remaining nonconforming timing rows require a selected Core transaction/
arbitration input and are not safely repairable at PIC call sites. No duplicate
owner, dead PIC path, or direct VM dependency was found: `pic.c` owns logical
state, `machine.c` owns composition, and the CPU executor consumes the bounded
PIC operations.

## Test Inventory

`core_machine_pic_command_priority_smoke.c` covers initialization, register
programming, EOI/rotation, AEOI and selected cascade priority.
`core_machine_pic_ocw3_smoke.c` covers IRR/ISR reads, poll, special mask and
SFNM. `core_machine_pic_irq_lifecycle_smoke.c` and
`core_machine_pic_lifecycle_s4_smoke.c` cover edge/level sources, master/slave
lifecycle, reset and PIT-owned IRQ0 binding. They do not cover ICW1
reinitialization or non-IRQ2 ICW3/SNGL topology, which remain explicit future
PIC regressions.
