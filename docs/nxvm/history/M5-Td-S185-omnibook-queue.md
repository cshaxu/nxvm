# M5 Td S185: OmniBook candidate registration

Owner requested queue registration on 2026-11-11. Added the fifth and final
pending M5 candidate with its [proposal](../proposals/m5-omnibook-emulator.md).
Existing priorities remain unchanged. No numeric T or implementation S is
admitted; T587 remains open without an active S.

Actual-change review confirms this is documentation only, records the three
card baselines without claiming guest execution, and keeps machine identity
separate from firmware revision. No firmware, source, media or EXE changed.
Documentation governance, LF and diff whitespace checks are the applicable
verification; compilation and runtime tests are not required for registration.

## Archived packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Standalone governance: M5 Td S185. |
| Admission And Approval | Owner requested adding the OmniBook emulator to the queue on 2026-11-11. |
| Objective | Append one unnumbered M5 candidate and a research-backed proposal. |
| Non-goals | No implementation admission, ROM import, binaries or Shared changes. |
| Reference Baseline | 9fa1fdcd7 and the read-only three-card investigation in this conversation. |
| Candidate Proposal | proposals/m5-omnibook-emulator.md. |
| Files And ABI Surface | NXVM queue, proposal and governance records only; no ABI. |
| Applicable Rules | Execution, Documentation, NXVM guide, Roadmap and source policy. |
| Verification | Actual diff review, documentation governance and LF checks. |
| Expected Markers | One appended candidate; existing priorities unchanged. |
| Asset Needs | None for queue registration; ROM originals remain external. |
| Reporting Requirements | Report queue position, scope and committed record. |
| Stop Conditions | Do not treat source investigation as guest-boot proof. |
| Exit Criteria | Candidate/proposal linked, checked and pushed; numeric T remains unallocated. |
| Original Owner Request | Add an OmniBook emulator task to the queue. |
| Similar-Issue Sweep | Check for duplicates and distinguish machine identity from system-card revision. |
