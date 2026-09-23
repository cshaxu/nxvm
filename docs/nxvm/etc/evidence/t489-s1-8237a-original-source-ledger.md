# T489 S1 IBM 5160 8237A Original-Source Ledger

`M5:T489:S1:8237A-ORIGINAL-SOURCE:ACCEPTANCE-CANDIDATE`

Primary sources remain owner-managed outside NXVM; neither PDF nor any
third-party implementation is imported. Intel's manual is the chip authority;
IBM is the selected XT board authority.

| Source | Archive identity and condition | Pages visually checked | Facts admitted to the following List 1 |
| --- | --- | --- | --- |
| Intel, *8237A High Performance Programmable DMA Controller*, order 231466-005, Sep. 1993 | Owner-managed Intel controller archive; 19 pages; SHA-256 `2A1AC5BBF5B4BE75BA9272647EFFB389BCC5E76CBFE293D5C56E75217E2BC8A8`. A clean rendered publication; visual pages, not extracted text, govern. | Printed pp. 1, 3, 5, 7 and 10. | Four channels; request/mask/priority; DREQ/DACK, HRQ/HLDA and EOP; single/demand/block/cascade; register/first-last access; compressed timing and address latch relation. |
| IBM, *Personal Computer XT Hardware Reference Library*, 1502237, Apr. 1983 | Owner-managed IBM controller archive; 628-page Paper Capture scan; SHA-256 `18CD473851FDFE40C5BF2C7CCC870772857D84509D95ADA7880473CD802C63B8`. OCR is only a locator; rendered pages govern. | System Unit pp. 1-7, 1-8, 1-9 and 1-14 (PDF pp. 19, 20, 21 and 26). | One 8237A-5 plus external DMA address/page latch; ports `000h`--`00Fh` and page registers `080h`--`083h`; expansion DRQ1--3/DACK0--3; channel 1 refresh; all DMA transfers five clocks/1.05 microseconds and refresh every 72 clocks/approximately 15 microseconds. |

## Source Boundary

The selected IBM board material exposes one 8-bit DMA controller, not an AT
second DMA controller. It establishes XT clocked transfer and refresh facts,
but does not by itself choose every peripheral's DREQ cadence or all device
service delays. Those rows must remain explicit in List 1 rather than being
estimated. MAME, 86Box, PCjs, Bochs and QEMU are potential corroborating
references only after the complete manual ledger is frozen; they cannot supply
or override a documented Intel or IBM rule.
