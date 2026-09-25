# Project Status

## Current Work

M6 T43 S5 is admitted for the native-audio failure reported by S4. S4 import
delivery is preserved; its non-green result is the input to this corrective S.

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: next unused S5 in open T43; P1 begins this S. |
| Admission And Approval | Owner on 2026-09-25 explicitly admits a new S to repair the NXVM repository audio failure reported by S4. Shared audio/test repair and both consumers' verification are in scope; sibling repositories remain read-only. |
| Objective | Correct the test boundary: deterministic units verify PCM submission and native adapter semantics without physical endpoint/mute dependence. Remove the physical loopback test, with no opt-in mode. |
| Non-goals | No unrelated UX/device changes, product audio redesign, sibling writes, global sound-setting changes or T closure. |
| Reference Baseline | `f67eaefed`; Shared import `83022ea9f`. Native loopback fails on x64/x86 while all other S4 tests pass. |
| Candidate Proposal | [T43 S5](../proposals/m6-t43-six-component-types-boundary-audit.md). |
| Files And ABI Surface | Shared test/lib test, registration and manifest; MyNES task records and verification of unchanged 0043 dual artifacts. No production source or public ABI changes. NXVM is a receiving regression consumer. |
| Applicable Rules | Shared Execution, Architecture, Coding, Documentation; MyNES source policy and guide; architecture/coding governance skills. Imported production bytes and Types ownership preserved; approved test-only correction intentionally differs from SoftPC. |
| Verification | Deterministic WASAPI adapter tests cover PCM format/bytes/order, wait/cancel, cleanup and failures on x64/x86; complete Shared/MyNES suites and NXVM units, six manifests, Types and documentation gates. Confirm no loopback registration or switch remains. |
| Expected Markers | Unit results do not depend on device availability, mute or physical output; sample equality catches silent/corrupt/reordered submissions; no production audio or ABI change. |
| Asset Needs | No new external asset or changed user configuration. |
| Reporting Requirements | Root cause, exact fix and scope, native/repository-only results, code-size delta, artifacts and target-separated pushed commits. |
| Stop Conditions | Report a required public ABI/product expansion or host change before performing it; no silent skip or relaxed acceptance. |
| Exit Criteria | Cause fixed with regression proof, complete required verification and actual-diff review, manifests/evidence and dual artifacts pushed; T43 stays open. |
| Original Owner Request | Admit an S task to fix the NXVM repository audio problem. |
| Similar-Issue Sweep | Inspect capture packet wait/drain/release, worker render/queue completion and repeated start/reuse; retain one audio owner and check native and deterministic probes. |

| Task | Progress |
| --- | --- |
| T43 S4 | Exact import delivered in Shared P3 `83022ea9f` and MyNES P4 `f67eaefed`. Non-green native-audio qualification is transferred to S5; the import remains unchanged. |
| T43 S5 | Deterministic WASAPI replacement complete; Shared/MyNES 130/130 and NXVM unit 335/335 pass on both widths. Executor delivery pending coordinator acceptance. [Evidence](../etc/evidence/m6-t43-s5-native-audio.md). |

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
