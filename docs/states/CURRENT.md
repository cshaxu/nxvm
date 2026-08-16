# Project Status

## Current Work

**T386 open; S1--S17 accepted; S18 active.** S18 receives the selected
Model-40 1.2 MB diskette-drive functional gap: its profile contract must choose
the reusable 5.25-inch 1.2 MB device profile while the shared Core FDC continues
to consume only the profile-neutral media contract. It is bounded functional
progress only. Flux, CRC, rotation/index, physical CHRN, service timing, board
waits and the Model-40 L3 audit remain open.

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M5 T386 S18. |
| Admission And Approval | The owner approved continuing the DeskPro functional-before-timing program and clarified that profile concerns belong in VM while universal hardware capabilities belong in Core. T386 S5 and S15 identify the selected 1.2 MB drive/media contract as an open receiver; no exception. |
| Objective | Close the selected Model-40 1.2 MB diskette drive functional binding: VM selects the reusable 5.25-inch 1.2 MB device profile (80 cylinders, two heads, 15 sectors per track and 512-byte sectors) for its raw-IMG media path, presents the resulting geometry through the existing profile-neutral media contract, and retains the selected FDC DMA2/IRQ6 composition. |
| Non-goals | No Core Model-40/Compaq policy, FDC command-state rewrite, real BIOS/firmware execution, guest-media import, physical flux/CRC/rotation/index/CHRN representation, data-rate service-duration claim, controller/ISA wait or phase timing, generic clone behavior, public profile/catalog exposure, test-only production API or Model-40 L3 claim. |
| Reference Baseline | T384 selected Model-40 capability audit; T386 S5 primary-backed 1.2 MB/500-kbit/s selection and generic FDC boundary; S7/S8 private composition; S15 functional ledger; accepted S16 at c25d46e1 and S17 at 12d606c5. |
| Candidate Proposal | docs/proposals/m5-deskpro-386-model40-device-functional-closure.md with the DeskPro shared closure context. |
| Files And ABI Surface | VM profile owns reusable device-profile specifications; VM machine media owns profile-neutral raw-IMG allocation and provider behavior; Model-40 private composition selects the reusable 5.25-inch 1.2 MB specification. Core media/FDC contracts and transactions remain profile-neutral and change only if a demonstrated generic capability is missing; no Core file may contain a Compaq name, selected geometry or profile decision. Existing default/5170 VM behavior retains its 1.44 MB selection. Tests use project-owned synthetic image bytes and existing production paths only. |
| Applicable Rules | docs/rules/EXECUTION.md, docs/design/ARCHITECTURE.md, docs/design/CODING.md, docs/rules/ARCHITECTURE.md, docs/rules/CODING.md, the Model-40 functional proposal and VM-to-Core dependency direction. VM owns bootable-machine/profile composition; Core owns only generic media/FDC transactions and contracts. |
| Verification | Sweep every VM FDD geometry initializer, create/insert/replace/reset path, media-provider geometry observation, all profile/session constructors and FDC binding callers. Add focused Model-40 private-session proof for selected device profile, 1,228,800-byte media acceptance, incorrect-size rejection, media observation, retained FDC DMA2/IRQ6 topology and reset persistence; retain default/5170 1.44 MB controls. Run affected media/FDC/composition tests, ownership checks, serial current gate, artifact rebuild, documentation governance, diff review and coordinator actual-change review. |
| Expected Markers | M5:T386:S18:MODEL40-FDD-GEOMETRY:OK, M5:T386:S18:MODEL40-FDD-MEDIA:OK, and M5:T386:S18:MODEL40-FDD-RESET-BINDING:OK. |
| Asset Needs | No ROM, firmware, guest media or external source is needed. T386 S5 retains the primary-backed selection facts; tests create only project-owned synthetic bytes in memory. |
| Reporting Requirements | Record the VM-versus-Core ownership decision, selected geometry/media size, default/5170 control result and explicit physical/timing transfers in indexed evidence. Push one complete implementation P with proof and gates, then conduct coordinator actual-change review and governance P. |
| Stop Conditions | Stop and transfer if selected behavior requires an actual BIOS policy, an unrepresented data-rate/drive-electrical contract, media encoding/physical sector identity, a Core profile-specific condition, or a change to public profile selection. Stop if a generic VM geometry mechanism cannot preserve existing 1.44 MB session consumers atomically. |
| Exit Criteria | Model-40 private composition selects the stated 1.2 MB reusable device profile; generic Core observes it only via its existing media contract; focused production-path proof covers exact media size, rejection, observation, reset and retained FDC route; default/5170 controls retain 1.44 MB; no protected asset is tracked; all applicable gates and coordinator review pass. |
| Original Owner Request | Achieve complete DeskPro 386 Model 40 functionality and L3 timing under the approved queue, capability-ledger-first order, with profile configuration in VM and universal hardware capabilities in Core. |
| Similar-Issue Sweep | Sweep all reusable VM floppy device-profile selection, VM FDD allocation, image replacement/insertion, sidecar and reset code; every session/profile construction path; all media-registry/FDC callers and focused media/FDC tests. Classify each hit as shared Core mechanism, reusable VM device mechanism, Model-40 profile selection, another profile consumer or later physical/timing work; repair the owner or explicitly transfer the boundary. |
## Current Technical Baseline

- **Current developer artifact:** T386 S18 `vm-0-5-0389` /
  `build/output/nxvm_0_5_0389.exe`, SHA-256
  `29A174DBC889A1220B0F4BD17805AAA78B326F01B31E5D68D5688F6B5DEC09E4`.
  Built from the S17 worktree after its 272-test serial current-gate; the
  dual-DMA mapping proof is retained in
  [S18 evidence](../etc/evidence/t386-s18-model40-floppy-device-profile.md).
- **Model-339 readiness:** T383 S1 accepts the current runnable source graph
  under the frozen deterministic event-and-bus L3 contract. This is only the
  selected Model 339 configuration and preserves every documented physical,
  fixed-disk, generic-PC/AT and later-machine boundary.
- **T285 display implementation:** `INT 10h` mode `10h` /
  `EGA-640x350x16-direct` has a VADP-owned planar frame path and copied-frame
  consumer boundary; mode 0Dh remains a separate retained path.
- **Core boundary:** T243--T246 retain checked physical memory, immutable ROM
  mapping, and validated real-mode entry plans. The obsolete post-`#UD`
  transition has no public or runtime path.
- **Product boundary:** `nxvm.exe` is the retained runnable product. `mantle`,
  `dos`, and `nxvdm.exe` remain future architecture commitments; they are not
  current runtime or release products.

## Recent M5 Closures

| Task | Compact result |
| --- | --- |
| T386 S17 | Accepted: Model-40 composition explicitly selects the generic dual-8237A/channel-4 cascade topology; Core validates it and retained/new proofs cover word/cascade/reset plus FDC DMA2 binding. DCLK, arbitration, expansion endpoints, remaining devices, board timing and L3 remain open. [Evidence](../etc/evidence/t386-s17-model40-dual-dma.md). |
| T385 | Closed: the selected Model-40 80386DX-16 consumes the shared 80386 CPU/state/retirement owners without a DeskPro CPU fork; a clean current-source 251/251 gate passed, while functionality and board timing transfer unchanged. [Closure audit](../etc/evidence/t385-s1-task-closure-audit.md). |
| T384 | Closed: the Model-40 capability audit freezes its original Compaq system board/storage ledger and owner-selected color EGA, keyboard and external-ROM constraint; its shared-VADP IBM/Compaq personality boundary transfers CPU, functional and timing work without making a runnable or L3 claim. [Closure audit](../etc/evidence/t384-s2-task-closure-audit.md). |
| T383 | Closed: independent current-source replay accepts the frozen 5170 Model 339 deterministic-L3 contract after T380's strict-start repair; 251/251 current gate and focused matrix are retained in [evidence](../etc/evidence/t383-s1-model339-current-source-reaudit.md). |
| T382 | Reclosed at S8: the prior S1--S7 closure is retained historically, while S8 corrects its Console 64-bit transport and final artifact baseline. [Corrective audit](../etc/evidence/t382-s8-reclosure-audit.md). |
| T381 | Closed: S1 P1 `9b6a86c6` delivered frozen YAML profile selection and the true zero-session Console; corrective S2 restored its task-level closure state without changing runtime evidence or the T381 artifact. [Evidence](../etc/evidence/t381-s1-console-yaml-profiles.md). |
| T380 | Closed: strict Model-339 startup, 512 KB FDC/BDA repair, input/FDC current-gate recovery, and the owner-replayed DOS 5/HIMEM no-XMS path are verified. It does not claim DOS completion or 5170 L3 closure. [Evidence](../etc/evidence/t380-s2-model339-start-and-current-gate-recovery.md). |
| T378 | Closed at `42f0402a`: production ingress proof and virtual-time ownership are corrected without a test-only API; Model-339 re-audit remains the sole L3 decision receiver. [Evidence](../etc/evidence/t378-s1-native-ingress-boundary-repair.md). |
| T379 | Closed at `160a34e1`: independent re-audit accepted the then-current frozen Model-339 deterministic L3 contract. Its result is historical and is superseded for the current graph by T383. [Audit](../etc/evidence/t379-s1-model339-final-reaudit.md). |

## Recent Governance

- **M5 Td S101 P1:** added a compact execution-entry table without creating
  new paths, and made coordinator semantic review explicit after structural
  documentation-gate success.
- **M5 Td S100 P1:** archived the closed T382 proposal, queued Model-339
  current-source re-audit before DeskPro, reconciled T380 S2 history, and made
  host/workspace path detection reject the external-assets form while retaining
  guest DOS-path examples.
- **M5 Td S99 P1:** compacted documentation gates, closure evidence, artifact
  identity, build hygiene, and recorder containment within Execution; design
  authorities remain unchanged.
- **M5 Td S98 P1:** made coordinator-review inputs, exit criteria, and the
  executor-report boundary explicit in Role cycle.
- **M5 Td S97 P1:** added explicit single-/separate-session review navigation
  without changing the lifecycle requirements.
- **M5 Td S96 P1:** compressed duplicate Role cycle prose into references to
  its retained authorities without changing lifecycle requirements.
- **M5 Td S95 P1:** removed duplicate M5 technical narrative from Queue while
  retaining all shared-context, candidate, and required proposal-index links.
- **M5 Td S94 P1:** locks the DeskPro 386 Model 40 and PC/XT 5160-268
  baselines, establishes capability-ledger then functional-before-timing
  closure for all three machines, and retains external ROMs outside Git.
