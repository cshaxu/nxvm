# M6 T39 TMNT3 Start-Path Diagnosis And Correction

T39 separated a real owner report from a broad compatibility claim. S1 proved
that Start reaches the standard controller serial bit and is consumed by the
ignored owner-local game input; it added a project-owned A/Start serial
regression. S2 then identified and repaired the omitted PPU A12 publication
from completed `$2006` writes and pattern-table `$2007` writes.

PPU is the sole publisher of represented A12 bus levels and Cartridge remains
the sole owner of the filtered MMC3 counter and IRQ. The focused Mapper-004
fixture proves a qualified `$0000` to `$1000` PPUADDR transition clocks an
enabled zero-latch IRQ. Full MyNES CTest suites passed in both configurations:
53 product cases on x64 and 53 on x86. Owner-local ROM data, paths, digests and
raw traces remain absent.

The closure does not claim every MMC3 revision, MMC6 or all-game compatibility.
The repaired missing standard Mapper-004 behavior is now covered without a
protected-ROM dependency.
