# Project Status

## Current Work

**Active: M5 T434 S1.**

## M5 T434 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New; T434 is the first queued Core timing-contract candidate and S1 consumes B1 of the Td S117 convergence ledger. |
| Admission And Approval | Owner objective in this conversation: implement the complete Core timing-plan internal integration and interface construction. Owner accepted Td S120's soft code-size governance before this admission. |
| Objective | Implement the neutral copied Core machine plan, validation and all 30 capability declaration seams, with plan-aware Core construction and focused low-level proof; production remains on its existing sole publisher until B2. |
| Non-goals | Do not migrate a VM production publisher, retain a temporary second production publisher, implement L3 formulas/scalars, add profile names/callback algorithms to Core, alter Queue order, model L4, or claim T434 closure. |
| Reference Baseline | `f0e145d6`; T434 proposal; Td S117 convergence ledger; T433 S7 source-sufficiency ledger; current Core machine/configuration source. |
| Candidate Proposal | [Core timing contract and machine plan](../proposals/m5-core-timing-contract-machine-plan.md). |
| Files And ABI Surface | Expected Core machine public/private headers and source, focused Core tests/build registration, T434 S1 evidence and current status. Public Core plan API is an intended ABI addition; no VM/profile/public-product behavior change in S1. |
| Applicable Rules | `docs/README.md` Task Reading Set; `docs/rules/EXECUTION.md`; `docs/rules/ARCHITECTURE.md`; `docs/rules/CODING.md`; `docs/rules/DOCUMENT.md` at closure; Core architecture/source-layout authorities; T433 S7 and Td S117. |
| Verification | Prove all 30 frozen IDs declare exactly one L2/NGT/required-L3 disposition through copied plan data; valid plans copy and construct; missing/duplicate/invalid and required-unavailable-L3 plans fail before publication; existing low-level configuration creation remains behaviorally unchanged; plan lifecycle/reset/trace declarations are observable. Run focused tests, applicable build/gates, 30-ID equality, code actual-delta report, documentation governance and `git diff --check`. |
| Expected Markers | `M5:T434:S1:PLAN-DECLARATIONS:OK`; `M5:T434:S1:PLAN-VALIDATION:OK`; `M5:T434:S1:PLAN-COPY:OK`. |
| Asset Needs | No protected assets, firmware/media, downloads or source import. |
| Reporting Requirements | Report actual added/removed/net code lines and method; explain any material positive increase; report each retained obsolete path or removal; identify the future B2 publisher migration boundary. |
| Stop Conditions | Stop for a required capability outside the frozen 30-ID universe, an unresolved Core/VM ownership contradiction, a need for VM production cutover to make B1 meaningful, an ABI conflict, or a plan representation that cannot provide exactly one disposition per ID. |
| Exit Criteria | One copied validated plan represents every frozen declaration seam and rejects invalid/required-unavailable-L3 input atomically; low-level proof covers copy, dispositions, lifecycle/observation declarations and failures; no production publisher is added; evidence maps all 30 rows; applicable checks pass and S1 P is pushed. |
| Original Owner Request | Begin and fully implement the Core timing plan internal integration and interface construction, while preventing incomplete consumer seams and preserving maintainable code. |
| Similar-Issue Sweep | Inspect all `core_machine_config` creation and Core configure/constructor paths, public Core callers, plan-like existing declarations, and focused tests for a duplicated plan, undeclared capability, hidden fallback, copied-lifetime gap or second creation seam. |
## Current Technical Baseline

- **Current developer artifact:** T432 S1 `vm-0-5-0432` / `build/output/nxvm_0_5_0432.exe`, SHA-256 `F29A7DF2BA6F43D2A7E1B4C9AABCD634123E4E57F92226796B5734B4A71C413C`. T432 separates KBC delayed Set-2 translation from native break typematic cancellation, fixing stuck keys in Model-339 Console and Window sessions.
  T386 closes selected-device functional completeness at S29; its retained
  [closure audit](../etc/evidence/t386-s29-functional-closure-audit.md) fixes
  HDC current-gate coverage and transfers board, firmware and physical work.
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
| T433 | Closed after corrective S7: 30 frozen Core capabilities have a source-sufficiency and ownership disposition; chip/manual semantics are separated from VM-profile inputs, host/product boundaries and the explicit ATA/HDC source block. [S7 ledger](../etc/evidence/t433-s7-core-source-sufficiency-ledger.md). |
| T432 | Closed: separate Core-private Set-2 translation and typematic break states prevent released keys from restarting typematic; focused regression, Model-339 contract and owner Console/Window verification pass. [Record](../history/M5-T432-keyboard-keyup-reliability-repair.md). |
| T431 | Closed: Model-40 selects a reference-derived unready-drive READ `ST0=48h`/IRQ/result completion through one Core policy; generic PC/AT stays generic, and physical FDC/L3 timing remains transferred. [Evidence](../etc/evidence/t431-s1-deskpro-fdc-not-ready-reference.md). |
| T430 | Closed: original/reference-backed Compaq WD 40 MB C:/D: selection now uses frozen Core media slots with optional Model-40 secondary backing; generic ATA stays unchanged, and physical/L3 timing remains transferred. [Evidence](../etc/evidence/t430-s1-deskpro-dual-fixed-disk.md). |
| T429 | Closed: generic-AT CECG port/aperture wait skeleton and D4-memory classification; physical CECG and monitor behavior remain TODO. [S1](../etc/evidence/t429-s1-cecg-8bit-bus-wait.md), [S2](../etc/evidence/t429-s2-d4-cecg-memory-class.md), [S3](../etc/evidence/t429-s3-cecg-aperture-wait.md).
| T428 | Closed: Model-40 selects a generic-AT Core prefetch reservation lifecycle with reset/HOLD/refresh priority; it publishes no physical cycle, page hit or timing result. [Evidence](../etc/evidence/t428-s1-generic-prefetch-reservation.md). |
| T427 | Closed: original-source CPU BUSRDY gate holds the existing Core external completion wait without retirement; Model-40 selects it and 5170 is isolated. [Evidence](../etc/evidence/t427-s1-deskpro-cpu-busready-gate.md). |
| T426 | Closed: independent audit retains all tier-labelled logical receivers but rejects DeskPro physical/L3 acceptance pending named source/corpus blockers. [Evidence](../etc/evidence/t426-s1-deskpro-physical-l3-reaudit.md). |


## Recent Governance

- **M5 Td S120 P1:** changed code-size restraint from a hard numerical admission/closure gate to a soft concise-design goal; actual-delta reporting, explanation and obsolete-path retirement/disposition remain mandatory. T434 remains unallocated.

- **M5 Td S119 P1:** compressed Td S118's code-size/cleanup rule without weakening its baseline, default non-positive budget, prior numeric approval, final reconciliation, retirement or closure-block requirements. T434 remains unallocated.

- **M5 Td S118 P1:** made a reproducible code-size baseline, default non-positive net code, owner-approved positive cap, obsolete-path disposition and actual-delta report mandatory for every code-changing S. T434 remains unallocated.

- **M5 Td S117 P1:** froze T434's complete 30-ID seam/default-disposition ledger, three current VM session materialization families and two bounded implementation batches; it requires one atomic plan-only publication cutover and records architecture/coding proof as a task-local closure barrier. T434 remains unallocated.

- **M5 Td S116 P1:** made default-plan equivalence concrete and prohibits T434 closure when any public capability is undisposed, any production path bypasses the plan, or plan semantics conflict with Core/VM ownership; T434 remains unallocated.

- **M5 Td S115 P1:** made the Core timing-plan candidate require one production publication path, all current consumer seams, explicit runnable L2 dispositions, atomic invalid-plan/required-L3 rejection, and no audit-only handoff; T434 remains unallocated.

- **M5 Td S114 P1:** established the mandatory three-level hardware implementation ledger for every M5 hardware candidate: document-primary complete implementation first, reproducible mature-emulator reference contract second, then deterministic explicit maintained boundary; no Rules, runtime or source import changed.

- **M5 Td S113 P1:** linked every Core L3 proposal directly to the shared timing design and its exact finite admission batch; the five earliest receiver batches cover all 30 frozen Core capability IDs once, without changing Queue order, runtime or architecture authority.

- **M5 Td S112 P1:** replaced pre-Windows Queue positions 7-10 with PC/AT 5170 root normalization, DeskPro child convergence, AT resolver/default-at migration, and YAML cutover; each has a proposal. The VM design now makes the current AT tree direct under pc-at-5170; XT remains deferred.

- **M5 Td S111 P1:** reordered all pre-Windows M5 candidates: six Core L3 tasks, 5170-derived runnable DeskPro, VM profile foundation, retained XT closure, then Windows; withdrew four stale candidates while preserving their evidence links.


- **M5 Td S110 P1:** recorded the target VM profile resolver and copied Core machine-plan design: 5160 and 5170 are the two roots; YAML has one built-in profile selector, no variant or compatibility-version layer; 486/P5 is deferred. No runtime, Queue, proposal, or ABI changed.

- **M5 Td S109 P1:** made Project timing levels explicit: L1 is ordered functional boot without timing guarantee; L2 is implemented macro-scale proportional timing; L3 is the current documentation-driven constant/formula/range target; L4 hardware-real timing is prohibited from implementation.

- **M5 Td S108 P1:** established Project A's Core specification-driven instruction and transaction timing design: L1--L4 vocabulary, admitted capability universe, explicit L2 exception ledger, and a neutral Core-to-VM timing-contract boundary; no runtime or Queue change.

- **M5 Td S107 P1:** retired the completed tiered-fallback foundation from the active Queue, promoted the DeskPro physical-cycle program, and ordered CPU-to-board timing, device phases, Compaq fixed disk, CECG, BYOB lifecycle, and independent L3 re-audit with retained evidence tiers.

- **M5 Td S106 P1:** corrected the retained PCjs DeskPro HDC configuration fact to two generic AT Type-1 10.16 MB drives; the Compaq WD 40 MB non-qualification and all physical receivers remain unchanged.
