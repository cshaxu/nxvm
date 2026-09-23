# M5 T34 S3 - Battery RAM Core Evidence

Core accepts battery backing only for the finite Mapper-4 profile that declares
8 KiB PRG-RAM. The cartridge retains `battery_backed` and `prg_ram_dirty`.
Writable enabled RAM marks dirty; fixed-size export is read-only; exact-size
import restores bytes and clears dirty. Other battery formats remain rejected.

The Mapper-4 fixture proves admission, dirty transition, export, import and
write protection. Cartridge-contract proof covers accepted and rejected header
boundaries. Both x64 and x86 focused runs passed.
