# T484 S17 Xebec Type-2 DMA Media

`M5:T484:S17:XEBEC-DMA-MEDIA:OK`

S17 completes the source-bounded logical-media receiver inside the existing
sole Core HDC owner.  It adds no ATA task-file reuse, VM memory/media route,
second CHS state, sector cache, physical MFM model, or Xebec object.

## Contract and implementation

IBM 6139790 describes `08h` Read Data as one 512-byte logical sector and
`0Ah` Write Data with DCB byte 4 as its block count.  The selected Type-2
descriptor remains the only allowed media geometry: 615 cylinders, four heads,
17 sectors/track, 512 bytes/sector and 41,820 logical sectors.

The Xebec HDC branch decodes the DCB's zero-based CHS fields against that
descriptor, then obtains bytes only through the frozen Core media registry.
Read loads the already-owned 512-byte HDC buffer and asserts its board-provided
DMA3 request.  Each DMA device-to-memory service consumes one byte; the 512th
byte releases DRQ and produces the normal one-byte completion response.  Write
uses the same buffer and DMA3 memory-to-device path, persists each completed
sector through the same registry, and advances only the HDC-owned DCB CHS state
for a positive documented block count.  A zero count is rejected: neither IBM's
text nor the retained corroboration proves its exact convention, so S17 does
not invent a 256-sector interpretation.

The board retains the DMA controller and validates the private request binding;
HDC receives only narrow assert/deassert callbacks.  Thus HDC owns when a
transfer is requested, while the board/DMA owner owns servicing it.  A terminal
count before a completed 512-byte sector releases DRQ and returns the sourced
error-result shape.  `323h` remains raw pattern state: its enable-bit meanings,
hardware-status read, completion IRQ gating, ROM and physical service timing
remain explicitly unsupported rather than guessed.

## Focused proof

`core-machine-xebec-wiring-smoke` binds a project-owned, frozen in-memory
Type-2 media provider and proves all of the following through the actual Core
port, HDC and DMA-provider boundary:

- `08h` enters the DMA-read phase, asserts DMA3, returns all 512 seeded bytes,
  then releases DMA3 and returns success;
- early terminal count releases DMA3 and returns an Xebec error response;
- `0Ah` with a count of one enters DMA-write, receives 512 bytes, releases DMA3,
  returns success and changes only the media provider's sector; and
- S15 DCB/sense/reset/323h state and the no-ATA-alias boundary remain covered.

Focused retained regressions are `core-machine-hdc-smoke`,
`core-machine-compaq-hdc-s5-smoke`, and
`core-machine-compaq-hdc-machine-s5-smoke`.

The external 86Box and PCjs trees were read-only corroboration only.  They
confirm a separate byte-stream/DMA controller personality, but do not override
IBM's manual; PCjs itself records uncertainty around the Write count's transfer
extent.  No third-party source, firmware or media is imported.
