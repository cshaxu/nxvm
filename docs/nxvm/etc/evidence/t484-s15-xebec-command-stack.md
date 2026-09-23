# T484 S15 Xebec Command Stack

`M5:T484:S15:XEBEC-STACK:OK`

`M5:T484:S15:XEBEC-NO-ATA-ALIAS:OK`

## Implemented Core Contract

The selected Xebec personality now has one bounded substate inside the sole
`core_machine_hdc` owner.  It is not a second controller and does not share
ATA task-file fields, command phases, status bits, CHS progression or PIO
words.  The accepted `320h`--`323h`, DMA3 and IRQ5 board binding remains
unchanged.

The Core path is `idle -> 322h controller-select pulse -> six-byte DCB ->
response -> idle`.  A normal DCB is exactly six bytes; Initialize Drive
Characteristics (`0Ch`) collects its source-defined eight trailing bytes
before producing a result.  Writing `321h` resets the state.  `323h` retains
the raw pattern byte and reset clears it; no bit meaning is invented.

Every IBM-defined command opcode is recognized.  An undefined opcode returns
the IBM invalid-command sense code `20h`.  With no profile-selected printed
drive type or logical-media binding, defined commands return the sourced
drive-not-ready sense code `04h`; they do not fall through to ATA or pretend
that a sector was transferred.  Completion contains only the source-defined
logical-unit/error bits.  Request Sense (`03h`) returns the saved four-byte
sense payload and consumes it, rather than incorrectly appending it to the
completion byte.

## Evidence Boundaries

IBM 6139790, pp. 4, 7--16, is the Manual-L3 authority for DCB shape, finite
command grammar, completion/sense layout, Initialize's eight bytes and error
codes.  Its interface table proves 321h reset, 322h select pulse and 323h
pattern write.  Read-only 86Box confirms the select-before-DCB ordering and
separate Request Sense data path.  PCjs independently stores the pattern but
does not define its bit semantics; therefore neither DMA-enable nor IRQ-enable
bits are promoted above L1.

`core_machine_hdc_dma_provider()` is now the owner-local DMA3 provider offered
to the existing board binding.  It intentionally has no byte callbacks: no
selected printed geometry/media exists, and no DRQ transfer can truthfully be
started.  The future selected-drive receiver must add the 512-byte logical
read/write DMA callbacks in this same HDC state; it may not add a VM memory
path, media cache or second CHS owner.  IRQ5 enable behavior, hardware-status
bits, physical MFM/ECC/service time and ROM mapping remain explicit transfers.

## Verification

The Xebec smoke proves no-pulse writes are ignored, pulse-gated DCB collection,
completion versus Request Sense separation, invalid-command versus not-ready
classification, Initialize's eight-byte extension, raw 323h reset and no ATA
port alias.  Existing ATA/WD1003 and Compaq HDC regressions pass.  A full
Debug build and the complete `current-gate` pass.  No firmware, guest image or
third-party code is imported.
