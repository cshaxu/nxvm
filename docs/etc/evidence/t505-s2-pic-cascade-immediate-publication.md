# T505 S2: PIC Cascade Immediate Publication

## Retained mechanism

S1 establishes that a paired slave request is an immediate ordered 8259A
relation, not an elapsed-Core-tick delay.  `t_pic` now retains its
construction-fixed master/slave pair solely so either existing PIC mutation
entry can call the existing `core_machine_pic_refresh()` calculation.  The
pair contains no request, vector, clock, profile, or VM state; `cascade_irr`
remains the sole derived state and is still calculated once in `pic.c`.

The existing source assertion/deassertion boundary and both PIC port-write
paths refresh that calculation immediately.  This covers source arrival and
level withdrawal, ICW topology programming, IMR mask/unmask, EOI/priority and
special-mask command changes.  Acknowledge retains its existing refresh after
the paired IRR-to-ISR transfer.  Thus the scheduler sees no transient
"slave IRR without master cascade" state from a production PIC transition and
does not classify this route as T504 L1 compatibility work.

## Complete selected-route proof

The retained command-priority smoke now proves, without advancing Core time:

- edge-triggered IRQ14 immediately publishes master IR2, and slave IMR
  mask/unmask immediately withdraws and republishes it;
- reset clears both the derived master request and slave IRR;
- level-triggered IRQ14 withdrawal immediately clears master IR2 and leaves no
  deliverable cascade request.

Existing PIC lifecycle, CPU/PIC delivery, programmed-ICW3, EOI and single-PIC
tests remain the selected sweep.  S1's Intel source and external-model result
remain the provenance; this S neither derives nor claims electrical timing.

## Simplicity and boundary result

Tracked code/test paths: `pic.[ch]` and one owner-local PIC smoke.  The change
adds a construction-fixed relationship and one small refresh helper, replacing
the prior scheduler-only publication lag; it adds no duplicate selector,
dispatcher, scheduler policy, compatibility path, Core-to-VM interface or
profile behavior.  The retained production path is:

`source or PIC port write -> PIC refresh -> existing selection/CPU delivery`.

The tracked implementation changes are +18 source lines (`pic.[ch]`) and +35
owner-local test lines, net +53.  The complete incremental build and
repository-only unit replay passed 312/312 with `-j 8`; documentation
governance passed.  No remaining T505 PIC cascade receiver is transferred.
