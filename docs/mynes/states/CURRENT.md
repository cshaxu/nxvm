# Project Status

## Current Work

M6 T43 remains open; no S is active. S3 was accepted after actual-diff review,
both complete configured suites passed (130/130 each), and the committed
six-component export passed all manifests and the test Types boundary gate.
Shared implementation: `f5a9bd34a`. Await owner review before further admission
or T closure.

Proposal: [six-component Types boundary audit](../proposals/m6-t43-six-component-types-boundary-audit.md).
Evidence: [T43 ledger and S3 correction](../etc/evidence/m6-t43-s1-six-component-types-boundary.md).

## Current Technical Baseline

- Product: MyNES; MIT. T43 S3 refreshes the optimized, stripped 0043 x64/x86
  pair in `assets/mynes/`; hashes and architecture are recorded in the evidence.
- S3 supersedes S2's incomplete Types-compliance conclusion. All six manifests
  now identify `shared-m6-t43-s3-p1` and hash the committed LF source bytes.
- Snapshot files are private versioned `MNS1` state images. App owns command
  and file policy; Core owns image state and cartridge identity; Common/Lib
  remain neutral. ROMs remain owner-local and ignored.
