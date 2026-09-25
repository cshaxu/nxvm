# Project Status

## Current Work

M6 T43 S6 is admitted to complete receiving artifacts and delivery governance.

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: next unused T43 S6, P1 onward. |
| Admission And Approval | Owner on 2026-09-25 requests governance update and necessary EXE build/commit/push; clarifies unchanged executable inputs require no refresh. Targets: Shared rules, NXVM artifacts/docs, MyNES records. |
| Objective | Prevent stale receiving artifacts after Shared changes; deliver all affected current NXVM profile x64/x86 EXEs and verify MyNES pair. |
| Non-goals | No production code, Shared corpus, user INI, firmware, version renumbering or T closure. |
| Reference Baseline | b34152e15; S4 production import 83022ea9f; S5 test-only fix b7cbb30a9. |
| Candidate Proposal | [S6](../proposals/m6-t43-six-component-types-boundary-audit.md). |
| Files And ABI Surface | Shared EXECUTION; NXVM current 0535 eight EXEs/evidence/status; MyNES task records and unchanged 0043 pair verification. No ABI change. |
| Applicable Rules | Execution target separation/artifact delivery, Documentation authority, both product guides and source policies. BYOB stays external. |
| Verification | Release product targets for default/XT/AT/Model40 x64/x86; MyNES dual targets; PE/hash checks; complete Shared/MyNES suites and NXVM unit suites on both widths; both documentation gates. |
| Expected Markers | Ten current EXEs accounted for; production import reflected; no config diff; changed binaries committed/pushed per product target. |
| Asset Needs | Existing external profile archive only; no acquisition or redistribution. |
| Reporting Requirements | Scope, builds/tests, source/hash evidence, pushed commits and clean worktree. |
| Stop Conditions | Missing assets/toolchain or build/test failure prevents acceptance; report rather than fabricate artifacts. |
| Exit Criteria | Actual-diff review accepts rules, necessary artifacts and evidence; separate target commits pushed; S6 closed and T43 open. |
| Original Owner Request | Write executable-delivery obligation into execution rules, generate/commit/push required EXEs; no update when unnecessary. |
| Similar-Issue Sweep | All four NXVM profiles and MyNES across x64/x86, not only default or packet-host App. |

| Task | Progress |
| --- | --- |
| T43 S5 | Accepted Shared P1 `b7cbb30a9` and MyNES P2 `3959be3de`: deterministic audio test replaces physical loopback. Both widths pass Shared/MyNES 130/130 and NXVM unit 335/335. [Evidence](../etc/evidence/m6-t43-s5-native-audio.md). |
| T43 S6 | Shared rule P1 e25aec65b and NXVM eight-artifact P2 735d155a9 pushed. MyNES targets unchanged; x64 130/130 and x86 confirmation 130/130, NXVM units 335/335 each. First x86 modal-test failure retained in TODO. Awaiting coordinator acceptance. |

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
