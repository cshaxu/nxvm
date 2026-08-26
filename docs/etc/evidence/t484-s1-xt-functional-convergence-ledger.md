# T484 S1 IBM PC/XT 5160-268 Functional Convergence Ledger

`M5:T484:S1:XT-FUNCTIONAL-LEDGER:OK`

`M5:T484:S1:XT-FUNCTIONAL-BATCHES:OK`

## Coverage Rule

The accepted T483 BOM is the complete T484 universe.  A row closes only after
its documented functional contract, sole state owner, reset/error behaviour
and focused proof are accepted.  A row may transfer only board availability,
service duration, arbitration or phase timing to the later XT timing candidate;
it may not use that transfer to hide missing functional state.

All batches retain one construction/data flow:

`typed session request -> immutable XT resolver values -> copied Core plan -> Core sole state owner -> copied product observation`.

## Complete Functional Universe

| ID | Selected capability and primary baseline | Current owner or absence | Required functional outcome | Batch and focused proof | Timing handoff |
| --- | --- | --- | --- | --- | --- |
| F1 | 5160-268 identity, 256 KiB, external ROM pair and fixed selections (T483 S1). | **Absent** XT descriptor; generic mapping/reset owners already reside in Core. | One immutable `ibm-5160-268` declaration validates the fixed BOM, BYOB ROM policy and absent devices, then copies one plan. | B1; profile-resolution and session rejection/atomicity smoke. | No profile physical qualification. |
| F2 | 8088 maximum-mode attachment, with retained 8086 semantic ledger (T483 S1). | **Absent** 8088 CPU profile in `cpu_interface.h`; CPU state remains Core-owned. | One 8088 attachment reuses only compatible semantic code and selects no 8086 board alias. | B1; 8088 semantic/real-mode entry and profile selection smoke. | Bus/prefetch/retirement duration transfers. |
| F3 | 8-bit channel, reset, parity NMI; single PIC IRQ0/IRQ1 and PIT/DMA refresh routes (IBM 5160 technical reference). | Core owns board, PIC, PIT and DMA state; current composition is PC/AT-oriented. | XT plan establishes one compatible one-PIC/one-DMA board binding, reset/NMI route and port map without profile-side IRQ/DRQ state. | B2; route/reset/NMI and PIC/PIT/DMA focused traces. | Cycle, contention and refresh phase transfer. |
| F4 | Enhanced keyboard over the documented XT serial board interface (T483 S1). | **Absent** XT attachment; current KBC route is AT-oriented. | One source-qualified XT keyboard binding reaches the existing input/controller ownership boundary; no AUX mouse state. | B3; make/break, reset and keyboard-IRQ trace. | Serial response/typematic timing transfers. |
| F5 | One selected half-height diskette path (T483 S1). | Core FDC/media owner exists; selected XT drive/controller binding is absent. | One source-qualified XT FDC drive/media declaration reaches the sole FDC/media state and its IRQ/DRQ route. | B3; reset, command/result, media insertion and diskette-boot consumer smoke. | Drive/controller service time transfers. |
| F6 | IBM CGA selection and `B8000h` aperture (T483 S1). | VADP owns CGA registers, VRAM and copied snapshots; XT binding absent. | The XT plan selects the existing CGA state/snapshot route and no VM/firmware mode mirror. | B4; port, aperture and copied-frame regression. | Adapter/board phase timing transfers. |
| F7 | IBM 20 MB Xebec adapter: byte stream `320h`--`323h`, DMA3, IRQ5 and external option ROM (T479 S6/T483 S1). | **Absent** Xebec HDC protocol; existing HDC personalities are ATA, Compaq WD and IBM WD1003. | One explicit Core HDC Xebec personality owns command phase, DRQ/IRQ, logical image/geometry and persistence; option ROM remains external BYOB mapping. | B5; command/reset/IRQ/DMA trace plus inserted-image and boot-consumer regression. | Adapter/drive service time transfers; no ATA estimate. |
| F8 | No RTC/CMOS, second PIC, 16-bit channel, PS/2 AUX or unselected expansion (T483 S1/S2). | Those independent Core capabilities exist for PC/AT profiles. | The XT declaration rejects/omits each one; no disabled controller is bound. | B1/B2; negative resolver and port/route absence proof. | None. |
| F9 | Product request, session construction and BYOB resources (T482 contract; T483 S1). | Typed catalog/session authority exists; no XT identity/resource contract. | One typed XT selection reaches the existing session authority without CLI re-encoding, embedded firmware/media or mutable hardware fields. | B6; catalog/session/open-failure atomicity and two-session isolation smoke. | No host time input. |

## Ordered Repair Batches

| Batch | Depends on | Cohesive change boundary | Explicit exclusions |
| --- | --- | --- | --- |
| B1 - immutable identity and CPU | F1, F2, F8 | Add the XT declaration and 8088 attachment through the existing resolver/Core plan boundary; reject unsupported selections. | Board routes, device state, firmware bytes and time-axis qualification. |
| B2 - XT board topology | B1; F3 | Bind the selected reset/NMI, single-PIC, PIT/DMA/refresh and 8-bit board facts through existing Core owners. | Keyboard/FDC/HDC/VADP controller repair and durations. |
| B3 - input and diskette | B2; F4, F5 | Add only source-qualified XT keyboard and diskette bindings to their sole owners. | AUX/PS2 behavior, AT drive substitution and timing estimates. |
| B4 - selected CGA | B1; F6 | Select the existing VADP CGA path through the XT plan and prove snapshot isolation. | A second display owner, EGA/VGA or physical display timing. |
| B5 - selected Xebec | B2; F7 | Add the one Core HDC Xebec personality and external-BYOB option-ROM/media construction boundary. | ATA shim, second CHS/media cache, raw MFM and service-time estimates. |
| B6 - product integration | B1--B5; F9 | Expose the completed fixed profile through the existing typed session route and prove construction/consumer integration. | Generic XT variants, Windows and a physical-speed claim. |

## Completion Predicate

T484 may close only when F1--F9 each have accepted functional proof, or the
proposal's stop condition transfers an under-sourced row to its earliest
receiver with no XT availability claim.  The present source inspection shows
F1/F2/F4/F7 absent and F3/F5/F6 present only as unbound Core mechanisms; no
current PC/AT profile is a functional substitute for any row.
