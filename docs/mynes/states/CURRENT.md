# Project Status

## Current Work

### M6 T43 S1 Six-Component Types Boundary Audit

| Field | Required record |
| --- | --- |
| Identifier Mode | New. |
| Admission And Approval | Owner admitted M6 T43 S1 on 2026-09-24 to audit the complete six-component Shared corpus for its `lib/types` vocabulary boundary. |
| Objective | Establish a complete, reviewable ledger for every external type, constant, function and macro reference in `src/{lib,common,x86}` and `test/{lib,common,x86}`, proving its `lib/types` owner or recording its exact receiver. |
| Non-goals | Do not modify Shared source/test code, public Shared ABI, product behavior, external assets, NXVM, or MyNES implementation semantics. This S is inventory and evidence only. |
| Reference Baseline | Clean M6 Td S2 closure `4b9696dca`; current MyNES delivery revision is 0042; `src/lib/types/types_interface.h` and its declaration groups are the only external-vocabulary authority. |
| Candidate Proposal | [M6 T43 six-component Types boundary audit](../proposals/m6-t43-six-component-types-boundary-audit.md). |
| Files And ABI Surface | MyNES task records/evidence and MyNES 0043 delivery metadata/artifacts only. The audit reads, but does not modify, all six Shared roots. Shared configuration may advance only the two MyNES current-build target names for the required T43 delivery pair. No ABI change. |
| Applicable Rules | `docs/rules/EXECUTION.md`, `ARCHITECTURE.md`, `CODING.md`, `DOCUMENT.md`; MyNES `ARCHITECTURE.md` and `CODING.md`; `src/lib/README.md` Types/component-dependency contract. Shared component code requires later owner review before modification. |
| Verification | Freeze an exhaustive six-root file inventory; scan includes and external vocabulary; manually classify each non-Types reference; run manifest/static gates applicable to all six roots; build MyNES 0043 x64/x86 artifacts; run the complete MyNES x64/x86 repository-only unit suites; run documentation governance and `git diff --check`. |
| Expected Markers | Every external vocabulary use is one of: Types-owned, component-owned (not external), test harness boundary, or a concrete violation with file/line/receiver. No unclassified item remains. |
| Asset Needs | No external input. Publish `assets/mynes/mynes_0_0_0043_{x64,x86}.exe` as the required T43 S1 pair. |
| Reporting Requirements | Record universe count, each category count and line-level ledger, all potential violation receivers, source/test change count, x64/x86 unit results, artifact architecture/hash, documentation result, target-scoped commits and push. |
| Stop Conditions | Stop before a Shared code modification, a new Types declaration, an ABI change, or any classification requiring a policy exception. Report the ledger and request owner review for a later S. |
| Exit Criteria | The frozen six-root ledger is complete, all exceptions have a defined disposition, no Shared code changed, the 0043 dual artifacts and full MyNES unit proof are delivered, and the S evidence/commit are pushed. T43 remains open for any owner-approved repair S. |
| Original Owner Request | "继续按 myNES序列准入一个新的T任务，目标是审计所有六个组件是不是都按照规范只引用 lib slash types里面的符号定义，包括函数和常量和类型定义。" |
| Similar-Issue Sweep | Cover production and test roots equally; inspect direct headers, direct external identifiers, macro aliases and declaration leakage, not only scalar type spellings. |

## Current Technical Baseline

- Product: MyNES; MIT. M6 T43 is active.
- T42 S1 published `assets/mynes/mynes_0_0_0042_x64.exe` and
  `assets/mynes/mynes_0_0_0042_x86.exe`; T43 S1 will publish the 0043 pair.
- Snapshot files are private versioned `MNS1` state images. App owns command
  and file policy; Core owns image state and cartridge identity; Common/Lib
  remain neutral. ROMs remain ignored and no remote is configured.
