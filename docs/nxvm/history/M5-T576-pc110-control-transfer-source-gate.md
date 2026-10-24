# M5 T576: PC110 protected control-transfer source gate

## Closure

T576 consumes the finite receivers left by T575 without extending the CPU into
a broad compatibility mode. S1 established the App-owned C shadow route; S2
admitted the single strict-by-default data reload policy; S3 corrected the
unshadowed TFT video-bank selection and traced ordinary INT 10 dispatch on
both host widths.

The resulting C header is `55 AA 47 EB`, and the real firmware executes
`CD 10` at `F000:4CA9`. Its IVT entry at physical `0000:0040` remains zero.
The normal Core INT path therefore reaches the uninitialized vector. That is a
display-firmware/board-initialization source gate, not evidence for relaxing
protected code, stack, far-control or interrupt semantics.

No source-qualified PC110 video BIOS or initialization contract is admitted in
the external asset bundle. The QEMU standard-VGA mode-set injection is not an
NXVM implementation input. T576 closes without a synthetic IVT, borrowed
IBM-PC ROM, or product/boot claim.

The detailed S3 map, two-width observation and limits are in
[the S3 evidence](../etc/evidence/t576-s3-pc110-int10-video-boundary.md).
The frozen task proposal is retained beside this history record as
[the T576 proposal](M5-T576-pc110-control-transfer-source-gate-proposal.md).

## Verification

- The seven MyPC110 unit/diagnostic registrations pass on x64 and x86.
- The complete repository-only unit suite passes **661/661** on each width.
- No MyPC110 EXE is published: the observed zero video vector does not qualify
  POST, Setup, DOS boot, display output, or a release artifact.
