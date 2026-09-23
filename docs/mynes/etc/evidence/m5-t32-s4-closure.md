# M5 T32 CNROM Closure Audit

T32 closes. Mapper 3 is admitted only for the finite 32 KiB PRG / 8--32 KiB
CHR profile. Cartridge alone owns fixed PRG reads, selected CHR reads, AND
conflict latch and fixed mirroring; Bus and PPU remain generic. The direct
fixture proves reset and all four CHR banks, conflicts, low-address no-op,
CHR-ROM write rejection and H/V mapping. Synthetic Media/Driver replacement
runs without a trap and retains the accepted machine on invalid replacement.

S3 records x64 and x86 114/114 full CTest passes. CNROM has no supplied
commercial input, so all proof remains expressly fixture qualification. Owner
ROMs remain ignored, artifacts remain tracked and documentation gates pass.
T33 is the next Mapper-4 completion receiver.
