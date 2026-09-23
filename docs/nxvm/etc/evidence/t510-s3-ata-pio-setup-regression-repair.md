# T510 S3 ATA PIO Setup Regression Repair

`M5:T510:S3:ATA-PIO-SETUP:OK`

## Reproduction And Cause

The owner reports that Release 0507 reaches Windows Setup from the approved
`runtime-hdd-ewin31-setup` input while Release 0508 stalls. The 0508 ATA
deadline change correctly made command completion asynchronous, but its
default-PC/AT INT 13h firmware consumer waited for DRQ only once, before a
multi-sector PIO transfer. After each 512-byte sector the Core HDC correctly
entered its pending next-sector phase; the firmware incorrectly kept reading
or writing the data port without a new DRQ observation.

## Sole Repair

The existing INT 13h read and write loops now retain their single total-word
counter and inspect the low-byte sector boundary. At each non-final boundary
they poll the existing ATA status port until BSY clears, reject ERR, require
DRQ, then resume the same data loop. No HDC phase, media owner, timing value,
profile configuration, VM waiting route or YAML file changes. Core remains
the only owner that reaches the pending deadline and publishes DRQ.

The existing ROM INT 13h smoke now creates two HDD sectors with distinct
initial contents, writes two sectors, reads them back through INT 13h, and
asserts both words. It fails if either inter-sector wait is omitted.

## Actual External-Asset Checkpoint And Gates

The existing `vm-windows31-setup-probe` ran against the approved logical HDD
input and reached `Windows Setup` / `Welcome to Setup.` after typing the
existing `EWIN31\\SETUP.EXE` command. This is the required Setup-entry
checkpoint; it makes no claim about completing installation.

| Check | Result |
| --- | --- |
| Focused two-sector INT 13h smoke | passed |
| Actual HDD Setup-entry probe | passed; `Welcome to Setup.` observed |
| Complete repository-only unit gate | 313/313 passed in 13.05 s |
| External-asset integration gate | 20/20 passed in 12.05 s |
| Stripped Release target | `vm-0-5-0508` rebuilt; no `.debug` or `.zdebug` section reported |
| Release artifact | `nxvm_0_5_0508.exe`, SHA-256 `6578166FB910433AE1BC78050E7756D8938ADCBF2859AB2E4ACCE151785360F0` |
