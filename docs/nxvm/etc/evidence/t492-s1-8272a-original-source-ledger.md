# T492 S1 IBM 5160 8272A FDC And Media Original-Source Ledger

`M5:T492:S1:8272A-SOURCE:IMPLEMENTED-PENDING-GOVERNANCE`

| Source | Role and visual/OCR disposition | Usable basis |
| --- | --- | --- |
| NEC `uPD765A/uPD765B Floppy-Disk Controller`, NECEL-000324, 17-page scan, SHA-256 `E8A37F1242D4E26B400A524519B0B2EFEA8A68B83B4770E7A081BB82F6EDFEBC` | Normative controller source. PDF text is searchable but has OCR glyph errors; its first page was visually checked and confirms NEC identity, controller title, 16-command list, DMA/non-DMA capability, DRQ/INT and 8 MHz maximum. | Pin/register/phase/command/timing tables, with every later numerical value visually checked before List 1 claims it. |
| IBM `IBM 5160 Technical Reference`, Apr. 1983, 628-page scan, SHA-256 `18CD473851FDFE40C5BF2C7CCC870772857D84509D95ADA7880473CD802C63B8` | Normative selected-board/drive source. OCR is searchable but imperfect; printed page 1-151 was visually checked and identifies the 5.25-inch adapter, NEC uPD765-compatible controller, double-density MFM, DMA and completion interrupt role. | Board I/O map, `3F2h`/`3F4h`/`3F5h`, selected drive/adapter wiring, DMA2/IRQ6, command phase and media parameters. |
| IBM printed pp. 1-153--1-167 | Same board source, text-search located for FDC register and phase material; visually recheck individual table/value pages before freezing them. | Two-register controller view, status/data semantics, command/execution/result phases and board register addresses. |

## Source reconciliation boundary

The NEC A/B document lists 16 commands, including `Version`; the IBM 5160
board text describes 15 commands and names only a uPD765-compatible controller,
not a verified A/B die revision.  S2 must treat the shared command corpus as
the selected-board basis and explicitly classify `Version` rather than silently
claim it for the 5160.  No external emulator can resolve that primary-source
ambiguity.  Electrical waveforms remain source material but are L4/out of
scope unless a later admitted Core physical axis consumes them.

No source or third-party code was imported; the two PDFs remain external
owner-managed research material.
