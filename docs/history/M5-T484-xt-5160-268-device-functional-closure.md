# M5 T484 IBM PC/XT 5160-268 Selected-Device Functional Closure

T484 consumes the closed T483 IBM PC/XT 5160-268 bill of materials,
capability matrix and timing partition.  It closes exactly the selected
platform/device functionality before the later XT board-and-device timing
candidate.  It must reuse compatible Core owners, preserve one immutable
profile-to-copied-plan construction flow, and never create an XT generic path,
ATA/Xebec shim or duplicate controller/media/video state.

| Subtask | Result |
| --- | --- |
| S1 | Accepted: the F1--F9 convergence ledger freezes every selected device, owner/absence, repair batch and timing handoff. |
| S2 | Accepted: rendered Intel/IBM evidence defines shared semantics and the mandatory Core-owned four-byte 8088 queue boundary. |
| S3 | Accepted: one Core-owned 8088 four-byte queue producer and one fixed 5160-268 resolver declaration; 297/297 current-gate tests and Release 0484 pass. The declaration remains explicitly unavailable until B2 binds the XT board. [Evidence](../etc/evidence/t484-s3-xt-8088-profile-implementation.md). |
| S4 | Accepted: complete B2 board-topology matrix identifies the sole owner and exact construction gap for every selected 5160 relation, without an AT alias or runtime path. [Evidence](../etc/evidence/t484-s4-xt-board-topology-gap-ledger.md). |
| S5 | Accepted: resolved profiles copy one board topology and session passes it directly to Core. The B2 sweep proves 256 KiB, one PIC/DMA, absent RTC/CMOS and reset persistence; only source-gated ROM manifest/mapping and 8255/PPI parity-NMI controls transfer to B6/B3, without an XT runtime path. [Evidence](../etc/evidence/t484-s5-xt-b2-core-topology.md). |
| S6 | Accepted: IBM-led B3 source/current-owner matrix proves the XT PPI/keyboard is not an AT 8042 alias and the FDC must not inherit AT `3F7h` or a 1.44 MB drive. [Evidence](../etc/evidence/t484-s6-xt-b3-source-owner-matrix.md). |
| S7 | Accepted: rendered IBM 101/102-key source qualifies the selected serial, reset, response, PPI-line and IRQ contract for one future XT attachment, with no AT 8042 alias. [Evidence](../etc/evidence/t484-s7-xt-keyboard-source-contract.md). |
| S8 | Accepted: one Core XT 8255/PPI keyboard owner replaces the false 8042 alias for the selected plan; focused make/break, clear/IRQ/reset-result and AT-isolation proof pass. [Evidence](../etc/evidence/t484-s8-xt-ppi-keyboard-implementation.md). |
| S9 | Accepted: IBM-led source contract selects one 360KiB `40 x 2 x 9 x 512` drive and the `3F2h`/`3F4h`/`3F5h`, IRQ6/DMA2 route; it excludes a PC/AT `3F7h` alias and transfers physical service timing. [Evidence](../etc/evidence/t484-s9-xt-fdc-source-contract.md). |
| S10 | Accepted: one immutable XT drive-A/FDC Core-plan route binds `3F2h`/`3F4h`/`3F5h`, IRQ6/DMA2 and no `3F7h`; the source-backed physical timing transfer remains explicit. [Evidence](../etc/evidence/t484-s10-xt-fdc-plan-implementation.md). |
| S11 | Accepted: selected CGA enters only through the accepted VADP, `B8000h` and copied-snapshot owner; focused route, shared-CGA/AT regressions, current-gate and stripped Release proof pass, while physical timing remains transferred. [Evidence](../etc/evidence/t484-s11-xt-cga-plan-implementation.md). |
| S12 | Implementing: IBM 6139790 and read-only 86Box/PCjs recheck confirms every X1--X17 disposition, retaining the sole Core HDC/media receiver; no ATA alias, guessed geometry or media/firmware import. [Evidence](../etc/evidence/t484-s12-xebec-source-contract.md). |
