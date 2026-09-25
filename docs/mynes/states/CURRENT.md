# Project Status

## Current Work

M6 T43 remains open for owner review; S6 is accepted and closed. No active S
packet remains.

| Task | Progress |
| --- | --- |
| T43 S6 | Accepted Shared P1 e25aec65b, NXVM P2 735d155a9 and MyNES P3 0c7a58792: receiving artifacts current; all final suites pass. Initial modal-test failure retained in TODO; unrelated default INI edit preserved. [History](../history/M6-T43-six-component-types-boundary-audit.md). |

Proposal: [six-component Types boundary audit](../proposals/m6-t43-six-component-types-boundary-audit.md).
Evidence: [T43 ledger and S3 correction](../etc/evidence/m6-t43-s1-six-component-types-boundary.md).
S4 evidence: [SoftPC unchanged-import audit](../etc/evidence/m6-t43-s4-softpc-import-audit.md).

## Current Technical Baseline

- Product: MyNES; MIT. The optimized, stripped 0043 developer pair remains in
  `assets/mynes/`; S5 rebuilt incrementally and verified the unchanged hashes.
- Shared production baseline remains S4 `83022ea9f` from SoftPC `dc9c34ce`.
  S5 `b7cbb30a9` intentionally corrects only `test/lib`; six manifests and
  Types checks pass. All required S suites pass; physical audibility is not
  a unit-test acceptance criterion or a claim of this delivery.
- Snapshot files are private versioned `MNS1` state images. App owns command
  and file policy; Core owns image state and cartridge identity; Common/Lib
  remain neutral. ROMs remain owner-local and ignored.
