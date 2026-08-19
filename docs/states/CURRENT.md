# Project Status

## Current Work

**Active: M5 T433 S6.**

## M5 T433 S6 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Corrective; M5 T433 S6 is the next unused subtask of the most recently closed numeric task. |
| Admission And Approval | Owner request in this conversation on 2026-08-19: reopen T433 and make every Core device/chip capability's actual L3 gap, manual/reference-driven feasibility, neutral-Core mechanism, implementation plan, and acceptance path explicit. The corrective boundary repairs the incomplete downstream-admission detail in the closed audit; no runtime timing behavior is admitted here. |
| Objective | Replace T433's summary-only dispositions with one complete, finite, per-capability L3 admission ledger that states whether Core can implement a documentation-driven L3 rule, what fact/contract is missing, the precise earliest receiver, required timing-plan/program fields, evidence class, and proof needed to close that row. Correct the T433 closure provenance/index defects discovered during the reassessment. |
| Non-goals | Do not implement a timing engine, fabricate timing constants, change CPU/device behavior, add a VM profile, import source/firmware/media, claim L4, or assert that every future external reference fact is already available. |
| Reference Baseline | `9bf609e6` / current `master`; T433 master ledger and S1-S4 reconciliations; the Core specification-driven timing design; existing T357/T359/T363/T369/T370/T401 retained evidence. |
| Candidate Proposal | Corrective execution remains within the retained [T433 proposal](../history/M5-T433-core-l3-ledger-reconciliation-proposal.md); the admissible current proposal context is [Core timing contract and machine plan](../proposals/m5-core-timing-contract-machine-plan.md). It supplies the missing exact downstream-admission detail required by the original T433 purpose. |
| Files And ABI Surface | Documentation/evidence/state/history/index only: `docs/etc/evidence/t433-*`, `docs/history/M5-T433-*`, `docs/etc/README.md`, and this packet. No public/private C API, YAML schema, runtime behavior, artifact identity, or ABI change. |
| Applicable Rules | `docs/README.md` Task Reading Set; `docs/rules/EXECUTION.md` corrective-S, coverage-ledger, evidence, P lifecycle, and closure rules; `docs/rules/DOCUMENT.md`; `docs/design/ROADMAP.md`; `docs/design/ARCHITECTURE.md`; `docs/etc/architecture/specification-driven-l3-timing.md`; source/research policy for any newly researched manual/reference source. |
| Verification | Reconcile all frozen 30 IDs against the new required fields; compare every receiver against the ordered Queue and the timing design; run `powershell -NoProfile -ExecutionPolicy Bypass -File tools/Verify-DocumentationGovernance.ps1 -RepositoryRoot .`; run `git diff --check`; inspect actual diffs and linked historical evidence. No build is required because no source/runtime artifact changes. |
| Expected Markers | `M5:T433:S6:L3-ADMISSION-LEDGER:OK`; `M5:T433:S6:FEASIBILITY-RECONCILIATION:OK`; documentation governance success. |
| Asset Needs | No protected assets. Existing committed primary-manual/source ledgers may be cited. If new external material becomes necessary, stop before importing or deriving it and apply the source/research policy. |
| Reporting Requirements | Confirm the corrective boundary; report after the complete 30-row feasibility ledger is reconciled; final report links the ledger, names the pushed commits, verification, and any explicit external evidence receiver. |
| Stop Conditions | Stop and request owner direction if a row requires a new architecture/Queue decision beyond the existing five Core candidates, an unlicensed/protected source, a contradiction among authoritative sources, or a runtime/API change to make the ledger truthful. |
| Exit Criteria | Every frozen T433 ID has actual missing L3 fact(s), feasibility classification, Core mechanism boundary, concrete plan/program field(s), evidence admission tier, proof/acceptance scenario, and exact earliest receiver; all non-Core/L4 facts are explicitly bounded; the original T433 closure provenance/index defects are corrected; governance/diff checks pass; complete P is pushed and coordinator-reviewed. |
| Original Owner Request | Reopen T433 and make the actual gaps and completion plans explicit for every device/chip, so later tasks can close; confirm whether Core can implement documentation-driven timing simulation for every device/chip with this level of clarity. |
| Similar-Issue Sweep | Documentation-closure defect class: inspect all T433 history, evidence-index and state references for malformed provenance, missing corrective closure facts, ambiguous receiver language, or a frozen capability without exact downstream admission information. Search `docs/history/M5-T433*`, `docs/etc/evidence/t433*`, `docs/etc/README.md`, `docs/states/CURRENT.md`, `docs/states/QUEUE.md`, and the timing design. |
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
| T433 | Closed: publishes the 30-row Core master ledger and conformance report; every frozen public Core capability has one owner, evidence, regression and truthful disposition. [Evidence](../etc/evidence/t433-core-l3-master-ledger.md). |
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
