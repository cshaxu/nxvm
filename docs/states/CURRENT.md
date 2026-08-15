# Project Status

## Current Work

## M5 T373 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; T373 remains the latest open numeric task and S2 follows accepted S1. |
| Admission And Approval | The owner approved continued single-agent L3 work and locked IBM 5170 Model 339/Type 3, 1986 original Compaq DeskPro 386 Model 40, and IBM PC/XT 5160-268 as the three baseline identities. |
| Objective | Qualify primary machine documentation and produce source-labelled bills of materials and unknown-field ledgers for all three locked baseline identities. |
| Non-goals | No machine/profile code, device implementation, timing scalar, ROM/media import, clone substitution, generic PC inference, reference-emulator measurement, or L3 claim. |
| Reference Baseline | T373 S1 support inventory; T372 S8 closure matrix; [capability-ledger proposal](../proposals/m5-baseline-device-capability-ledger.md); IBM Model-339 evidence; owner-locked DeskPro Model 40 and PC/XT 5160-268 identities. |
| Candidate Proposal | [Baseline-machine and supported-device capability ledger](../proposals/m5-baseline-device-capability-ledger.md). |
| Files And ABI Surface | New T373 S2 primary-source/BOM evidence, history progress, `docs/etc/README.md`, and `docs/states/CURRENT.md` only; no source, ABI, profile, build, ROM, or guest-media surface. |
| Applicable Rules | `docs/README.md` Task Reading Set; `docs/rules/EXECUTION.md` continuation, research/evidence, P lifecycle and closure rules; `docs/rules/DOCUMENT.md`; `CONTRIBUTING.md`; `docs/etc/operations/policy/source-policy.md`; architecture/coding rules are not applicable because S2 changes no production source or build. |
| Verification | Use primary IBM/Compaq materials where available; record title, edition/date, stable location, exact supported fact and non-fact for each BOM field; cross-check every stated profile field against S1 source support; run documentation governance and `git diff --check`. |
| Expected Markers | One indexed three-machine BOM/unknown-field matrix names only documented facts, marks every unresolved field as unknown, and distinguishes Model-339's aftermarket 1.44 MB drive from factory configuration. |
| Asset Needs | Read-only public documentation only. Required BIOS ROMs remain external owner-managed assets; no download, path, byte, hash, media, or executable reference runtime enters Git. |
| Reporting Requirements | Report initial source-boundary confirmation, primary-source qualification, every source-exhausted field, focused verification, pushed P1, and the next functional-classification receiver. |
| Stop Conditions | Stop and transfer when no primary source identifies a field, a source conflicts with the locked identity, a fact requires a ROM/media run, or a candidate is only a clone/emulator observation; do not fill an unknown with a default-PC/AT assumption. |
| Exit Criteria | An indexed BOM/unknown-field ledger covers all three machines and maps each documented device to S1 support status or an absent/unknown receiver; source-exhausted fields are explicit; documentation checks pass. |
| Original Owner Request | Continue the ordered L3 program with DeskPro Model 40 and PC/XT 5160-268 as fixed baselines, using ROMs only externally and never entering Windows work. |
| Similar-Issue Sweep | Research/audit-only: sweep profile labels, CPU/clock, board/revision, memory, ROM, display, input, storage, DMA/PIC/PIT/RTC/NMI/reset facts and current-source assumptions for all three machines; every unsupported fact becomes unknown rather than inferred. |

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
| T373 S1 | P1 `a3cc3c5f` inventories the actual default-PC/AT and Model-339 descriptors, product-visible device paths, absent peripheral families, and exact machine/product ledger receivers. No profile, device, timing, or L3 claim changes. [Support inventory](../etc/evidence/t373-s1-current-support-inventory.md). |
| T372 | Closed at `3f56c72c`: S1--S8 establish that Model 339 has selected logical ownership and deterministic ordering, but lacks selected-device functional closure and source-backed board/phase timing. **5170 model-L3 is not ready.** [Closure audit](../etc/evidence/t372-s8-task-closure-transfer-audit.md). |
| T370 | Closed at `77a73c04`: S1--S5 reconciled all selected Model-339 device-service owners and transferred unavailable duration to phase refinement; **5170 model-L3 is not ready**. [Closure audit](../etc/evidence/t370-s5-planar-cga-transfer-audit.md). |
| T369 | Closed at `08a64bea`: S1--S4 lock and audit the 5170 Model-339 bus-stage input, logical 80286 DMA handoff and selected FDC/PIC visibility. Board waits, physical waveforms, device service, phase refinement and final 5170 L3 remain exact ordered transfers; **5170 model-L3 is not ready**. [Closure audit](../etc/evidence/t369-s4-pcat-bus-stage-closure-audit.md). |
| T368 | Closed at `4da84be8`: completes the source-labelled 80286 successful-retirement CPU ledger, with exact values or declared source-undefined transfers. The next Queue receiver is bus-timed PC/AT operation; CPU waits, bus/device timing and IBM 5170 L3 remain open. [Closure audit](../etc/evidence/t368-s7-80286-retirement-closure-audit.md). |
| T367 | Closed at `f60d87ea`: concrete machine selection and CPU/timing contract binding are VM-owned; the default-PC/AT option path now selects a VM contract before the sole core materialization boundary, while Model 339 remains descriptor-selected. Focused profile/session regressions pass; an unrelated platform-request compile failure blocks a full-gate claim. No CPU-timing, bus, device or L3 receiver is closed. [Binding evidence](../etc/evidence/t367-s2-vm-profile-contract-binding.md). |
| T366 | Closed at `743edc18`: locks the Model 339 baseline, planar-parity NMI and selected topology, and accepts bounded 80286 source-retirement rows. Complete CPU retirement, bus availability, device service timing and profile-L3 closure transfer explicitly; **5170 model-L3 is not ready**. [Closure audit](../etc/evidence/t366-s32-closure-transfer-audit.md). |
| T365 | Closed at `febc9352`: IBM PC/AT parity/I/O-check NMI sources cannot be selected without a profile input, status/latch/clear and lifecycle contract; CPU and CMOS remain delivery/mask only. [Closure audit](../etc/evidence/t365-s2-pcat-nmi-nonadmission-closure-audit.md). Blocks physical/cycle-exact L3 closure; no synthetic source. |
| T364 | Closed at `7d574ae3`: all selected PC/AT components, ports/routes and lifecycle/timing owners are inventoried; optional and physical gaps retain exact Queue/TODO receivers. [Closure audit](../etc/evidence/t364-s2-pcat-device-completeness-closure-audit.md). No model-L3 claim. |

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
