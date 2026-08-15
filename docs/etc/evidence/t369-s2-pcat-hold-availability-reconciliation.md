# T369 S2: IBM PC/AT HOLD And Availability Reconciliation

## Decision

S2 does not admit a Model 339 wait count or a runnable availability state. The
primary evidence is sufficient to state the 80286 local-bus contract, but not
to reduce the selected IBM board to a universal CPU-memory or CPU-port delay.
It also exposes a concrete repository mechanism gap: a CPU execution round
finishes every currently visible memory/port transaction synchronously before
the scheduler advances the DMA owner, so it has no transition at which a DMA
HOLD request can be observed, acknowledged, retained, or released.

The next T369 receiver is therefore S3: a bounded execution-round and
transaction-lifecycle design that must establish one shared, traceable logical
HOLD handoff without assigning an unproved duration. S3 may use the Intel
semantic boundary, but must keep all Model-339 wait/READY values and physical
waveform claims transferred until qualified evidence exists.

## Source-To-Signal Matrix

| Signal/family | Primary evidence | Determinate fact | Non-determinate fact | Current route and S2 disposition |
| --- | --- | --- | --- | --- |
| READY | [Intel 210760-002, local-bus chapter](https://www.dosdays.co.uk/media/intel/210760-002_80286_Hardware_Reference_Manual_1987.pdf) specifies READY as part of the CPU bus interface and shows zero-wait bus timing; [IBM 5170 Technical Reference, September 1985](https://minuszerodegrees.net/manuals/IBM_5170_Technical_Reference_6280070_SEP85.pdf) supplies the board/ISA topology. | READY may extend CPU bus activity; it is a local-bus condition, not an instruction scalar. | Which selected Model 339 memory, ROM, CGA, FDC or ISA access asserts it, for how many cycles, or how it maps to project ticks. | `cpu_instructions.c` completes checked physical/port access immediately. S3 must retain this as a source-undefined board wait transfer; no ready flag or tick is added. |
| HOLD/HLDA | Intel 210760-002, figures 3-63 through 3-67 and associated text. | CPU completes its current bus sequence, floats the bus and asserts HLDA; minimum entry/exit bounds are documented. A requester must not use the bus before HLDA and must release it before CPU resumption. | Maximum latency depends on executing software, current bus work, LOCK, interrupts and any wait states; Intel does not choose a Model 339 requester or elapsed-tick conversion. | `machine.c` runs DMA only from its post-retirement arbitration callback. S3 must create the logical request/acknowledgement lifetime at the existing transaction owner, without claiming the documented minimum as a selected duration. |
| Dual 8237A request/grant | IBM 5170 technical reference establishes the board's dual-DMA topology; Intel describes the generic DMA/HOLD use. | DMA is a valid class of local-bus requester and must wait for HLDA-style handoff. | A physical DREQ/DACK/AEN waveform, refresh cadence, arbitration latency, or a selected per-channel duration. | `dma.c` owns controller selection, preflight and transfer commit; `core_machine_arbitration_tick` gives its retained logical opportunity before PIT/PIC. S3 must preserve that owner and add no DMA waveform or duration. |
| FDC DRQ, DMA2 and IRQ6 | T366 locked the controller route, FDC DMA2/IRQ6 binding and field-upgrade boundary; IBM establishes the diskette-controller/topology context. | DRQ/service, DMA2 movement and IRQ6 visibility are distinct logical events. | TEAC mechanics, DRQ-to-HOLD timing, controller busy/ready duration and same-tick physical ordering. | `core_machine_readiness_tick` advances/refreshes FDC after arbitration. T370, not S2/S3, owns device service timing after an availability mechanism exists. |
| PIC acknowledgement / INTA | IBM establishes PIC/CPU hardware context; Intel identifies `COD/INTA` as a bus-cycle distinction. | Logical interrupt delivery/acknowledgement is not identical to a physical INTA sequence. | INTA bus cycles, spacing and waveform. | Existing CPU/PIC code owns logical acknowledgement. S3 may preserve its ordering only; physical INTA remains a later transfer. |
| 86Box, MAME and PCjs | S1 records their permitted reference boundary. Current 86Box notes validate 512 KB as the IBM AT on-board limit; MAME's base `ibm5170` uses 6 MHz/1664 KB defaults and its `ibm5170a` changes clock only; PCjs's checked configuration selects 8 MHz but 640 KB, dual 1.2 MB drives and an HDC. | They can expose a qualitative emulator behavior to be compared after a primary/board rule exists. | An authoritative Model 339 wait, HOLD or service value; any one of their incompatible defaults. | No secondary value is accepted. The local sibling PCjs read remains a non-imported research observation and is not a project dependency. |

## Production-Mechanism Reconciliation

`core_machine_run` executes CPU work and publishes a source-cost retirement
tick; `core_machine_advance_scheduler` then runs due callbacks. The first
callback advances DMA, followed by PIT and PIC. CPU memory/port helpers begin,
commit or cancel the shared transaction while executing that CPU round. DMA
begins, commits or cancels the same owner only later in the arbitration
callback. The state rejects an overlapping begin, but this is an error guard,
not an observable HOLD request/HLDA acknowledgement:

1. no DMA request is represented while the CPU owns a transaction;
2. no CPU execution boundary carries a pending DMA reservation into the next
   CPU round; and
3. trace has CPU/DMA begin/commit/cancel, but no request/acknowledgement/release
   event to prove the required handoff.

S3 must repair this one shared mechanism, not add device-local delays or a
parallel scheduler. It must validate the requester before publication, avoid
modifying stopped/paused APIs, cancel pending state on reset, preserve source
instruction timing ownership, and expose copied trace facts. Its design must
state exactly what remains a logical handoff rather than a physical duration.

## Similar-Issue Sweep

The S2 sweep rechecked all `HOLD`, `HLDA`, `READY`, `core_machine_transaction_*`,
DMA advance, CPU port/memory helper, PIC refresh/acknowledgement, FDC advance/
refresh and Model-339 descriptor routes identified by S1. No other production
availability owner exists. The external `core_machine_bus_*` and stopped/paused
memory/port APIs remain excluded: they are lifecycle-validated configuration or
debug operations outside executing guest time.

## Exact Transfers

- Model-339 memory, ROM, CGA, ISA and port-specific READY/wait values;
- maximum HOLD latency, because it is software/LOCK/interrupt and
  board-wait-dependent rather than a fixed CPU number;
- physical DREQ/DACK/AEN/HOLD/HLDA/INTA waveforms and refresh cadence; and
- FDC/TEAC controller, mechanical and DRQ service duration, which remains T370.

These transfers prevent S3 from turning the CPU's documented minimum bound or
an incompatible reference emulator into an L3 timing claim.
