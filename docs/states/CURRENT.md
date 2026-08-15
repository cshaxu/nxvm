# Project Status

## Current Work

## M5 T374 S10 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; T374 S9 is accepted and remains the sole compact progress row. |
| Admission And Approval | The owner approved continued T374 functional closure before timing/L3. Accepted S8/S9 establish that Deleted Data needs provider-owned address-mark metadata and that byte-only raw FDD images must not claim it without persistence. |
| Objective | Audit and select the smallest truthful lifecycle and persistence contract by which a VM raw floppy medium could retain per-logical-sector address-mark metadata across save, remove and reinsert, or establish the exact supported-surface removal required if no bounded representation is admissible. |
| Non-goals | No FDC command implementation; no implicit sidecar, hidden host path, flux/index/CRC/GAP/IDAM model, physical timing, ROM/media import, ATA/HDC change, Scan/MT/SK/DTL repair, or Model-339 L3 claim. No external asset is read or modified. |
| Reference Baseline | Accepted T374 S8/S9 evidence; VM `fdd.{h,c}` and `media_save.*`; atomic media-save test; media registry contract; source policy; selected 1.44 MB compatibility configuration. |
| Candidate Proposal | [IBM PC/AT 5170 selected-device functional closure](../proposals/m5-5170-selected-device-functional-closure.md). |
| Files And ABI Surface | Evidence/history/index/status only for this audit; read-only inspection includes VM FDD lifecycle/save path, media save helper, session media binding and relevant tests. A later implementation names any FDD state, owned sidecar/manifest format, lifecycle and test changes. |
| Applicable Rules | `docs/README.md` Task Reading Set; `docs/rules/EXECUTION.md` lifecycle/evidence/similar-issue rules; `docs/rules/DOCUMENT.md`; `CONTRIBUTING.md`; source policy. Any later code change also requires architecture and coding rules. The media contract remains core-owned; persistence and local user-selected file identity remain VM-owned. |
| Verification | Trace every FDD create/replace/insert/remove/finalize/save route and existing atomic-write helper; determine collision, failure-atomicity, read-only, stale metadata and identity behavior; identify every caller and fixture; run unchanged VM media-provider and FDC/Model-339 focused targets, documentation governance and `git diff --check`. |
| Expected Markers | One explicit decision identifies metadata ownership, file identity, format/version, atomic write/rename collision, remove/reinsert, read-only and cleanup behavior; ordinary raw payload remains unchanged and unsupported until an admitted implementation proves the selected representation. |
| Asset Needs | None. No ROM, firmware, guest image, user media, external source or local asset is opened, copied, written or committed. |
| Reporting Requirements | Report the lifecycle matrix, selected or rejected representation, complete consumer/fixture sweep, focused baseline, pushed P1, coordinator acceptance and the exact next implementation receiver. |
| Stop Conditions | Stop and transfer if a truthful format needs flux/physical address-ID modeling, requires committing guest media or host-local paths, cannot use existing atomic persistence safely, or would change the public raw-image contract without explicit owner approval. |
| Exit Criteria | Evidence classifies every raw-FDD persistence/lifecycle route and selects a bounded implementation or explicit support-surface removal; focused baseline and documentation checks pass; no hidden sidecar, raw-byte inference, FDC semantic or L3 claim is introduced. |
| Original Owner Request | Implement each selected machine's functional gaps before timing/L3 closure, keep ROM/media external to Git, and use Git-Bash CMake for builds. |
| Similar-Issue Sweep | Search all FDD image filename, byte replacement, insert/remove/finalize, atomic save and session media binding routes, plus every raw-media fixture, so metadata cannot be persisted, discarded or re-associated through an unreviewed path. |

## Current Technical Baseline

- **Current developer artifact:** T369 S4 `vm-0-5-0369` /
  `build/output/nxvm_0_5_0369.exe`; its SHA-256 and source commit are
  recorded in the T369 S4 closure audit.
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
| T374 S9 | P1 `969c9431`, P2 `74cf5de1` and P3 `74aaad39` add and validate the provider-owned normal/deleted address-mark contract, retain raw FDD/HDD unsupported, and reconcile the full current-gate ledgers. [Contract evidence](../etc/evidence/t374-s9-media-address-mark-contract.md). |
| T373 | Closed at `06246a8e`: S1--S4 freeze the three-machine source-labelled capability ledger and exact functional/timing/current-product receivers. **5170, DeskPro Model 40 and PC/XT 5160-268 L3 are not ready.** The next candidate is 5170 selected-device functional closure. [Closure audit](../etc/evidence/t373-s4-task-closure-audit.md). |
| T372 | Closed at `3f56c72c`: S1--S8 establish that Model 339 has selected logical ownership and deterministic ordering, but lacks selected-device functional closure and source-backed board/phase timing. **5170 model-L3 is not ready.** [Closure audit](../etc/evidence/t372-s8-task-closure-transfer-audit.md). |
| T370 | Closed at `77a73c04`: S1--S5 reconciled all selected Model-339 device-service owners and transferred unavailable duration to phase refinement; **5170 model-L3 is not ready**. [Closure audit](../etc/evidence/t370-s5-planar-cga-transfer-audit.md). |
| T369 | Closed at `08a64bea`: S1--S4 lock and audit the 5170 Model-339 bus-stage input, logical 80286 DMA handoff and selected FDC/PIC visibility. Board waits, physical waveforms, device service, phase refinement and final 5170 L3 remain exact ordered transfers; **5170 model-L3 is not ready**. [Closure audit](../etc/evidence/t369-s4-pcat-bus-stage-closure-audit.md). |
| T368 | Closed at `4da84be8`: completes the source-labelled 80286 successful-retirement CPU ledger, with exact values or declared source-undefined transfers. The next Queue receiver is bus-timed PC/AT operation; CPU waits, bus/device timing and IBM 5170 L3 remain open. [Closure audit](../etc/evidence/t368-s7-80286-retirement-closure-audit.md). |
| T367 | Closed at `f60d87ea`: concrete machine selection and CPU/timing contract binding are VM-owned; the default-PC/AT option path now selects a VM contract before the sole core materialization boundary, while Model 339 remains descriptor-selected. Focused profile/session regressions pass; an unrelated platform-request compile failure blocks a full-gate claim. No CPU-timing, bus, device or L3 receiver is closed. [Binding evidence](../etc/evidence/t367-s2-vm-profile-contract-binding.md). |
| T366 | Closed at `743edc18`: locks the Model 339 baseline, planar-parity NMI and selected topology, and accepts bounded 80286 source-retirement rows. Complete CPU retirement, bus availability, device service timing and profile-L3 closure transfer explicitly; **5170 model-L3 is not ready**. [Closure audit](../etc/evidence/t366-s32-closure-transfer-audit.md). |
| T365 | Closed at `febc9352`: IBM PC/AT parity/I/O-check NMI sources cannot be selected without a profile input, status/latch/clear and lifecycle contract; CPU and CMOS remain delivery/mask only. [Closure audit](../etc/evidence/t365-s2-pcat-nmi-nonadmission-closure-audit.md). Blocks physical/cycle-exact L3 closure; no synthetic source. |

## Recent Governance

- **M5 Td S94 P1:** locks the DeskPro 386 Model 40 and PC/XT 5160-268
  baselines, establishes the capability-ledger then functional-before-timing
  closure sequence for all three machines, and adds their bounded proposals.
  It records the changed owner media identity truthfully, retains external ROMs
  outside Git, and removes closed T367/T368 work from future Queue positions.
  Documentation governance and diff check passed; no runtime, artifact, or task
  activation change.

- **M5 Td S93 P1:** splits T366's transferred 80286 retirement ledger from
  5170 bus work; orders 5170 CPU, bus, device, phase and audit closure; and
  pre-splits DeskPro 386 into profile/CPU, board/device and audit candidates.
  A new pre-Windows L3 admission audit gates every Windows candidate. The
  Roadmap, exact T366 receivers and downstream dependencies agree.
  Documentation governance and diff check passed; no runtime, artifact or task
  activation change.

- **M5 Td S92 P1:** reconciled M5's executable Queue with closed T362--T365
  history; makes T366's profile-lock, selected-NMI-source, and bus-allocation
  order explicit; and adds a pre-Windows closure for every current-product
  device capability. TODO retains only genuinely unplanned, unsupported, or
  80186-only debt. Documentation governance and diff check passed; no runtime,
  artifact, or task-activation change.

- **M5 Td S91 P1:** made the architecture design explicitly retain one shared
  core decode/execution and CPU/DMA transaction lifecycle across machine
  profiles. 8088 owns only its documented external-bus and prefetch/timing
  difference; 80286/80386 retain only their documented architectural and
  board-local differences. Documentation governance and diff check passed; no
  runtime, artifact, task-activation, or queue change.

- **M5 Td S90 P1:** ordered M5 physical L3 closure as IBM PC/AT 5170 80286,
  exact Compaq DeskPro 386 80386, distinct 8088 CPU profile, and IBM 5150/XT
  8088 before the DeskPro-based Windows 3.1 corpus. It retains standalone
  8086/80186 CPU profiles, adds bounded 8088/DeskPro proposals, and changes no
  runtime, artifact, or task activation. Documentation governance and diff
  check passed.

- **M5 Td S89 P1:** made the queued complete instruction-timing proposal the
  sole current explanation of the shared four-profile audit method and the
  distinct 8086/80186/80286/80386 accounting boundaries. It changes neither
  rules, README, Queue ordering, TODO, runtime, nor the documentation
  topology. Documentation governance and diff check passed.

- **M5 Td S90:** made completed PC/AT device-service timing an explicit
  prerequisite of selected-profile model-L3 phase refinement; retained the
  established Queue order and withdrew the unstarted T362 preparation.
  Documentation governance and diff check passed; Td work has no runtime,
  artifact, or task-activation change.

- **M5 Td S89:** converted the remaining primary-source and PC/AT NMI debt
  into three bounded proposals; ordered authority review before the complete
  timing corpus, and device audit before NMI ownership and physical timing.
  Documentation governance and diff check passed; Td work has no runtime,
  artifact, or task-activation change.

- **M5 Td S88:** reconciled the active T359 mechanism plan after S3 acceptance:
  S4 owns the stateful string/repeat/ordinary-I/O timing mechanism, and the
  secondary, privileged, and closure units follow as S5--S7. Documentation
  governance, the T359 inventory verifier, and diff check passed; Td work has
  no runtime or artifact change.
