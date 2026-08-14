# Project Status

## Current Work

**Active.** M5 T358 S4 audits paging and linear-memory access from translation
validation through `#PF` delivery and access publication.

## M5 T358 S4 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approved execution of the ordered Queue through L3 closure on 2026-08-14. Single-agent continuation follows accepted T358 S1--S3 and Td S87 handoff. |
| Objective | Audit the 80386 paging and linear-memory mechanism as one decision-to-publication route: page walk, PDE/PTE permission and A/D updates, CR2 and `#PF` producer/delivery state, cross-page transaction cancellation, and every adopted real/protected/VM86 caller. Repair only reproduced accidental construction divergence. Also make the default current-gate reproducible by isolating the independently reproduced host-thread timer smoke scheduling failure without changing guest timer behavior. |
| Non-goals | PAE, large pages, persistent TLB or `INVLPG` behavior, paging-policy expansion, VME/PVI, debugger raw-control mutation, physical wait/bus timing, device timing, host time, numerical x87, or Windows compatibility. The added CTest isolation may not alter timer logic, guest ticks, device ownership, or test assertions. |
| Reference Baseline | `1d5655b1` / current task artifact `vm-0-5-0358`. |
| Candidate Proposal | [Cross-mode mechanism coherence](../proposals/m5-cross-mode-mechanism-coherence.md), retained [T311 paging boundary](../history/M5-T311-paging-boundary-family.md), and current [Queue](QUEUE.md). |
| Files And ABI Surface | Existing private CPU translation, logical memory, execution-finalization, and adopted instruction callers; CMake CTest scheduling metadata for the existing timer firmware smoke; owner paging evidence, task history/status, and current artifact only. No public CPU, memory, pager, debugger, provider, or device ABI. |
| Applicable Rules | Task Reading Set, execution, architecture, coding, documentation, and source/research policy; one checked translation/transaction owner, validation before A/D or guest-data publication, and explicit Intel-required mode/permission differences. |
| Verification | Enumerate translation/page-walk, logical read/write, preview, instruction fetch, data/stack/string, and exception-finalizer callers. For each classify permissions, A/D and CR2 mutation, fault delivery, and commit/cancellation. Reproduce any divergence with focused variants; run the owner paging marker, retained delivery/VM86 markers, the timer smoke alone and repeated default parallel current gates, governance, artifact rebuild/hash, and diff checks. |
| Expected Markers | Retained `M5:T258:S2:I386-PAGING:OK`, `M5:T311:S3:PAGING-PERMISSIONS:OK`, `M5:T311:S4:CROSS-PAGE:OK`, `M5:T320:S1:VM86-DELIVERY:OK`, and rebuilt `vm-0-5-0358` artifact. |
| Asset Needs | Intel 80386 paging and page-fault authority plus project-owned synthetic page tables, GDT/IDT/TSS fixtures only; no guest media, firmware, or external-source import. |
| Reporting Requirements | One complete P1 only: caller/write ledger, reproduced outcomes, every in-scope repair and similar-issue sweep, focused and retained markers, full gate, artifact hash, commit, and push. Do not report a single permission bit, fault vector, or page-crossing probe as delivery. |
| Stop Conditions | Stop and split if the correction needs TLB/INVLPG or PAE/large-page semantics, a public paging/debugger/provider interface, physical bus/device timing, VME/PVI policy, a timer-runtime behavioral change, or a new paging-policy decision rather than a shared-mechanism inconsistency; record exact callers and transfer. |
| Exit Criteria | Every adopted translation and linear-memory caller has one classified validation-to-publication contract; every reproduced accidental divergence is repaired and swept; Intel-required permission/mode distinctions are retained; default parallel current-gate scheduling is reproducible without changing timer runtime behavior; all gates pass and excluded paging state-machine work remains transferred. |
| Original Owner Request | Continue the ordered Queue holistically until L3 closure audit, avoiding symptom-patch tactics. |
| Similar-Issue Sweep | Search tracked production translation, logical-memory, `#PF`/CR2, A/D, preview, and execution-finalizer paths plus their current-gate owners; inspect all current host-thread firmware smokes for an equivalent CTest scheduling contract; classify every hit as repaired, covered common owner, Intel-required difference, or explicit Queue/TODO transfer before closure. |

## Current Technical Baseline

- **Current developer artifact:** T358 S2 active `vm-0-5-0358` /
  `build/output/nxvm_0_5_0358.exe` as
  `59FAC0E73FED23BA47F3F25C1946C8D2949CFB186C480CE9841588E47E754E2E`.
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
| T358 S2 | Accepted I/O-permission and exception/IRQ-entry coherence across real, protected, and VM86 paths; repaired 80286 protected `#SS/#TS` vector classification, proved 16-bit error frames, retained distinct frame layouts, passed 240 current-gate tests, and rebuilt `vm-0-5-0358`. [History](../history/M5-T358-cross-mode-mechanism-coherence.md) |
| T357 | Closed finite source-backed 8086/80186/80286/80386 instruction timing at one successful-retirement publisher: selected form ledgers, 80386 protected/VM86 ordinary I/O, Appendix-B `NOP = 3` consumer reconciliation, 240 current-gate tests, and artifact SHA-256 `83A249015C03CF875896A440D5B43A430C2D875A63F0BE4B9709014A753CAD95`. Unselected forms, waits, physical bus/prefetch, device latency and cycle-exact work remain explicit transfers. [Closure audit](../etc/evidence/t357-s8-cross-profile-closure-audit.md). |
| T356 | Closed the M5 closure audit with an explicit M5-open decision: all selected device owners are evidenced, but instruction timing, bus availability/wait states and selected-profile cycle exactness remain ordered implementation prerequisites; corpus-gated peripheral, storage and product boundaries remain explicit transfers. [History](../history/M5-T356-m5-closure-audit.md). |
| T355 | Closed the bounded Windows 3.x readiness map: an opt-in BYOB HDD/INT13 checkpoint passed, all four probes remain host-observation/non-current, T347/T354 storage/L3 is a verified prerequisite, and all display, input, NMI, physical-timing, Setup, and compatibility breadth remains explicitly transferred. [History](../history/M5-T355-windows-3x-readiness-map.md). |
| T354 | Closed selected L3 bus timing: core-local checked CPU/DMA transaction begin/commit/cancel trace, deterministic CPU-retire then DMA/PIT/PIC then FDC/HDC ordering, reset cancellation/trace continuity, and retained firmware/DOS consumers. Exact bus cycles, pin waveforms, physical wait tables, host time, and Windows readiness remain explicit transfers. 234 current-gate tests passed. [History](../history/M5-T354-physical-l3-bus-timing-convergence.md). |
| T353 | Closed selected PC/AT topology: the default profile's 75 exact directional leaves and five named routes validate before core publication, remain exact through session reset and deterministic-L3 rearm, and have firmware-visible reset proof. Optional PPI/speaker, NMI, and interface breadth remain corpus-gated TODO transfers; physical bus timing transfers to the next L3 package. 231 current-gate tests passed. [History](../history/M5-T353-pcat-port-topology-and-peripherals.md). |
| T352 | Closed selected digital CGA/EGA: one VADP owner for registers, apertures, raster, planar state, reset, and capture; deterministic VADP-after-KBC timing and copied session/mailbox composition; its recorded developer artifact and SHA-256 remain in task history; 229 current-gate tests passed. [History](../history/M5-T352-vadp-digital-video-completeness.md). |
| T351 | Closed selected IBM-PC/AT 8042 controller, AT keyboard, and compatible three-byte AUX at deterministic L3: one FIFO, controller/keyboard/AUX head promotion, IRQ1/IRQ12 source lifecycle, delayed/reset/finalize, copied ingress, and exact advanced-AUX/NMI/port-61 transfers. 228 current-gate tests passed. [History](../history/M5-T351-kbc-aux-completeness.md). |
| T350 | Closed the selected PC/AT signal graph: retained 8254/IRQ0 and MC146818/CMOS/IRQ8 owners now have focused lifecycle proof; RTC alarm and IRQF defects are repaired; speaker/PPI and parity/I/O-channel NMI remain exact corpus/contract-gated TODO transfers. 228 current-gate tests passed. [History](../history/M5-T350-pcat-platform-signals.md). |

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
