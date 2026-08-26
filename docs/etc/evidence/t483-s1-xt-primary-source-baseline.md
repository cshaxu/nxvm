# T483 S1 IBM PC/XT 5160-268 Primary-Source Baseline

`M5:T483:S1:XT-PRIMARY-SOURCE-BASELINE:OK`

`M5:T483:S1:XT-BOM:OK`

## Source Quality And Limits

| Source | Class and inspection result | Permitted use | Limit |
| --- | --- | --- | --- |
| IBM, *PC/XT 5155/5160 Technical Reference*, revised March 1986, [archive index](https://www.ibm-pc.org/manuals/ibm/5160/5160.htm) | Primary IBM scan, 422 pages. It has an OCR text layer; rendered pages and extracted text agree for the cover/revision, section headings and sampled board prose. OCR visibly confuses isolated glyphs, so tables, diagrams and exact values require rendered-page cross-reading. | 8088/oscillator/bus, memory/ROM, PIT/DMA/PIC/NMI, I/O-channel and keyboard-board facts. | The archive filename's part number and an older ledger's bibliographic number differ; this audit cites title/edition, not an unverified number. |
| IBM, *PC/XT Guide to Operations*, revised April 1984, [archive index](https://www.ibm-pc.org/manuals/ibm/5160/5160.htm) | Primary IBM scan, 364 pages, with the same OCR limitation. | Operational diskette, display-option and installed-fixed-disk distinctions. | It describes earlier XT configurations too; it cannot identify 5160-268 without the product reference. |
| IBM, *Personal System/2 and IBM PC Product Reference*, version 4.0, April 1987 | Primary IBM product catalogue already qualified by T373. | `5160-268`: 256 KB, enhanced keyboard, one half-height diskette drive, no factory fixed disk. | It does not select an adapter, board timing or ROM revision. |
| IBM, *PC 20 MB Fixed Disk Drive Adapter*, 6139790, 17 March 1986 | Primary IBM scan, OCR cross-read in T479 S6. | Explicit selected Xebec adapter grammar, DMA3, IRQ5 and external `C8000h`--`C9FFFh` ROM fact. | It does not select a system ROM, a local disk image, or physical drive-service timing. |
| 86Box and PCjs Xebec implementations | Read-only Other-L3 corroboration retained by T479 S6. | Cross-check the separate byte-stream/DMA architecture only. | Never authoritative for board selection, undocumented status bits or timing. |

No downloaded PDF, ROM, firmware digest, local path, guest media, or third-party
source is committed. The scans remain external research inputs.

## Frozen Baseline Bill Of Materials

| Selected item | Source-labelled fact | T483 interpretation and owner boundary |
| --- | --- | --- |
| System identity | IBM catalogue identifies `5160-268` as 256 KB, enhanced-keyboard, one-half-height-diskette system unit. | The only XT system baseline; not a 5150, generic clone or an AT inheritance. VM profile construction will own the frozen descriptor. |
| CPU and board clock | The technical reference identifies an 8088 in maximum mode at 4.77 MHz from a 14.31818 MHz crystal divided by three; normal bus cycles are four 210 ns clocks, with some I/O cycles five clocks. | Distinct 8088 external-bus/prefetch attachment is required. It reuses 8086 instruction semantics but cannot reuse an 8086 profile or claim full timing from this S. |
| Planar memory and ROM | The 256/640K board supports the documented RAM/ROM layout; its two ROM sockets hold 32K devices and the ROM supplies POST, I/O drivers, graphics characters and diskette bootstrap. | Select 256 KB baseline RAM. BIOS is a future owner-supplied external ROM pair; no bytes, revision, digest or path is selected here. |
| Timer, DMA, PIC and NMI | IBM documents a three-channel PIT at 1.193182 MHz, IRQ0 timebase, channel-1 refresh/DMA request, eight IRQ levels, three external DMA channels plus one refresh channel, keyboard IRQ1 and parity NMI. | Existing Core controller owners may be consumed later, but no PC/AT board binding is inherited. The XT functional receiver must construct these exact routes. |
| Keyboard/input | The system board has a keyboard serial interface; the manual separately describes 83-key and 101/102-key keyboard families. The product catalogue selects enhanced keyboard. | This is an XT keyboard/PPI/serial binding question, not authorization to reuse the AT 8042/AUX topology. Mouse is unselected. |
| Diskette | The catalogue selects one half-height diskette drive. | The exact drive/media profile remains source-qualified work for the XT functional receiver; do not substitute the current AT 1.44 MB route. |
| Display | The 5160 system unit does not itself choose MDA, CGA or EGA. IBM maps the color/graphics aperture at `B8000h`. | Select IBM CGA as the product baseline because it is the current source-backed VADP capability; the later XT functional receiver must bind its actual adapter/port/ROMless contract. This is an explicit profile choice, not an inference that every 5160 shipped with CGA. |
| Fixed disk | The factory 5160-268 catalogue configuration has no fixed disk. The owner-required expansion is IBM's 20 MB Xebec adapter: 8-bit byte stream, `320h`--`323h`, DMA3, IRQ5, and external `C8000h`--`C9FFFh` adapter ROM. | Select the adapter personality only. The printed drive-type/geometry must be frozen from a source-qualified local BYOB media declaration by the XT functional receiver; no ATA task-file, internal ROM, MFM cache or guessed geometry is allowed. |
| Reset and I/O channel | IBM documents `RESET DRV`, AEN DMA bus ownership, IRQ2--IRQ7 and the 8-bit channel. | Reset/port/DMA effects belong to the profile-to-Core construction plan. No standalone XT global or alternate transaction path is admissible. |

The one deliberate deferment is Xebec drive type/geometry: the primary sources
prove allowed tables but do not associate one with the no-fixed-disk factory
`5160-268` unit. Selecting it without an owner-supplied BYOB image/manifest
would manufacture a hardware fact. T483 S2 records the exact functional
receiver that must make that bounded selection before opening the machine.
