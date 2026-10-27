# M5 T580 S75 — PC110 system-loader handoff

## Method and input

The existing real-firmware diagnostic continues the S74 partition handoff.
For the reviewed task-local derived master, it reads the FAT boot parameters
and first root entry independently, computes the initial loader location,
and reads three sectors from the read-only disk. It then compares those bytes
with guest memory when execution reaches the selected boot code's 0000:0700
loader handoff. The original external image remains read-only; the derived
master is not a physical factory image. No guest boot bytes are injected.

For this input, independent inspection found 512-byte sectors, two sectors per
cluster, root at LBA 57, data at LBA 89 and the first system file in cluster 2.
The partition's loader copies three consecutive sectors and jumps to 0700h.
These observed facts identify this specific disk, not a universal DOS rule.

## Verification

Both host widths reached the primary MBR, the matching partition boot sector,
the system-name success branch and the matching three-sector loader. Each
reported `system-loader=1`, retired at 0070:0138 following the initial jump,
`POWER:result=1`, status 0 and Core fault 0 after 256,592,581 instructions.
The wrapper's native exit-code field was empty; the diagnostic pass marker and
its return contract are the evidence, not a fabricated native exit code.

- x64 log SHA-256: `dfe3dab1a529fe3e4df739e6607801992328043bc3bb97516023d07387298584`.
- x86 log SHA-256: `94245f8d05c490f2f8aebb70582d9b53122898e7c46b02c5b80ab56fe6d63d14`.
- Both widths built using the existing Ninja trees, passed all 12 MyPC110
  units and rejected a missing primary image with exit code 1.
- The original external image and derived master retained the hashes recorded
  in S74. Only the existing App diagnostic and task records were changed.

Reaching this loader is not proof of a DOS prompt, full Personaware operation,
original-panel behavior, desktop colours or PC110 product qualification.
