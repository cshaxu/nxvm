# T456 S4: PIC Retained-Command Closure

## Scope and ownership

This closes the frozen T450/T456 18-row `PIC-R1`--`PIC-T5` universe against
Intel 8259A `231468-003` and the selected IBM AT two-controller wiring. PIC
owns controller state and command interpretation in `pic.c`; CPU has one
consumer route through the accepted S2 logical acknowledgement transaction.
No VM/profile state, extra dispatch route, electrical delay or unselected
board topology is introduced.

## Closure matrix

| Row | Current owner and focused proof | Disposition |
| --- | --- | --- |
| PIC-R1 | PIC IRR/ISR/IMR state; IRQ lifecycle and lifecycle smokes. | Proven. |
| PIC-R2 | `core_machine_pic_begin_initialization`; command smoke reinitializes a pending cascade. | Proven. |
| PIC-R3 | ICW2--4 parser and S3 ICW3/SNGL relation; command smoke covers AT, programmed, mismatched and single mode. | Proven. |
| PIC-R4 | OCW1/2/3 parser; command-priority and OCW3 smokes. | Proven. |
| PIC-R5 | IMR, IRR/ISR reads and one-shot poll; OCW3 smoke. | Proven. |
| PIC-F1 | Edge counts and level reassertion/clear; IRQ lifecycle and lifecycle smokes. | Proven. |
| PIC-F2 | Nested fixed/rotating priority; command-priority smoke. | Proven. |
| PIC-F3 | EOI, AEOI and all selected rotation forms; nested rotate-specific-EOI regression. | Proven. |
| PIC-F4 | OCW3 ESMM/SMM priority treatment; OCW3 smoke. | Proven. |
| PIC-F5 | Master and paired slave poll acknowledgements; OCW3 smoke. | Proven. |
| PIC-F6 | One ICW1.SNGL/ICW3 paired relation for selection/refresh/SFNM; S3 command smoke. | Proven for one paired controller. |
| PIC-F7 | AT ports and IRQ8--15 cascade route; initialization, lifecycle and device smokes. | Proven for selected AT wiring. |
| PIC-F8 | One CPU route: logical acknowledgement then vector/frame entry; S2/hardware-delivery smokes and S5 default-IR7 acknowledgement proof. | Proven logically; waveform excluded. |
| PIC-T1 | No PIC advance loop or clock state; source operation/lifecycle sweep. | Static device property, proven. |
| PIC-T2 | Request/priority/acknowledgement causal order; S2 transaction trace. | Logical order proven; elapsed latency remains L2. |
| PIC-T3 | No Core conversion of electrical tables/waveforms. | Explicit L4 boundary retained. |
| PIC-T4 | `core_machine_pic_initialize` owns the four selected AT ports; S3 relation covers the selected route. | Proven for selected AT wiring. |
| PIC-T5 | S2 `CPU_INTERRUPT_ACKNOWLEDGE` transaction and phase smoke. | Proven logical Core contract. |

## S4 repair and minimalism audit

`OCW2=111` (rotate on specific EOI) previously cleared the highest-priority
ISR bit, not the command's specified level. A nested ISR regression exposed
the distinction; the PIC-local handler now uses its existing OCW2 level field.
No branch, state owner or command path was added.

The sweep also found four header macros that only referred to the file-local
`GetRegTopId`; three had no caller and the remaining one had one PIC-local
caller. They are removed. The historical `ocw1` preprocessor alias is removed
as well: OCW1 writes directly to its sole state, `imr`. This removes misleading
surface without copying the register state.

The complete command/caller sweep found one production PIC delivery consumer,
`cpu_instructions.c`; it retains S2's single `scan` then `get` route. PIC test
callers directly exercise the same controller owner. No parallel delivery,
superseded helper or undocumented production hit remains.

## Retained boundary

Polling a selected master cascade request acknowledges that master request;
polling the selected slave subsequently acknowledges the slave request. The
OCW3 regression proves both operations through their existing per-controller
ports. Arbitrary cascade boards, APIC/ELCR routing, spurious-race reproduction,
electrical INTA/read/write timing and an elapsed PIC scheduling quantum remain
outside this admitted contract, as recorded in `TODO(High)` and T456 S1.

## Verification

- The five dedicated PIC smokes (`irq-lifecycle`, `command-priority`, `ocw3`,
  `lifecycle-s4`, `phase-s2`) pass after the S4 repair.
- The affected 11 PIC/IRQ/CPU smokes pass; the complete configured build has
  539 steps; documentation governance passes; and the isolated full
  current-gate replay passes 293/293 in 205.60 seconds.

The S4 production change is one corrected level selection plus deletion of the
unused priority macros and OCW1 alias; test additions exercise the missing
nested specific-EOI and cascade-poll distinctions. S5 corrects the subsequently
found default-IR7 acknowledgement omission. No new target, wrapper, state copy
or ABI is present.
