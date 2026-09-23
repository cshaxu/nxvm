# Project Status

## Current Work

**Active: M6 T38 S1 owner verification.** The owner-selected MyNes product-tree
move has passed automated closure checks and awaits the owner's binary review.

## M6 T38 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New |
| Admission And Approval | Owner requested T38 after T37 closure to make the manually selected product layout buildable, testable and accurately documented. |
| Objective | Reconnect MyNes source, tests, packaged artifacts and active documentation to `app-mynes` and `binary-mynes`. |
| Non-goals | No Lib/Common source change, emulator behavior change, ROM-content change or historical-document rewrite. |
| Reference Baseline | Owner-selected move committed in `7666ddd`; M6 T37 completed App/Core quality and dual-architecture verification. |
| Candidate Proposal | [M6 product layout reconciliation](../proposals/m6-product-layout-reconciliation.md) |
| Files And ABI Surface | CMake traversal and include roots; `assets/binary-mynes/mynes.ini`; versioned x64/x86 executables and manifest. |
| Applicable Rules | Architecture, coding, documentation, execution and source rules; preserve the shared Lib/Common corpus. |
| Verification | Fresh x64 and x86 builds each passed 118 CTest cases; documentation governance and release-manifest generation/verification passed. |
| Expected Markers | No active old-layout references; both trees register the same product targets and test suite; sole configuration and binaries are packaged together. |
| Asset Needs | Existing ignored owner-local ROMs remain ignored; the sole packaged `mynes.ini` remains versioned. |
| Reporting Requirements | Record changed surfaces, add/remove totals, x64/x86 results, generated artifact paths and any remaining external-reference exclusions. |
| Stop Conditions | Stop only for a new behavior defect, an irreconcilable build constraint or a required owner choice. |
| Exit Criteria | Owner validates the rebuilt binaries or reports a reproducible regression. |
| Original Owner Request | Reorganize source, tests and documentation around the owner-selected directory/file structure and ensure everything runs. |
| Similar-Issue Sweep | Search build scripts, include directives, artifact tooling, active documentation and documentation-governance fixtures for former product paths. |

## Current Technical Baseline

- Product: MyNes; MIT. M0--M5 and M6 T36 are closed.
- Delivery kind: `product-execution`; current target: `mynes-0-1-0011`; artifacts
  are `assets/binary-mynes/mynes_0_1_0011_x64.exe` and
  `mynes_0_1_0011_x86.exe`.
- MyNes snapshots are private versioned `MNS1` state images. App owns command/file
  policy and direct writer lifetime; Core owns image state and cartridge identity;
  Common/Lib remain neutral. ROMs remain ignored and no remote is configured.

## Recent M6 Closures

| Task | Compact result |
| --- | --- |
| T36 | Closed: SoftPC-shaped `save`/`load`, Core/Driver state stream, matching-cartridge restore, rejected malformed/missing/truncated inputs and dual-architecture integration proof are recorded in [history](../history/M6-T36-snapshot-state.md). |
| T37 | Closed in `7666ddd`: the completed App/Core quality work and the owner-selected source, test and packaged-artifact move were retained together for T38 reconciliation. |
| T35 | M5 closed in `d6da730`: six owner-local ROM roles passed both presenters/x64/x86. |

## Recent Governance

- **M6 Td S1:** Owner-approved snapshot milestone and proposal retained in [M6 snapshot state](../proposals/m6-snapshot-state.md).
