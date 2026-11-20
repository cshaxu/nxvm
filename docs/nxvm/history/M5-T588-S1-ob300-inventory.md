# M5 T588 S1: closed inventory

Owner authorized autonomous in-task planning and execution after reviewing
the inventory and PC110-style architecture. Coordinator reviewed 6954bab1c:
ROM identity, archive versus sector-image distinction, twelve ledger domains,
local-document links, LF and documentation gate are recorded. No production
or artifact changes occurred. S1 is closed; actual reset execution belongs
to S2, not this inventory.

## Archived packet

# M5 T588 S1 - Delivered for review: OB300 identity and implementation inventory

The [S1 inventory](../etc/evidence/t588-ob300-convergence.md) verifies both
external ZIPs and the ROM identity, inventories twelve finite domains and
assigns their proof and implementation receivers. No production or artifact
change is included. S2/S3 remain planned; T588 remains open.

| Field | Required record |
| --- | --- |
| Identifier Mode | New M5 T588 S1; T587 is owner-closed at cd6477a5c; latest task allocation checked. |
| Admission And Approval | Owner approved closing the current T and admitting the first OB300 queue task on 2026-11-19. |
| Objective | Freeze OB300/System ROM 1.0 identity and a finite hardware/software/performance gap ledger against current code. |
| Non-goals | No App/Shared implementation, ROM copying into repository, runtime boot or performance success claim; no 425/430 implementation. |
| Reference Baseline | cd6477a5c; owner-supplied 1.0 ABA archives and the already reviewed HP service manual 5965-0244. |
| Candidate Proposal | proposals/m5-ob300-design.md; S1 consumes identity and ownership inventory rows of the T588 convergence ledger. |
| Files And ABI Surface | NXVM task, queue, proposal and evidence documents only; no public ABI or production changes. |
| Applicable Rules | NXVM guide, Contributing, Roadmap, architecture/layout, shared Execution/Documentation/Architecture/Coding and NXVM source policy. |
| Verification | Read-only ZIP CRC/hash/metadata analysis; current CPU/memory/device interfaces and test ownership inspection; ledger/proposal diff, links, docs and LF gates. |
| Expected Markers | Exact card baseline; known versus unknown hardware facts separated; every boot route has an owner, proof requirement and next receiver. |
| Asset Needs | Read existing owner archives in memory; no extraction, redistribution or build scratch. External masters remain unchanged. |
| Reporting Requirements | Report evidence-backed feasibility, gaps, ownership, proposed S2/S3 work and no-runtime-verification boundary. |
| Stop Conditions | Do not infer physical bank decoding from dump repetition, compatible silicon from profile names, or measured OB300 speed from historical PC110 samples. |
| Exit Criteria | S1 inventory/ledger and receiver design reviewed, docs checks pass and complete P pushed; no claim of T588 completion. |
| Original Owner Request | Close current T and admit the new OB300 implementation task at the queue head. |
| Similar-Issue Sweep | CPU identity versus family names; reset aliases versus card address space; ROM modules versus extracted files; current versus historical performance evidence. |

