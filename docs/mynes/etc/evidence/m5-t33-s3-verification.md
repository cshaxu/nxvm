# M5 T33 S3 - MMC3 Regression Evidence

S3 extends the existing owner-local product probe with separate optional
`MYNES_OWNER_MMC3_SMB3_ROM` and `MYNES_OWNER_MMC3_TMNT3_ROM` inputs. Each uses
the normal read-only media lifecycle, requires Mapper 4, runs bounded Driver
slices without a trap, requires a published graphics or Core-generated text
frame, and drains queued audio when the heartbeat stops. The older generic
MMC3 input remains available.

The Mapper-4 direct owner header probe no longer assumes the old 128 KiB PRG /
128 KiB CHR fixture. It accepts the S2 finite profile: 32--512 KiB PRG and
8--256 KiB mapped CHR, including 8 KiB CHR-RAM.

No owner-ROM environment input was configured for this run, so no ignored ROM
path or byte was read or recorded. The no-input path passed on both supported
architectures.

Full serial CTest on 2026-09-22 passed:

| Architecture | Result |
| --- | --- |
| x64 | 114 / 114 |
| x86 | 114 / 114 |

The versioned product artifacts remain the rebuilt S2 pair in `assets/binary/`.
S4 independently audits the complete standard profile, its evidence, and the
transfer to persistence/release work.
