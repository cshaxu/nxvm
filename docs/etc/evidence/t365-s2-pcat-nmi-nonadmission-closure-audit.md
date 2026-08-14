# T365 S2: PC/AT NMI Non-Admission Closure Audit

## Decision

T365 closes with a truthful non-admission. The selected PC/AT profile lacks a
documented, observable parity or I/O-channel-check producer contract. Its
absence remains a single T365/TODO receiver and is a prerequisite blocker for
physical bus/cycle-exact L3 closure.

## Receiver audit

The profile exposes no parity/I/O-check route or state. CMOS `70h` bit 7 owns
only `flagMaskNMI`; RTC owns IRQ8; PIC owns IRQ routing; and CPU `flagNMI` is a
delivery input exercised directly only by delivery tests. None is a board NMI
producer. Port `61h` remains the separate speaker/PPI TODO and cannot be used
as an implicit NMI status register.

The IBM PC/AT source describes the board signals, but without a selected
profile source, assertion/clear/latch/reset/timeline contract, modeling a
producer would be speculative. The Queue's later bus/service work may not
silently close this receiver.

## Verification

The S2 sweep rechecked all source, profile, composition, delivery, mask,
PIC/RTC, test, Queue and TODO hits named in S1. Documentation governance and
`git diff --check` passed. No runnable path or artifact changed.
