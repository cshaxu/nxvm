# T426 S1: DeskPro Physical/L3 Re-audit

## Decision

The selected DeskPro 386 Model 40 is **not accepted for physical/L3 timing**.
The deterministic logical profile and every completed lower-tier mechanism
remain valid. The decision rejects only the unsupported physical claim.

| Receiver | Owner and completed evidence | Tier | Remaining acceptance blocker |
| --- | --- | --- | --- |
| CPU-to-board | Core transaction/external-cycle/retirement owner; T423 completion wait. | original partial | BWAIT/ISA availability signal phase and an asynchronous external-prefetch producer are not evidenced. |
| Device phase | Core FDC/HDC/KBC/DMA/PIC/NMI owners; T425 reconciliation. | generic-at/reference-derived | No selected-machine rates, DACK/INTA/IRQ propagation or physical arbitration phase. |
| Fixed disk | Core Compaq WD normal CHS PIO and VM 925/5/17 RAW-IMG binding. | original logical protocol | Long/format/ECC/recovery, sector identity, drive service and physical DRQ/IRQ timing lack a bounded source/corpus. |
| CECG | Core VADP CECG digital controls, pages and copied frame; Model-40 binding. | original digital controls | Monitor/blanking/sync/live status, option-ROM modes, raster/service duration and ISA availability are unresolved. |
| BYOB lifecycle | VM manifest validation, copied immutable mapping, reset-vector execution/replay. | original lawful interface | Owner-supplied vendor POST/service/cancellation checkpoint remains external to the repository. |
| Independent audit | This record. | n/a | All prior blockers remain; no cumulative lower-tier result can replace them. |

## Verification

Focused representative current-gate regressions pass for CPU locality/refresh,
Model-40 FDC, BYOB, Compaq HDC, and CECG routes. They prove their stated
logical owners only; they are not physical timing measurements.

## Required Re-admission

The Queue retains the DeskPro receiver program but it is not eligible for an
acceptance claim until a new bounded input supplies an original source,
controller-matched reference, or owner-supplied corpus for a named blocker.
Generic-AT mechanisms may be extended only as labelled skeletons and cannot
alter this decision by themselves.
