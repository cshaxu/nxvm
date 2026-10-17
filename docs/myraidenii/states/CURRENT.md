# MyRaidenII Status

## Current Work

M0 Td S1 governance/design is delivered for owner review on 2026-10-17:
rename MyArcade to MyRaidenII, transfer former NXVM queue entry 10, establish
independent governance and design the implementation. M0 awaits design
acceptance; no numeric T or M1 code is admitted.

| Field | Required record |
| --- | --- |
| Identifier Mode | Governance, MyRaidenII M0 Td S1, first milestone-local Td; P starts at P1. Single-session coordinator/executor roles. |
| Admission And Approval | Owner request on 2026-10-17 explicitly selects NXVM entry 10, names MyRaidenII and src/myraidenii, test/myraidenii, assets/myraidenii, docs/myraidenii, and requires M0 planning/M1 implementation. Shared governance registration and NXVM queue transfer are necessary bootstrap targets, not production Shared changes. |
| Objective | Establish an independently governed product and an evidenced minimal architecture/implementation queue for original V30-based RAIDEN II. |
| Non-goals | No emulator implementation, ROM acquisition/import/embedding, third-party code import, new Shared API, neighboring App code/configuration/EXE change or M1 admission. |
| Reference Baseline | Clean repository da9e84b88; former NXVM MyArcade candidate and current Lib/Emulator/X86 Core contracts; fixed official MAME research revision in the reference register. |
| Candidate Proposal | Owner-admitted governance request; original candidate is archived in NXVM history. Product architecture, roadmap and unnumbered M1 proposals form the design output. |
| Files And ABI Surface | MyRaidenII documentation and source/test/assets root README files; Shared rules, AGENTS/Contributing, existing documentation checker and release ignore rule; NXVM queue/roadmap/candidate archive. No C/H, runtime ABI, CMake product target or binary. |
| Applicable Rules | Shared execution target isolation/linear IDs/Td lifecycle; architecture single owners/inward dependencies; coding no frameworks/aliases/placeholders; document separate product authorities; source policy reference-only third-party research and external ROMs. Use architecture-governance then coding-governance. |
| Verification | All three product documentation gates, relative Markdown links, exact root/owner map, ordered Queue/proposal correspondence, fixed source identity and local capability audit, diff/whitespace/target review; unchanged existing source/test/CMake/assets executable hashes. No runtime qualification claimed. |
| Expected Markers | Documentation gates and links pass, product paths have no app- prefix, independent M0/M1 and target are registered, proposals are unnumbered, no production/ROM import or executable artifact. |
| Asset Needs | None for M0; future owner ROMs live outside this repository in the owner-managed nxvm-assets archive. Do not invent a ROM set inventory. |
| Reporting Requirements | Explain real reuse/gaps, design boundaries, M1 sequence, remaining source/asset gates and pushed commits; await owner M0 design acceptance before M1. |
| Stop Conditions | Unreviewed source/license import, protected payload, another App runtime edit or Shared production/API change; record prerequisites instead of bypassing them. |
| Exit Criteria | Reviewed design/governance and queue are committed/pushed in separate Shared/NXVM/MyRaidenII P targets; worktree clean; M0 planning delivered for owner acceptance, M1 unadmitted. |
| Original Owner Request | Admit queue entry 10; rename to MyRaidenII with the four exact roots; establish independent governance/task queue and design, with M0 planning and M1 implementation under MTSP. |
| Similar-Issue Sweep | Search active docs/root pointers/checker target enums for MyArcade and fixed two-product assumptions; migrate current references, preserve historical facts; no broad neighboring product cleanup. |

## Technical Baseline

Governance/design only. The three source/test/assets roots contain descriptive
README files, not a runnable scaffold. Existing shared components and all five
existing Apps are unchanged. No MyRaidenII EXE, game compatibility, V30 support,
sound implementation or licensed ROM inventory exists yet.

## Delivery Evidence

[Bootstrap evidence](../etc/evidence/m0-s1-bootstrap.md) records the complete
Shared P1 / NXVM P2 / MyRaidenII P3 delivery, three passing documentation gates,
155 checked relative links, packet/queue review and unchanged ten EXE hashes.
No runtime or desktop qualification is claimed for this documentation-only work.

## Admission Boundary

M0 design delivery does not admit M1. [Queue](QUEUE.md) owns the next ordered
candidate; the first M1 T receives T1 only after owner approval. Do not run
production work between accepted/admitted packets.
