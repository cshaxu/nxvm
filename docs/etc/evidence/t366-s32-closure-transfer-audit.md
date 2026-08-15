# T366 S32: Bus-Timed PC/AT Closure And Transfer Audit

## Decision

T366 is closed as a bounded preparatory and source-retirement task.  It does
**not** close IBM PC/AT 5170 model-L3 fidelity.  The selected Model 339 Type 3
baseline is locked, and its selected planar-parity/NMI and topology work is
durably evidenced, but source-labelled CPU completion, bus availability and
device service timing remain prerequisite work.

## Accepted evidence

| Area | Accepted T366 result | Durable evidence |
| --- | --- | --- |
| Baseline identity | IBM 5170 Model 339 Type 3, 8 MHz 80286, 512 KB planar RAM, Rev.3 ROM slots, 101-key keyboard, CGA, no fixed disk, and the TEAC 1.44 MB drive only as a field upgrade. | [S2 profile lock](t366-s2-5170-profile-lock.md), S5--S7 history. |
| NMI and planar memory | One opt-in planar-RAM parity producer/latch/clear path is bound to selected 512 KB planar memory; CMOS masks and does not produce NMI.  I/O-channel check and adapter parity remain unselected. | [S3 NMI lifecycle](t366-s3-planar-parity-nmi.md), S4 history. |
| Device/topology pre-audit | The selected Model 339 session isolates ATA/HDC, selects CGA and declares the abstract Rev.3 firmware slot, FDC ports, IRQ6, DMA2 and field-upgrade drive. | S5--S7 history and [S7 topology](t366-s7-model-339-firmware-fdc-topology.md). |
| CPU retirement rows | S11--S31 add source-backed successful 80286 rows, including Group-2, FLAGS/XLAT/LAHF/SAHF, segment moves/stack, BOUND, ARPL, selector queries, MSW and table-selector stores. | T366 S11--S31 history and indexed timing evidence. |
| Honest non-admissions | No uPD765 clock conversion or global physical CPU clock was invented.  The unallocated successful-retirement publisher inventory is retained. | [S8 FDC non-admission](t366-s8-fdc-service-timing-nonadmission.md), [S9 physical-time non-admission](t366-s9-physical-time-nonadmission.md), [S10 publisher inventory](t366-s10-80286-unallocated-publisher-inventory.md). |

## Transfers

| Unfinished requirement | Exact receiver | Reason it remains open |
| --- | --- | --- |
| Complete 80286 successful-retirement source ledger, including prefixed and default/unallocated forms and all remaining system/control families | [80286 successful-retirement timing closure](../../proposals/m5-80286-retirement-timing-closure.md), using the shared [four-profile CPU-completeness program](../../proposals/m5-four-profile-cpu-completeness-program.md). | S9/S10 prove `elapsed_ticks` still receives successful unallocated values; the accepted rows do not define a global physical 8 MHz clock. |
| CPU memory/I/O waits, READY, DMA HOLD/HLDA-style exclusion, arbitration, INTA distinction, and transaction availability/cancellation trace | T369 closes logical HOLD/transaction availability; physical waits and INTA distinctions transfer to [selected-profile phase refinement](../../proposals/m5-cycle-exact-selected-profile.md). [Retained T369 proposal](../../history/M5-T369-bus-timed-pcat-operation-proposal.md). | T366 records no selected physical bus availability contract or numeric wait-state source. |
| PIT/PIC/RTC/DMA/KBC/PPI/NMI/CGA/FDC busy/ready, DRQ/IRQ, reset/cancel and command-service timing; uPD765 clock-domain conversion | [PC/AT device service-timing corpus](../../proposals/m5-device-service-timing-corpus.md), after its bus prerequisite. | S8 explicitly prohibits translating controller milliseconds through retirement ticks. |
| CPU prefetch, bus-phase and device-microstate representation for the fixed selected profile | [Selected-profile model-L3 phase refinement](../../proposals/m5-cycle-exact-selected-profile.md), after bus and device service timing. | No complete source-labelled CPU/bus/device phase model exists. |
| Component-by-component 5170 ready/not-ready decision | [M5 L3 machine closure audit](../../proposals/m5-l3-machine-closure-audit.md), after the preceding work. | This audit cannot substitute for implementation and must decide readiness from its full evidence matrix. |
| IBM MFM/ST-506 fixed disk / controller route | [TODO: IBM 5170 MFM/ST-506 fixed-disk route](../../states/TODO.md#hardware-and-compatibility-debt). | Model 339's selected baseline deliberately has no fixed disk; current ATA/HDC is not IBM MFM. |

## Closure conclusion

The factual result is **5170 model-L3: not ready**.  T366 establishes a
source-labelled baseline and removes selected ambiguities, but it neither
allocates bus/device timing nor demonstrates the complete CPU ledger required
to give project elapsed ticks physical-clock meaning.  The Queue's later
device, phase-refinement and L3-audit candidates remain ordered prerequisites;
DeskPro 386, 8088/5150/XT and Windows work are not advanced by this closure.
