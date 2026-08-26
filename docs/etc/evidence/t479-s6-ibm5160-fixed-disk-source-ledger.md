# T479 S6 IBM 5160-268/Xebec Fixed-Disk Source Ledger

`M5:T479:S6:IBM5160-SOURCE:OK`

## Source Quality And Use

| Source | Quality and scope | Use in this ledger |
| --- | --- | --- |
| IBM, *PC 20MB Fixed Disk Drive Adapter*, 6139790, 17 March 1986, printed pages 1--17, [scan](https://minuszerodegrees.net/oa/OA%20-%20IBM%20PC%2020MB%20Fixed%20Disk%20Drive%20Adapter.pdf) | Primary IBM scan with OCR. The port, DCB, status and interface tables were cross-read against the rendered PDF; schematic OCR is not used for numeric or logical claims. | Normative host-visible contract for the selected later XT 20 MB adapter. |
| 86Box, read-only `src/disk/hdc_st506_xt.c` | Other-L3 implementation observation. It has a named IBM/Xebec variant, a six-byte command block, DMA/IRQ enable state and 320h port family. Its timers and extra controller variants are not evidence. | Cross-check only; no code or constants imported. |
| PCjs, read-only `machines/pcx86/modules/v2/hdc.js` | Other-L3 implementation observation. It separately models XTC data/status/config/pulse/pattern ports and IRQ5, and labels undocumented status-bit interpretation as a guess. | Cross-check only; its guessed status meanings do not promote a row. |
| MAME, Bochs, QEMU | Not inspected in a bounded local checkout for this S. | No claim derives from their absence. |

The IBM adapter, not the earlier AT/ATA ledger, is the authority.  This is an
8-bit byte-stream/DMA controller; it cannot inherit the AT task-file grammar.

## Finite Functional And Timing Universe

| ID | Required observable | Primary basis | Cross-model result | Level and S6 disposition |
| --- | --- | --- | --- | --- |
| X1 | Selected adapter connects one or two drives and uses system DMA plus IRQ5. | IBM pp. 1, 14--15. | 86Box and PCjs select IRQ5 for XTC. | Manual L3; one HDC/PIC/DMA route. |
| X2 | Host has four 8-bit ports: read/write `320h` data, read `321h` hardware status/write reset, read `322h` switches/write controller-select pulse, write `323h` DMA/IRQ mask pattern. | IBM p. 14. | Both references model the same family. | Manual L3; distinct port grammar. |
| X3 | During DMA, AEN disables the adapter I/O decoder. | IBM pp. 14--15. | 86Box models a DMA-specific data path. | Manual L3; Core DMA provider, not CPU-port reuse. |
| X4 | DRQ3 requests transfer and remains active until DMA channel 3 acknowledges; IRQ5 reports the ending status byte when enabled. | IBM pp. 14--15. | 86Box and PCjs both retain DMA/IRQ enable state. | Manual L3 logical handoff/IRQ; electrical level duration remains out of scope. |
| X5 | Data is an 8-bit command/parameter/status stack.  A command begins with a six-byte DCB. | IBM pp. 1, 7. | Both references use a six-byte Xebec block. | Manual L3; one bounded byte-stream state in Core. |
| X6 | DCB fields encode opcode, drive/head, 10-bit cylinder and 6-bit sector, count/interleave, retry/ECC/step controls. | IBM p. 7. | Both references decode equivalent CHS fields. | Manual L3; no task-file aliases. |
| X7 | Drive types 1, 2, 13 and 16 have the printed cylinder/head/write-precomp/landing geometry table. | IBM pp. 2--3. | 86Box has matching Xebec geometry family. | Manual L3; configuration must select a printed type only. |
| X8 | The status byte is read from the data stack; bit 5 identifies logical drive and bit 1 reports error. | IBM p. 4. | PCjs distinguishes this byte from hardware status. | Manual L3; one response-stack owner. |
| X9 | An error status is followed by four request-sense bytes with the printed address/error layout. | IBM p. 4. | 86Box and PCjs retain sense/error structures. | Manual L3; implement only with the documented request-sense command. |
| X10 | Test-ready, recalibrate, request-sense, format/verify, read, write, seek, initialize-characteristics, buffer/ECC and diagnostic/long command opcode families are finite. | IBM pp. 10--16. | 86Box command names match; PCjs has the same public Xebec set. | Manual L3 grammar; physical-only commands remain explicit unsupported operations. |
| X11 | Read/write transfer logical 512-byte sectors through DMA, with DCB block count. | IBM pp. 10--14. | 86Box uses a 512-byte buffer. | Manual L3 logical media transfer through the sole media provider. |
| X12 | Initialize characteristics carries eight bytes after the DCB, in the printed field order. | IBM p. 15. | PCjs independently recognizes the additional bytes. | Manual L3; source-qualified extension of the single request collector. |
| X13 | Step control values identify 3 ms, 200 us and 70 us alternatives; undocumented values are not assigned a duration. | IBM p. 7. | 86Box has implementation delays but does not replace this table. | Manual L3 for selector semantics; no guessed service deadline. |
| X14 | Controller status bit meanings, beyond IBM's stated ready-to-transfer/status behavior, are not fully specified by the manual. | IBM pp. 3--4; PCjs explicitly marks its bit meanings as guesses. | No independent authoritative resolution. | L1 only for causal command/data/status ordering; no invented bit contract. |
| X15 | MFM, ECC correction, long sectors, formatting, recovery, drive-ready/index/seek-complete and controller diagnostics require physical media/signals absent from a logical image. | IBM pp. 4--16. | 86Box has a richer physical-drive model. | Explicit unsupported physical boundary, never simulated from a byte image. |
| X16 | The adapter maps ROM at `C8000h`--`C9FFFh`. | IBM p. 14. | 86Box and PCjs load external controller ROMs. | Manual L3 fact, but BYOB ROM binding belongs to the future approved 5160 profile receiver. |
| X17 | The source supports the later 20 MB XT adapter; it does not by itself authorize a general 5160 profile or imported IBM ROM/media. | IBM document identity and source policy. | References require their own firmware assets. | Explicit profile/firmware admission boundary. |

## Cross-Validation Result

The three examined sources agree on the architectural split: Xebec is a
byte-stream controller with a distinct CPU port route and DMA channel 3, while
the existing 5170/Compaq/default-at personalities are task-file controllers.
86Box's extra variants/timers and PCjs's explicitly guessed hardware-status
bits are deliberately not adopted.  All controller and media state therefore
remain in one Core HDC object; only a compact, source-proven Xebec branch may
be added there.
