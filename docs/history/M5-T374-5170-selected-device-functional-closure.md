# M5 T374: IBM PC/AT 5170 Selected-Device Functional Closure

## Task Record

T374 repairs or explicitly removes the functional gaps of devices selected by
the locked IBM PC/AT 5170 Model 339/Type 3 profile. It follows T373's
capability/ownership ledger and precedes the Model-339 board/device phase
timing closure. It does not assign timing values or decide Model-339 L3
readiness.

## Accepted Work

T374 begins with a source-and-test inventory of the selected Model-339 device
surface. Later subtasks may repair only evidence-backed shared or
profile-selected mechanisms; every service duration, board wait, arbitration
or phase issue transfers unchanged to the queued timing candidate.

### S1: selected-device owner inventory

S1's [owner inventory](../etc/evidence/t374-s1-model339-functional-owner-inventory.md)
maps every Model-339 selected component to its actual core/profile/session
owner, observable route, focused test boundary, concrete functional gap and
later timing handoff. It excludes generic ATA/EGA/AUX and no-fixed-disk MFM
work without classifying any selected component complete.

### S2: functional contract and repair selection

S2's [functional contract matrix](../etc/evidence/t374-s2-model339-functional-contract-matrix.md)
compares each selected S1 owner with the Model-339 primary-document boundary.
It selects the missing 8042 command-port `F0h`--`FFh` output-port pulse reset
behavior as the first owner-local repair. The pulse duration remains timing
work and no other output-port bit gains an invented consumer.

### S3: 8042 output-port pulse reset

S3's [pulse-reset repair evidence](../etc/evidence/t374-s3-8042-pulse-reset-repair.md)
records the owner-local `F0h`--`FFh` command handling, preserving D1h's
persistent output-port/A20 route and assigning no pulse duration. Fresh KBC,
Model-339 composition and AUX regressions pass; other selected controller
functional gaps remain in T374.

### S4: FDC functional contract audit

S4's [FDC functional contract audit](../etc/evidence/t374-s4-fdc-functional-contract-audit.md)
reconciles the selected controller command/state surface against the uPD765
primary documentation and current FDC owner. It selects missing
Ready-transition attention IRQ / `Sense Interrupt Status` delivery as the next
bounded owner-local functional repair. It preserves disk-change versus Ready
distinction and transfers physical signal, DMA and drive timing unchanged.

### S5: FDC Ready-transition attention

S5's [Ready-attention repair evidence](../etc/evidence/t374-s5-fdc-ready-attention-repair.md)
records an owner-local 8272A/uPD765 Ready edge: `Specify` enables polling, an
edge reaches the existing IRQ6 / `Sense Interrupt Status` contract, and media
generation remains DIR disk-change only. Focused FDC and Model-339 topology
regressions pass. Controller/drive/board timing and remaining FDC commands
remain open in T374 and the later timing candidate.

### S6: FDC command-capability audit

S6's [command-capability audit](../etc/evidence/t374-s6-fdc-command-capability-audit.md)
classifies all fifteen 8272A commands and the current FDC dispatcher. It
selects removal of the over-admitted enhanced `VERSION` command as S7, while
recording Deleted/Scan and parameter/media gaps as explicit later T374 work.

### S7: FDC `VERSION` invalid correction

S7's [invalid-command repair evidence](../etc/evidence/t374-s7-fdc-version-invalid-repair.md)
records removal of the non-8272A `10h`/`90h` response and focused proof that
the existing invalid-command route now returns ST0 `80h` without IRQ. Remaining
documented command and media semantics remain T374 work.
