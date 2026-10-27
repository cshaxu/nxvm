# M5 T580 S74 — PC110 partition boot investigation

## Input and boundary

This continuation uses the unchanged TFT firmware and the S73 task-local,
geometry-adjusted test master, plus the original Personaware image as the
read-only slave. The master is not a factory disk dump; its preparation,
provenance and SHA-256 are recorded in the S73 evidence. The emulator uses
overlays and does not write either source image. The Setup reference is a
comparison oracle, not injected guest state.

The diagnostic reads the active partition entry from the master independently
and compares all 512 bytes at the guest's second 7C00h execution point. This
tests actual BIOS/MBR loading, not DOS startup. The subsequent system-file
checkpoint is a separate diagnostic.

## Observations

Both host widths reached the primary MBR and the matching active partition
boot sector: `boot-sector=1`, `partition-sector=1`, `POWER:result=1`,
and `fault=0`. The diagnostic records the first instruction after its
retirement, hence 0000:7C01 for the MBR and 0000:7C3E after the partition
sector's initial jump. The bounded wrapper did not retain a native exit code;
the explicit pass marker and diagnostic's source return contract are the
evidence, not a fabricated wrapper result.

- x64 log SHA-256: `55bf3be36323a7e10681069c267078ec0862c40b03d0cdd089c5d918e7613ef8`.
- x86 log SHA-256: `7415249e0d12a15ed5d85f7ff8d3dbfe08802f2a37496fa10c4c3542fb2cbe4b`.

These runs predate a subsequent preflight guard for invalid media and the
optional next checkpoint; the normal successful path is unchanged. Both
architectures rebuilt afterward, and a missing primary image was rejected on
each with exit code 1. No production or shared component was changed.

On x64, the follow-up used the final diagnostic source and reached the
partition boot code's successful comparison of its first two system-file
names: `system-files=1`, target 0000:7D08 after instruction retirement,
`POWER:result=1`, `fault=0`, and 256,589,990 guest instructions. This is
evidence that the guest reached that branch, not that DOS has started or the
files were loaded successfully. The matching root names were also inspected
independently. The system-file watchpoint is specific to this documented test
disk's boot code; it is not a general DOS detection interface.

- x64 follow-up log SHA-256: `e4589f2abecac85905b5fe0cb6b6e069833a3099691ad88b16963bee44fa2b66`.
- MyPC110 units: 12/12 x64 and 12/12 x86. A nonexistent primary image is
  rejected with exit code 1 on each width.
- The original external image SHA-256 remains
  `5149db391d13cfeab330016fcf0edbe6b0d379cbb66a3aed91dbf7684142d52e`;
  the derived test master remains
  `3cd463cc5d05d17f4709da98f9e18f457bf7cce279a78bdacc12abc2da49598a`.

No desktop or RDP display verification, original DSTN firmware boot, actual
DOS prompt, or PC110 release qualification is claimed by these checks.
