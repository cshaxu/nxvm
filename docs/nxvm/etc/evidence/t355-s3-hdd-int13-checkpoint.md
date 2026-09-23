# T355 S3: HDD/INT13 readiness checkpoint

## Approved input boundary

S3 validated the local logical asset `runtime-hdd-ewin31-setup` against the
size and SHA-256 already approved in
[Current Runtime Media](fixtures/current-runtime-media.md).  The comparison
passed.  No local path, duplicate hash, media bytes, MBR/VBR content, or guest
artifact is retained here or elsewhere in Git.

The run used only the S2 `run-windows31-hdd-checkpoint` command.  That command
forwards its explicitly configured local HDD input to the existing
`vm-windows31-hdd-admission-probe`; the probe uses a temporary synthetic FDD
and deletes it on completion.  It does not execute Windows Setup.

## Checkpoint result

The probe completed with both markers:

```text
M5:T287:S16:HDD-ADMISSION:OK lba=63 chs=0/1/1 spt=63 heads=16 ata_commands=2
M5:T355:S2:WINDOWS31-HDD-CHECKPOINT:OK
```

This is a stable, bounded result: BIOS `INT 13h` geometry and two read
requests reached the retained ATA path, and the guest-visible MBR/VBR bytes
matched the owner-supplied read-only HDD input.  It reaffirms T347/T354 as a
working storage/readiness prerequisite for this checkpoint.

## Boundary and transfers

The result is a host-observation diagnostic, not a guest-time measurement,
Windows boot, Setup, Standard Mode, Enhanced Mode, driver, or compatibility
claim.  `vm_windows31_checkpoint`, `vm_windows31_setup_probe`, and
`vm_windows31_int13_trace_probe` remain unexecuted, non-current diagnostic
tools.  No new core/device defect is selected from this successful checkpoint.

S4 must reconcile this result with the four-probe inventory, current-gate
partition, media/provenance record, and the device/timing transfers already
named by T346--T354.  Any later Windows interaction needs a separately
admitted checkpoint; it cannot silently reuse this success as a broader
acceptance criterion.
