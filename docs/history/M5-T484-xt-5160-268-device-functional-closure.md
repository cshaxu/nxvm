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
| S8 | Active: B3 implementation of the selected XT PPI/keyboard attachment, immutable plan selection and focused IRQ/reset isolation proof. |
