# T505 S1: PIC Cascade Observable-Progress Ledger

## Scope, sources, and method

This is the finite convergence ledger for the selected 8259A cascade route:
an already asserted slave request, its publication at the paired master input,
and the resulting CPU-visible interrupt.  It consumes neither arbitrary
multi-slave wiring nor electrical bus simulation.

The normative source is Intel *8259A Programmable Interrupt Controller*,
order `231468-003` (December 1988), visually checked against the supplied PDF:
the cascade description and sequence identify the slave `INT` to master `IR`,
master CAS selection, and slave vector response; ICW1/ICW3 define single versus
cascaded operation and the paired identity; the application discussion requires
the corresponding master/slave EOI handling.  The document's INTA/CAS pin
waveforms provide electrical values, but no mapping from those values to the
current Core elapsed-tick axis.  They therefore do not authorize a Core delay.

86Box `src/pic.c`, Bochs `iodev/pic.cc`, and PCjs
`machines/pcx86/modules/v2/chipset.js` were read as non-normative behavioral
corroboration.  Each recomputes/propagates master visibility when the slave
request, masks, or EOI state changes; Bochs directly raises master IRQ2 from
its slave service routine.  PCjs also contains BIOS-compatibility instruction
delays, so those values are explicitly rejected as timing evidence.  No local
MAME or QEMU checkout was available in this S; their absence contributes no
positive or negative disposition.

The current code trace covers `pic.c`, `machine_scheduler.c`, the CPU PIC
binding, and all production PIC-source users (PIT, RTC, KBC, XT keyboard, FDC
and HDC).  Direct test field mutation is test-only and is not a production
request path.

## List 1 and List 2 convergence ledger

| ID | Source-qualified required relation (List 1) | Current owner/path (List 2) | Disposition and receiver |
| --- | --- | --- | --- |
| C1 | ICW1 `SNGL=0`, master ICW3 declaration, and slave ICW3 identity must agree before a paired cascade can exist. | `core_machine_pic_cascade_line()` is shared by selection, SFNM and refresh. | Accepted Manual L3 function. |
| C2 | A slave source assertion changes only the selected slave controller's request state; it may be edge-latched or level-held according to that controller. | `core_machine_pic_irq_source_assert()` resolves IRQ 8--15 to the slave and updates its sole IRR/asserted state. | Accepted; source owner is retained. |
| C3 | An eligible slave request drives the declared master cascade input as a logical connection.  The source gives causal order, not a Core-tick duration. | `core_machine_pic_refresh()` derives the sole `cascade_irr` from selected slave eligibility, but scheduler arbitration is its normal publication site. | **Gap: immediate ordered PIC work.** S2 must publish/withdraw through the existing PIC owner when a device source changes; no deadline or compatibility tick. |
| C4 | A masked, lower-priority, or in-service slave request must not make a deliverable master cascade request; an EOI/unmask can make it eligible again. | PIC selection already applies IMR/ISR/priority; refresh derives only from that selection. | Accepted semantic predicate; S2 must include command-state regression with C3's immediate publication. |
| C5 | The CPU-visible request is selected through the master cascade input, then the selected slave supplies the vector during interrupt acknowledge. | `core_machine_pic_select()`, `core_machine_pic_peek_interrupt()` and `core_machine_pic_get_interrupt()` retain this route. | Accepted Manual L3 function; no CPU or VM alternate route. |
| C6 | Withdrawal of the last level-held eligible slave request removes the master cascade request; edge-latched requests retain normal PIC semantics. | Source deassertion updates only the slave currently; `core_machine_pic_refresh()` later recomputes master `cascade_irr`. | **Gap: same S2 immediate ordered publication/withdrawal receiver.** |
| C7 | A cascade acknowledgment transfers the master cascade request and selected slave request to their respective ISR state; master and slave EOI remain controller commands. | `core_machine_pic_get_interrupt()` acknowledges both and refreshes; OCW2 retains per-controller EOI. | Accepted. S2 regression must prove no stale re-publication after acknowledge/EOI. |
| C8 | Initialization/reinitialization and reset clear transient request/cascade state before later programmed operation. | `core_machine_pic_reset()` and ICW1 handling clear controller state including `cascade_irr`. | Accepted. S2 regression includes reset while a slave source is asserted. |
| C9 | The selected PC/AT-style consumers use the cascaded topology; selected XT single-PIC topology has no slave route. | Machine construction freezes topology; production sources bind to the one PIC request interface. | Accepted/N/A by topology. S2 sweeps IRQ8/12/14 and single-PIC non-participation. |
| C10 | Electrical INTA/CAS timing values remain at the pin/board boundary until a verified Core physical-time axis and board contract exist. | No such Core mapping exists. | Explicitly outside this T; no inferred L2 ratio and no L1 compatibility use. |

## Required S2 correction boundary

The correction is one PIC-local behavior: after a bound source changes the
slave's request state, refresh the already owned paired cascade relation before
the scheduler publishes a time observation or CPU delivery decision.  The same
owner must apply the corresponding withdrawal for level-triggered sources.
This replaces the scheduler's T504 L1 classification for this route; it does
not add PIC state, a second dispatcher, a profile hook, a host clock path, or a
guessed delay.

The S2 similar-issue sweep must cover source assert/deassert, source-independent
PIC command transitions, selection, acknowledgment, EOI, reset, single-PIC
topology, and every selected production consumer.  The focused proof must
assert immediate cascade visibility and withdrawal without advancing guest
time; complete repository-only unit and documentation gates remain required.

## S1 verification

The manual was rendered and visually checked at its cascade/priority and
timing figures; extracted text was used only as navigation.  The source and
owner sweeps used tracked `src/` and `test/` paths for PIC requests, cascade
state and scheduler observation.  The complete repository-only unit replay
passed 312/312 with `-j 8`; the documentation-governance target passed.
