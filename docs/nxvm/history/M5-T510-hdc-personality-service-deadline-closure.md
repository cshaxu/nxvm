# M5 T510: HDC Personality Service-Deadline Closure

T510 closes the selected fixed-disk personality service boundary through one
Core HDC owner and one media boundary.

## Accepted Result

The [List 1](../etc/evidence/t510-s1-hdc-personality-function-timing-list-1.md)
and [List 2](../etc/evidence/t510-s1-hdc-personality-current-gap-list-2.md)
exhaust the selected default-AT ATA, IBM 5170 WD1003/ST-506, DeskPro Compaq/WD
and IBM 5160 Xebec personalities. The accepted batch makes task-file command
and next-sector service distinct immutable Core inputs, reuses the existing HDC
deadline owner for delayed Xebec DCB completion, and removes the unqualified
Model-40 duration.

The retained configurations are ATA `200/200` Other-L2, Model-339 `16000/7840`
Other-L2, Model-40 `0/0` explicit non-eligible duration, and XT Xebec `250/0`
Other-L2. This does not claim physical media mechanics: Xebec has no invented
inter-sector phase, and Model-40 has no fabricated duration. Core owns phase,
elapsed time, deadline and completion; VM composition copies frozen generic
values only.

T510 additionally repaired the 0508 ATA PIO consumer regression exposed by
Windows Setup: the default-PC/AT INT 13h loop now waits for DRQ at each
multi-sector boundary. It does not reintroduce immediate completion or change
the HDC deadline model.

## Verification

The [S3 repair evidence](../etc/evidence/t510-s3-ata-pio-setup-regression-repair.md)
records an actual approved-HDD Setup-entry result at `Welcome to Setup.`, the
two-sector read/write regression, unit 313/313, integration 20/20 and the
rebuilt stripped Release 0508 artifact: `nxvm_0_5_0508.exe` SHA-256
`6578166FB910433AE1BC78050E7756D8938ADCBF2859AB2E4ACCE151785360F0`.
