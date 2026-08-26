# T484 S18 Xebec DMA3-to-RAM Route

`M5:T484:S18:XEBEC-DMA-RAM:OK`

S18 proves the full already-owned Core route, rather than adding a new
implementation path.  The focused Xebec smoke configures the existing primary
8237A channel 3 through its real ports with an inclusive count of 511 bytes,
unmasks that same channel, and advances the existing DMA service engine.

For Xebec Read `08h`, the HDC owns DCB validation and the sector buffer, raises
only its validated DMA3 request, and the 8237A device-to-memory transfer writes
all 512 seeded Type-2 bytes into Core RAM.  For Write `0Ah` with count one, the
test seeds Core RAM, configures the corresponding memory-to-device channel-3
mode, and proves all 512 bytes reach only the frozen media provider.  Both
directions reach terminal count, release DRQ and yield the existing Xebec
completion response.

This is deliberately a test/evidence-only result.  It introduces no DMA
wrapper, no HDC memory pointer, no VM-side transfer route, no second buffer or
media owner, and no ATA behavior.  The S17 early-terminal error cleanup and
the S15 command/reset/no-alias cases remain in the same focused smoke.

Physical MFM/ECC/service timing, `323h` DMA/IRQ mask-bit meanings, completion
IRQ gating, hardware status, option ROM and a runnable XT session remain
explicitly outside this source-bounded route.
