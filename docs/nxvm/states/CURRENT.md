# Project Status

## Current Work

M5 T539 S1 is admitted for independent-chip extraction research and design.
Production migration is not admitted in this S. The owner first requests the
directory/dependency design, per-chip gaps, required diffs and prior decisions.

| Task | Progress |
| --- | --- |
| T539 S1 | Active: NXVM-only architecture audit/design; no source or artifact changes. |

| Field | Required record |
| --- | --- |
| Identifier Mode | New: M5 T539 S1, after closed T538 and Td S174. |
| Admission And Approval | Owner admits queue candidate 1 in this conversation on 2026-09-27; research/design first, no production migration yet. |
| Objective | Freeze all devices files and caller/test boundaries; design independent shared chips, dependency rules, chip-specific deltas and decisions for owner review. |
| Non-goals | No source/test/build/Shared/MyNES/INI/asset changes; no new hardware, timing upgrades, ibmpc implementation or App split. |
| Reference Baseline | Clean master 996a19a17; T538 runtime and eight 0538 EXEs unchanged. |
| Candidate Proposal | [Independent chips](../proposals/m5-shared-chip-extraction.md); [task history](../history/M5-T539-independent-shared-chips.md). |
| Files And ABI Surface | NXVM docs only: active packet, queue, proposal, history, evidence index and design/ledger; proposed APIs only, no ABI edits. |
| Applicable Rules | NXVM guide, Architecture/Coding/Roadmap, source policy; shared Execution/Architecture/Coding/Document rules; architecture-governance and coding-governance skills. |
| Verification | Tracked-file ledger coverage; include/call/field/caller/build/test inspection; Markdown links, git diff --check and NXVM documentation gate. Design-only scope requires no runtime rebuild or invented runtime proof. |
| Expected Markers | All tracked devices files classified; every chip group has current boundary, target, concrete change and regression owner; decisions separated from approved facts. |
| Asset Needs | None; original code and existing evidence only; no external ROM/source import. |
| Reporting Requirements | Report target tree, dependencies, per-chip gaps/diffs, sequencing and pending decisions with source anchors; stop before production work. |
| Stop Conditions | Missing owner decision needed for implementation; forbidden scope edit; claimed chip identity unsupported by current evidence. Record these as design decisions, not invented hardware contracts. |
| Exit Criteria | Requested design and complete finite inventory reviewed against actual source; governance proof and documentation delivery; T remains open for owner review of implementation. |
| Original Owner Request | Admit independent chip extraction; first research directory/dependency structure, each chip's decoupling gaps, needed diffs and architecture decisions. |
| Similar-Issue Sweep | Inventory entire devices subtree plus production callers, build descriptions and owning tests for direct peer state, private machine dependencies, fixed board routes, time ownership and public-boundary bypasses. No semantic completeness claim. |

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
