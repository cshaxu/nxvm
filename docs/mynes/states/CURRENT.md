# Project Status

## Current Work

M6 T43 S9 is accepted and closed. S10 is implemented and verified: shared test
registration is root-local and lib_bool is lib_u8. All receiving tests and ten
current artifacts are verified. S10 awaits owner manual acceptance; T43 is open.

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M6 T43 S10; sequential coordinator/executor roles. |
| Admission And Approval | Owner request on 2026-09-25 accepts S9 and authorizes helper relocation and lib_u8 boolean with all affected receiver builds, tests, commits and pushes. |
| Objective | One neutral test registration helper and one byte-sized Types boolean contract, integrated without duplicate paths. |
| Non-goals | No product behavior, INI, media, native ABI or serialized format changes; no sibling writes. |
| Reference Baseline | f91686808; accepted S9 and clean worktree. |
| Candidate Proposal | ../proposals/m6-t43-six-component-types-boundary-audit.md, S10. |
| Files And ABI Surface | Shared src/test Lib/Common/x86, test/register.cmake and transfer docs; NXVM and MyNES current artifacts and evidence. Copied value/callback ABI changes require rebuilding all receivers. |
| Applicable Rules | EXECUTION, ARCHITECTURE, CODING, DOCUMENT; both product guides/design/source policy; coding and architecture governance skills. One Types owner and registration path; separate per-target commits; no protected assets. |
| Verification | Independent Lib/Common/x86 CMake builds and CTest; full MyNES configured suites and NXVM unit suites on x64/x86; six manifest checks; strict compiler callback/layout checks; eight NXVM and two MyNES stripped product builds, PE/hash verification and both documentation gates. |
| Expected Markers | lib_bool is lib_u8; all three test suites independently register using the shared helper; no stale path, native ABI mismatch or persistence change. |
| Asset Needs | Existing build trees and repository-only fixtures; no new external assets. Retain trees for T43 owner testing. |
| Reporting Requirements | Report ABI/native/persistence audit, exact tests, source/test delta, all receiving artifact hashes and pushed commits; await owner verification. |
| Stop Conditions | Unowned concurrent modifications or required semantic expansion; do not weaken native/wire contracts or tests to hide narrowing. |
| Exit Criteria | All requested changes and receiving builds pass actual-diff review and applicable tests; manifests and required artifacts current; scoped commits pushed, worktree clean. |
| Original Owner Request | Close S9; S10 moves test/lib/register.cmake to test/register.cmake and changes lib_bool from lib_i32 to lib_u8; build/test/push and wait. |
| Similar-Issue Sweep | All registration consumers and transfer docs; bool callbacks, casts, native nonzero results, atomic storage, copied layouts and snapshot serialization across six corpora and both Apps. |

| Task | Progress |
| --- | --- |
| T43 S10 | Delivered: Shared P1 75099c178, NXVM P2 c5b17f671; MyNES P3 completes test synchronization, evidence and 0043 pair. MyNES 130/130 and NXVM 335/335 on both widths; standalone 49/49, 18/18, 10/10. Await owner verification. |
| T43 S9 | Accepted: d29c26b42 fixes snapshot resume at the MyNES driver. Full configured suites pass 130/130 on both widths; current 0043 pair rebuilt and manually accepted. |
| T43 S8 | Accepted: six-corpus quality repairs and numeric-only INI booleans delivered with all receiving artifacts. |

Proposal: [six-component Types boundary audit](../proposals/m6-t43-six-component-types-boundary-audit.md).
History: [T43 accepted work](../history/M6-T43-six-component-types-boundary-audit.md).
Evidence: [S9 snapshot resume](../etc/evidence/m6-t43-s9-snapshot-resume.md).
Current evidence: [S10 registration and byte boolean](../etc/evidence/m6-t43-s10-registration-byte-bool.md).

## Current Technical Baseline

- MyNES: optimized stripped 0043 x64/x86 artifacts in `assets/mynes/`,
  rebuilt for S10 and recorded in its evidence; awaiting owner verification.
- Shared: S10 75099c178; transfer the six roots plus test/register.cmake.
  Six manifests and Types gates pass. All receiving Apps were rebuilt together.
- Snapshot `MNS1` guest state remains Core-owned; transient driver stop/frame
  publication state is reset only after successful restore. T43 stays open.
