# Project Status

## Current Work

**Active.** M5 T359 S1 inventories every implemented 8086, 80186, 80286, and
80386 instruction form before allocating complete timing mechanisms.

## M5 T359 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New |
| Admission And Approval | Owner approved ordered single-agent execution of the M5 Queue through L3 closure on 2026-08-14. T358 closed at `2a2008e3`; this admits the next Queue candidate's inventory-only first S. |
| Objective | Produce one complete four-profile implementation inventory mapping every primary and `0F` form, profile gate, applicable mode/prefix/addressing class, handler owner, and current timing disposition to an Intel primary source row, dynamic formula, unsupported/rejected classification, or explicit source/implementation receiver. Allocate only coherent shared timing mechanisms for later S units. |
| Non-goals | No isolated clock edit, timing-value change, CPU semantic repair, source import, bus wait/HOLD/DMA/prefetch/cache/pin timing, device service latency, x87 execution, VME/PVI, or Windows claim. S1 creates no implementation task merely because a handler exists. |
| Reference Baseline | `2a2008e3` / current task artifact `vm-0-5-0359`. |
| Candidate Proposal | [Complete instruction-timing corpus](../proposals/m5-complete-instruction-timing-corpus.md) and [Queue](QUEUE.md). |
| Files And ABI Surface | CPU metadata/dispatch and timing publisher inspection, existing timing ledgers/tests, CMake inventory verifier and current-artifact preset identity, new inventory evidence, task history/status, Queue/TODO transfer records, and current artifact only. No production, provider, device, or public ABI change in S1. |
| Applicable Rules | Task Reading Set, execution, architecture, coding, documentation, and source/research policy; one successful-retirement elapsed-tick publisher, source provenance before timing assertion, mechanism-level allocation, and explicit physical-timing transfer. |
| Verification | Mechanically enumerate implementation dispatch/metadata and current timing calls for primary and `0F` spaces; reconcile profile thresholds, 66/67/F0/F2/F3 and real/protected/VM86 dispositions against retained T357 evidence and Intel source authorities; verify every row has exactly one classification and every missing source/implementation receiver has a Queue/TODO destination. Run inventory consistency checks, retained timing owners, governance, artifact verifier/hash, and diff checks. |
| Expected Markers | Retained T357 timing-preview and four-profile ledger markers, an S1 inventory consistency marker, and rebuilt `vm-0-5-0359` artifact. |
| Asset Needs | Intel 8086/80186/80286/80386 primary timing manuals and project-owned metadata/tests only; no guest media, firmware, third-party source import, or copied emulator code. |
| Reporting Requirements | One complete P1 only: complete inventory and source map, mechanism allocation or transfer table, duplicate/missing classification sweep, consistency proof, retained markers, full required gates, artifact hash, commit, and push. Do not report a subset opcode table or a single timing probe as S1 delivery. |
| Stop Conditions | Stop and request source/owner direction if a primary timing table is unavailable or contradictory without an approved precedence rule, an implementation table cannot be mechanically enumerated, a row requires a CPU semantic change, or a value depends on physical bus/device timing. Record the exact forms and transfer; do not infer clocks. |
| Exit Criteria | Every currently implemented instruction form and relevant selected classifier variant has one auditable timing disposition; no unclassified row is concealed by a broad family label; later implementation S units are ordered by shared timing mechanism and dependency; all non-CPU timing is explicitly transferred. |
| Original Owner Request | Execute the current Queue until a comprehensive L3 timing closure audit, with holistic mechanism planning rather than patch-on-patch clock additions. |
| Similar-Issue Sweep | Search all dispatch tables, metadata, timing publishers, profile gates, prefix classifiers, timing previews, current timing tests, T357 ledgers, Queue, and TODO. Classify every hit as current source-backed evidence, allocated mechanism, unsupported/profile rejection, source gap, or physical-timing transfer. |

## Current Technical Baseline

- **Current developer artifact:** T359 S1 `vm-0-5-0359` /
  `build/output/nxvm_0_5_0359.exe` as
  `A9F0D342753C7F3BDC2C0492E08E103F3A0D8C7BB651ACE5ABAD5D75F0C39134`.
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
| T358 | Closed cross-mode mechanism coherence: reconciled I/O permission, exception/IRQ entry, selector/task/return transition, paging/linear access, and prefix/LOCK/width/string classifiers; repaired 80286 delivery, LOCK group legality, and repeated-width preview divergence; passed 240 current-gate tests. [Closure audit](../etc/evidence/t358-cross-mode-mechanism-closure-audit.md). |
| T357 | Closed finite source-backed 8086/80186/80286/80386 instruction timing at one successful-retirement publisher: selected form ledgers, 80386 protected/VM86 ordinary I/O, Appendix-B `NOP = 3` consumer reconciliation, 240 current-gate tests, and artifact SHA-256 `83A249015C03CF875896A440D5B43A430C2D875A63F0BE4B9709014A753CAD95`. Unselected forms, waits, physical bus/prefetch, device latency and cycle-exact work remain explicit transfers. [Closure audit](../etc/evidence/t357-s8-cross-profile-closure-audit.md). |
| T356 | Closed the M5 closure audit with an explicit M5-open decision: all selected device owners are evidenced, but instruction timing, bus availability/wait states and selected-profile cycle exactness remain ordered implementation prerequisites; corpus-gated peripheral, storage and product boundaries remain explicit transfers. [History](../history/M5-T356-m5-closure-audit.md). |
| T355 | Closed the bounded Windows 3.x readiness map: an opt-in BYOB HDD/INT13 checkpoint passed, all four probes remain host-observation/non-current, T347/T354 storage/L3 is a verified prerequisite, and all display, input, NMI, physical-timing, Setup, and compatibility breadth remains explicitly transferred. [History](../history/M5-T355-windows-3x-readiness-map.md). |
| T354 | Closed selected L3 bus timing: core-local checked CPU/DMA transaction begin/commit/cancel trace, deterministic CPU-retire then DMA/PIT/PIC then FDC/HDC ordering, reset cancellation/trace continuity, and retained firmware/DOS consumers. Exact bus cycles, pin waveforms, physical wait tables, host time, and Windows readiness remain explicit transfers. 234 current-gate tests passed. [History](../history/M5-T354-physical-l3-bus-timing-convergence.md). |
| T353 | Closed selected PC/AT topology: the default profile's 75 exact directional leaves and five named routes validate before core publication, remain exact through session reset and deterministic-L3 rearm, and have firmware-visible reset proof. Optional PPI/speaker, NMI, and interface breadth remain corpus-gated TODO transfers; physical bus timing transfers to the next L3 package. 231 current-gate tests passed. [History](../history/M5-T353-pcat-port-topology-and-peripherals.md). |
| T352 | Closed selected digital CGA/EGA: one VADP owner for registers, apertures, raster, planar state, reset, and capture; deterministic VADP-after-KBC timing and copied session/mailbox composition; its recorded developer artifact and SHA-256 remain in task history; 229 current-gate tests passed. [History](../history/M5-T352-vadp-digital-video-completeness.md). |
| T351 | Closed selected IBM-PC/AT 8042 controller, AT keyboard, and compatible three-byte AUX at deterministic L3: one FIFO, controller/keyboard/AUX head promotion, IRQ1/IRQ12 source lifecycle, delayed/reset/finalize, copied ingress, and exact advanced-AUX/NMI/port-61 transfers. 228 current-gate tests passed. [History](../history/M5-T351-kbc-aux-completeness.md). |

## Recent Governance

- **M5 Td S87:** reconciled the T358 continuation handoff: removed the
  duplicate empty history heading, retained the approved Queue order, and
  restored compact open-T358 status. Documentation governance and diff check
  passed; Td work has no runtime or artifact change.

- **M5 Td S87:** reordered M5 around four-profile timing authority,
  device/chip/port/bus completeness, bus availability, service timing,
  cycle-exact selected-profile work, and L3 closure before final BYOB Windows
  3.1 Standard/386 Enhanced lifecycle proof. M6 now follows the M5 closure
  decision. T358 S1 was isolated during this Td delivery and is now restored
  as the sole active packet. Documentation governance and diff check passed;
  Td work has no runtime change.

- **M5 Td S86:** replaced the completed instruction-timing TODO with ordered
  candidates for the remaining full instruction corpus and the independent
  80286 Appendix-B source reconciliation; retained bus-timed and cycle-exact
  work in their dependency order. Documentation governance and diff check
  passed; Td work has no runtime change.

- **M5 Td S85:** corrected the Windows readiness candidate so FDC/ATA
  pending/readiness service is an accepted regression baseline, rather than a
  stale missing-feature blocker. Physical device timing and Windows
  compatibility remain unclaimed. Documentation governance and diff check
  passed; Td work has no runtime change.

- **M5 Td S84:** retired the stale FDC/ATA command-service TODO after the
  accepted T347 pending-command/readiness-service closure and T354 consumer
  audit reconfirmed its claim was no longer true. Physical wait/cycle fidelity
  remains in the existing bus-timed and cycle-exact debt entries. Documentation
  governance and diff check passed; Td work has no runtime change.

- **M5 Td S83:** corrected the NXVM self-sibling instruction in `AGENTS.md`;
  renamed the VM-platform injected test macro to
  `VM_PLATFORM_TEST_FAILURE_STAGE` across CMake, implementation, and smoke
  sources; and aligned CMake's project identity and diagnostics with NXVM.
  Documentation self-test, default governance check, exact macro/search audit,
  and diff check passed. Td work has no intended runtime or artifact behavior
  change.

- **M5 Td S82:** restored all 19 historical NXVM README screenshots as static
  documentation assets; rebuilt the public README around the current CMake
  path and truthful NXVM-first, future-NXVDM boundary; and aligned the current
  design/rule wording and README schema self-test. Documentation self-test,
  default governance check, README-link check, and diff check passed. Td work
  has no runtime or artifact change.

- **M5 Td S81:** withdrew the duplicate 80286 `LOCK` candidate after the
  T339 closure audit reconfirmed that accepted T328 already owns the complete
  pre-386 prefix-policy matrix. The Queue now advances directly from T339 to
  the 80386DX form closure; current proposals and T339 transfer ledgers name
  T328, T341, and T342 truthfully. Documentation governance and diff check
  passed. Td work has no runtime or artifact change.
