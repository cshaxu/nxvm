# Project Status

## Current Work

**Active.** M5 T358 S2 audits and reconciles the shared exception and IRQ
entry mechanism across real, protected, and VM86 execution.

## M5 T358 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approved execution of the ordered Queue through L3 closure on 2026-08-14. Single-agent execution follows accepted T358 S1. |
| Objective | Build one source-to-commit ledger for every currently delivered synchronous exception and hardware IRQ path from producer/classification through `ExecFinal`, serializer, frame/stack preflight, state publication, PIC acknowledgment, and diagnostic boundary; repair every reproduced accidental divergence within this shared entry mechanism. |
| Non-goals | VME/PVI, 486+ extensions, numerical x87 behavior, task-gate/task-switch redesign, IRET return semantics, paging algorithm redesign, NMI/PIC policy redesign, device-side IRQ production, or physical interrupt/bus timing. |
| Reference Baseline | `dce4b9fe` / current task artifact `vm-0-5-0358`. |
| Candidate Proposal | [Cross-mode mechanism coherence](../proposals/m5-cross-mode-mechanism-coherence.md). |
| Files And ABI Surface | Existing exception/IRQ execution-finalizer and serializer owners, every direct producer/caller, one owner-bound smoke and evidence ledger if existing evidence is insufficient, CMake current-gate/artifact wiring, and task history/status; no public CPU, PIC, provider, or TSS ABI. |
| Applicable Rules | Task Reading Set, execution, architecture, coding, documentation, and source/research policy; preserve genuine 16-/32-bit frame/TSS layouts and maintain one validation-to-publication owner. |
| Verification | Mechanically enumerate delivered exception masks and all `ExecInt` callers; prove real, protected same-CPL, protected outer-CPL, and VM86 applicable frame/restart/IF/TF/segment/stack outcomes, synchronous-fault priority over pending IRQ, and PIC acknowledge only after successful entry. Include failure preflight/nonpublication and retained full current gate, governance, artifact hash, and diff checks. |
| Expected Markers | `M5:T358:S2:EXCEPTION-IRQ:OK`; full current-gate and rebuilt `vm-0-5-0358` artifact. |
| Asset Needs | Intel 80386 exception/interrupt authority and project-owned synthetic IDT/TSS/PIC fixtures only; no guest media, firmware, or external-source import. |
| Reporting Requirements | One complete P1 only: producer/caller/frame ledger, exact reproduced outcomes, every shared repair and similar-issue sweep, focused marker, full gate, artifact hash, commit, and push. Do not report an opcode-local or one-vector result as delivery. |
| Stop Conditions | Stop and split if closure requires new PIC/NMI/device policy, task switching, inverse IRET, paging/TLB semantics, a public ABI, or a genuine Intel layout difference outside the entry owner; record the exact transfer and affected callers. |
| Exit Criteria | All currently supported entry producers and hardware IRQ callers have one classified delivery contract with mode-appropriate frame and commit evidence; any accidental shared-owner divergence is repaired and swept; all required gates pass; excluded system mechanisms remain explicit transfers. |
| Original Owner Request | Continue the ordered Queue holistically until L3 closure audit, avoiding symptom-patch tactics. |
| Similar-Issue Sweep | Enumerate `ExecFinal` exception-mask branches, `_e_except_n`/`_e_intr_n` callers, real/protected serializers, and every PIC/NMI invocation before changing code; classify each as fixed, covered, Intel-required distinct layout, or explicit transfer. |

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
| T358 S1 | Accepted one shared I/O-permission decision boundary for ordinary and string I/O across real, protected, TSS-map, and VM86 outcomes; repaired legacy word-`OUT` checked propagation, expanded caller proof, passed 240 current-gate tests, and recorded the `vm-0-5-0358` artifact. [History](../history/M5-T358-cross-mode-mechanism-coherence.md) |
| T357 | Closed finite source-backed 8086/80186/80286/80386 instruction timing at one successful-retirement publisher: selected form ledgers, 80386 protected/VM86 ordinary I/O, Appendix-B `NOP = 3` consumer reconciliation, 240 current-gate tests, and artifact SHA-256 `83A249015C03CF875896A440D5B43A430C2D875A63F0BE4B9709014A753CAD95`. Unselected forms, waits, physical bus/prefetch, device latency and cycle-exact work remain explicit transfers. [Closure audit](../etc/evidence/t357-s8-cross-profile-closure-audit.md). |
| T356 | Closed the M5 closure audit with an explicit M5-open decision: all selected device owners are evidenced, but instruction timing, bus availability/wait states and selected-profile cycle exactness remain ordered implementation prerequisites; corpus-gated peripheral, storage and product boundaries remain explicit transfers. [History](../history/M5-T356-m5-closure-audit.md). |
| T355 | Closed the bounded Windows 3.x readiness map: an opt-in BYOB HDD/INT13 checkpoint passed, all four probes remain host-observation/non-current, T347/T354 storage/L3 is a verified prerequisite, and all display, input, NMI, physical-timing, Setup, and compatibility breadth remains explicitly transferred. [History](../history/M5-T355-windows-3x-readiness-map.md). |
| T354 | Closed selected L3 bus timing: core-local checked CPU/DMA transaction begin/commit/cancel trace, deterministic CPU-retire then DMA/PIT/PIC then FDC/HDC ordering, reset cancellation/trace continuity, and retained firmware/DOS consumers. Exact bus cycles, pin waveforms, physical wait tables, host time, and Windows readiness remain explicit transfers. 234 current-gate tests passed. [History](../history/M5-T354-physical-l3-bus-timing-convergence.md). |
| T353 | Closed selected PC/AT topology: the default profile's 75 exact directional leaves and five named routes validate before core publication, remain exact through session reset and deterministic-L3 rearm, and have firmware-visible reset proof. Optional PPI/speaker, NMI, and interface breadth remain corpus-gated TODO transfers; physical bus timing transfers to the next L3 package. 231 current-gate tests passed. [History](../history/M5-T353-pcat-port-topology-and-peripherals.md). |
| T352 | Closed selected digital CGA/EGA: one VADP owner for registers, apertures, raster, planar state, reset, and capture; deterministic VADP-after-KBC timing and copied session/mailbox composition; its recorded developer artifact and SHA-256 remain in task history; 229 current-gate tests passed. [History](../history/M5-T352-vadp-digital-video-completeness.md). |
| T351 | Closed selected IBM-PC/AT 8042 controller, AT keyboard, and compatible three-byte AUX at deterministic L3: one FIFO, controller/keyboard/AUX head promotion, IRQ1/IRQ12 source lifecycle, delayed/reset/finalize, copied ingress, and exact advanced-AUX/NMI/port-61 transfers. 228 current-gate tests passed. [History](../history/M5-T351-kbc-aux-completeness.md). |
| T350 | Closed the selected PC/AT signal graph: retained 8254/IRQ0 and MC146818/CMOS/IRQ8 owners now have focused lifecycle proof; RTC alarm and IRQF defects are repaired; speaker/PPI and parity/I/O-channel NMI remain exact corpus/contract-gated TODO transfers. 228 current-gate tests passed. [History](../history/M5-T350-pcat-platform-signals.md). |

## Recent Governance

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

- **M5 Td S80:** reordered the four-profile CPU-completeness Queue into
  dependency-bounded 8086/80186, 80286 descriptor-transfer, 80286 `LOCK`,
  80386DX form, 80386DX state, 80386DX audit, and cross-profile closure
  candidates; added the corresponding unnumbered proposals. Documentation
  governance and diff check passed. Td work has no runtime or artifact change.
