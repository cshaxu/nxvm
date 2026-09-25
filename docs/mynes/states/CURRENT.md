# Project Status

## Current Work

M6 T43 S8 is accepted and closed. S9 snapshot resume repair is implemented and
verified: successful restore clears the prior run's driver stop latch and frame
publication cache. Dual-width suites pass 130/130 each; both 0043 artifacts are
updated. S9 awaits owner gameplay verification; T43 stays open. Only MyNES changed.

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M6 T43 S9; single-session sequential coordinator/executor roles. |
| Admission And Approval | Owner accepts S8 and admits S9 on 2026-09-25 for MyNES snapshot resume freezing. Normal scoped commit/push remains authorized. |
| Objective | Reproduce and repair the snapshot load/resume execution stall at its owning boundary; prove actual guest/frame/input progress, not just Running status. |
| Non-goals | No new snapshot format, second executor, forced lifecycle state, NXVM edits, INI/media edits or Shared source change without separate owner review. |
| Reference Baseline | 1e86b8e7a; accepted S8, clean initial worktree. |
| Candidate Proposal | ../proposals/m6-t43-six-component-types-boundary-audit.md, S9. |
| Files And ABI Surface | MyNES driver/snapshot/App paths as evidence requires; product-owned tests, docs and 0043 x64/x86 artifacts. Read Shared contracts without changing them. |
| Applicable Rules | EXECUTION, ARCHITECTURE, CODING, DOCUMENT; MyNES architecture/coding/UI/source policy; architecture/coding governance skills. |
| Verification | Reproduce before repair with repository-owned snapshot fixture; cover save/pause/resume, stop/load/resume, repeated restore, CPU/frame/controller progress and failure atomicity. Run complete configured MyNES suites on x64/x86, build both product artifacts, verify PE/hash and documentation gate. |
| Expected Markers | Restored machine advances cycles and frames and consumes input; pause/stop remain responsive; malformed snapshots preserve prior state; all configured tests pass. |
| Asset Needs | Repository-owned ROM bytes and bounded snapshot files in build test directories; existing product build trees retained for T43. No protected media required or committed. |
| Reporting Requirements | Report reproduced root cause, owner-local correction, similar-case results, exact tests and pushed artifact links; await owner gameplay verification. |
| Stop Conditions | Report any required Shared modification before editing, unsupported snapshot-format change, or unowned concurrent edits; do not replace runtime evidence with a state enum. |
| Exit Criteria | Mechanism fixed and regression fails before/passes after; full suites, dual artifacts and actual-diff review pass; pushed delivery with remaining manual acceptance explicit. |
| Original Owner Request | Snapshot resume reports running while the game is frozen with no effective input/output. Repair this in S9 after closing S8. |
| Similar-Issue Sweep | Inspect driver stop/wake/debug/pacing latches, snapshot capture/restore and reset/media restart paths, plus Common lifecycle calls; distinguish persistent guest state from transient host execution state. |

| Task | Progress |
| --- | --- |
| T43 S9 | P1 delivery: reproduced cycle stall; MyNES driver correction plus graphics/text snapshot-input/frame regression. Full x64/x86 130/130; 0043 pair rebuilt, PE/stripping/hash checked. Awaiting owner manual acceptance. |
| T43 S8 | Accepted by owner; closure P6 1e86b8e7a pushed. Shared/MyNES 130/130, NXVM 335/335 on both widths and standalone 49/49, 18/18, 10/10. Numeric-only INI correction and all receiving artifacts delivered. |
| T43 S7 | Shared P1 `38ed0f26f`, NXVM P2 `6db4a9298`, MyNES P3 `6682a341c` accepted after actual-diff review. Shared/MyNES 130/130 and NXVM units 335/335 on both widths; ten affected EXEs rebuilt, checked and pushed. Six manifests/Types/docs gates pass. Native failure exclusions and async follow-up are explicit in TODO. Existing unrelated default INI edit remains untouched. |

Proposal: [six-component Types boundary audit](../proposals/m6-t43-six-component-types-boundary-audit.md).
History: [T43 accepted work](../history/M6-T43-six-component-types-boundary-audit.md).
Approved repair ledger: [S8 findings, repairs and verification](../etc/evidence/m6-t43-s8-six-corpus-quality-audit.md).
Evidence: [S7 repair ledger and artifact hashes](../etc/evidence/m6-t43-s7-shared-quality-repairs.md).
Current evidence: [S9 snapshot resume](../etc/evidence/m6-t43-s9-snapshot-resume.md).

## Current Technical Baseline

- Product: MyNES; MIT. Optimized, stripped 0043 x64/x86 developer artifacts in
  `assets/mynes/` are refreshed by S9; their hashes are in S9 evidence.
- Shared baseline: S8 `bfcbd31b2`, following the S4 SoftPC `dc9c34ce`
  import and subsequent approved local corrections. No sibling repository was
  modified. Six manifests and Types gates pass; physical audibility is not
  a unit-test acceptance criterion or a claim of this delivery.
- Snapshot files are private versioned `MNS1` state images. App owns command
  and file policy; Core owns image state and cartridge identity; Common/Lib
  remain neutral. ROMs remain owner-local and ignored.
