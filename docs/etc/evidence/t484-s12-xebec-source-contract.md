# T484 S12 XT Xebec Source Contract

`M5:T484:S12:XEBEC-SOURCE-CONTRACT:OK`

`M5:T484:S12:XEBEC-NO-ATA-ALIAS:OK`

## Source Qualification

The normative source is IBM, *PC 20MB Fixed Disk Drive Adapter*, 6139790,
17 March 1986, printed pp. 1--17.  Its OCR is adequate for prose discovery,
but every port, DCB, status, geometry and interface fact below was cross-read
against the rendered table/page.  The source describes an optional expansion:
factory 5160-268 has no fixed disk, so no drive type or image is selected here.

86Box `src/disk/hdc_st506_xt.c` and PCjs
`machines/pcx86/modules/v2/hdc.js` were inspected read-only as Other-L3
observations.  Both retain a separate Xebec/XTC byte-stream route, its four
port family, DMA/IRQ enable state and six-byte command form.  PCjs explicitly
labels several hardware-status meanings and transitions as guesses; 86Box adds
ROM loading, timers and controller variants.  Neither implementation supplies
normative bits, media geometry, physical timing or source code for NXVM.
No MAME, Bochs or QEMU checkout was available for this bounded recheck; no
conclusion relies on that absence.

## Reconciled X1--X17 Contract

| Rows | IBM Manual result | Other-L3 comparison and admitted disposition |
| --- | --- | --- |
| X1 | One/two-drive adapter uses system DMA and IRQ5 (pp. 1, 14--15). | 86Box and PCjs both retain XTC IRQ5. **Manual L3:** one Core HDC/PIC/DMA3 route. |
| X2 | `320h` data R/W; `321h` status R/reset W; `322h` switch R/select-pulse W; `323h` unused R/mask-pattern W (p. 14). | Both match the four-port grammar. **Manual L3:** no task-file alias. |
| X3 | AEN disables the CPU I/O decoder during DMA (p. 14). | 86Box has a separate DMA path. **Manual L3:** DMA data transfer must not reuse CPU ports. |
| X4 | DRQ3 persists until DACK3; enabled IRQ5 returns ending status (pp. 14--15). | Both retain DMA/IRQ enable state. **Manual L3** for causal handoff; electrical duration is transferred. |
| X5--X6 | Data stack receives a six-byte DCB: opcode, drive/head, 10-bit cylinder/6-bit sector, count/interleave and controls (pp. 1, 7). | Both use a six-byte XTC block. **Manual L3:** one byte-stream collector/response stack. |
| X7 | Printed types 1, 2, 13, 16 give cylinder/head/precomp/landing values (pp. 2--3). | 86Box recognizes this Xebec family. **Manual L3:** a later BYOB declaration may select only a printed type; none is selected now. |
| X8--X9 | Completion status is returned by the data stack; bit 5 is logical unit and bit 1 error; error results have four sense bytes (p. 4). | PCjs distinguishes data status from hardware status. **Manual L3:** retain response/sense ownership in Core. |
| X10--X12 | Finite command grammar includes ready/recalibrate/sense/format/verify/read/write/seek/init/buffer/ECC/diagnostic/long operations; initialize takes eight extra bytes (pp. 10--16). | Both implement the public set. **Manual L3** grammar; physical operations may fail explicitly, never imitate ATA. |
| X11 | Logical reads/writes use 512-byte sectors and DCB block count (pp. 10--14). | 86Box has a 512-byte buffer. **Manual L3:** use the existing sole logical-media provider and Core sector buffer. |
| X13 | Control step selectors include 200 us, 70 us and 3 ms; unspecified patterns are not assigned (p. 7). | 86Box delay choices add no authority. **Manual L3** selector semantics, but no deadline until board/media timing is sourced. |
| X14 | Manual does not completely define controller hardware-status bits beyond transfer/status ordering (pp. 3--4). | PCjs marks several bits/transitions as guesses. **L1:** preserve only sourced causal ordering; do not publish guessed bits. |
| X15 | ECC, long sectors, format/recovery, ready/index/seek signals and diagnostics require physical drive/media signals (pp. 4--16). | 86Box has richer physical behavior. **L1:** unsupported from a logical image; no estimated service time. |
| X16 | Option ROM occupies `C8000h`--`C9FFFh` (p. 14). | Both load external ROMs. **Manual L3 fact**, transferred to the future BYOB ROM mapping receiver; no bytes/path enter NXVM. |
| X17 | IBM source defines an adapter, not a generic XT profile or bundled resources. | Both projects need their own resource policy. **Admission boundary:** later fixed profile/session only. |

## Current-Owner Gap And Next Mechanism

Current `core_machine_hdc_config` admits only ATA PIO, Compaq WD 40 MB and IBM
WD1003/ST-506; it is an unconditional nine-register task-file shape.
`machine_board.c` validates and registers that shape, while `hdc.c` transfers
task-file words.  Core already owns HDC state and the sole logical-media
boundary, but it has no HDC DMA3 binding or Xebec personality.  This confirms
the retained T479 diagnosis without widening it:

```text
profile -> copied tagged HDC plan -> Core HDC Xebec branch -> existing media provider
                                      |              |
                                    DMA3            IRQ5
```

The next implementation S must replace the universal task-file configuration
with one immutable tagged shape, preserve the three existing task-file
personalities and their tests, and register exactly the sourced Xebec ports.
It must not create a second controller, CHS state, media cache, ATA shim or
firmware path.

## Transfers

No local BYOB geometry/image is available, so no drive type is frozen.  The
future implementation accepts only a validated selected printed geometry from
the profile/session construction boundary.  Physical MFM/ECC, drive signals,
rotation/seek/service duration and adapter ROM mapping remain separate future
receivers.  This S makes no runnable-5160 or host-pacing claim.

## Coordinator Acceptance

The coordinator rechecked every X1--X17 row against the accepted primary
source record, the two read-only Other-L3 observations and the actual current
HDC shape.  The new record imports neither code nor protected resources and
does not claim a selected disk or timing.  It identifies one cohesive next
mechanism: a tagged immutable Core HDC plan and DMA3/IRQ5 wiring seam.  S12 is
accepted and that bounded mechanism is S13's sole receiver.
