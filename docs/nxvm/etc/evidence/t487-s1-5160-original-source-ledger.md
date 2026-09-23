# T487 S1 IBM 5160 Original-Source Ledger

`M5:T487:S1:IBM-5160-ORIGINAL-SOURCE:ACCEPTANCE-CANDIDATE`

## Corpus And Inspection Boundary

The primary corpus is IBM, *Personal Computer XT Hardware Reference Library*,
publication 1502237, the April 1983 IBM 5160 technical-reference edition. It
is retained only in the owner-managed external manual archive; no manual,
firmware, guest media or third-party source enters this repository.

The PDF is a 628-page raster scan with an Acrobat Paper Capture OCR text layer.
OCR is a navigation aid only. The cover and the following source pages were
rendered and visually inspected at 130 DPI; printed page labels, diagrams and
numeric values below are taken from those images, not from unreviewed OCR.

| Topic | IBM printed page(s) | Visually verified primary fact | List-1 consequence |
| --- | --- | --- | --- |
| Board clock origin | System Unit 1-6; I/O Channel Description 1-16 | The board data-flow diagram names a 14.31818 MHz source. `OSC` is 14.31818 MHz with 70-ns period and 50% duty; `CLK` is OSC divided by three, 4.77 MHz, 210-ns period and 33% duty. | Admit oscillator and derived-clock relations with their stated formulae. |
| Processor, DMA and refresh bus cycles | System Unit 1-14 | The channel description gives four 210-ns clocks (840 ns/byte) for the stated processor memory read/write condition, five clocks (1.05 us/byte) for processor I/O and DMA transfers, and refresh once per 72 clocks for four clocks (about 15 us, about 7% bandwidth). | Admit only these stated cycle classes and qualification; do not invent instruction, device-service or arbitrary wait timing. |
| RESET | System Unit 1-16 | `RESET DRV` resets/initializes system logic at power-up or low line voltage, is synchronized to CLK falling edge, and is active high. | Admit board reset polarity and synchronization; controller-local reset behavior remains in its unit T. |
| Board data routes | System Unit 1-6 and 1-7 | The data-flow diagrams show 8088, 8284A clock generator, 8288 bus controller, 8259A, 8237A, 8254 and 8255A roles, plus board/expansion buses. | Freeze only board interconnect facts; the listed chips remain separate hardware-unit tasks. |
| I/O map and NMI mask | System Unit 1-8 | The map assigns `0A0h` to the NMI mask register; power-on masks NMI, `80h` enables and `00h` disables it. | Admit the board NMI-mask route; PIC/PPI/NMI producer semantics remain separate units. |
| RAM, ROM and expansion map | System Unit 1-11 and 1-12 | Board R/W memory is 128--256 KiB; expansion and reserved/ROM regions are explicitly laid out, including base BIOS/BASIC at `F0000h`--`FFFFFh`. | Admit map/availability facts; no firmware bytes, ROM discovery or profile mapping change follows from this ledger. |
| ISA channel and signals | System Unit 1-14 through 1-16 | The I/O channel is a demultiplexed/repowered 8088 extension with the stated data, address, interrupt, DMA, control, refresh and check signals; the diagram gives the slot pin signal names. `I/O CH CK` is documented as an NMI source. | Admit the finite board-channel signals and their source-stated routing; card personalities remain controller/device tasks. |

## Evidence Limits And Reference Boundary

- The visual scan is authoritative for the listed publication facts; the OCR
  layer contains typography-sensitive values and must not be treated as an
  independent transcription authority.
- This edition does **not** by itself establish every 5160 board revision,
  every wait-state condition, full 8088 instruction retirement timing,
  controller-local service timing, card timing, or host-wall-clock pacing.
  Each is either a separate unit's List 1 or an explicit later boundary.
- 86Box, MAME, PCjs, Bochs and QEMU may be inspected during List 1 only to
  corroborate an unresolved observable board fact. They cannot override an
  IBM fact, supply a silent timing number, or be copied into NXVM. No such
  derived fact is admitted by S1.

## S1 Exit Check

Every intended T487 board topic has a visually verified IBM page/formula or an
explicit absence boundary. The next S may freeze the complete board List 1; it
must keep chip-local behavior out of this board unit.
