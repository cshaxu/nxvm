# T484 S14 Xebec Logical Implementation Brief

`M5:T484:S14:XEBEC-LOGICAL-CONTRACT:OK`

`M5:T484:S14:XEBEC-NO-GUESSED-STATUS:OK`

`M5:T484:S14:XEBEC-IMPLEMENTATION-BRIEF:OK`

## Decision

The accepted Xebec plan is a controller personality of the existing
`core_machine_hdc`, not a second controller or an ATA adaptation.  The next
implementation receiver must add its command/response stack, DCB fields and
DMA3 callbacks inside that sole owner.  It retains the existing Core media
registry as the sole image, geometry and persistence owner and uses the
existing HDC 512-byte buffer only while a logical sector is in flight.

The profile contributes only an immutable selected printed drive type and its
media binding at construction.  It cannot mutate a command, response, CHS,
DMA, IRQ or media-cache state at runtime.  No current profile makes that
selection, so this brief does not make an XT disk runnable.

## Complete X1--X17 Receiver Matrix

| Rows | Source-proven fact | Current state | Smallest correct receiver |
| --- | --- | --- | --- |
| X1--X2 | DMA3, IRQ5 and `320h`--`323h` have the IBM byte-stream grammar. | S13 provides the tagged four-port, DMA3 and IRQ5 wiring. | Keep that plan; replace only the Xebec provider's unsupported branch. |
| X3--X4 | DMA, not CPU port I/O, transfers data; DREQ persists through DACK and enabled IRQ5 reports completion. | The board owns an HDC DMA3 binding, but its callbacks are empty. | One HDC DMA provider asserts/deasserts the existing binding and transfers one byte between the Core buffer and DMA latch; Core HDC alone raises/clears IRQ5 at sourced phase boundaries. |
| X5--X6 | A six-byte DCB is collected through the data stack. | No Xebec data-stack state exists. | Add a bounded six-byte request collector and a bounded response/sense stack to the Xebec branch of the existing HDC state. |
| X7 | Only printed types 1, 2, 13 and 16 may be selected. | No selected geometry or media exists. | A later immutable profile/media plan validates one printed type against the existing media geometry before commands may use it. |
| X8--X9 | The data stack returns completion status; bit 5 is logical unit, bit 1 is error; an error has four sense bytes. | No response stack exists. | The Xebec state owns the response cursor and four-byte sense payload.  It publishes only these sourced result bits; it does not reuse ATA status/error bits. |
| X10--X12 | The command grammar is finite; logical read/write use 512-byte sectors and DCB count; initialize has eight trailing bytes. | No command decoder exists. | One finite Xebec decoder consumes exactly its source-defined request bytes.  Logical read/write reuse the existing HDC buffer and media registry only after a selected geometry exists.  Physical-only commands have documented unsupported results, not ATA behavior. |
| X13 | DCB control records 200us, 70us and 3ms step selectors. | No Xebec control state exists. | Preserve only the source-defined selector value in the Xebec command state; it creates no deadline without a source-backed board/media timing receiver. |
| X14 | Hardware-status bit meanings are incomplete. | The Xebec hardware-status read is unsupported. | Retain unsupported rather than invent bit values or transitions.  Completion belongs to the sourced response stack and IRQ ordering. |
| X15 | Physical media, ECC, long-sector, format, recovery and drive-signal behavior require facts not present in a logical image. | Not implemented. | Keep unsupported; no synthetic MFM, rotational time, seek time, status or cache. |
| X16 | The option ROM occupies `C8000h`--`C9FFFh`. | No ROM selection or mapping exists. | Future BYOB-ROM/profile receiver only; no bytes or path enter this implementation. |
| X17 | The adapter does not define a generic XT profile. | No XT fixed-disk profile exists. | Future selected-5160 construction receiver only; no generic configuration path. |

## Exact Code Boundary

`src/core/machine/hdc.c` currently returns `TYPE_STATUS_UNSUPPORTED` for the
Xebec tag before entering its task-file code.  That separation is retained.
The receiver adds `core_machine_hdc`-local Xebec port handlers and a provider
returned by `core_machine_hdc`; `machine_board.c` replaces its S13 empty HDC
DMA provider with that provider, retaining the already-held machine-private
DMA3 binding.  It must use the same Core PIC source and media registry already
connected by `core_machine_hdc_connect`.

This explicitly excludes reuse of task-file `status`, `error`, drive select,
LBA, PIO words or ATA command phases as Xebec state.  A small selected-personality
substate within the one HDC owner is permitted; a sibling Xebec object, a VM
transfer path, a second CHS cursor, a second sector buffer or a media cache is
not.

## Source And Cross-Check Basis

IBM *PC 20MB Fixed Disk Drive Adapter*, 6139790, printed pp. 1--17, remains
the normative rendered-source basis for every row.  The accepted S12 record
cross-read those pages and used the read-only 86Box XTC and PCjs HDC routes
only as Other-L3 confirmation of the separate byte-stream/DMA shape.  Their
timers, ROM handling, variants and PCjs hardware-status guesses are not
admitted behavior.

## Focused Proof Required Of The Receiver

The implementation must prove all of the following without a profile, ROM or
guest image: exact four-port directions; six-byte collection; bounded response
and error-sense reads; no task-file/ATA alias; data changes only through the
DMA3 provider; DREQ release and IRQ5 completion ordering; reset and mask
handling; and explicit unsupported physical commands/status.  Existing ATA,
Compaq WD and IBM WD1003 smokes remain boundary regressions.

This brief makes no service-time, host-pacing, raw-media, exact hardware-status
or runnable-XT claim.
