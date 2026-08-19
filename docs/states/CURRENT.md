# Project Status

## Current Work

**Active: M5 Td S113.**

## M5 Td S113 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Governance |
| Admission And Approval | Owner request in this conversation on 2026-08-19: perform the Td governance task that makes the L3 principle design and T433 S6 admission ledger explicit references of the Queue and five Core L3 proposals. Scope is link/mapping clarification only. |
| Objective | Make the existing L3 design and T433 S6 ledger directly consumable by every queued Core implementation candidate: add direct links, identify each candidate's frozen capability-ID batch, and state that candidates implement their assigned receiver without redefining the shared L3/Core-VM boundary. |
| Non-goals | Do not alter L3 definitions, Queue order, T433 dispositions, runtime behavior, C/YAML APIs, profile policy, timing constants, evidence provenance, or task identifiers. |
| Reference Baseline | `c4f60185` / current `master`; `docs/states/QUEUE.md`; the five queued Core proposals; `docs/etc/architecture/specification-driven-l3-timing.md`; T433 S6 admission ledger. |
| Candidate Proposal | Not applicable: standalone governance clarification of already queued proposals. |
| Files And ABI Surface | `docs/states/CURRENT.md`, `docs/states/QUEUE.md`, and the five existing `docs/proposals/m5-core-*.md` documents only. No runtime, ABI, artifact or source change. |
| Applicable Rules | `docs/README.md` Task Reading Set; `docs/rules/EXECUTION.md` governance/P lifecycle and closure audit; `docs/rules/DOCUMENT.md`; `docs/states/QUEUE.md`; `docs/design/ROADMAP.md`; the L3 timing design and T433 S6 evidence. |
| Verification | Inspect every changed proposal and Queue reference; mechanically confirm all five proposals link both shared documents and state a non-empty consumed ID batch; run `powershell -NoProfile -ExecutionPolicy Bypass -File tools/Verify-DocumentationGovernance.ps1 -RepositoryRoot .`; run `git diff --check`. |
| Expected Markers | Direct links to the L3 timing design and T433 S6 ledger in Queue and all five proposals; one capability-ID allocation map; documentation governance success. |
| Asset Needs | None. No source, firmware, media, external research, or third-party material is used. |
| Reporting Requirements | Report the shared references, the exact proposal-to-ID allocation, pushed commit and governance verification. |
| Stop Conditions | Stop if a requested mapping would change a T433 disposition, Queue ordering, a proposal's technical scope, or architecture authority rather than merely clarifying its existing receiver. |
| Exit Criteria | Queue and all five Core proposals directly link the shared design and T433 S6 ledger; every frozen hardware-facing capability has one unambiguous earliest Core candidate receiver, while integration-only rows are assigned to the final integration/export candidate; no authority is duplicated or redefined; governance/diff checks pass and the Td P is pushed. |
| Original Owner Request | Perform this reference-governance task now as a Td, commit it, and push it. |
| Similar-Issue Sweep | Sweep `docs/states/QUEUE.md` and all five `docs/proposals/m5-core-*.md` candidates for indirect-only master-ledger wording, missing direct links, missing T433 IDs, contradictory receiver wording, or duplicated L3 authority. |
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
| T433 | Closed after corrective S6: all 30 frozen Core capabilities now have an actual L3 fact gap, Core-feasibility result, required plan/program declaration, evidence admission, focused proof and exact receiver. [Admission ledger](../etc/evidence/t433-s6-core-l3-admission-feasibility-ledger.md). |
| T432 | Closed: separate Core-private Set-2 translation and typematic break states prevent released keys from restarting typematic; focused regression, Model-339 contract and owner Console/Window verification pass. [Record](../history/M5-T432-keyboard-keyup-reliability-repair.md). |
| T431 | Closed: Model-40 selects a reference-derived unready-drive READ `ST0=48h`/IRQ/result completion through one Core policy; generic PC/AT stays generic, and physical FDC/L3 timing remains transferred. [Evidence](../etc/evidence/t431-s1-deskpro-fdc-not-ready-reference.md). |
| T430 | Closed: original/reference-backed Compaq WD 40 MB C:/D: selection now uses frozen Core media slots with optional Model-40 secondary backing; generic ATA stays unchanged, and physical/L3 timing remains transferred. [Evidence](../etc/evidence/t430-s1-deskpro-dual-fixed-disk.md). |
| T429 | Closed: generic-AT CECG port/aperture wait skeleton and D4-memory classification; physical CECG and monitor behavior remain TODO. [S1](../etc/evidence/t429-s1-cecg-8bit-bus-wait.md), [S2](../etc/evidence/t429-s2-d4-cecg-memory-class.md), [S3](../etc/evidence/t429-s3-cecg-aperture-wait.md).
| T428 | Closed: Model-40 selects a generic-AT Core prefetch reservation lifecycle with reset/HOLD/refresh priority; it publishes no physical cycle, page hit or timing result. [Evidence](../etc/evidence/t428-s1-generic-prefetch-reservation.md). |
| T427 | Closed: original-source CPU BUSRDY gate holds the existing Core external completion wait without retirement; Model-40 selects it and 5170 is isolated. [Evidence](../etc/evidence/t427-s1-deskpro-cpu-busready-gate.md). |
| T426 | Closed: independent audit retains all tier-labelled logical receivers but rejects DeskPro physical/L3 acceptance pending named source/corpus blockers. [Evidence](../etc/evidence/t426-s1-deskpro-physical-l3-reaudit.md). |


## Recent Governance


- **M5 Td S112 P1:** replaced pre-Windows Queue positions 7-10 with PC/AT 5170 root normalization, DeskPro child convergence, AT resolver/default-at migration, and YAML cutover; each has a proposal. The VM design now makes the current AT tree direct under pc-at-5170; XT remains deferred.

- **M5 Td S111 P1:** reordered all pre-Windows M5 candidates: six Core L3 tasks, 5170-derived runnable DeskPro, VM profile foundation, retained XT closure, then Windows; withdrew four stale candidates while preserving their evidence links.


- **M5 Td S110 P1:** recorded the target VM profile resolver and copied Core machine-plan design: 5160 and 5170 are the two roots; YAML has one built-in profile selector, no variant or compatibility-version layer; 486/P5 is deferred. No runtime, Queue, proposal, or ABI changed.

- **M5 Td S109 P1:** made Project timing levels explicit: L1 is ordered functional boot without timing guarantee; L2 is implemented macro-scale proportional timing; L3 is the current documentation-driven constant/formula/range target; L4 hardware-real timing is prohibited from implementation.

- **M5 Td S108 P1:** established Project A's Core specification-driven instruction and transaction timing design: L1--L4 vocabulary, admitted capability universe, explicit L2 exception ledger, and a neutral Core-to-VM timing-contract boundary; no runtime or Queue change.

- **M5 Td S107 P1:** retired the completed tiered-fallback foundation from the active Queue, promoted the DeskPro physical-cycle program, and ordered CPU-to-board timing, device phases, Compaq fixed disk, CECG, BYOB lifecycle, and independent L3 re-audit with retained evidence tiers.

- **M5 Td S106 P1:** corrected the retained PCjs DeskPro HDC configuration fact to two generic AT Type-1 10.16 MB drives; the Compaq WD 40 MB non-qualification and all physical receivers remain unchanged.

- **M5 Td S105 P2:** restored a usable tiered DeskPro work path, recorded the five-receiver ledger and admitted next target T406 for a labelled generic-AT KBC serial-delivery mechanism; this is not a physical/L3 acceptance.

- **M5 Td S104 P1:** added the owner-authorized reference-derived DeskPro timing-bridge candidate ahead of CPU audit; it preserves physical-L3 not-ready and requires a fresh independent re-audit.

- **M5 Td S103 P1:** adopted task-scale convergence and coverage governance: coverage-bearing T work requires a durable universe, batch dispositions, completion predicate and coordinator reconciliation before whole-domain or L3 acceptance.
