# M4 T22: MMC3 Cartridge Profile

T22 delivers the bounded no-trainer/no-battery Mapper-4/MMC3 profile needed by
the owner-local Super Mario Bros. 2 support input. It accepts exactly eight
16 KiB PRG-ROM and sixteen 8 KiB CHR-ROM units. CPU bank registers select the
four 8 KiB PRG windows, CHR registers select their documented 1 KiB/2 KiB
windows, and the cartridge controls vertical or horizontal CIRAM mapping.

The PPU reports actual pattern-table address levels to the cartridge. MMC3
advances only on a rising A12 edge preceded by at least eight low PPU ticks;
there is no host timer or scanline IRQ shortcut. The machine combines that
cartridge IRQ with the existing external IRQ line, and $e000 disables and
clears the mapper line.

Media intake now permits the historically documented 127- or 128-byte iNES
display-title suffix only at the removable-media boundary. Cartridge receives
the exact declared payload, so other surplus lengths remain rejected. The
owner-local Mario 2 image was inserted through that production path on x64
without copying ROM data.

`mynes.core.mapper4-smoke` covers CPU-bank modes, PPU-bank modes, mirroring,
qualified A12 edges and mapper IRQ assertion/deassertion. `mynes.core.media-
title-suffix-smoke` covers the title suffix and rejects a one-byte surplus.
The focused mapper/media/PPU/interrupt set passes on x64 and x86, and both
architectures build successfully.

The full CTest baseline currently has two failures on each configuration in
unchanged imported shared code: `library.types-layout-selftest` rejects an
existing `kvm-window`/`kvm-base` include, and `common.verifier-negative` fails
because its expected forbidden edge is accepted. Neither test nor its source
is modified by T22; this is recorded for shared-corpus reconciliation rather
than repaired locally. All M4-owned Core/App/integration tests pass.

This task does not claim Mario 2 execution or gameplay. Its APU register use,
audible output, pacing and dual-KVM acceptance remain transferred to the three
following M4 tasks. Lib and Common source remain unchanged.
