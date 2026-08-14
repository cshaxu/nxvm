# Project Status

## Current Work

**Active.** M5 T361 S1 legacy dynamic arithmetic primary-source audit is
admitted in single-session mode.

## M5 T361 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New |
| Admission And Approval | Owner approved the next Queue candidate after Td S89 `c2ae7b49`: begin the legacy dynamic arithmetic timing-authority task in single-session mode. |
| Objective | Audit the primary Intel record for every T360-transferred 8086/80186 range-only arithmetic form and decide whether a complete operand-to-clock formula exists; if none exists, specify the exact owner decision and observation-contract requirements needed before any timing allocation. |
| Non-goals | No runtime clock allocation, endpoint/midpoint guess, later-profile borrowing, host benchmark, Bochs/PCjs authority, hardware observation run, instruction-semantics change, bus/device/delivery timing, global timing refactor, or ABI change. |
| Reference Baseline | `c2ae7b49`, T360 S3/S5 evidence, T359 S2 arithmetic ledger, 8086 Tables 2-20/2-21 and 80186 Table 1-16 primary editions recorded by T360 S1. |
| Candidate Proposal | [Legacy dynamic arithmetic timing authority](../proposals/m5-legacy-dynamic-arithmetic-timing-authority.md), bounded S1 source-formula audit. |
| Files And ABI Surface | Evidence/history/CURRENT and TODO only if a precise admission condition needs correction. No `src/`, CMake, test, artifact, public API, or runtime timing change. |
| Applicable Rules | Task Reading Set; source policy; architecture one-publisher rule; `EXECUTION.md` mechanism/evidence/transfer/similar-issue/closure rules; `DOCUMENT.md` topology rule. |
| Verification | Build a form-by-form 8086/80186 source ledger for `F6`/`F7` `/4`--`/7` and 80186 `69`/`6B`; inspect all source timing consumers and retained timing smokes; search primary manual/provenance records for an operand formula; prove the present fallback remains unallocated; run focused 8086/80186 timing smokes, documentation governance, diff check, and full current gate. |
| Expected Markers | Indexed S1 authority ledger; each range form marked formula-complete or no-formula; no numeric runtime delta; one exact observation-contract decision point if primary authority remains incomplete. |
| Asset Needs | Existing primary manual scans and committed evidence only; no imported source, firmware, guest media, hardware probe, host measurement, or external executable. |
| Reporting Requirements | Record form/source wording, formula applicability, all consumer dispositions, explicit no-formula evidence if applicable, the required owner choice, and no-L3/cycle-exact boundary. |
| Stop Conditions | Stop before any allocation if the primary record lacks a complete formula, if a formula requires an unapproved processor stepping/measurement assumption, if a hidden numeric allocation is found, or if an observation contract would require owner approval; report the exact decision rather than inferring a value. |
| Exit Criteria | Every transferred form has a primary-source formula disposition; no endpoint or borrowed timing is introduced; all existing consumers remain one-publisher/unallocated unless a formula proves otherwise; any necessary observation path is a bounded owner decision; focused and full-gate proof passes. |
| Original Owner Request | Reach a complete L3 timing decision without hiding source gaps: add the remaining timing work in dependency order and advance the next admitted package in single-session mode. |
| Similar-Issue Sweep | Search all tracked timing owners, arithmetic handlers and tests, T357/T359/T360 records, Queue and TODO for `MUL`, `IMUL`, `DIV`, `IDIV`, `69`, `6B`, range, parenthesized clock, dynamic formula, unallocated ticks, and hardware observation wording; classify every hit as T361 scope, fixed source row, or explicit other receiver. |

## Current Technical Baseline

- **Current developer artifact:** T359 S6 `vm-0-5-0359` /
  `build/output/nxvm_0_5_0359.exe` as
  `65823D830715FC8925DBD810C2BCDEC3AEEB654D3B6B96BB022870ABF9B6783D`.
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
| T360 | Closed four-profile timing source reconciliation: primary-authority inventory, owner-selected 80286 NOP table precedence, exact legacy range transfer, contextual classifier audit, and final source/receiver closure. 245/245 current-gate tests passed; no L3 or cycle-exact claim. [Closure audit](../etc/evidence/t360-s5-four-profile-timing-source-closure-audit.md). |
| T359 | Closed its bounded four-profile instruction-retirement corpus: S1 inventory plus S2--S6 source classifiers use one successful-retirement publisher; S7 verifies all unallocated rows transfer to T360, bus/service, cycle-exact, or x87/VME receivers. 245/245 current-gate tests passed. No L3 or cycle-exact claim is made. [Closure audit](../etc/evidence/t359-s7-complete-instruction-timing-closure-audit.md). |
| T358 | Closed cross-mode mechanism coherence: reconciled I/O permission, exception/IRQ entry, selector/task/return transition, paging/linear access, and prefix/LOCK/width/string classifiers; repaired 80286 delivery, LOCK group legality, and repeated-width preview divergence; passed 240 current-gate tests. [Closure audit](../etc/evidence/t358-cross-mode-mechanism-closure-audit.md). |
| T357 | Closed finite source-backed 8086/80186/80286/80386 instruction timing at one successful-retirement publisher: selected form ledgers, 80386 protected/VM86 ordinary I/O, Appendix-B `NOP = 3` consumer reconciliation, 240 current-gate tests, and artifact SHA-256 `83A249015C03CF875896A440D5B43A430C2D875A63F0BE4B9709014A753CAD95`. Unselected forms, waits, physical bus/prefetch, device latency and cycle-exact work remain explicit transfers. [Closure audit](../etc/evidence/t357-s8-cross-profile-closure-audit.md). |
| T356 | Closed the M5 closure audit with an explicit M5-open decision: all selected device owners are evidenced, but instruction timing, bus availability/wait states and selected-profile cycle exactness remain ordered implementation prerequisites; corpus-gated peripheral, storage and product boundaries remain explicit transfers. [History](../history/M5-T356-m5-closure-audit.md). |
| T355 | Closed the bounded Windows 3.x readiness map: an opt-in BYOB HDD/INT13 checkpoint passed, all four probes remain host-observation/non-current, T347/T354 storage/L3 is a verified prerequisite, and all display, input, NMI, physical-timing, Setup, and compatibility breadth remains explicitly transferred. [History](../history/M5-T355-windows-3x-readiness-map.md). |
| T354 | Closed selected L3 bus timing: core-local checked CPU/DMA transaction begin/commit/cancel trace, deterministic CPU-retire then DMA/PIT/PIC then FDC/HDC ordering, reset cancellation/trace continuity, and retained firmware/DOS consumers. Exact bus cycles, pin waveforms, physical wait tables, host time, and Windows readiness remain explicit transfers. 234 current-gate tests passed. [History](../history/M5-T354-physical-l3-bus-timing-convergence.md). |
| T353 | Closed selected PC/AT topology: the default profile's 75 exact directional leaves and five named routes validate before core publication, remain exact through session reset and deterministic-L3 rearm, and have firmware-visible reset proof. Optional PPI/speaker, NMI, and interface breadth remain corpus-gated TODO transfers; physical bus timing transfers to the next L3 package. 231 current-gate tests passed. [History](../history/M5-T353-pcat-port-topology-and-peripherals.md). |

## Recent Governance

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
