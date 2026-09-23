# T450 S1 Controller Source And Code Baseline

## Scope And Method

This is the finite admission ledger for T450's eight-controller inventory.
It records external-manual identity and checksum, not manual bytes or local
paths. Each file was downloaded into the owner-managed external manual archive,
opened as a PDF, and hashed with SHA-256 on 2026-08-24. The listed online
locations are acquisition provenance, not product dependencies or download
mechanisms.

All rows were then matched to the current Core owner source and retained T433
source-sufficiency evidence. `Partial` means the existing implementation has
some behavior, but its complete function/timing disposition is still unproven;
it is not an L3 claim.

## Frozen Coverage Ledger

| Unit | Admitted primary documentation | Archive identity and SHA-256 | Required board/personality source | Current owner paths | Baseline disposition |
| --- | --- | --- | --- | --- | --- |
| PIC 8259A | Intel, *8259A Programmable Interrupt Controller*, order 231468-003, Dec. 1988, 24 PDF pages; [vendor scan](https://www.pcjs.org/documents/datasheets/intel/INTEL_8259A_PIC.pdf). | `231468-003_8259A_Programmable_Interrupt_Controller_Dec1988.pdf`, `71F62B2FA1F13F6547DCEE26A2D570BF9BCFCFE6F2F84B24BC01053BB6153533` | IBM 5170 technical reference: cascade and IRQ wiring. | `src/core/machine/pic.c`, `pic.h`, `machine.c`; consumer `cpu_instructions.c`. | Partial: chip semantics and topology exist; acknowledgement/visibility phase is unregistered. |
| DMA 8237A | Intel, *8237A High Performance Programmable DMA Controller*, order 231466-005, Sep. 1993, 19 pages; [vendor scan](https://www.pcjs.org/documents/datasheets/intel/INTEL_8237A_DMA.pdf). | `231466-005_8237A_High_Performance_Programmable_DMA_Controller_Sep1993.pdf`, `2A1AC5BBF5B4BE75BA9272647EFFB389BCC5E76CBFE293D5C56E75217E2BC8A8` | IBM 5170 technical reference: channel, cascade, page and refresh bindings. | `src/core/machine/dma.c`, `dma.h`, `machine.c`; contracts `controller_interface.h`. | Partial: transfer semantics exist; DREQ/service/grant/cascade timing is unregistered. |
| PIT 8254 | Intel, *8254 Programmable Interval Timer*, order 231164-005, Sep. 1993, 21 pages; [vendor scan](https://www.cs.cmu.edu/~410/doc/8254.pdf). | `231164-005_8254_Programmable_Interval_Timer_Sep1993.pdf`, `E5397A387D24FFF21C8B531A320251F89B103274BBCE0C057A654873E494D726` | IBM 5170 technical reference: oscillator, channel GATE/OUT and IRQ0/speaker/refresh routes. | `src/core/machine/pit.c`, `pit.h`, `clock.c`, `machine.c`. | Partial: counter modes exist; source clock, reset phase and board routes are unregistered. |
| RTC/CMOS | Motorola, *MC146818A Real-Time Clock Plus RAM*, 21 pages; [manufacturer scan](https://www.ardent-tool.com/datasheets/Motorola_MC146818A_alt.pdf). | `MC146818A_Real_Time_Clock_Plus_RAM.pdf`, `2BC7AE3D16D6A2D234481EFF4FAFB04955BDF88075BC626887121293400D1F1D` | IBM 5170 technical reference: 32.768-kHz source, CMOS and IRQ8/cascade wiring. | `src/core/machine/rtc.c`, `rtc.h`, `clock.c`, `machine.c`. | Partial: calendar/register behavior exists; divider-reset and IRQ delivery phase are unregistered. |
| KBC 8042/NMI | Intel, *UPI-41A/41AH/42/42AH User's Manual*, order 231318-006, Oct. 1993, 69 pages; [vendor scan](https://games.rossiters.com/manuals/Fluke/UPI-41a.pdf). | `231318-006_UPI-41A_42AH_Users_Manual_Oct1993.pdf`, `95D56D2BF93B0D3A4D062F0C86B2B4746933E42F3A65CA263C57474497E29A45` | IBM 5170 technical reference: host ports, keyboard/reset/A20/NMI routes; keyboard protocol is a distinct input source. | `src/core/machine/kbc.c`, `kbc.h`, `machine.c`; VM composition input binding. | Partial: FIFO/host interface exists; controller/serial cadence and output-port/NMI phases are unregistered. |
| FDC uPD765 plus logical media | NEC, *uPD765A/uPD765B Single/Double Density Floppy-Disk Controller*, 17 pages; [vendor scan](https://hxc2001.com/download/datasheet/floppy/thirdparty/FDC/NEC/UPD765_Datasheet_OCRed.pdf). | `uPD765A_uPD765B_Floppy_Disk_Controller.pdf`, `E8A37F1242D4E26B400A524519B0B2EFEA8A68B83B4770E7A081BB82F6EDFEBC` | IBM 5170 technical reference: controller, DMA2, IRQ6 and drive route; selected logical format/drive data remains needed. | `src/core/machine/fdc.c`, `fdc.h`, `fdc_observation_interface.h`, `media_interface.h`, `machine.c`; VM device composition. | Partial: command/result phases exist; drive pacing, motor/rotation and logical media format are unselected. |
| VADP | IBM, *Technical Reference: Options and Adapters, Volume 2*, revised Apr. 1984, 692 pages, EGA section; [archive scan](https://bitsavers.trailing-edge.com/pdf/ibm/pc/cards/Technical_Reference_Options_and_Adapters_Volume_2_Apr84.pdf). | `IBM_Technical_Reference_Options_and_Adapters_Volume_2_Apr1984.pdf`, `B5BF24EA3E63082D5C637DB8B08469C6D4929B4B9F6B7B24C7A211338B42A15F` | Selected IBM CGA/EGA or Compaq CECG personality, CRTC clock and monitor/bus binding. | `src/core/machine/vadp.c`, `vadp.h`, `display_interface.h`, `clock.c`, `machine.c`; VM profile display data. | Partial: digital state exists; selected adapter cadence/status/contention universe is unregistered. |
| HDC ATA | X3T13, *AT Attachment-3 Interface*, 2008D Rev. 7b, Jan. 1997, 182 pages; [committee draft scan](https://www.scs.stanford.edu/11wi-cs140/pintos/specs/ata-3-std.pdf). | `X3T13_2008D_ATA-3_Rev7b_Jan1997.pdf`, `FC9D1C42B0B3EF916C790BAFED05E96D9EBA3B221C0E0D3E914E6F948DDD0237` | A selected ATA controller/personality plus logical backing-media contract; IBM MFM/ST-506 evidence is not ATA. | `src/core/machine/hdc.c`, `hdc.h`, `controller_interface.h`, `media_interface.h`, `machine.c`; VM device composition. | Blocked: an ATA standard is now admitted, but no selected ATA controller/backing-media personality is admitted. |

The shared IBM board source is *IBM Personal Computer AT Technical Reference*,
1502243, Mar. 1984, 462 pages, [archive scan](https://www.minuszerodegrees.net/manuals/IBM_5170_Technical_Reference_1502243_MAR84.pdf), archive identity
`IBM_5170_Technical_Reference_1502243_Mar1984.pdf`, SHA-256
`F6CB4A270E0F49A79940FA1EE1084CA419AF15B463C870D652EA74CED011EA0D`.

## T450 Follow-On Batches

S2 consumes this ledger to create the eight complete function/timing checklists
from the admitted documents. S3 audits every checklist row against the owner
paths and current tests, then produces the eight implementation-gap checklists.
No later candidate may use a document or board fact absent from this ledger
without first revising the coverage universe under the source policy.

## Review Limits

The ledger admits documentation as research evidence only. It does not grant a
license to copy source text, firmware, media, or any third-party implementation;
it does not select an ATA personality; and it does not promote physical
waveforms, analog behavior, host time, or unselected board facts to L3.
