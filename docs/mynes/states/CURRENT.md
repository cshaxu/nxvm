# Project Status

## Current Work

M6 T43 remains open; no S is active. S4 audit delivery `87a45b7dd` is accepted
after actual-change review. SoftPC `dc9c34ce` is eligible for unchanged source
import with a unified rebuild and receiving-product regression. S4 imported
no production code and delivered no new executable. Await owner direction.

| Task | Progress |
| --- | --- |
| T43 S4 | Closed audit: 227 paths, 17 classified differences; candidate manifests/boundaries and bounded compatibility probes pass. |

Proposal: [six-component Types boundary audit](../proposals/m6-t43-six-component-types-boundary-audit.md).
Evidence: [T43 ledger and S3 correction](../etc/evidence/m6-t43-s1-six-component-types-boundary.md).
S4 evidence: [SoftPC unchanged-import audit](../etc/evidence/m6-t43-s4-softpc-import-audit.md).

## Current Technical Baseline

- Product: MyNES; MIT. T43 S3 refreshes the optimized, stripped 0043 x64/x86
  pair in `assets/mynes/`; hashes and architecture are recorded in the evidence.
- S3 supersedes S2's incomplete Types-compliance conclusion. All six manifests
  now identify `shared-m6-t43-s3-p1` and hash the committed LF source bytes.
- Snapshot files are private versioned `MNS1` state images. App owns command
  and file policy; Core owns image state and cartridge identity; Common/Lib
  remain neutral. ROMs remain owner-local and ignored.
