# M5 T516: External-ROM Boot-Contract Baseline And Repair

The retained [proposal](M5-T516-external-rom-boot-contract-repair-proposal.md)
defines the owner-approved scope: a shared external-ROM boot-contract baseline,
IBM 5170 repair first, then DeskPro Model 40, without weakening the T515 asset
boundary.

## Task Record

| Subtask | Scope | State |
| --- | --- | --- |
| S1 | Freeze five-row external input corpus and replace copied test media with the YAML-declared VM overlay route. | Closed: `eee9cd9a` |
| S2 | Repair the source-qualified IBM 5170 owner mechanism. | Closed: FDC terminal-count repair; evidence `t516-s2-ibm5170-fdc-terminal-count.md` |
| S3 | Repair the source-qualified DeskPro Model 40 owner mechanism. | Closed: single-BAT KBC repair and checked seed; evidence `t516-s3-deskpro-kbc-post.md` |
| S4 | Run complete matrix, gates and closure audit. | Closed: unit 302/302; Release integration 44/44; documentation governance and diff check pass. |

## Closure Audit

The integration support opens each YAML session against its declared external
ROM, CMOS and media identity.  Media is read through the sole VM-owned overlay;
it is not copied or rewritten into a CTest work directory.

IBM 5170's FDC retains one controller state and one DMA-TC result path.  The
fixed physical 1.2 MB drive and the mounted 360 KB medium are separate facts;
the latter no longer changes the former.  DeskPro's 8042 retains one output/BAT
path: `FFh` owns its BAT result, while a later `AEh` is only a line release.
No BIOS recognition, profile-side controller state, synthetic input, or second
firmware/media path was added.

Release integration uses the product-equivalent optimized build: all 44 cases
pass.  In particular, 5170 360 KB and 1.2 MB reach `installer-ready` in about
28 seconds, and DeskPro Model 40 reaches the same terminal in about 66 seconds.
The complete repository-only Debug unit suite is 302/302.  `git diff --check`
and documentation governance pass.  The stripped runnable artifact is
`build/output/nxvm_0_5_0516.exe`, SHA-256
`E1C948EF17BFFF1BEB86CC1396AFBF75D6BF2F450F68C7F57CBAB12F2047D84E`.
