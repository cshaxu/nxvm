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
| S12 | Accepted: IBM 6139790 and read-only 86Box/PCjs recheck confirms every X1--X17 disposition, retaining the sole Core HDC/media receiver; no ATA alias, guessed geometry or media/firmware import. [Evidence](../etc/evidence/t484-s12-xebec-source-contract.md). |
| S13 | Accepted: one immutable tagged HDC plan keeps ATA, Compaq and WD1003 task-file personalities while Xebec receives only its IBM four-port/DMA3/IRQ5 Core wiring; no command state, geometry, ROM or service-time behavior is invented. [Evidence](../etc/evidence/t484-s13-xebec-tagged-plan-wiring.md). |
| S14 | Accepted: the X1--X17 implementation brief places Xebec byte-stack, command/result and DMA callbacks in the sole Core HDC owner, explicitly excluding ATA reuse, selected geometry, ROM, physical behavior and guessed hardware status. [Evidence](../etc/evidence/t484-s14-xebec-logical-implementation-brief.md). |
| S15 | Accepted: one sole-HDC Xebec select/DCB/completion/request-sense/reset stack replaces the former unsupported port path, while selected-media DMA transfer, enable-bit semantics, physical behavior and ROM remain explicit transfers. [Evidence](../etc/evidence/t484-s15-xebec-command-stack.md). |
| S16 | Accepted: owner-selected Type 2 is one immutable copied 5160 Xebec descriptor with exact 615 x 4 x 17 x 512 geometry and the existing 320h--323h/DMA3/IRQ5 route; no runtime selection, ATA alias or second media owner. [Evidence](../etc/evidence/t484-s16-xebec-type2-selection.md). |
| S17 | Accepted: Type-2 Read `08h` and positive-count Write `0Ah` use the sole Core HDC buffer, frozen media registry and DMA3 request route; terminal cleanup is covered, while 323h enable bits, IRQ gating, ROM and physical timing remain explicit transfers. [Evidence](../etc/evidence/t484-s17-xebec-dma-media.md). |
| S18 | Accepted: configured 8237A DMA3 transfers the full Type-2 sector between Core RAM and the sole HDC/media owner in both directions; it adds no production path. [Evidence](../etc/evidence/t484-s18-xebec-dma-ram.md). |
| S19 | Accepted: source-qualified IBM 5160 PPI-B parity/I/O-check controls and PPI-C status inputs use the sole XT PPI owner and one Core NMI request edge; physical fault generation/timing remains transferred. [Evidence](../etc/evidence/t484-s19-xt-ppi-parity-nmi.md). |
| S20 | Accepted: B6 source/session audit freezes the lawful external-ROM/BYOB manifest, selected XT profile and sole session/Core construction gap before product-route implementation. [Evidence](../etc/evidence/t484-s20-xt-b6-session-contract.md). |
| S21 | Accepted: the sole typed session route validates and maps owner-provided XT BYOB ROMs, composes the fixed 360 KiB/Type-2 media routes and retains no AT-firmware alias or duplicate integrity loader. [Evidence](../etc/evidence/t484-s21-xt-byob-session.md). |
| S22 | Accepted: every frozen F1--F9 selected functional route has one owner and product path; all remaining phase/physical work transfers only to the next XT timing candidate. [Closure audit](../etc/evidence/t484-s22-xt-functional-closure-audit.md). |

T484 is closed.  Its retained proposal is
[the historical companion](M5-T484-xt-5160-268-device-functional-closure-proposal.md).
