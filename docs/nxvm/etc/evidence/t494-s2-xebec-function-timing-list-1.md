# T494 S2 IBM 5160 Xebec Function And Timing List 1

`M5:T494:S2:XEBEC-LIST-1:OK`

This is the finite source universe for T494. IBM 6139790 is normative. A
direct printed value, field or ordering fact is Manual L3; a controller
physical behavior without a printed model is explicitly bounded rather than
estimated. `Other L3` means read-only corroboration only and never overrides
the IBM row.

| ID | Complete capability or state relation | IBM basis | Cross-check and disposition |
| --- | --- | --- | --- |
| X1 | The adapter is an 8-bit byte-stream controller with one/two daisy-chained drives; command, parameter and result bytes use the data-register stack. | pp. 1, 7 | 86Box and PCjs maintain a distinct Xebec/XTC route. **Manual L3**; never use ATA task-file state. |
| X2 | A DCB is exactly six bytes: command class/opcode, selected drive/head, 10-bit cylinder, 6-bit sector, block/interleave and control. | pp. 7--8 | Both references collect a six-byte XTC request. **Manual L3**. |
| X3 | DCB control defines retry/reread choices and selector values: 200 us, 70 us, or 3 ms per step; reserved selector values remain as printed. | pp. 8--9 | 86Box has extra scheduler delays, which are not authority. **Manual L3** for fields/selectors; no inferred service deadline. |
| X4 | Four printed drive types (1, 2, 13, 16) define cylinders, heads, write-precompensation and landing zone; switch settings select each drive. | pp. 2--3 | 86Box recognizes an IBM/Xebec family. **Manual L3** immutable geometry input; profile/session selection is outside List 1. |
| X5 | Completion is a data-stack status byte: bit 5 logical unit, bit 1 error, remaining listed bits zero; when enabled, IRQ follows readiness to transfer it and busy clears when it transfers. | p. 4 | PCjs separately identifies this byte. **Manual L3** response order and result bits. |
| X6 | Error completion is followed by four sense bytes with printed address-valid, drive/head/cylinder/sector and error layout; printed error-table values remain finite outcomes. | pp. 4--7 | Both references keep a sense structure. **Manual L3** response/sense grammar; no invented error class. |
| X7 | CPU port grammar is exactly data `320h` R/W; hardware status `321h` R and reset W; switches `322h` R and controller-select pulse W; `323h` R unused and W DMA/interrupt mask pattern. | p. 14 | 86Box and PCjs match the four ports. **Manual L3**. |
| X8 | AEN disables CPU I/O decoding during DMA; DMA access uses the data signals, not a CPU-port alias. | pp. 14--15 | 86Box has a distinct DMA path. **Manual L3** causal routing. |
| X9 | DRQ3 asserts when DMA data is available and remains asserted until DACK3; IRQ5 is controller-driven when enabled to report ending status. | p. 15 | 86Box and PCjs retain DMA/IRQ state. **Manual L3** signal/order relation; electrical pulse width is not specified. |
| X10 | RESET forces initial power-up condition; the option window is `C8000h`--`C9FFFh`. | p. 15 | 86Box/PCjs load external ROMs. **Manual L3** facts; ROM bytes and boot execution remain BYOB/profile work. |
| X11 | `00h` Test Drive Ready, `01h` Recalibrate and `03h` Request Sense Status have the printed DCB forms; `02h` is reserved. | p. 10 | 86Box/PCjs name the same public family. **Manual L3** grammar; drive-ready and seek signals are physical inputs, not synthesized. |
| X12 | `04h` Format Drive, `05h` Read Verify, `06h` Format Track and `07h` Format Bad Track have the printed DCB forms. | pp. 10--11 | 86Box implements richer media behavior. **Manual L3** request grammar; format/verify physical result is unsupported from a logical image unless a later receiver supplies the needed model. |
| X13 | `08h` Read Data and `0Ah` Write use printed CHS/control fields and block count; `09h` is reserved. | p. 11 | Both references implement corresponding logical transfer families. **Manual L3** command grammar and logical-sector receiver; service timing is not inferred. |
| X14 | `0Bh` Seek and `0Ch` Initialize Drive Characteristics have printed forms; Initialize is followed by exactly eight printed parameter bytes. | p. 12 | PCjs retains the eight-byte extension. **Manual L3** finite collector; physical seek completion is not guessed. |
| X15 | `0Dh` Read ECC Burst Length, `0Eh` Read Buffer and `0Fh` Write Buffer have printed forms. | p. 12 | 86Box has equivalents. **Manual L3** grammar; ECC/buffer semantics require a source-qualified receiver before claiming logical-image behavior. |
| X16 | Class 7 publishes RAM diagnostic `00h`, drive diagnostic `03h`, controller internal diagnostics `04h`, Read Long `05h`, Write Long `06h`; `01h`/`02h` are reserved. | pp. 12--13 | 86Box implements extra physical detail. **Manual L3** command universe; diagnostics/long/ECC physical behavior is explicitly unsupported. |
| X17 | The manual leaves controller hardware-status bit meanings incomplete beyond its distinct data-stack completion status and stated transfer ordering. | pp. 1, 4, 14--15 | PCjs labels its status-bit interpretation and status-read transitions as guesses/hacks. **L1** causal ordering only; no synthetic register-bit contract. |
| X18 | The source names ECC, index, ready, seek-complete, write fault, track-0, long sectors and diagnostics, but does not supply a complete logical-image physical drive model or service-time formula. | pp. 4--16 | 86Box's timers and physical models are implementation choices. **Unsupported physical boundary**; no L2 numeric estimate is admitted. |

## Cross-Model Reconciliation

The local 86Box source provides one IBM/Xebec variant, separate `320h`--`323h`
handling, DMA/IRQ state and a broader ST-506 family. Its non-Xebec variants,
ROM dependency and `250 us`/millisecond timer constants do not belong in this
IBM List 1. Local PCjs separates XTC from ATC and agrees on the byte stack,
ports, six-byte request and IRQ5/DMA pattern, but explicitly documents guessed
hardware-status bits and BIOS-driven status hacks; those are excluded.

No local MAME or QEMU Xebec implementation is present. The local Bochs
checkout provides ATA-oriented hard-disk behavior, not this controller, so it
is non-applicable rather than negative evidence. No conclusion depends on an
unavailable or non-applicable reference.

## Completion Predicate For S3

All 18 source families have one disposition. S3 must map every row to current
HDC/media/DMA/PIC/profile ownership and identify one complete implementation
batch, or explicitly prove a row already has its source-qualified sole path.
No code may repair an isolated row before that List 2 is accepted.
