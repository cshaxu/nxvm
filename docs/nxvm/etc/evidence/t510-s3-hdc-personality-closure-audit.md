# T510 S3 HDC Personality Pre-Setup Verification

`M5:T510:S3:HDC-PERSONALITY-PRE-SETUP:PARTIAL`

## Final Ledger Disposition

| Personality | Final construction input | Sole Core completion path | Disposition |
| --- | --- | --- | --- |
| default-AT ATA PIO | `200/200` Other-L2 | task-file pending command, then pending read/write sector | accepted L2 input |
| IBM 5170 WD1003/ST-506 | `16000/7840` Other-L2 | the same task-file pending command and sector transitions | accepted L2 input |
| DeskPro Model 40 Compaq/WD | `0/0` | the same task-file transitions at the owner deadline | explicit non-eligible duration boundary |
| IBM 5160 Xebec DMA | `250/0` Other-L2 | Xebec `PENDING_COMMAND`, then existing DMA/result route | accepted L2 command input; no invented inter-sector phase |

All four configurations enter `core_machine_hdc`; only
`core_machine_hdc_schedule_service`, `core_machine_hdc_advance_elapsed` and
`core_machine_hdc_next_due_tick` write, complete and publish an HDC deadline.
The sweep finds no `service_ticks` construction, VM time writer, HDC-specific
scheduler, controller alias, media cache or profile-specific Core branch.

## Pre-Setup Verification

| Check | Result |
| --- | --- |
| Complete repository-only unit gate | 313/313 passed in 14.11 s |
| External-asset integration gate | 20/20 passed in 11.79 s, including DOS, FDC, HDD/ATA and Windows 3.1 checkpoints |
| Documentation governance | passed |
| Current Release target | `vm-0-5-0508` rebuilt successfully |
| Compiler debug sections | none reported by `objdump -h` for the produced executable |
| Release artifact | `build/output/nxvm_0_5_0508.exe`, SHA-256 `1CD6A4ED85A4AC71EB3074274B487F7C6FBC93E4A832D20949079DAB50554550` |

The release preserves the runtime debugger. Stripped concerns compiler
debug information only; it does not remove that product feature.

This pre-Setup result did not cover the owner-reported current-HDD
`EWIN31\\SETUP.EXE` path. Its follow-up repair and successful Setup-entry
checkpoint are recorded in
[the S3 regression evidence](t510-s3-ata-pio-setup-regression-repair.md).
