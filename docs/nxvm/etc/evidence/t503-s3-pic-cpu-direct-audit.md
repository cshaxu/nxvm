# T503 S3: 8259A PIC To CPU Direct Audit

`T503-S3-PIC-CPU-ROUTE`

## Method and source form

This is a fresh controller-to-consumer audit, not acceptance by reference to
T456 or T488.  The normative source is Intel *8259A Programmable Interrupt
Controller*, order `231468-003` (December 1988), supplemented only for
selected board topology by IBM 5160 and 5170 technical references.  The local
Intel PDF was rendered and visually read at printed pages 4, 7 and 15: it is a
readable scan with usable text, but the rendered diagrams and prose, not OCR,
govern this record.  The IBM PDFs were directly checked for the selected port
and topology facts already visually catalogued in T488 (`5160` pp. 1-6, 1-8)
and T456 (`5170` pp. 1-10, 1-24).

The read-only external implementations were inspected only as behavioral
corroboration.  86Box `src/pic.c` explicitly performs two 80x86 acknowledge
phases in `pic_irq_ack_read`; Bochs `iodev/pic.cc` retains IRR/ISR, priority,
poll and EOI ownership; PCjs `chipset.js` has the same IRR/ISR-to-vector
structure but explicitly leaves some PIC modes incomplete.  No local MAME or
QEMU source checkout exists, so neither is claimed as inspected.  No external
source was copied or derived.

## Complete PIC-to-CPU route reconciliation

| Route/state batch | Primary requirement | Current NXVM route | External comparison | Disposition |
| --- | --- | --- | --- | --- |
| Request, mask and priority | Intel pp. 4--5: IRR records eligible input, IMR masks it and priority selects an interrupt. | `pic.c`: source handles update `irr`; `core_machine_pic_select` resolves the same sole state; CPU first calls `scan`. | All three inspected implementations retain IRR/IMR/ISR and priority before CPU delivery. | Manual-L3; one PIC owner. |
| Normal acknowledge/vector | Intel p. 7: first 8086/88 INTA transfers the selected IRR bit to ISR; second supplies the vector. | CPU opens `CPU_INTERRUPT_ACKNOWLEDGE`; `core_machine_pic_get_interrupt` performs the first logical state transition and reserves the vector before `_e_intr_n` consumes it. | 86Box exposes both bus callbacks; Bochs and PCjs couple the same IRR-to-ISR/vector order to their CPUs. | Manual-L3 logical order.  No electrical pulse duration or elapsed placement is claimed. |
| Masked and nested request | Intel pp. 4--6, 9--11: mask and in-service priority gate the selected request; OCW2 can release/rotate it. | `core_machine_pic_select`, `RespondINTR`, and OCW2 parsing in `pic.c`; PIC command/priority smokes cover the variants. | 86Box and Bochs model the same selection/EOI relation; PCjs is corroboration only because it declares unsupported modes. | Manual-L3. |
| Cascade and slave vector | Intel pp. 6--8, 15; IBM 5170 pp. 1-10, 1-24: master/slave relation and both EOIs apply to a cascaded service. | `core_machine_pic_select`, `core_machine_pic_refresh`, and `core_machine_pic_get_interrupt` move master cascade and selected slave state through one path; selected profiles supply topology at construction. | 86Box recursively acknowledges the selected slave; Bochs uses selected PC/AT IRQ2 wiring; PCjs also consolidates slave propagation before CPU INTR. | Manual-L3 for selected topology; no profile-side delivery shim. |
| AEOI and explicit EOI | Intel pp. 7, 9--11, 15: AEOI releases ISR at acknowledge completion; otherwise EOI releases service, including slave then master for cascade. | `RespondINTR` applies AEOI; OCW2 owns explicit EOI and rotation. | 86Box and Bochs implement both forms; PCjs is not authoritative for omissions. | Manual-L3. |
| Initialization/reset and empty acknowledgement | Intel pp. 12--16: ICW sequence defines controller state; an acknowledge without a request produces the default level-7 result. | `core_machine_pic_begin_initialization`, `core_machine_pic_reset`, and initialized no-selection branch in `get_interrupt`; command/lifecycle tests cover it. | 86Box and Bochs reset the same controller state; PCjs documents a non-normative reset choice, so it cannot alter the Intel result. | Manual-L3. |
| CPU wait/visibility time | Intel documents causal order but does not map INT/INTA to NXVM elapsed ticks. | CPU observes PIC only at its execution boundary; no PIC loop or VM polling path advances state. | Each external project uses a framework-specific scheduler. | L2 only for elapsed visibility; receiver is the Core time-axis/board-timing work, not PIC or VM. |

## Gap disposition and proof

The direct read found no unaddressed PIC-to-CPU owner gap.  The previously
missing logical acknowledgement and default-IR7 cases are present in the one
PIC/CPU path, while separate electrical INTA waveforms and an elapsed
visibility quantum are neither inferred nor hidden by a sleep/polling path.
The retained focused proof is the PIC phase trace plus PIC lifecycle,
command-priority and OCW3 smokes.  During the required full-unit replay, the
first causal blocker was not a PIC workaround opportunity: DMA correctly owns
page-register port `80h`, while nine CPU timing fixtures still tried to install
a synthetic provider there.  They now use the already unowned test port `E0h`;
protected-I/O fixtures also move their TSS bitmap byte from `80h / 8` to
`E0h / 8`.  No DMA, PIC or CPU production path changed.

The final Model-40 blocker was likewise a stale assertion: the frozen
composition and dedicated FDC test both specify two 80-cylinder double-sided
drives with active-low Track0 inputs (`03h`), whereas the integration test
expected a zero Track0 mask.  The integration assertion now tests that same
single profile fact.  This repairs the affected controller/test chain without
adding a second topology or compatibility path.

Final verification was an isolated `ctest --test-dir build/mingw-gcc-x64 -L
unit -j 8 --output-on-failure` replay on 2026-08-29: **312/312 passed** in
34.02 seconds real time.  The result retains one PIC state and one CPU delivery
path.  A later controller source that asserts an IRQ must still be audited by
its own S; this conclusion does not prove its producer or deadline route.
