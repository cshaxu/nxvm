# Project Status

## Current Work

M6 T43 S12 is active. T43 remains open.

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M6 T43 S12, following accepted S11. |
| Admission And Approval | Owner admits S12 on 2026-09-25 to repair the MyNES production audit; explicitly forbids six-corpus changes and approves deferring atomic file replacement to MyNES TODO. |
| Objective | Repair snapshot completeness, decode invariants, explicit field serialization, battery-save error propagation and boolean vocabulary. |
| Non-goals | No Shared/NXVM edits, INI changes, native file workaround, generic serializer or T closure. Atomic destination replacement remains approved debt. |
| Reference Baseline | 1ee811710; clean worktree, audit findings in the S12 proposal section. |
| Candidate Proposal | [T43 S12](../proposals/m6-t43-six-component-types-boundary-audit.md#s12-mynes-production-quality-repair). |
| Files And ABI Surface | MyNES only: src/app-mynes, test/app-mynes, docs/mynes and assets/mynes EXEs. Version the corrected private MNS1 schema; no Shared ABI changes. |
| Applicable Rules | Execution packet/P lifecycle and dual-width artifacts; architecture single owner and validate-before-commit; coding explicit fields/booleans; documentation authority and truthful deferral. |
| Verification | Build existing build/mynes-gcc-x64-release and build/mynes-gcc-x86-release; ctest --test-dir each tree --output-on-failure -j 4; MyNES documentation gate; git diff --check; six-root diff empty; PE and SHA-256 verification. |
| Expected Markers | Complete configured suites green on both widths; malformed state leaves machine untouched; omitted-field regression passes; failed save prevents media replacement and reports exit failure; both 0043 EXEs current. |
| Asset Needs | Tests generate their own bytes; no external ROM/media or user INI changes. |
| Reporting Requirements | Report scope confirmation, discovered boundaries, tests, code delta, artifacts and pushed commits; wait for owner gameplay verification. |
| Stop Conditions | Need for Shared API/host bypass, unexpected unrelated changes, failing complete verification or push; never claim atomic persistence. |
| Exit Criteria | Audit batch fixed or owner-approved atomic-replacement debt recorded; actual-diff review, full dual-width verification, MyNES-only commit/push and coordinator closure. |
| Original Owner Request | Admit S12 for correct repair of the reported MyNES quality issues; do not change six components; retain atomic replacement in MyNES TODO. |
| Similar-Issue Sweep | All MyNES snapshot scalar spans and decoded index/shift/count fields; battery save callers at replacement/startup/exit; boolean returns/fields. Shared and NXVM are read-only and excluded from repair. |

| Task | Progress |
| --- | --- |
| T43 S12 | Implementation verified: snapshot v3 and validation, explicit fields, checked battery saves and bool cleanup. Full x64/x86 suites 132/132; current pair rebuilt. Atomic file replacement is owner-deferred TODO. Awaiting pushed-P coordinator review. |
| T43 S11 | G1-G7 repaired: Shared a5dbd9339, NXVM 6ab8aae5e, MyNES 0ae25cdb9. Both documentation gates and the 28-file live link/control-character sweep pass. Documentation only; existing EXEs remain current. |
| T43 S10 | Accepted and closed: Shared P1 75099c178, NXVM P2 c5b17f671 and MyNES P3 057d8c9aa. Dual-width MyNES 130/130 and NXVM 335/335; standalone 49/49, 18/18, 10/10. Ten current artifacts delivered; owner accepts S10. |
| T43 S9 | Accepted: d29c26b42 fixes snapshot resume at the MyNES driver. Full configured suites pass 130/130 on both widths; current 0043 pair rebuilt and manually accepted. |
| T43 S8 | Accepted: six-corpus quality repairs and numeric-only INI booleans delivered with all receiving artifacts. |

Proposal: [six-component Types boundary audit](../proposals/m6-t43-six-component-types-boundary-audit.md).
History: [T43 accepted work](../history/M6-T43-six-component-types-boundary-audit.md).
Evidence: [S9 snapshot resume](../etc/evidence/m6-t43-s9-snapshot-resume.md).
Current audit: [S12 production quality repair](../etc/evidence/m6-t43-s12-production-quality.md).
Artifact evidence: [S10 registration and byte boolean](../etc/evidence/m6-t43-s10-registration-byte-bool.md).

## Current Technical Baseline

- MyNES: optimized stripped 0043 x64/x86 artifacts in `assets/mynes/`,
  rebuilt for S10 and recorded in its evidence; accepted by the owner.
- Shared: S10 75099c178; transfer the six roots plus test/register.cmake.
  Six manifests and Types gates pass. All receiving Apps were rebuilt together.
- Snapshot `MNS1` guest state remains Core-owned; transient driver stop/frame
  publication state is reset only after successful restore. T43 stays open.
