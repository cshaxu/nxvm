# M5 Td S186: OmniBook 300 queue planning

On 2026-11-19 the owner requested multiple bounded T candidates for OB300,
all at the head of the NXVM queue. Five linked proposals replace the previous
single family candidate. Numeric T/S allocation waits for implementation
admission; T587 remains open and S5 stays closed.

## Actual-change and closure review

The queue has five sequential OB300 candidates followed by the four previously
pending non-OmniBook candidates in their existing order. The family proposal
retains the original card hashes and research, corrects the OB300 CPU identity
against the already reviewed HP manual, and removes the old OB430-first plan.
425/430 remain research references rather than duplicate implementation scope.

Each new proposal defines objective, ownership/scope, prerequisites, non-goals,
completion and stop conditions. Hardware design freezes a finite coverage
ledger and measurement budgets; ROM DOS supplies the real workload; the
performance gate precedes Windows qualification. The performance task does
not require unnecessary changes if the baseline already meets its budgets.
Windows qualification includes original OBSETUP, ROM modules, Word/Excel and
persistence. Final product acceptance reconciles required devices and explicit
limits rather than accepting a static boot screen or silently dropping gaps.

Source ownership remains in existing packages plus the future machine App.
ROM software remains external under current source policy. No source, tests,
Shared interfaces, ROMs, user media or EXEs changed; no new build is warranted.
Actual diff, proposal links/dependencies, documentation governance, LF and
whitespace checks are the registration verification. This complete governance
P closes Td S186 and introduces no active implementation task.

## Archived packet

# M5 Td S186 - Active: OmniBook 300 queue planning

| Field | Required record |
| --- | --- |
| Identifier Mode | Standalone governance: M5 Td S186; latest governance allocation S185 checked against Current/history. |
| Admission And Approval | Owner explicitly requests multiple OmniBook 300 candidate T proposals at the queue head on 2026-11-19. |
| Objective | Replace the broad family candidate with five ordered, bounded OB300 implementation candidates. |
| Non-goals | No implementation admission, numeric T allocation, source/test/Shared changes, ROM import or binary refresh. |
| Reference Baseline | 7307336ab; accepted T587 S5 and existing OmniBook family research proposal. |
| Candidate Proposal | proposals/m5-omnibook-emulator.md and its five linked OB300 receivers. |
| Files And ABI Surface | NXVM Queue, Current, umbrella/receiver proposals and governance history only; no ABI. |
| Applicable Rules | NXVM guide, Roadmap, Execution, Documentation, Contributing and source/research policy. |
| Verification | Actual proposal/queue diff review, link and dependency review, documentation governance, LF and whitespace checks. |
| Expected Markers | Five OB300 candidates occupy positions 1-5; other pending candidates retain order; no duplicate family task. |
| Asset Needs | No new asset access or scratch directory; existing external ROM provenance is referenced only. |
| Reporting Requirements | Report five task goals, queue positions, commit/push and remaining admission boundary. |
| Stop Conditions | Do not claim emulation/performance results or authorize implementation through queue registration. |
| Exit Criteria | Five proposals linked and reviewed, queue consolidated, checks pass, governance committed/pushed; T587 stays open. |
| Original Owner Request | Add OmniBook 300 emulator to the queue, moderately split into multiple T tasks, all at the head. |
| Similar-Issue Sweep | Check prior family proposal duplication, CPU identity, ROM/software ownership, stage dependencies and performance acceptance. |
