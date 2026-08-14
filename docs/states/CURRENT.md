# Project Status

## Current Work

**Active.** M5 T361 S2 legacy dynamic-arithmetic cross-reference research is
admitted under an owner-approved external-observation contract.

## M5 T361 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | The owner approved this S on 2026-08-14 after S1: retain S1's primary-source no-formula result, record it, and directly consult local Bochs 2.6, PCjs, and public material. This is a bounded source-policy exception for research only: external implementations are observational inputs, never Intel authority, imported source, runtime dependency, or code-copy source. |
| Objective | Build a reproducible cross-reference record for the S1 8086/80186 dynamic arithmetic forms, compare candidate operand-to-clock models with Intel's documented ranges, and determine whether a profile-local formula can be admitted without guessing. |
| Non-goals | No copying, transliteration, import, dependency, or runtime integration of Bochs/PCjs; no host benchmark; no real-hardware claim without a separate contract; no bus/prefetch/device/cycle-exact work; no 80286/80386 timing change; no range endpoint/midpoint selection. |
| Reference Baseline | `2e19c914` / `vm-0-5-0359`; S1 evidence `docs/etc/evidence/t361-s1-legacy-dynamic-arithmetic-authority-audit.md`; candidate `docs/proposals/m5-legacy-dynamic-arithmetic-timing-authority.md`. |
| Candidate Proposal | [Legacy dynamic arithmetic timing authority](../proposals/m5-legacy-dynamic-arithmetic-timing-authority.md), with this owner-approved S2 research exception superseding only its ban on Bochs/PCjs values. |
| Files And ABI Surface | `docs/etc/evidence/t361-s2-legacy-dynamic-arithmetic-cross-reference.md`, `docs/etc/README.md`, `docs/history/M5-T361-legacy-dynamic-arithmetic-timing-authority.md`, and `docs/states/CURRENT.md`; source/timing code only if the exit criteria authorize a converged model. No public ABI, product dependency, or external source import. |
| Applicable Rules | Read `docs/README.md` Task Reading Set, `docs/rules/EXECUTION.md` S/P lifecycle and mechanism clauses, `docs/rules/DOCUMENT.md`, `docs/rules/ARCHITECTURE.md` source/research invariant, `docs/rules/CODING.md` if code is changed, and `docs/etc/operations/policy/source-policy.md`. Evidence records exact external repository revision/path or URL, observed behavior, license boundary, and non-import conclusion. |
| Verification | Re-run S1 focused legacy timing smokes; mechanically enumerate every S1 transferred form; for each candidate formula, enumerate representative operands and prove every predicted value lies within the profile's Intel table range. Verify every cited external path/URL is readable and every formula is independently attributed. Run documentation governance and `git diff --check`; run current-gate if any code changes. |
| Expected Markers | Existing `current.core-machine-8086-instruction-timing-ledger-smoke` and `current.core-machine-80186-instruction-timing-ledger-smoke` remain passing. Any new model test must have a `M5:T361:S2` owner marker. |
| Asset Needs | Owner-provided read-only Bochs 2.6, PCjs, and PC110-EMU checkouts plus public technical sources; no firmware, guest media, binary trace, download, or repository asset. |
| Reporting Requirements | Record candidate formulas, exact provenance, profile/form applicability, input domain, comparison result, disagreement, and non-import conclusion. Report a blocker if external candidates disagree, lack a derivable model, exceed Intel ranges, or imply architecture beyond this scope. |
| Stop Conditions | Stop before code allocation if no candidate converges; if an external code import, license review, real hardware, bus/prefetch model, shared timing publisher, or change outside S1 forms is needed; or if candidate evidence conflicts with Intel range. Transfer rather than guess. |
| Exit Criteria | Every S1 transferred form has a cross-reference disposition. Runtime allocation occurs only when at least two independent observational sources converge on a reproducible profile-local formula, the formula stays within Intel's documented range for its enumerated domain, focused proof covers the consumer and faults remain nonpublishing, and the evidence labels the result as owner-approved observational triangulation. Otherwise the task records no allocation and an exact next admission condition. |
| Original Owner Request | Record the S1 observation and directly consult Bochs, PCjs, and existing public material; the owner issued this request on 2026-08-14. |
| Similar-Issue Sweep | Sweep all S1 transferred 8086/80186 Group-3 and immediate-IMUL forms, all successful-retirement timing consumers, and all direct external candidate sources consulted; distinguish a real shared formula from profile/layout-specific behavior. |

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
| T361 S1 | Accepted the legacy dynamic-arithmetic source audit: 8086/80186 tables supply ranges but no formula, current routes remain explicitly unallocated, and a real-hardware observation contract is the only alternative path. [Evidence](../etc/evidence/t361-s1-legacy-dynamic-arithmetic-authority-audit.md). |
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
