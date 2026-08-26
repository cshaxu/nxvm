# T484 S22 IBM 5160-268 Functional Closure Audit

`M5:T484:S22:XT-FUNCTIONAL-CLOSURE-AUDIT:OK`

This audit compares the frozen S1 F1--F9 universe with accepted implementation
evidence and the actual product/session/Core route.  It is a functional result
only: no row below claims XT board phase timing, physical service duration or
host-time pacing.

| S1 row | Accepted functional owner and proof | Residual receiver |
| --- | --- | --- |
| F1/F9 | Immutable 5160 resolver and one typed request/session factory now validate the transient base ROM and map it through Core's generic immutable provider; S21 direct and product smokes cover hash, mapping, reset and fixed media. | XT board timing; external ROM execution remains owner-supplied. |
| F2 | The Core-owned 8088 profile and four-byte queue were accepted in S2/S3. | 8088 bus/prefetch/retirement duration. |
| F3/F8 | One copied topology selects one PIC/DMA, XT PPI and the required absences; S5/S19 cover reset/NMI and non-AT bindings. | ISA/refresh/contention phase timing. |
| F4 | XT PPI owns input, response, IRQ1 and parity/I/O-check controls; no 8042/AUX alias exists in the XT plan. | Serial/typematic and physical fault timing. |
| F5 | The existing sole Core FDC/media owner receives the fixed `40 x 2 x 9 x 512` drive, `3F2h/3F4h/3F5h`, IRQ6/DMA2 route; S21 inserts a 360 KiB image. | FDC and drive service duration. |
| F6 | VADP remains the only CGA port/VRAM/snapshot owner; S11 proves the `B8000h` and copied-frame path. | CGA/board phase timing. |
| F7 | Core HDC owns the selected Xebec command, DMA3/IRQ5, Type-2 logical image and persistence route; S15--S18 cover command/DMA behavior and S21 inserts the fixed image. | Xebec ROM execution and adapter/drive physical service timing. |

The current source sweep finds one finite firmware selector, not the former
non-Model-40 default-PC/AT branch: default-PC/AT synthesized firmware,
Model-40 BYOB, and XT BYOB select distinct provider contexts.  The XT route
uses copied resolved topology and existing Core media owners; the fixed plan
does not retain an AT descriptor, ATA shim, second CHS/cache, VM video state,
or session-visible mutable hardware selector.  The shared BYOB byte validator
is intentionally neutral; Model-40's interleave and XT's contiguous mapping
remain distinct profile responsibilities.

The adjacent board-and-device phase-timing candidate is the sole receiver for
all listed timing residuals.  The subsequent XT final-model-L3 audit is the
sole receiver for a ready/not-ready decision.  Therefore no selected functional
gap, duplicate owner, or false AT fallback remains in T484; it is eligible for
task closure after independent acceptance of this audit.
