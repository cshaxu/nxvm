# Project Status

## Current Work

M6 T43 S8 is admitted for six-corpus quality audit and reviewed remediation.
Owner-approved A-F repairs and receiving builds/tests are complete. Shared P1
and NXVM P2 plus MyNES P3 are pushed. NXVM P4 5373a1a05 delivers the owner-required
0/1-only INI parser, four supplied INIs, regression, UX contract and eight
rebuilt EXEs. Owner accepts S8 closure and admits the next S9 snapshot-resume
repair. T43 is not closed; the S9 packet is being prepared before implementation.

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M6 T43 S8; single-session sequential coordinator/executor roles. |
| Admission And Approval | Owner on 2026-09-25 approves all reported quality repairs within S8, including public boolean layout migration and test registration cleanup; also authorizes verification and submission of existing MyNES composition/native-test and NXVM.ini edits. |
| Objective | Repair approved A-F boolean, unused-reference, comment and duplication findings across all six roots; verify receiving products and owner's extra edits. |
| Non-goals | No semantic changes to native ABI/atomics/bitsets, assembler-table rewrite, new public API, sibling edits or unrelated product behavior. Preserve owner configuration content. |
| Reference Baseline | ed818b047; preserve unrelated default NXVM.ini edit. |
| Candidate Proposal | ../proposals/m6-t43-six-component-types-boundary-audit.md, S8. |
| Files And ABI Surface | Shared six roots; boolean copied-value fields, predicate declarations/callers, static gates/manifests and test CMake. NXVM and MyNES receiving adapters/tests, current dual-width artifacts and evidence; existing owner edits reviewed without discarding. Each P changes one target. |
| Applicable Rules | Shared EXECUTION, ARCHITECTURE, CODING, DOCUMENT; product guide and design; architecture/coding governance skills; existing six-corpus contracts. |
| Verification | Freeze tracked inventory; classify query hits by real reads/writes/callers; verify six manifests and existing static gates; report proof versus candidates. After approved fixes, full dual-width suites and affected receiving artifacts are required. |
| Expected Markers | Concrete line references, semantic exceptions, proposed minimal owner-local repairs and separate discussion items; no count presented as exhaustive semantic proof. |
| Asset Needs | Existing build trees and admitted BYOB product inputs; ten current optimized stripped EXEs rebuilt, no protected asset committed. |
| Reporting Requirements | Report pushed repairs, exact verification and artifacts; include owner-approved 0/1-only INI correction and await owner manual verification. |
| Stop Conditions | Stop on newly required behavior/ownership changes beyond approved findings or unapproved targets; report rather than hiding a failing gate. |
| Exit Criteria | Audit phase: indexed finite inventory and findings report. S closure: subsequently approved repairs, verified source/tests/manifests, affected binaries, scoped pushed delivery and coordinator acceptance. |
| Original Owner Request | Audit and repair boolean vocabulary, unused references, stale comments and duplicated/over-layered implementation; first report real findings and discuss individually. |
| Similar-Issue Sweep | All six tracked roots; distinguish predicates from counters, atomics, serialized/native ABI, bitsets, tri-state/status and deliberate negative fixtures; review duplicate responsibility before proposing merging. |

| Task | Progress |
| --- | --- |
| T43 S8 | Shared P1 bfcbd31b2, NXVM P2 14403bf51, MyNES P3 239c0fcd1 and NXVM P4 5373a1a05 pushed. Shared/MyNES 130/130 and NXVM unit 335/335 on both widths; standalone Lib/Common/X86 49/49, 18/18, 10/10. Numeric-only INI correction verified with fresh NXVM units 335/335 per width and all eight receivers rebuilt. Actual-diff review passed; awaiting owner manual verification, no S/T closure. |
| T43 S7 | Shared P1 `38ed0f26f`, NXVM P2 `6db4a9298`, MyNES P3 `6682a341c` accepted after actual-diff review. Shared/MyNES 130/130 and NXVM units 335/335 on both widths; ten affected EXEs rebuilt, checked and pushed. Six manifests/Types/docs gates pass. Native failure exclusions and async follow-up are explicit in TODO. Existing unrelated default INI edit remains untouched. |

Proposal: [six-component Types boundary audit](../proposals/m6-t43-six-component-types-boundary-audit.md).
History: [T43 accepted work](../history/M6-T43-six-component-types-boundary-audit.md).
Approved repair ledger: [S8 findings, repairs and verification](../etc/evidence/m6-t43-s8-six-corpus-quality-audit.md).
Evidence: [S7 repair ledger and artifact hashes](../etc/evidence/m6-t43-s7-shared-quality-repairs.md).

## Current Technical Baseline

- Product: MyNES; MIT. Optimized, stripped 0043 x64/x86 developer artifacts in
  `assets/mynes/` are refreshed by S8; their hashes are in S8 evidence.
- Shared baseline: S8 `bfcbd31b2`, following the S4 SoftPC `dc9c34ce`
  import and subsequent approved local corrections. No sibling repository was
  modified. Six manifests and Types gates pass; physical audibility is not
  a unit-test acceptance criterion or a claim of this delivery.
- Snapshot files are private versioned `MNS1` state images. App owns command
  and file policy; Core owns image state and cartridge identity; Common/Lib
  remain neutral. ROMs remain owner-local and ignored.
