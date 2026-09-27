# Project Status

## Current Work

M5 T538 remains open. S2, S3 and S4 are accepted and closed; S5 is active.
The owner approved local repair of the imported cross-owner white-box test;
SoftPC will receive that test-only correction from this repository. See
[S5 evidence](../etc/evidence/t538-s5-input-reset-import.md).
Shared implementation 0c71110b0 is pushed. All required suites and ten builds
pass; target-scoped receiver deliveries and coordinator closure follow.
[Proposal](../proposals/m5-deployed-boot-pairs.md) and
[ledger](../etc/evidence/t538-boot-pairs.md) own the remaining whole-task repeated boot qualification.

## Active Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M5 T538 S5, next unused S after accepted S4. |
| Admission And Approval | Owner on 2026-09-27 requests audit and unchanged import, then approves fixing the test boundary here for SoftPC to import. Targets: Shared and both NXVM/MyNES receiving builds, tests and evidence. S4 is already closed. |
| Objective | Import pinned production unchanged; replace the cross-owner test with owner-local coverage and a boundary check, then verify both receiving products. |
| Non-goals | No sibling writes, product-specific shared fork, product policy change, INI/media/snapshot modification, or T-level boot closure. |
| Reference Baseline | NXVM 52e5da766; clean SoftPC 40da7d00 (full source pin in evidence), compared against b79769c1. |
| Candidate Proposal | [Deployed boot pairs](../proposals/m5-deployed-boot-pairs.md), S5 shared input-loss batch. |
| Files And ABI Surface | Imported source/test/manifest paths in four roots; x86 pair unchanged. Adds KVM_EVENT_INPUT_RESET and worker reset operation; consumers rebuilt together, no mixed enum ABI. Local corrections split the cross-owner fixture and update retirement-test event admission for reset without weakening single-retirement assertions. |
| Applicable Rules | EXECUTION, ARCHITECTURE, CODING, DOCUMENT; both product guides, architecture/coding/UI and source policies. Lib owns native loss facts and matcher state; Common owns delivered-key ledger; product drivers own guest input. |
| Verification | Production roots, test/x86 and registration helper remain pinned; enumerate approved test/lib and test/common deltas. Six manifests, Types/DAG/corpus gates; full NXVM units on x64/x86, external integration, MyNES full suites on both widths; ten optimized stripped builds; both documentation gates and actual diff review. |
| Expected Markers | Focus loss, freeze and Console handoff reset the source without replaying held prefixes; Common releases only its recorded source keys; duplicate reset is harmless; rejected reset fails explicitly. |
| Asset Needs | Existing lawful external inputs, read-only/overlay as configured. Preserve owner INIs/snapshot. Retain existing build trees for this receiver run and immediately following T qualification. |
| Reporting Requirements | Audit API and receiver fit before import; report root equality, test counts, ten artifact hashes, code-size delta and per-target pushed commits. |
| Stop Conditions | Independent third-party notice, unreviewed source change, unapproved shared divergence, incompatible consumer contract or regression requires coordinator disposition before closure. |
| Exit Criteria | Pinned production unchanged, approved test-only delta recorded for SoftPC; all required tests/builds/gates pass; scoped commits pushed; coordinator actual-change review; clean tree. T remains open. |
| Original Owner Request | Close old S, admit new S, audit and import all six SoftPC components; subsequently fix the identified test boundary here for SoftPC to import. |
| Similar-Issue Sweep | Inspect reset/retirement producers and consumers across Lib, Common and both Apps: focus, freeze, handoff, repeated reset, paused/stale generations, sink failure and source-specific releases. Reconcile this finite batch separately from the T boot matrix. |

## Retained Progress

| Task | Progress |
| --- | --- |
| T538 S4 | Accepted Shared 4ca7e6401, NXVM 882e6959a and MyNES 57f0e79e5; all six roots exactly match SoftPC b79769c1; ten receivers and both-width suites verified. |
| T538 S3 | Accepted Shared 6a3f3cb25, NXVM 7d29f409b and MyNES a60dcb906; full buffer repair, ten receivers, both-width suites and manifests verified. |
| T538 S2 | Accepted implementation 1d80f11d8: 5170 KBC repair, x64/x86 335/335 units, external integration 20/20, both-width Setup/reset/stop-start proof, eight isolated 0538 products. Separate Console repair transfers to S3. |
| T538 S1 | Accepted baseline inventory: 5e6ed82cd and acceptance ad99ae0fa; no production repair claimed. |
| T41 | Owner-accepted closed baseline; Shared b94ea4ffe, NXVM f3a681422 and closure 6a548c43b. |

## Current Technical Baseline

Four fixed products remain XT, AT, Model 40 and default PC/AT; PC110 is not
runnable. The eight optimized stripped 0538 EXEs replace 0535, with unchanged
owner INIs. S2 source/artifact SHA-256 values remain historical in the ledger.
The canonical Shared baseline is 4ca7e6401, identical to SoftPC b79769c1 across
all six roots; receiving hashes are in
[S4 evidence](../etc/evidence/t538-s4-console-import.md).

S4 verification passes: NXVM 335/335 units per width and 20/20 integration;
MyNES 132/132 per width. All ten receivers are rebuilt and pushed in target-separated commits. Whole-task three-fresh-launch qualification
remains pending. The separate cooked-history rollback debt remains in TODO.

## Historical Context

[Archived status and packets](../history/M6-T41-and-prior-status-archive.md)
preserve earlier deliveries and their original hosting context. The five
unrelated [Queue](QUEUE.md) candidates are unchanged.
