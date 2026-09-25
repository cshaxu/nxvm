# Project Status

## Current Work

M6 T43 S13 implementation is verified: startup stays stopped with or without a ROM.
T43 remains open. Only MyNES source, tests, documentation and artifacts may change.

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation, M6 T43 S13; one-session coordinator/executor review. |
| Admission And Approval | Owner explicitly approves stopped startup repair, dual builds/tests and commit/push on 2026-09-25; no Shared changes. |
| Objective | Attach the configured cartridge without automatic RESET; await explicit start in STOPPED. |
| Non-goals | No snapshot/schema, explicit reset/media-command, Shared, NXVM or INI changes. |
| Reference Baseline | S12 c00ce0e41; automatic initial_reset in MyNES command open causes PAUSED. |
| Candidate Proposal | [T43 S13](../proposals/m6-t43-six-component-types-boundary-audit.md#s13-stopped-startup). |
| Files And ABI Surface | MyNES product command/composition, their tests and documents; internal initializer loses initial_reset parameter; refresh 0043 pair. |
| Applicable Rules | Execution lifecycle and artifacts; Coding single path/types; Architecture Common lifecycle owner; Document authority; MyNES design/CODING, ARCHITECTURE, UI and App/Core contract. |
| Verification | Both configured Release builds and complete ctest suites with -j 4; MyNES documentation gate; diff/check and actual-change review. Transient focus: command and native Console/Window startup tests. |
| Expected Markers | No automatic lifecycle request on open; initial STOPPED completion arms prompt; explicit start runs; reset and snapshot behavior retained; all tests pass. |
| Asset Needs | Repository-generated test ROMs only; preserve owner ROM, snapshot and INI files. |
| Reporting Requirements | Confirm scope, report discoveries, record source/test counts and artifact hashes; push complete P then coordinator closure P; await manual verification. |
| Stop Conditions | Shared change needed, unexpected unrelated modifications, failed required verification or ambiguous startup semantics. |
| Exit Criteria | Four startup variants covered, obsolete auto-reset path removed, dual current EXEs verified/committed/pushed; T43 remains open. |
| Original Owner Request | Start MyNES like SoftPC in init/stopped, not paused; admit new S, compile/test/commit/push and deliver for verification. |
| Similar-Issue Sweep | Search MyNES source/tests/live design for initial_reset, startup reset and initializer callers; distinguish explicit insert/reset from automatic startup; update every affected fixture and contract. |

| Task | Progress |
| --- | --- |
| T43 S13 | Executor delivery ready for coordinator review: removed startup RESET, four startup variants and native start checked, x64/x86 132/132; current 0043 pair refreshed. |
| T43 S12 | P1 d92bccd7d accepted after actual-diff review: snapshot v3, candidate checks, explicit fields, checked saves and bool cleanup. Full x64/x86 suites 132/132 and current pair delivered; atomic replacement remains owner-deferred TODO. |
| T43 S11 | G1-G7 repaired: Shared a5dbd9339, NXVM 6ab8aae5e, MyNES 0ae25cdb9. Both documentation gates and the 28-file live link/control-character sweep pass. Documentation only; existing EXEs remain current. |
| T43 S10 | Accepted and closed: Shared P1 75099c178, NXVM P2 c5b17f671 and MyNES P3 057d8c9aa. Dual-width MyNES 130/130 and NXVM 335/335; standalone 49/49, 18/18, 10/10. Ten current artifacts delivered; owner accepts S10. |
| T43 S9 | Accepted: d29c26b42 fixes snapshot resume at the MyNES driver. Full configured suites pass 130/130 on both widths; current 0043 pair rebuilt and manually accepted. |

Proposal: [six-component Types boundary audit](../proposals/m6-t43-six-component-types-boundary-audit.md).
History: [T43 accepted work](../history/M6-T43-six-component-types-boundary-audit.md).
Evidence: [S9 snapshot resume](../etc/evidence/m6-t43-s9-snapshot-resume.md).
Current audit: [S12 production quality repair](../etc/evidence/m6-t43-s12-production-quality.md).
Artifact evidence: [S10 registration and byte boolean](../etc/evidence/m6-t43-s10-registration-byte-bool.md).

## Current Technical Baseline

- MyNES: optimized stripped 0043 x64/x86 artifacts in `assets/mynes/`,
  rebuilt for S12 P1 d92bccd7d and recorded in S12 evidence; awaiting owner gameplay verification.
- Shared: S10 75099c178; transfer the six roots plus test/register.cmake.
  Six manifests and Types gates pass. All receiving Apps were rebuilt together.
- Snapshot `MNS1` guest state remains Core-owned; transient driver stop/frame
  publication state is reset only after successful restore. T43 stays open.
