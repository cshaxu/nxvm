# Project Status

## Current Work

M6 T43 S4 is active again by explicit owner instruction to perform the audited
unchanged import in S4. Earlier audit commits remain immutable; P numbering
continues at P3. This owner-directed S reuse overrides the usual next-S rule.

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation of S4 by explicit owner exception to closed-S allocation; no new T. |
| Admission And Approval | Owner on 2026-09-25 requests unchanged import and audit within S4, then explicitly directs commit/push with the reported audio failure retained; no audio repair or green-suite waiver is authorized. |
| Objective | Adopt the audited SoftPC six roots verbatim and verify their boundaries and use by MyNES/NXVM. |
| Non-goals | No NXVM 43/50-row hardware implementation, sibling changes, Shared local patches, INI/media changes or T closure. |
| Reference Baseline | Receiver `33874cfe1`; source `dc9c34ce3fa2794840762e26c26801cd098c41ab`; S4 audit `87a45b7dd`. |
| Candidate Proposal | [T43 S4](../proposals/m6-t43-six-component-types-boundary-audit.md). |
| Files And ABI Surface | Shared: six src/test roots only. MyNES: task records and refreshed 0043 dual EXEs. Both products consume enlarged text records; rebuild dependencies together. |
| Applicable Rules | Shared Execution, Architecture, Coding, Documentation; MyNES source policy and guide; architecture/coding governance skills. Exact import and Types ownership preserved. |
| Verification | Compare every imported byte to committed source; six manifests; full Shared and MyNES tests on x64/x86; receiving NXVM unit regressions where configured; documentation governance and actual-diff review. |
| Expected Markers | Zero source-pin mismatches, passing six-root gates/tests, no lost canonical capability, complete regenerated MyNES pair. |
| Asset Needs | No new external asset or changed user configuration. |
| Reporting Requirements | Source identity, code-quality findings, manifest/Types results, dual-width tests, artifacts and target-separated pushed commits. |
| Stop Conditions | Report a Shared defect or incompatible consumer requiring expansion; preserve original source instead of silently fixing the import. |
| Exit Criteria | Verbatim import, required checks and review complete, evidence and binaries pushed; S4 closed with T43 open. |
| Original Owner Request | Complete the six-component unchanged import in S4, then audit code quality, manifests and six-component tests and report. |
| Similar-Issue Sweep | Recheck all imported changes and both products' text capacity versus active geometry, ownership and failure handling; retain the audit's NXVM 25-row limitation. |

| Task | Progress |
| --- | --- |
| T43 S4 | Exact import delivered in Shared P3 `83022ea9f`; MyNES P4 delivers evidence and both EXEs by owner direction. Shared tests 76/77 and MyNES 53/53 per width; NXVM unit 334/335 per width. Audio failure remains recorded; no repair admitted, S/T not closed. |

Proposal: [six-component Types boundary audit](../proposals/m6-t43-six-component-types-boundary-audit.md).
Evidence: [T43 ledger and S3 correction](../etc/evidence/m6-t43-s1-six-component-types-boundary.md).
S4 evidence: [SoftPC unchanged-import audit](../etc/evidence/m6-t43-s4-softpc-import-audit.md).

## Current Technical Baseline

- Product: MyNES; MIT. S4 refreshes the optimized, stripped 0043 x64/x86
  developer pair in `assets/mynes/`; hashes and test limitations are in S4 evidence.
- Shared baseline: P3 `83022ea9f`, identical to SoftPC `dc9c34ce` across six roots.
  Manifest hashes pass; their retained S3 comment labels are not revision pins.
  S3's Types correction remains intact. Native-audio qualification is not green.
- Snapshot files are private versioned `MNS1` state images. App owns command
  and file policy; Core owns image state and cartridge identity; Common/Lib
  remain neutral. ROMs remain owner-local and ignored.
