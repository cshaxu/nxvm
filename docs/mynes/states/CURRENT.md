# Project Status

## Current Work

M6 T43 S9 is accepted and closed after owner gameplay verification on
2026-09-25. T43 remains open; no subtask is active.

| Task | Progress |
| --- | --- |
| T43 S9 | Accepted: d29c26b42 fixes snapshot resume at the MyNES driver. Full configured suites pass 130/130 on both widths; current 0043 pair rebuilt and manually accepted. |
| T43 S8 | Accepted: six-corpus quality repairs and numeric-only INI booleans delivered with all receiving artifacts. |

Proposal: [six-component Types boundary audit](../proposals/m6-t43-six-component-types-boundary-audit.md).
History: [T43 accepted work](../history/M6-T43-six-component-types-boundary-audit.md).
Evidence: [S9 snapshot resume](../etc/evidence/m6-t43-s9-snapshot-resume.md).

## Current Technical Baseline

- MyNES: optimized stripped 0043 x64/x86 artifacts in `assets/mynes/`,
  recorded in S9 evidence and accepted by the owner.
- Shared: S8 bfcbd31b2, following the pinned SoftPC import and approved local
  corrections. Six manifests and Types gates pass.
- Snapshot `MNS1` guest state remains Core-owned; transient driver stop/frame
  publication state is reset only after successful restore. T43 stays open.
