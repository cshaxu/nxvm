# Project Status

## Current Work

M6 T43 S4 is active: audit the latest committed SoftPC six-component corpus
against the canonical NXVM baseline for unchanged adoption by both products.

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M6 T43 S4 follows accepted S3. |
| Admission And Approval | Owner admitted S4 on 2026-09-25 to compare and assess unchanged import; audit and MyNES evidence only. |
| Objective | Classify every six-root difference and assess source, Types, ABI, tests and MyNES/NXVM consumer compatibility. |
| Non-goals | No production import, Shared repair, sibling modification, product behavior change or T closure. |
| Reference Baseline | NXVM `440ae83bc`; clean SoftPC `dc9c34ce3fa2794840762e26c26801cd098c41ab`. |
| Candidate Proposal | [T43 proposal](../proposals/m6-t43-six-component-types-boundary-audit.md), S4 audit extension. |
| Files And ABI Surface | MyNES proposal, status, history and audit evidence. Read all six Shared roots and both product consumers; temporary audit builds may use an isolated source snapshot. |
| Applicable Rules | Shared Execution, Documentation, Architecture and Coding; MyNES guide and source policy; local architecture-governance and coding-governance skills. Unique owners, neutral contracts, Types vocabulary and exact pinned provenance are required. |
| Verification | Compare tracked inventories and bytes; review every diff; validate all manifests and Types/corpus gates; inspect product frame producers and test assumptions. Run bounded isolated builds/tests if needed to substantiate compatibility. Run MyNES documentation governance and diff check. |
| Expected Markers | Every changed path classified; preserved canonical capabilities; explicit ABI and consumer implications; no claim of runtime acceptance from a static audit. |
| Asset Needs | No external ROM/media. No new product artifact for this design/audit S. |
| Reporting Requirements | Report pinned commits, per-root counts, actual code changes, blockers, required receiving changes and import recommendation. |
| Stop Conditions | Record any incompatible API, loss of canonical behavior, stale source identity or insufficient evidence; do not silently patch candidate Shared code. |
| Exit Criteria | Complete difference ledger, evidence-backed import verdict, reviewed and pushed MyNES audit records; T43 remains open. |
| Original Owner Request | Admit T43 S4 to compare this repository and SoftPC six components and audit unchanged import for MyNES and NXVM. |
| Similar-Issue Sweep | Review all frame capacity/default-size uses in both products and shared tests; distinguish storage capacity from active geometry and preserved ABI. |

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
