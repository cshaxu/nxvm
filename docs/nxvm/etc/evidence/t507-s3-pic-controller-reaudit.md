# T507 S3: Intel 8259A PIC Controller Re-audit

M5:T507:S3:PIC-CONTROLLER-REAUDIT:OK

## Source and comparison boundary

The primary source is Intel 231468-003, *8259A Programmable Interrupt
Controller* (December 1988), visually reviewed at the initialization,
operation-command, cascade and timing figures. The PDF is text-native enough
for navigation, but visual pages were used to verify command-bit diagrams and
the 8086/8088 two-INTA description. Local 86Box and Bochs PIC sources were
read only to cross-check state-machine consequences; they are not copied.

The manual gives electrical INTA/CAS setup and valid-data values. NXVM has no
source-backed mapping of those values to the Core elapsed-tick axis, so those
numbers remain an explicit board/physical L2 boundary rather than guessed
deadline data.

## List 1 and List 2

| ID | Required semantic or timing fact | Current sole owner and proof | Disposition |
| --- | --- | --- | --- |
| P1 | ICW1 starts initialization, clears transient request/service state and resets edge sense. | PIC command owner clears transient registers; command-priority regression verifies reinitialization. | Manual L3 |
| P2 | ICW2--ICW4 sequence selects vector, cascade, AEOI/SFNM and 8086 mode. | PIC data-port state machine; command-priority and OCW3 regressions. | Manual L3 |
| P3 | IRR, IMR, ISR and rotating priority select only an eligible request. | PIC selector and priority rank; command-priority regression. | Manual L3 |
| P4 | OCW2 non-specific/specific EOI and rotation mutate only the selected in-service state. | PIC command owner; command-priority regression. | Manual L3 |
| P5 | OCW3 selects IRR/ISR reads, special mask and one poll operation. | PIC command/read owner; OCW3 regression. | Manual L3 |
| P6 | A poll read acknowledges its selected controller request and reports its level. | PIC command/read owner; OCW3 regression. | Manual L3 |
| P7 | A programmed eligible slave request is immediately visible through its declared master cascade line; acknowledge and EOIs remain controller operations. | PIC cascade derivation and CPU route; command-priority, OCW3 and PIC phase regressions. | Manual L3 |
| P8 | Edge requests latch; level requests remain held while an asserted source remains and withdraw at its last deassertion. | PIC IRQ-source reference count and refresh; IRQ lifecycle and command-priority regressions. | Manual L3 |
| P9 | CPU interrupt acknowledgement transfers the chosen request to ISR and returns the programmed vector. | PIC acknowledge plus CPU binding; PIC phase and CPU/PIC lifecycle regressions. | Manual L3 |
| P10 | Reset removes transient request, service and derived cascade state. | PIC reset owner; lifecycle and command-priority regressions. | Manual L3 |
| P11 | PIT, RTC, KBC/XT keyboard, FDC and HDC publish IRQs only through a bound PIC source. | Each device owns its source; PIC owns IRR/cascade and CPU delivery. | Manual L3 function |
| P12 | INTA/CAS pin pulse delays and board propagation have no admitted Core physical-axis conversion. | No producer falsely returns a PIC deadline. | L2 boundary |

## Complete repair

The scheduler retained a stale L1 compatibility predicate for a state where a
slave IRR was nonzero but its master cascade request had not been published.
That state was the pre-T505 lag: current production PIC source and command
entries already refresh the sole cascade state immediately, while reset clears
it directly. The predicate was therefore a duplicate scheduler path, not a
real receiver. S3 deletes it from fast-advance blocking and Turbo L1
eligibility. No PIC state, API, timer, device route or host-to-Core tick route
was added.

## Verification and simplicity

Focused registered PIC tests pass 6/6 after the deletion. They cover command
priority, OCW3/poll/special-mask/SFNM, source lifecycle, reset, CPU binding
and interrupt entry phase. Complete repository-only unit passes 312/312.
The tracked production delta is -8 lines in the
single scheduler owner; no test source was required because the owner-local
PIC tests already prove the immediate publication invariant.
