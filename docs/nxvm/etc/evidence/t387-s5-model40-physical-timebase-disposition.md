# T387 S5: Model-40 Physical-Timebase Disposition

`M5:T387:S5:PORT61-PIT-RECONCILIATION:OK`

## Primary Board Reconciliation

Compaq's *D3PE (80386) Processor Board Circuit Descriptions* (5 January 1987),`nwhose source provenance is retained in T384 S1, identifies system
timer counter 0 at `40h`, refresh-request timer counter 1 at `41h`, and speaker
timer counter 2 at `42h`. Its port-`61h` register definition is composite:
bits 0--3 are writable speaker/failsafe/I/O-channel controls; bit 4 is the
read-only refresh-detect state from timer counter 1; bit 5 is the read-only
timer-counter-2 output; bits 6--7 are the I/O-channel-error and failsafe
states. Thus the D4 NMI controls established in T386 and the counter-1
observation established in the shared PC/AT port-B owner coexist; neither
overrides the other.

The same corpus supplies the physical board facts: a 16 MHz CPU-related
reference, a 1.19318 MHz TIMCLK for both 8254 blocks, DCLK at one half of
BCLK, and refresh counter divisors 18 or 19. These facts identify hardware
domains but do not prove the unit of NXVM's shared elapsed-time axis.

## Actual Timebase Root Cause

`core_machine_publish_elapsed_ticks()` is the only successful-retirement
publisher and advances every scheduled device clock from the published value.
The selected 80386 source classifiers return source-backed values for admitted
forms, but `CORE_MACHINE_SOURCE_UNALLOCATED_TICKS` remains one for successful
prefix/default/unsupported contexts. This is the same mechanism previously
recorded for the 80286 in T366 S9 and T368 S7: a one-tick sentinel is explicit
deterministic progress, not an Intel or Compaq oscillator-cycle claim.

Consequently, a Model-40 `{ 596591, 8000000 }` PIT ratio would be a valid
mathematical quotient only if every incoming elapsed tick were one 16 MHz CPU
cycle. The present graph cannot establish that premise. Applying the ratio
would misclock every successful unallocated 80386 path; retaining the Model-40
neutral plan is therefore correct.

## Virtual-Time Receiver Disposition

The existing VM virtual-time source is called only while the CPU is waiting
and publishes batches through `core_machine_advance_time()`. The Model-339
binding is explicitly host pacing during HLT, not an IBM board-duration fact.
It neither repairs successful-retirement units during active CPU execution nor
creates a source-backed Model-40 CPU-to-PIT conversion. Installing it at
16 MHz would therefore hide, rather than repair, the same mixed-unit defect.
No Core or VM code is admitted in S5.

## Similar-Issue Sweep And Transfer

The sweep examined the sole publisher, all four profile classifiers, the
clock-domain plan, Model-339 virtual-time binding, both Model-40 private
constructions, T366 S9/S10, T368 S7, T375 S17, T386 S25, and T387 S1--S4.
The common defect class is successful-retirement sentinel time entering a
clock-domain receiver. The 80286 and 80386 profiles are direct affected
consumers; 8086 and 80186 retain the same shared publisher and are included in
the new four-profile receiver.

The new queued [four-profile CPU physical-timebase closure](../../history/M5-T388-four-profile-cpu-physical-timebase-closure-proposal.md)
owns the missing shared prerequisite. It must establish a clock-eligible unit
or prohibit every unallocated successful route before any selected profile
can convert CPU execution to a physical board clock. T387 retains all
independent board/device timing work, but transfers CPU-to-PIT calibration,
DCLK placement and physical-time/L3 conclusions until that prerequisite
closes. No 86Box, MAME, PCjs, ROM, media, host measurement, or generic-AT
substitution was used to allocate a scalar.