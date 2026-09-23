# T365 S1: PC/AT NMI Source Decision

## Decision

T365 S1 does not select or implement an NMI producer. IBM's PC/AT Technical
Reference identifies parity and I/O-channel check as board NMI sources, while
the selected profile has neither source input, status/latch/clear owner nor
port declaration. The CMOS `70h` bit-7 NMI mask is not a producer. Selecting
either source now would fabricate board state.

## Source and path reconciliation

| Candidate or boundary | Current result | Disposition |
| --- | --- | --- |
| Memory parity | Documented board NMI source, but no parity RAM/model, latch, status or profile input exists. | Non-admitted; retain T365 receiver until a selected board contract supplies all lifecycle state. |
| I/O-channel check | Documented NMI source for uncorrectable adapter memory parity, but no selected adapter, status port, clear operation or profile route exists. | Non-admitted; do not use a synthetic flag or port `61h`. |
| CMOS `70h` bit 7 | `machine.c` maps it only to `core_machine_set_nmi_mask`; VM composition configures `80h`. | Retain mask-only owner. |
| RTC/PIC/device IRQ routes | RTC produces IRQ8 and selected devices use PIC sources. | Not NMI producers. |
| CPU `flagNMI` and `ExecInt` | Existing real/protected/VM86 delivery consumer clears a pending input after accepted delivery. | Delivery proof only; no board-source ownership. |

## Required future admission

Before a producer can be selected, the profile must name one IBM PC/AT board
source plus assertion, observable status/latch, clear/deassert, NMI-mask
interaction, reset/finalize, deterministic timeline visibility and delivery
consumer proof. The incomplete source remains the existing T365/TODO receiver;
it does not permit bus-timed or cycle-exact L3 closure.

## Verification

The sweep covered all tracked NMI/parity/channel-check/port-61, CPU delivery,
CMOS mask, profile, reset and timeline paths. It confirms no selected producer
or secondary timing path exists. No runnable path changed; documentation
governance and diff check pass.
