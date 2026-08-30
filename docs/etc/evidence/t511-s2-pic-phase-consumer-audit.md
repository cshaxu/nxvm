# T511 S2 PIC Phase-Consumer Audit

`M5:T511:S2:PIC-PHASE-CONSUMER:NOT-APPLICABLE`

## Sources And Model

The primary source is Intel *8259A Programmable Interrupt Controller*, order
231468-003 (December 1988), owner-managed copy
`assets/manuals/controllers/intel/231468-003_8259A_Programmable_Interrupt_Controller_Dec1988.pdf`.
Its 8086 interrupt sequence states that the first INTA selects the highest
request, sets its ISR bit and clears its IRR bit; the second supplies the
vector. A later request is a fresh IRR event, not a continuation of the first
acknowledgement. EOI clears the selected ISR boundary.

86Box's PIC/CPU split corroborates the same distinction: CPU execution calls
its PIC acknowledge operation for a selected pending interrupt, while device
sources independently assert/deassert their IRQ line. It is an Other-L2 design
reference only and no external source was copied.

## Current Owner And Consumer Trace

| Boundary | Sole production path | Disposition |
| --- | --- | --- |
| Source edge/level publication | `core_machine_pic_irq_source_assert` / `_deassert` updates the bound PIC source and refreshes cascade selection. | Each source transition is independently published. |
| Mask, EOI, initialization and cascade change | PIC port handlers update PIC state and call `core_machine_pic_refresh_bound`. | A prior acknowledgement is not reused after command-state change. |
| Logical acknowledgement | `core_machine_pic_get_interrupt` selects and moves only the current request to ISR. | One acknowledgement matches one interrupt delivery, as required by the 8086-mode manual sequence. |
| CPU delivery | CPU execution consumes one returned vector before its interrupt frame; `core-machine-pic-phase-s2-smoke` proves acknowledgement precedes the frame. | No multi-phase consumer exists. |
| Repeated source delivery | `core-machine-pic-irq-lifecycle-smoke`, command/priority and OCW3 tests exercise fresh IRR/cascade selection after source/EOI/mask transitions. | The next IRQ is selected afresh. |

## Conclusion

PIC has no ATA-style production hit. Treating a later IRQ as another data phase
would be a category error: it is a new owner-published IRR event. The existing
source-count/IRR/ISR owner is retained unchanged. Physical INTA waveform timing
is a known separate board/bus boundary and is not a reason to add a logical
consumer delay.
