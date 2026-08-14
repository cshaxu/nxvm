# T354 S3: CPU/DMA and controller-service competition

## Decision

T354 S2 made selected CPU and DMA memory/port activity synchronous transactions.
S3 does not add a guessed bus duration or suspend an instruction midway.  The
existing core executor remains atomic.  Its one deterministic guest-tick
ordering is the competition rule for the selected scope:

1. the CPU instruction's checked transaction has already committed or
   cancelled before `CORE_MACHINE_TRACE_CPU_RETIRE`;
2. the due arbitration callback grants at most the existing DMA service,
   including the request/priority/cascade/EOP state machine, then advances PIT
   and refreshes the PIC;
3. the equal-due readiness callback advances then refreshes FDC, then advances
   then refreshes ATA/HDC; and
4. a PIC source made visible by a later refresh is eligible when the CPU next
   performs its existing interrupt scan.  Successful CPU interrupt delivery
   acknowledges the selected PIC request through `core_machine_pic_get_interrupt`.

This is a deterministic visibility and ownership rule, not a claim for
HOLD/HLDA, DACK, INTA, wait-state, or sub-instruction waveform accuracy.

## Owner and boundary sweep

| Requester / state | Validation and owner | Commit / visibility rule | Evidence and disposition |
| --- | --- | --- | --- |
| CPU physical-memory and port action | S2 transaction owner around checked memory/port paths | Commit or cancel completes before CPU retirement | S2 owner smoke and S3 competition trace; no instruction continuation is introduced. |
| 8237A ordinary and M2M DMA | Existing request binding, priority/cascade, EOP and checked transfer state in `dma.c` | The arbitration callback services the retained one grant before PIT/PIC; S2 transaction records the selected checked memory portion | S2 transaction and retained DMA-channel proofs; no second bus owner. |
| PIT / PIC | Existing clock-domain advance and PIC refresh | PIT advance follows DMA service; PIC refresh follows PIT advance | S3 trace proof; controller command semantics are retained. |
| FDC DRQ and completion | FDC service and direct bound DMA request callbacks | FDC advance/refresh occur after arbitration.  A newly asserted DRQ is consequently eligible on a later arbitration callback. | Existing FDC/DMA boundary smoke plus S3 event order; no invented same-tick transfer. |
| ATA/HDC PIO readiness | Existing HDC service/refresh owner | HDC advance/refresh follow FDC in readiness order; ATA remains retained PIO, not a fabricated DMA requester | Existing storage/timeline evidence and S3 trace order. |
| PIC acknowledgement | CPU `ExecInt` after an instruction and PIC selection | A successful architectural delivery consumes the selected PIC request; physical INTA waveform is outside scope | Existing PIC/interrupt owner tests; S3 records the exact logical boundary only. |

## Focused proof

`core-machine-competition-s3-smoke` runs a reset-vector NOP while an asserted,
bound primary-DMA channel 2 has one byte ready.  Its trace proves, in order:

`CPU memory begin < CPU memory commit < CPU retire < DMA memory write begin <
DMA memory write commit < DMA advance < PIT advance < PIC refresh < FDC advance
< FDC refresh < HDC advance < HDC refresh`.

The smoke also reads the DMA destination and proves that the one byte published
only through the DMA transaction.  It deliberately uses no time value other
than the already selected guest tick.

## Explicit transfers

- Physical bus pins, DMA cycle steal/wait duration, instruction-internal CPU
  suspension, and INTA/HOLD/DACK waveforms remain outside selected L3.
- A device that needs same-instruction or same-refresh availability requires a
  separately admitted mechanism; it must not add a per-device delay or bypass
  the transaction owner.
- S4 receives reset/finalize/trace-consumer closure and the final selected-L3
  audit.
