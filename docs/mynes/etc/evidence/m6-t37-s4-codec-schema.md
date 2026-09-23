# M6 T37 S4 Snapshot Codec Schema

S4 replaces the former raw-layout image with an ordered little-endian stream.
Each field has its declared scalar width; byte arrays are written as their
declared element sequence and `lib_bool` is one byte restricted to zero or one.
The reader first fills a candidate machine and candidate mapper state, then
commits only after every field and RAM payload has validated.

## Header and identity

`MNS1`, format version, cartridge content identity, mapper number, PRG-RAM
length and CHR-RAM length are fixed-width fields. They are validated before
any candidate state is read.

## Machine state

The payload covers CPU registers and RAM; interrupt, DMA and DMC-DMA state;
cycle/slot/instruction counters; breakpoint and trace state; controller latch
state; PPU registers, memory, sprite evaluation, shifters, delayed pixels,
timing and completed frame; and APU registers, timers, counters, sequencer and
IRQ state. Every multi-byte value is emitted little-endian.

## Cartridge state

The payload covers mirroring, Mapper 1 shift/register state, Mapper 2/3 bank
selection, Mapper 4 bank/IRQ/A12/PRG-RAM state, battery/dirty flags, followed
by PRG-RAM and CHR-RAM bytes when those memories exist. ROM pointers, ROM
bytes, RAM pointers and `lib_size` values are not serialized.

## Excluded host output

APU PCM FIFO samples, FIFO cursor/count and dropped-sample telemetry are host
delivery state rather than emulated hardware state. They are reset before the
candidate is committed; Driver already clears its own staging queue and Lib
stream after a successful restore.

## Compatibility and verification

Version 2 deliberately rejects version 1 raw-layout images. A bad magic,
version, identity, mapper, enum, boolean, count, RAM read or truncated payload
does not reach the live machine. The codec writes the declared 8192-byte MMC3
PRG-RAM payload rather than the unrelated PRG-ROM byte count.

`mynes.core.snapshot-smoke` proves round trip plus live-state preservation for
bad magic, truncation, version and a malformed serialized boolean. The Core
and App snapshot transaction tests pass for both x64 and x86.

The deterministic fixture also asserts the exact version-2 payload length
(`136006`) and FNV-1a byte hash (`5e1fb997`) on both targets. This proves the
serialized stream is identical across x64 and x86, rather than merely
independently readable.
