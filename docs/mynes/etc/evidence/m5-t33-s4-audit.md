# M5 T33 S4 - MMC3 Closure Audit

The independent audit traced the T33 design rows against constructor admission,
Cartridge/Bus/PPU behavior, direct fixtures and S3 regression evidence.

| Design row | Receiver | Decision |
| --- | --- | --- |
| 32--512 KiB PRG, 0--256 KiB CHR | Descriptor bounds and four-corner fixture | Pass |
| CHR-ROM / 8 KiB CHR-RAM | Allocator and shared MMC3 PPU read/write offset fixture | Pass |
| Bank pairs/modes, fixed banks, H/V mirror, qualified A12 IRQ | Mapper-4 smoke fixture | Pass |
| `$A001` PRG-RAM enable/protect | Cartridge + Bus and Mapper-4 smoke fixture | Pass |
| Full x64/x86 product regression | S3, 114/114 on each architecture | Pass |
| Generic, SMB3, TMNT3 optional lifecycle inputs | S3 product probe, no configured ROM input | Pass with explicit no-input boundary |
| Declared optional 8 KiB PRG-RAM format | iNES descriptor header validation | **Finding** |

The descriptor rejects every nonzero iNES header byte 8 before mapper profile
selection. That field declares PRG-RAM capacity in iNES 1.0, so a non-battery
standard Mapper-4 ROM that declares its 8 KiB RAM cannot reach the otherwise
implemented `$A001` behavior. This is a format-admission gap, not a Lib/Common
or App concern. Battery-backed persistence remains deliberately transferred to
T34; S5 must admit the non-battery, 8 KiB declaration and reject unsupported
sizes/flags explicitly with direct fixtures.

T33 cannot close. S5 is required to resolve the finding, then S6 will perform
the final independent closure audit.
