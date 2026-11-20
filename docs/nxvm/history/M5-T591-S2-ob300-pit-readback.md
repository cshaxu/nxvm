# M5 T591 S2: Intel 8254 read-back correction

Coordinator review accepted Shared `530e40076`, NXVM `923a69dd0`,
MyNES `05e3aa0bc` and MyRaidenII `85493bb7b`. The existing 8254 decoder now
selects the documented counters. The updated chip and board tests preserve
the independent waveform and count contracts; x64 and x86 each completed
the 717-unit inventory after correction. All fourteen receiving EXEs were
cached-built, published, independently checked and pushed at their existing
versions. No product input or configuration changed.

[Verification and artifact record](../etc/evidence/t591-s2-ob300-pit-readback.md).
A real-card run against this final production commit is pending external
volume availability. The verified scratch run and separate protected-mode
stop are recorded in S1. The T remains open for original DOS qualification.
Removed 26 task-owned temporary files totaling 472,709 bytes after task exit.

## Archived packet

# M5 T591 S2 - Active: Intel 8254 read-back selection

[S1 is closed](../history/M5-T591-S1-ob300-original-rom.md).
[Original firmware evidence](../etc/evidence/t591-s1-ob300-original-rom.md)
establishes a shared 8254 register defect, independently of the next
protected-mode shutdown.

| Field | Required record |
| --- | --- |
| Identifier Mode | M5 T591 S2 after S1 receiver acceptance. |
| Admission And Approval | Owner authorized ordered OmniBook work and autonomous S allocation; the standing x86 chip scope covers this behavior repair. S1 recorded the demonstrated Shared gap before this admission. |
| Objective | Correct counter selection in the existing 8254 read-back decoder, reconcile tests that encoded the inverted mask, and qualify receiving products. |
| Non-goals | No API, state, timing, compatibility path, other Shared package, ROM modification, or guessed repair for the next protected-mode stop. |
| Reference Baseline | S1 commit 8209ce97c and its production/scratch replay; Intel 8254 data sheet Figures 10 and 12. |
| Candidate Proposal | proposals/m5-ob300-rom-dos.md T591 S2. |
| Files And ABI Surface | Shared x86-nxvm PIT chip and corresponding test/manifest; NXVM MyOB300 unit command encoding; publication follows each existing product scope. No ABI change. |
| Applicable Rules | NXVM guide, Contributing, Roadmap, source policy and shared Execution/Documentation/Architecture/Coding. |
| Verification | Focused cached x64/x86 PIT/board regressions and full repository qualifications following Shared rules; cached release, verify and publish 14 receiving EXEs. Recheck the original card with the actual production build when the owner input is readable. |
| Expected Markers | Status-only counter 0 and all-counter selection work; counter-only, status-plus-count and 8253 behavior retain their distinct semantics. Real POST diagnostic changes from 1100h to 0000h. |
| Asset Needs | Use build/t591-s2 for disposable diagnostics, at most 64 MiB and 300 seconds per native probe. Owner System Card remains external. |
| Reporting Requirements | Record exact changed paths, cached build and verification results, artifact identity/hashes and any pending original-media verification. |
| Stop Conditions | No new Shared interface or public change outside this PIT scope without fresh evidence and scope review. Do not use a higher boot budget to conceal a repeatable shutdown. |
| Exit Criteria | Existing PIT semantics and the corrected polarity pass on both widths; Shared verification and pushed product artifact P records are complete, with a clean worktree. |
| Original Owner Request | Implement original OmniBook hardware, wiring, ROM machine and product in ordered batches. |
| Similar-Issue Sweep | Exact selection masks, repeated reads, per-counter latch behavior, unselected counters and 8253 behavior. |
