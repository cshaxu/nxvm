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

### S8: FDC Deleted-Data media contract audit

S8's [Deleted-Data media-contract audit](../etc/evidence/t374-s8-fdc-deleted-media-contract-audit.md)
establishes that an 8272A deleted-data address mark is metadata distinct from
payload bytes.  It inventories every production and fixture provider, records
the raw-image persistence limit, and assigns a shared media ABI receiver before
any FDC command route can truthfully be added.  No synthetic disk encoding,
functional-completion or L3 claim is made.

### S9: shared media address-mark contract

S9's [media address-mark contract evidence](../etc/evidence/t374-s9-media-address-mark-contract.md)
adds a provider-owned normal/deleted logical-sector metadata capability,
validated query/set operations and complete initializer migration.  Byte-only
raw FDD/HDD images truthfully remain unsupported; a later FDC receiver may use
the shared contract but must not invent raw-image persistence.

### S10: raw-IMG address-mark support boundary

S10's [raw-IMG address-mark boundary](../etc/evidence/t374-s10-raw-img-address-mark-boundary.md)
records the owner decision to retain ordinary `.img` as the sole floppy
exchange format and to admit neither a private container nor a sidecar. The
Deleted Data and normal-read Control Mark form an explicit raw-media boundary;
the independently unfinished Scan family remains deferred. Ordinary BIOS/DOS
raw-sector behavior is unchanged. This does not close the FDC,
selected-device, timing or Model-339 L3 work.

### S11: Model-339 CGA functional audit

S11's [CGA functional audit](../etc/evidence/t374-s11-model339-cga-functional-audit.md)
classifies the selected digital-CGA VRAM, snapshot, port, reset and firmware
consumer surface. It selects the incorrect read visibility of write-only CGA
ports as S12's bounded repair, while transferring full 6845 geometry and all
timing to later receivers.

### S12: CGA write-only port repair

S12's [write-only port repair evidence](../etc/evidence/t374-s12-cga-write-only-port-repair.md)
removes invalid Model-339 reads of CGA `3D4h`, `3D8h` and `3D9h`, retains
permitted `3D5h`/`3DAh` behavior, and preserves the unrelated EGA Mode/Color
read route through EGA-only registration. Full 6845 and timing work remains
open.

### S13: CGA 6845 register audit

S13's [6845 register audit](../etc/evidence/t374-s13-cga-6845-register-audit.md)
classifies every CGA CRTC register: retained cursor/start routes are present,
geometry/sync registers transfer to display timing, and light pen remains an
unselected input path. It selects no synthetic register repair.

### S14: CGA/EGA CRTC index gate

S14's [index-gate evidence](../etc/evidence/t374-s14-cga-ega-crtc-index-gate.md)
rejects EGA-only CRTC `13h` for Model-339 CGA while retaining the EGA offset
route. Geometry, light pen and timing remain open.

### S15: platform-controller functional audit

S15's [platform-controller functional audit](../etc/evidence/t374-s15-model339-platform-controller-functional-audit.md)
classifies the selected PIC, PIT, DMA and RTC/CMOS register, reset,
IRQ/DRQ and firmware-consumer routes without assigning service timing. Its
focused controller replay passes, while the generic default-PC/AT composition
smoke exposes a VADP profile-leaf direction contradiction introduced by the
accepted S12 preservation claim. The next bounded corrective S must reconcile
that generic EGA descriptor/handler assertion; 5170 timing and L3 remain open.

### S16: default-EGA VADP direction corrective repair

S16's [direction-repair evidence](../etc/evidence/t374-s16-default-ega-vadp-direction-repair.md)
changes only the default-PC/AT declaration for write-only CRTC-index port
`3D4h`, bringing it into agreement with the retained VADP registration. The
formerly failing default-PC/AT composition test and focused Model-339/EGA
regressions pass. It adds no display capability or timing claim.

### S17: 8042 and 101-key keyboard functional audit

S17's [8042/keyboard audit](../etc/evidence/t374-s17-model339-kbc-keyboard-functional-audit.md)
classifies the selected command, FIFO, reset, IRQ1 and firmware-consumer
routes, excludes AUX/IRQ12, and selects native Set-2 versus 8042 Set-1
translation as the remaining concrete functional mechanism. Timing remains
transferred.

### S18: native keyboard input and 8042 translation

S18's [native-input boundary evidence](../etc/evidence/t374-s18-native-keyboard-input-boundary.md)
replaces generic direct scan-code submission with a real native-keyboard-byte
production operation. The VM host adapter emits Set 2; the KBC owns Set-2 to
Set-1 translation, FIFO and IRQ1. It covers supported Set 1/2 selection,
ordinary/extended make-break and the exact Pause stream. Timing, AUX/IRQ12,
Set 3 and nonstandard streams remain excluded.
