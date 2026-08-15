# Project Status

## Current Work

## M5 T375 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; T375 S1 is accepted in the compact progress row below. |
| Admission And Approval | Owner: user, continuing the owner-approved pre-Windows Model-339 L3 program and direct instruction to continue work; the T375 Queue candidate was admitted as T375 before S1. Scope is restricted to the primary-source-backed RTC and PIT clock relations allocated by S1. |
| Objective | Replace the Model-339 descriptor's generic RTC and PIT cadence with rational clock-domain relations from its nominal 8 MHz CPU source, and align its RTC second divisor with IBM's selected 32.768-kHz base. Prove descriptor-to-composition propagation and deterministic reset-phase conversion without claiming board availability, host time, or final L3. |
| Non-goals | No CPU wait/prefetch/RAM/ROM/ISA timing, DMA scalar, FDC/KBC/CGA scalar, IRQ/INTA waveform, HLT/external-unavailability clock redesign, device functional repair, raw-IMG sidecar work, ROM/media import, host-time coupling, or final 5170 L3 decision. |
| Reference Baseline | Accepted T375 S1 `6ecfd316` / `4f4fa77f`; frozen Model 339: 8 MHz Type-3, Rev.3 BIOS configuration, 512 KiB, CGA, no fixed disk. IBM PC/AT Technical Reference March 1986, 6280099, System Board 1-22 and 1-57. |
| Candidate Proposal | [IBM PC/AT 5170 board and device phase-timing closure](../proposals/m5-5170-board-phase-timing-closure.md). |
| Files And ABI Surface | `src/vm/profile/default_profile/pc_at_profile.c`; profile/composition smoke and rational-clock test coverage; `docs/etc/evidence/`; `docs/etc/README.md`; this packet. No new public ABI or test-only API. |
| Applicable Rules | `docs/rules/ARCHITECTURE.md`: one profile-to-core timing owner and no parallel device path; evidence: descriptor `clock_plan` remains the sole copied contract. `docs/rules/CODING.md`: bounded named constants and no test-only public surface; evidence: focused existing-owner tests. `docs/rules/DOCUMENT.md`: evidence is supporting only and indexed; evidence: indexed S2 record. `docs/rules/EXECUTION.md`: one bounded packet/P, source-labelled proof, self-review and independent acceptance. |
| Verification | Build and run the focused default-profile, Model-339 composition, and rational-clock smoke targets; prove Model-339 values independently from the generic descriptor; run documentation governance, `git diff --check`, and changed-surface/similar-issue review. Record any unrelated full-build limitation truthfully. |
| Expected Markers | Existing focused test markers plus `M5:T375:S2:MODEL339-CLOCK-CONTRACT:OK` from a new focused contract smoke. |
| Asset Needs | No asset required. Research is documentary only; no firmware, ROM, media, binary, or third-party source may enter Git. |
| Reporting Requirements | Report the source-labelled two-rate decision, code/test/evidence files, pushed P1, focused verification, and retained timing transfers. Report any material conflict before expanding scope. |
| Stop Conditions | Stop and transfer if the profile contract cannot express a rational conversion without a second timing owner, if a source contradicts the selected rates, if a test requires host time or excessive full-machine replay, or if an unresolved functional defect appears. |
| Exit Criteria | Model-339 alone exposes RTC `32768/8000000` (reduced rational form) and PIT `1193182/8000000` (reduced rational form), RTC uses 32768 source ticks per second, reset phase is deterministic, targeted propagation/conversion tests pass, evidence records source and limits, and all non-goals remain transferred. |
| Original Owner Request | Owner-directed continuation: implement the current plan to full L3 precision and stop before Windows 3.1; 86Box/MAME/PCjs are only secondary behavior cross-checks when primary manuals do not determine an instruction timing. |
| Similar-Issue Sweep | Inspect generic/default PC/AT and every clock-plan consumer/test. The generic profile must not inherit Model-339 board facts; DMA/FDC/KBC/VADP remain unchanged and S1-labelled reference-exhausted. Search for duplicated RTC/PIT divisors and parallel timing owners before delivery. |

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
| T375 S1 | Accepted P1 `6ecfd316`: source-labelled Model-339 timing inventory rejects generic descriptor ratios as board facts, selects source-backed RTC/PIT repair, and transfers unsupported DMA/FDC/KBC/CGA scalars without a false L3 claim. [Inventory](../etc/evidence/t375-s1-model339-timing-inventory.md). |
| T374 | Closed at `f742433c`: S1--S19 complete the selected Model-339 functional closure and preserve raw-IMG 765 Deleted/Control-Mark/Scan as explicit TODO debt. Board/device timing, final Model-L3, DeskPro 386, PC/XT and Windows 3.1 remain open. [Closure audit](../etc/evidence/t374-s19-task-closure-audit.md). |
| T373 | Closed at `06246a8e`: S1--S4 freeze the three-machine source-labelled capability ledger and exact functional/timing/current-product receivers. **5170, DeskPro Model 40 and PC/XT 5160-268 L3 are not ready.** The next candidate is 5170 selected-device functional closure. [Closure audit](../etc/evidence/t373-s4-task-closure-audit.md). |
| T372 | Closed at `3f56c72c`: S1--S8 establish that Model 339 has selected logical ownership and deterministic ordering, but lacks selected-device functional closure and source-backed board/phase timing. **5170 model-L3 is not ready.** [Closure audit](../etc/evidence/t372-s8-task-closure-transfer-audit.md). |
| T370 | Closed at `77a73c04`: S1--S5 reconciled all selected Model-339 device-service owners and transferred unavailable duration to phase refinement; **5170 model-L3 is not ready**. [Closure audit](../etc/evidence/t370-s5-planar-cga-transfer-audit.md). |
| T369 | Closed at `08a64bea`: S1--S4 lock and audit the 5170 Model-339 bus-stage input, logical 80286 DMA handoff and selected FDC/PIC visibility. Board waits, physical waveforms, device service, phase refinement and final 5170 L3 remain exact ordered transfers; **5170 model-L3 is not ready**. [Closure audit](../etc/evidence/t369-s4-pcat-bus-stage-closure-audit.md). |
| T368 | Closed at `4da84be8`: completes the source-labelled 80286 successful-retirement CPU ledger, with exact values or declared source-undefined transfers. The next Queue receiver is bus-timed PC/AT operation; CPU waits, bus/device timing and IBM 5170 L3 remain open. [Closure audit](../etc/evidence/t368-s7-80286-retirement-closure-audit.md). |
| T367 | Closed at `f60d87ea`: concrete machine selection and CPU/timing contract binding are VM-owned; the default-PC/AT option path now selects a VM contract before the sole core materialization boundary, while Model 339 remains descriptor-selected. Focused profile/session regressions pass; an unrelated platform-request compile failure blocks a full-gate claim. No CPU-timing, bus, device or L3 receiver is closed. [Binding evidence](../etc/evidence/t367-s2-vm-profile-contract-binding.md). |
| T366 | Closed at `743edc18`: locks the Model 339 baseline, planar-parity NMI and selected topology, and accepts bounded 80286 source-retirement rows. Complete CPU retirement, bus availability, device service timing and profile-L3 closure transfer explicitly; **5170 model-L3 is not ready**. [Closure audit](../etc/evidence/t366-s32-closure-transfer-audit.md). |

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
