# M5 T576: PC110 protected control-transfer source gate

## Corrected closure

The S3 C000 video-bank conclusion conflicted with stronger already-admitted
live-map evidence. Corrective S4 restores the actual topology: C000 is the
reset-visible read-only copy of bank 2, E000 exposes raw bank 2 and F000
exposes bank 3. This is now the retained App mapping truth.

T576 consumes the finite receivers left by T575 without extending the CPU into
a broad compatibility mode. S1 established the App-owned C shadow route; S2
admitted the single strict-by-default data reload policy; S3 traced ordinary
INT 10 dispatch on both host widths.

With S4's corrected map, the real firmware still executes `CD 10` at
`F000:4CA9` before retiring any instruction in C000 or E000.
Its IVT entry at physical `0000:0040` remains zero.
The normal Core INT path therefore reaches the uninitialized vector. That is a
display-firmware/board-initialization source gate, not evidence for relaxing
protected code, stack, far-control or interrupt semantics.

No source-qualified PC110 video BIOS or initialization contract is admitted in
the external asset bundle. The QEMU standard-VGA mode-set injection is not an
NXVM implementation input. The S3 conclusion retains no synthetic IVT,
borrowed IBM-PC ROM, or product/boot claim.

The detailed S3/S4 map, two-width observation and limits are in
[the S3 evidence](../etc/evidence/t576-s3-pc110-int10-video-boundary.md).
The frozen task proposal is retained beside this history record as
[the T576 proposal](M5-T576-pc110-control-transfer-source-gate-proposal.md).

## Verification

- The seven MyPC110 unit/diagnostic registrations pass on x64 and x86.
- The complete repository-only unit suite passes **661/661** on each width.
- No MyPC110 EXE is published: the observed zero video vector does not qualify
  POST, Setup, DOS boot, display output, or a release artifact.
