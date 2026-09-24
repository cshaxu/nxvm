# Project Status

## Current Work

### M6 T43 S2 Test Types Boundary Repair

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation. |
| Admission And Approval | Owner admitted M6 T43 S2 on 2026-09-24 to repair the bounded test-side receivers found in S1. This is the required owner approval for Shared Types/test changes. |
| Objective | Remove direct external types, constants, functions, macros and headers from `test/{lib,common,x86}` so all six Shared components use the `lib/types` vocabulary boundary. |
| Non-goals | Do not alter product behavior, production component ownership, public product APIs, external assets, NXVM or MyNES semantics. Do not create a test-only parallel platform facade or an undocumented exception. |
| Reference Baseline | S1 delivery `3c410c2f4` and its complete ledger; `src/lib/types/types_interface.h` and declaration groups remain the only external-vocabulary owner. |
| Candidate Proposal | [M6 T43 six-component Types boundary audit](../proposals/m6-t43-six-component-types-boundary-audit.md). |
| Files And ABI Surface | `src/lib/types/` may gain only declarations required by the existing tests; `test/{lib,common,x86}` receives the corresponding direct-use migration and static enforcement; manifests, T43 evidence, and the existing 0043 dual artifacts are refreshed. Any Types API remains neutral and cross-consumer. |
| Applicable Rules | `docs/rules/EXECUTION.md`, `ARCHITECTURE.md`, `CODING.md`, `DOCUMENT.md`; MyNES `ARCHITECTURE.md` and `CODING.md`; `src/lib/README.md` Types/component-dependency contract. |
| Verification | Re-run the exhaustive six-root scan; prove no external header is included outside `src/lib/types`; prove no unprefixed external identifier remains in Shared production/test roots; run all six-root static/manifest gates and full MyNES x64/x86 repository-only unit suites; rebuild the 0043 x64/x86 pair; run documentation governance and `git diff --check`. |
| Expected Markers | Every external vocabulary reference in all six roots is Types-owned; component-owned symbols remain unchanged; no test-harness exception or unclassified receiver remains. |
| Asset Needs | No external input. Refresh `assets/mynes/mynes_0_0_0043_{x64,x86}.exe` as the required T43 pair. |
| Reporting Requirements | Record pre/post external header and raw-symbol counts, exact new Types declarations, all affected tests, x64/x86 unit results, artifact architecture/hash, source/test line delta, documentation result, target-scoped commits and push. |
| Stop Conditions | Stop and report if compliance requires product-aware behavior, a platform abstraction beyond declaration ownership, a shared ABI/lifecycle change, or an unapproved exception. |
| Exit Criteria | The complete six-root scan is clean outside `src/lib/types`, all external test use routes through Types, all gates and MyNES x64/x86 units pass, 0043 artifacts are refreshed, evidence is updated and pushed. T43 may close only after the coordinator's actual-diff review. |
| Original Owner Request | "继续按 myNES序列准入一个新的T任务，目标是审计所有六个组件是不是都按照规范只引用 lib slash types里面的符号定义，包括函数和常量和类型定义。" |
| Similar-Issue Sweep | Migrate every S1 ledger row, then scan all six roots for direct headers, raw external identifiers, macro aliases and declaration leakage; do not stop at the files that happened to include a header directly. |

## Current Technical Baseline

- Product: MyNES; MIT. M6 T43 is active.
- T42 S1 published `assets/mynes/mynes_0_0_0042_x64.exe` and
  `assets/mynes/mynes_0_0_0042_x86.exe`; T43 S2 refreshes the 0043 pair
  while repairing the test Types boundary.
- Snapshot files are private versioned `MNS1` state images. App owns command
  and file policy; Core owns image state and cartridge identity; Common/Lib
  remain neutral. ROMs remain ignored and no remote is configured.
