# M5 T371: Selected-Profile Model-L3 Phase Refinement

## Task Record

T371 receives T369's physical bus/availability transfers and T370's selected
device-duration transfers for the IBM PC/AT 5170 Model-339 baseline. It must
establish a source-labelled model phase representation before any 5170 L3 audit.

## Active Progress

### S1: phase owner and source ledger

S1 inventories every selected CPU, transaction, bus, device, trace and reset
boundary before selecting a phase scalar or behavior. Its [ledger](../etc/evidence/t371-s1-model339-phase-owner-ledger.md)
retains deterministic ordering but transfers every unqualified physical mapping
to a bounded later S.

S1 is accepted at `e230aceb`. S2 receives CPU retirement/prefetch and
memory/I/O phase vocabulary with the same no-guessed-value constraint.

### S2: CPU and bus phase boundary

S2 retains Appendix-B retirement facts and the one transaction vocabulary while
rejecting a prefetch or board-wait conversion. Its [boundary record](../etc/evidence/t371-s2-cpu-bus-phase-boundary.md)
assigns DMA/PIC/INTA composition to S3.

### S3: DMA/PIC phase composition

S3 retains the single arbitration order and logical controller lifecycle while
transferring physical waveforms and device-clock work to S4.

### S4: selected device phase boundaries

S4 retains device callback/lifecycle order and transfers all controller/board
clock conversions to S5's replay and task transfer audit.

### S5: replay and transfer audit

S5 composes retained timeline, transaction, competition and HOLD replay proof
and transfers every physical boundary to the 5170 L3 closure audit.

S5 is accepted at `d8c81c2b`. T371 is ready only to transfer its complete
logical-phase evidence and residual physical gaps to that audit.

S4 is accepted at `a2a28b2b`. S5 receives deterministic replay, copied trace,
reset/cancellation and phase-transfer audit only.

S3 is accepted at `9c41623f`. S4 receives FDC, RTC, KBC, planar-parity and
CGA/VADP device-clock and microstate boundaries.

S2 is accepted at `f0d7aeb5`. No source-supported Model-339 wait or prefetch
value was found; S3 receives only logical DMA/PIC/INTA composition.
