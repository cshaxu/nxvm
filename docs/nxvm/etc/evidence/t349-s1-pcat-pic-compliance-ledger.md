# T349 S1: PC/AT 8259A Compliance Ledger

## Scope and authorities

This finite PC/AT-specific ledger is required by the active T349 S1 packet.
It uses the Intel 8259A datasheet for command/controller semantics and the IBM
PC/AT cascade contract: slave `INT` reaches master `IR2`, leaving fifteen
device IRQs. It does not model electrical waveforms, INTA bus cycles, APIC,
arbitrary cascades, or a host bridge.

- Intel 8259A datasheet: [8259A Programmable Interrupt Controller](https://www.cs.umb.edu/cs341/Intel8259/I8259APIC.pdf).
- PC/AT cascade context: [IBM PC/AT interrupt assignments](https://citeseerx.ist.psu.edu/document?doi=7692a609b46d55ea6785b9e02ef15f106153a484&repid=rep1&type=pdf).

The controller state owner is `src/core/machine/pic.c` / `pic.h`.
`core_machine_pic_irq_source` is the sole device-facing request boundary;
`machine.c` supplies the deterministic `DMA -> PIT -> PIC` refresh boundary;
the CPU executor performs scan, vector peek, then acknowledgement through the
same PIC pair. No device owns a vector, IRR, ISR, priority, or EOI state.

## Complete mechanism inventory

| Adopted PC/AT mechanism | Source and caller/write inventory | Current focused proof | S1 disposition and bounded receiver |
| --- | --- | --- | --- |
| Master/slave ports and ICW sequencing | `pic.c`: `io_{read,write}_00x0`, `io_{read,write}_00x1`; ports `20/21/a0/a1`; `machine.c` creates/resets the pair; VM firmware emits `VPIC_POST`. | `core_machine_pic_irq_lifecycle_smoke` initializes both chips and obtains `08h`/`70h` bases. | ICW restart/state-clear and invalid/incomplete sequence semantics have no owner proof. S2. |
| IMR, IRR, ISR ordinary lifecycle | `t_pic_data`, request assert/deassert, `RespondINTR`, OCW1/OCW2. | Lifecycle smoke proves mask/unmask, edge latch, acknowledgement, ISR, ordinary master/slave EOI. | Accepted only for ordinary fixed-priority paths. S2 adds command/state cross-product proof. |
| Fixed priority and cascade | `GetRegTopId`, `HasINTR`, scan/peek/get, `core_machine_pic_refresh`; slave eligibility materializes master IR2. | Lifecycle smoke proves IRQ1 before IRQ6 and IRQ14 -> `76h`, with both ISR bits and two EOIs. | Basic PC/AT cascade only. S2 owns priority blocking, masked slave/cascade withdrawal, and acknowledgement atomicity. |
| Ordinary/specific EOI | OCW2 decode in `io_write_00x0`; ISR mutation occurs there or in AEOI acknowledgement. | Lifecycle smoke uses non-specific EOIs. | Specific EOI, invalid/no-ISR EOI, nesting, rotate-on-EOI have no proof. S2. |
| Rotating priority and AEOI | OCW2 `R/SL/EOI` cases and `RespondINTR` AEOI branch. | No focused current-gate owner. | Partial construction is not a compliance claim. S2 owns full fixed/rotating/AEOI probe and any local repair. |
| OCW3 IRR/ISR reads | `io_read_00x0`, OCW3 `RR/RIS` state. | No focused owner; direct field observations do not exercise ports. | S3 owns command persistence and port-read proof. |
| Poll command | OCW3 `P` returns `80h | highest`; it does not call acknowledgement. | No focused owner. | Potential divergence from poll acknowledgement semantics, unconfirmed pending a focused probe. S3 owns probe, repair if reproduced, and poll/EOI sweep. |
| Special mask mode | OCW3 `ESMM/SMM`; OCW1 currently mutates `isr &= ~imr`. | No focused owner. | Potentially conflates masking and ISR state; interaction is unproven. S3 owns the full model or an explicit manual-backed PC/AT exclusion. |
| SFNM | `HasINTR` changes comparison from `<` to `<=` for ICW4.SFNM. | No focused owner. | Bare comparison does not establish cascade/SFNM completion. S3 owns the PC/AT-relevant decision/proof. |
| Edge/level and multiple sources | Per-line `asserted` counts; refresh restores asserted level IRR. Producers: PIT0, KBC1/12, FDC6, HDC14, RTC8. | Lifecycle smoke proves one edge and one level path; KBC/AUX and RTC smokes consume binding. | S4 owns same-line, reset/deassert, priority/mask/EOI interaction and producer sweep. |
| Spurious IRQ7/IRQ15 | No explicit state, synthetic request, or vector path; `get_interrupt` assumes an existing request. | No focused owner. | S4 owns the adopted PC/AT IRQ7/15 contract, including master-only EOI for IRQ15. Any INTA/CPU-frame redesign transfers out. |
| CPU delivery ownership | `cpu_instructions.c:ExecInt` gates scan/peek/`_e_intr_n`/get; machine binds one pair. | Hardware delivery, VM86/IRET and instruction PIC smokes; T346 timeline trace. | No redesign admitted. S2/S4 use it only to prove PIC acknowledgement ordering. |
| L3 refresh/reset/finalize | `machine.c:core_machine_arbitration_tick`, cold reset, PIC finalize. | T346 arbitration/timeline and T348 DMA/producer tests. | S4 owns reset/finalize/source composition; L3 cadence remains accepted T346 ownership. |

## Producer and consumer sweep

| Boundary | Exact production sites | Disposition |
| --- | --- | --- |
| IRQ0 | `machine.c` binds PIT output through `core_machine_pic_timer_output`. | S4 lifecycle consumer. |
| IRQ1/IRQ12 | `kbc.c` binds/asserts/deasserts its two sources. | S4 source/reset sweep; T351 retains 8042 protocol. |
| IRQ6 | `fdc.c` binds and owns completion/reset deassertion. | S4 source/reset sweep; T347 retains FDC timing. |
| IRQ8 | `rtc.c` binds and owns periodic assertion/deassertion. | S4 source/reset sweep; T350 retains RTC signal semantics. |
| IRQ14 | `hdc.c` binds and owns ATA readiness/reset assertion/deassertion. | S4 source/reset sweep; T347 retains ATA timing. |
| CPU consumer | `cpu_instructions.c:ExecInt` is the only production scan/peek/get caller. | Retain delivery; T349 proves only acknowledgement ordering. |

## Receiver plan

| Later S | Complete bounded result |
| --- | --- |
| **S2: dual-PIC command, priority, and cascade core** | ICW initialization/reinitialization, ICW/OCW read state, IMR/IRR/ISR, fixed/rotating priority, specific/non-specific EOI, AEOI, master/slave cascade and acknowledgement state. Repair only PIC-local command/priority code after a focused reproducer; sweep CPU scan/peek/get. |
| **S3: OCW3 controller modes** | Poll acknowledgement, IRR/ISR read selection persistence, special-mask behavior, and PC/AT-relevant SFNM. Prove each supported mode or retain a precise manual-backed exclusion. |
| **S4: request lifecycle, spurious, and composition reconciliation** | Edge/level/multiple-source behavior, IRQ7/15 spurious semantics, reset/finalize and every producer lifecycle, plus CPU/L3 ordering regression. Transfer any true INTA/CPU-frame redesign outside T349. |

## Explicit transfers and non-claims

- T350 owns PIT/PPI/RTC/NMI signal generation; T349 consumes only asserted and
  deasserted IRQ state.
- T351 owns 8042/AUX protocol semantics; T349 consumes only IRQ1/IRQ12.
- T347 owns FDC/HDC service timing and T348 owns DMA lifecycle; T349 verifies
  their request lifecycle only at the controller boundary.
- Physical INTA cycles, arbitrary multi-slave wiring, APIC, host IRQ bridges,
  and CPU exception/IRET redesign are not T349 completion claims.

## S1 verification

On 2026-08-13, the source/caller sweep used `rg` over tracked `src`, `tests`,
and CMake files for `core_machine_pic_`, `VPIC_`, and the PIC trace boundary.
The retained focused set passed: PIC IRQ lifecycle, CPU/PIC lifecycle, T346
timeline and arbitration, DMA/RTC authority, and hardware delivery. A fresh
`mingw-gcc-x64` configure, documentation governance, `git diff --check`, and
the complete current-gate suite also passed: 223 of 223 tests. This validates
the retained baseline only; it does not turn unexercised command modes into
accepted behavior.
