# Project Status

## Current Work

M5 T539 remains open. S1 is closed; S2 is active for concrete chip-boundary
contracts and the next migration brief. The owner goal is to implement chip/device
architecture separation. Shared source changes still require design review;
S2 changes NXVM documentation only, not runtime code.

| Task | Progress |
| --- | --- |
| T539 S2 | Active: resolve live consumers and concrete bus/signal/time contracts; prepare PIT extraction for owner review. |

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M5 T539 S2, after S1 P2 2197486b0. |
| Admission And Approval | Owner's continuing goal on 2026-09-28 is chip/device architecture separation; this prerequisite S is concrete design only. Existing owner review before Shared source modification remains binding; no such edits admitted here. |
| Objective | Resolve CPU firmware-hook consumers and FDC response ownership; define bus/interrupt/DMA/time/lifetime boundaries and a bounded PIT migration contract. |
| Non-goals | No source/test/build/Shared/MyNES/INI/asset edits, new hardware, timing requalification, ibmpc implementation, App split or speculative framework. |
| Reference Baseline | Clean master 2197486b0; 81-file S1 inventory and T538 runtime baseline unchanged. |
| Candidate Proposal | [Independent chips](../proposals/m5-shared-chip-extraction.md); [task history](../history/M5-T539-independent-shared-chips.md). |
| Files And ABI Surface | NXVM Current/proposal/history and supporting design/evidence index; proposed owner-local contracts only, no public ABI changes. |
| Applicable Rules | NXVM guide, Architecture/Coding/Roadmap, source policy; shared Execution/Architecture/Coding/Document rules; architecture-governance and coding-governance skills. |
| Verification | All tracked firmware-provider definitions and assignments; FDC policy selections, consumers and existing evidence; PIT production/readback/test/clock/binding paths; document links, diff check and NXVM documentation gate. No runtime build/test claim for design-only edits. |
| Expected Markers | Named consumer dispositions, explicit units/effects/ownership, no invented replacement hook, concrete PIT API and fixture/build acceptance, unresolved hardware behavior separated from structural decisions. |
| Asset Needs | None; original code and existing evidence only; no external ROM/source import. |
| Reporting Requirements | Report new evidence and exact proposed change, request review of first Shared extraction, preserve full T objective and stop before unapproved Shared changes. |
| Stop Conditions | Unresolved behavior may not be disguised by a neutral name or asserted as hardware truth; Shared source permission and chip-specific semantic evidence required before its implementation. |
| Exit Criteria | Reviewable contracts and PIT migration brief grounded in source; governance checks and pushed design record. T539 remains open; no chip reported migrated. |
| Original Owner Request | Implement chip/device architecture separation; preserve prior request for directory/dependency/per-chip design and S breakdown. |
| Similar-Issue Sweep | Search all tracked production/test firmware providers for the proposed obsolete interception hook; inspect all FDC unready policy sites and PIT routes, including Model-40 auxiliary timer and output-binding conflict validation. |

[Task history](../history/M5-T538-deployed-boot-pairs.md) retains reviewed packets
and actual-change acceptance. [Archived proposal](../history/M5-T538-deployed-boot-pairs-proposal.md),
[convergence ledger](../etc/evidence/t538-boot-pairs.md), and
[S7 evidence](../etc/evidence/t538-s7-orphan-release.md) record scope, revised
acceptance, tests and hashes.

M5 Td S174 queued the three-stage migration. Its first candidate is now T539;
the seven remaining [Queue](QUEUE.md) candidates retain their dependency order.

## Current Technical Baseline

Four fixed products remain XT, AT, Model 40 and default PC/AT; PC110 is not
runnable. Eight optimized stripped 0538 EXEs are deployed with unchanged owner
INIs. MyNES retains its two rebuilt 0043 receivers; its T43 remains closed.

Canonical Shared revision is 268464d49; executable behavior is 064b9619b, with
P4's key-specific comment clarification only. Lib source/test add the approved orphan
release repair to 0c71110b0. S5's test-path return corrections also remain for
SoftPC to import. Common/x86 production is unchanged; full sibling parity is
not claimed. No sibling repository was modified.

Verification: NXVM 336/336 units, 21/21 static checks and 20/20 external
integration per width; MyNES 132/132 per width; all six manifests and both
documentation gates pass. Current deployed qualification is 8/8, including
confirmed pause/Debug handoff. Cooked-history rollback debt remains in
[TODO](TODO.md). This bounded boot acceptance does not claim complete hardware
timing qualification or indefinite absence of intermittent faults.

## Historical Context

[Earlier status](../history/M6-T41-and-prior-status-archive.md) preserves prior
deliveries and their hosting context. T538 history preserves S6's negative
qualification and S7's unclassified observations alongside the final accepted
single-pass matrix.
