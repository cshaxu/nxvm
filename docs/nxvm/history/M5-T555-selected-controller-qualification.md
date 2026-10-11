# M5 T555 — Selected Controller And Board Qualification

## Admission

T555 is admitted from the first M5 Queue candidate on 2026-10-10. It qualifies
retained controller and board behavior only after a current-path inventory; it
does not assume the pre-migration proposal's old component names are current.

## S1 — Complete: Baseline audit

Read the current Core and fixed-App composition graph, all relevant unit and
integration registrations, existing evidence/manual ledgers and the selected
controller proposal. Produce one implementation/evidence/receiver matrix for
PIC, PIT, DMA, RTC/CMOS, KBC/keyboard, FDC, HDC, video and board glue. Every
gap must state its owner, fact class, evidence tier, current receiver and a
bounded repair/proof recommendation. The resulting [baseline ledger](../etc/evidence/t555-s1-controller-board-baseline.md)
uses the current Core/App paths and is the only admission input for follow-up
controller work. This S did not modify source, tests, assets or external
materials.

## Remaining Ordered S Work

| S | Boundary | Exit condition |
| --- | --- | --- |
| S2 | PIC/PIT/DMA and board IRQ/refresh | Every selected request, counter output, cascade, refresh and scheduler observation has one current proof or one bounded Core/board repair. |
| S3 | RTC/CMOS and keyboard/KBC/XT-PPI | Current reset, port, NMI, clock and firmware-facing routes are classified; no controller-local timing or board duplicate is introduced. |
| S4 | FDC/media and HDC personalities | Each selected controller/media route has an explicit grammar, request/IRQ/DMA consumer and reset/deadline disposition. |
| S5 | Video and selected board display routes | CGA/EGA/CECG owner, aperture, clock and copied-frame routes are qualified without adding a renderer or second framebuffer. |
| S6 | Cross-profile closure | Four fixed-App route matrix, exact remaining L1/L2/L4 exclusions, relevant unit/integration proof and documentation closure are complete. |

## Frozen Boundaries

Vendor manuals and ROMs remain in `nxvm-assets`; S1 may only read their
repository evidence/provenance. Shared Lib/Emulator/x86 code is read-only. No
asset, firmware, media, INI, test or production change is permitted until the
owner reviews the audit and admits a later S.
