# Project Status

## Current Work

M6 T43 S7 is admitted for the owner-approved shared quality repairs. T43 stays open.

S7 executor verification is complete: Shared/MyNES 130/130 and NXVM units
335/335 on each width; all ten affected EXEs are refreshed. Six manifests,
Types and both product documentation gates pass. Scoped delivery and coordinator
acceptance follow. [Evidence](../etc/evidence/m6-t43-s7-shared-quality-repairs.md)
records exact hashes, the finite repair ledger and native failure boundaries.

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M6 T43 S7; coordinator/executor roles performed sequentially in one session. |
| Admission And Approval | Owner approved the revised resolved-label design and remaining audit repairs on 2026-09-25. Shared changes and receiving NXVM/MyNES artifacts are authorized. |
| Objective | Repair unresolved assembly labels, audio partial-delivery/cancellation accounting, audio test synchronization, Common request admission failures, and two small corpus inconsistencies. |
| Non-goals | No new public API, parser rewrite, polling workaround, sibling edits, INI changes, or T closure. |
| Reference Baseline | f1ce6763e; preserve unrelated default NXVM.ini modification. |
| Candidate Proposal | ../proposals/m6-t43-six-component-types-boundary-audit.md, S7. |
| Files And ABI Surface | Shared src/test lib/common/x86 and manifests; MyNES task records and receiving 0043 binaries; NXVM receiving 0535 binaries/evidence. No public ABI expansion. |
| Applicable Rules | Shared EXECUTION, ARCHITECTURE, CODING, DOCUMENT; product guides/designs; existing component contracts; architecture/coding skills. One owner and production path, original assembler style, separate target commits. |
| Verification | Regression tests for every repaired mechanism; all six manifests and boundary gates; complete configured Shared/MyNES and NXVM unit suites on x64/x86; affected product builds and hashes; both documentation gates. |
| Expected Markers | Unresolved labels fail without output; audio accepted prefixes never replay; test waits for real completion; failed admission never waits; manifests/gates/suites pass. |
| Asset Needs | Existing build-time BYOB assets only; no host-audibility test or asset modification. |
| Reporting Requirements | Report verified design, new blockers, full verification and target-separated pushed commits; retain T open. |
| Stop Conditions | Common completion failure requires a new unapproved synchronization design, public API expansion, or unavailable build prerequisite. Complete independent repairs and report instead of inventing a workaround. |
| Exit Criteria | Actual-diff review against approved design, regression/full-unit evidence, current affected dual-width artifacts, manifests, pushed scoped commits and governance closure. No false all-green claim. |
| Original Owner Request | Implement the reviewed six-component audit fixes, using one temporary resolved flag inside the original assembler matching loop. |
| Similar-Issue Sweep | Inspect all label materialization exits; audio cancellation/clear/destroy and partial acceptance paths; Common request registration/completion variants; audio test completion signals. Record dispositions in S7 evidence. |

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
