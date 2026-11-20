# M5 T592 S2 — Native verification and dual-width publication

The coordinator accepted the Shared release registration at ebd5fcc55 and the
PC delivery at 2d74d45d5, with the MyNES and MyRaidenII rebuilds in 625c2a6c6 and
4016d5cc0. Each target has a separate pushed commit. The
[qualification record](../etc/evidence/t592-s2-ob300-native-qualification.md)
contains the complete 16-EXE identity matrix and double-width test evidence.

Both exact MyOB300 release copies execute original DOS VER via native Console.
The owned fixtures remained byte-identical, and no owner INI or media changed.
All registered integration tests passed on each host width. Manual desktop and
RDP verification, the remaining product/device qualifications and the recorded
cold-start shortfalls are carried to their planned receivers. This closes S2.
The overall Product T remains active.

## Archived packet

# M5 T592 S2 - Active: OmniBook release entry and native DOS qualification

[S1 is accepted and closed](../history/M5-T592-S1-ob300-product-entry.md).
The accepted build-only candidate validates external inputs on both host widths.

| Field | Required record |
| --- | --- |
| Identifier Mode | M5 T592 S2 after ordered S1 acceptance. |
| Admission And Approval | Continue the owner's ordered OmniBook implementation with single-session coordinator/executor review. Necessary X86-NXVM and X86 Core changes are owner-approved. |
| Objective | Register MyOB300 in the existing build and publication tools, qualify the ordinary Product entry through interactive original ROM DOS, and publish the x64/x86 pair. |
| Non-goals | No protected bytes in Git, alternative entry, unrelated Shared edits or claim of Windows qualification. |
| Reference Baseline | T592 S1 entry evidence and the original DOS Machine evidence from T591. |
| Candidate Proposal | proposals/m5-ob300-product-qualification.md and m5-ob300-architecture.md O12. |
| Files And ABI Surface | MyOB300, common build/publication registration and owned documentation/test fixtures. Reuse the existing Product binding. Touch X86-NXVM/Core only for a reproduced defect that requires it. |
| Applicable Rules | NXVM guide, Contributing, Roadmap, Architecture/Layout, source and UI policy, shared Execution/Documentation/Architecture/Coding. |
| Verification | Cached x64/x86 build, release identity and deployment hash, relevant tests, original external card and disposable C medium. Exercise command delivery including start, DOS input, pause/resume, reset/resume and stop/start through the ordinary executable. Report desktop results separately. |
| Expected Markers | Both native executables reach the original DOS prompt, echo input and remain reusable across the lifecycle. Media writes do not affect the source. |
| Asset Needs | Owner System 1.0 ABA card and valid disposable C fixture. build/t592-s2 owns transient probes/media, 200 MiB and 300 seconds per probe. |
| Reporting Requirements | Preserve artifact hashes, provenance, actual native coverage, performance results and explicit remaining limitations. |
| Stop Conditions | Do not declare native usability or release qualification when command delivery or the original DOS marker is unverified. Do not commit third-party bytes or machine-local paths. |
| Exit Criteria | One proven release path, both published artifacts, repeatable BYOB setup and dual-width native original DOS verification. |
| Original Owner Request | Complete the OmniBook in hardware, board, Machine, Product and verification order. |
| Similar-Issue Sweep | INI and media relative paths, release inventory, built-on time, synthetic versus native input, and retained owner files. |
