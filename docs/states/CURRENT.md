# Project Status

## Current Work

**Active: M5 T433 S7.**

## M5 T433 S7 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Corrective; M5 T433 S7 is the next unused subtask of the most recently closed numeric task. |
| Admission And Approval | Owner request in this conversation on 2026-08-19: reopen T433 and determine, for every current Core capability, which usable manuals/specifications/reference contracts currently exist, which do not, and whether a missing fact is a Core rule, profile-supplied machine input, host/product boundary, or unsupported scope. |
| Objective | Produce a finite 30-ID source-sufficiency ledger that replaces feasibility-only language with evidence status and a precise implementation precondition: available admissible source, source incomplete/conflicting, source absent/unselected, or not a guest-time rule. It must distinguish Core-owned chip/state-machine rules from external profile facts and non-Core boundaries. |
| Non-goals | Do not invent numeric timing, download/import source/firmware/media, alter runtime code or L3 definitions, promote a reference emulator into authority, claim an unavailable source exists, or implement any timing rule. |
| Reference Baseline | `2dccc6ee` / current `master`; T433 S1/S6; retained T357/T359/T363/T369/T370/T373/T377/T401 source ledgers; source/research policy; shared L3 timing design. |
| Candidate Proposal | Corrective work remains within the retained T433 proposal; the current admissible consumer context is [Core timing contract and machine plan](../proposals/m5-core-timing-contract-machine-plan.md). |
| Files And ABI Surface | Documentation/evidence/history/index/state only: new `docs/etc/evidence/t433-s7-*`, T433 history/master/S6/index and this packet as justified. No code, ABI, profile, artifact or runtime behavior change. |
| Applicable Rules | `docs/README.md` Task Reading Set; `docs/rules/EXECUTION.md` corrective-S and coverage-ledger rules; `docs/rules/DOCUMENT.md`; `docs/etc/operations/policy/source-policy.md`; L3 timing design; T433 frozen universe. |
| Verification | Reconcile exactly the T433 30-ID universe; for each row cite existing evidence/manual class or a bounded `none` finding, assign exactly one missing-fact ownership class, and compare downstream receivers to Queue/proposals. Run documentation governance and `git diff --check`; inspect actual evidence and history changes. |
| Expected Markers | `M5:T433:S7:SOURCE-SUFFICIENCY:OK`; `M5:T433:S7:OWNERSHIP-RECONCILIATION:OK`; a 30-ID equality check; documentation governance success. |
| Asset Needs | No protected assets or new downloads. Public manual/reference links already retained in committed evidence may be inspected; any new research source must follow the source policy and be recorded by tier without importing material. |
| Reporting Requirements | Report the source-sufficiency categories and any genuine source blocker; final delivery links the ledger, pushed commits, verification and the exact next implementation precondition. |
| Stop Conditions | Stop and ask the owner if a row requires a new device/chipset selection, protected material, an external reference not permitted by policy, a new architecture authority, or a choice between incompatible source tiers. |
| Exit Criteria | Every T433 ID has an evidence-status classification, named current source/evidence or explicit bounded absence, missing-fact ownership classification, implementation eligibility decision and exact receiver/precondition; no row claims that generic Core feasibility implies sources are already sufficient; history/index corrections and governance checks pass; complete commits are pushed and reviewed. |
| Original Owner Request | Reopen T433 and confirm exactly which Core items have usable manual specifications, which do not, and whether missing pieces are external inputs to Core or outside its scope; state this task's next subtask. |
| Similar-Issue Sweep | Search all T433 evidence/history, retained CPU/transaction/device source ledgers, Queue/proposals and TODO for `source`, `manual`, `reference`, `unavailable`, `unselected`, `L2`, and `TODO`; reconcile any capability whose current wording conflates Core feasibility with source sufficiency. |
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

- **M5 Td S113 P1:** linked every Core L3 proposal directly to the shared timing design and its exact finite admission batch; the five earliest receiver batches cover all 30 frozen Core capability IDs once, without changing Queue order, runtime or architecture authority.

- **M5 Td S112 P1:** replaced pre-Windows Queue positions 7-10 with PC/AT 5170 root normalization, DeskPro child convergence, AT resolver/default-at migration, and YAML cutover; each has a proposal. The VM design now makes the current AT tree direct under pc-at-5170; XT remains deferred.

- **M5 Td S111 P1:** reordered all pre-Windows M5 candidates: six Core L3 tasks, 5170-derived runnable DeskPro, VM profile foundation, retained XT closure, then Windows; withdrew four stale candidates while preserving their evidence links.


- **M5 Td S110 P1:** recorded the target VM profile resolver and copied Core machine-plan design: 5160 and 5170 are the two roots; YAML has one built-in profile selector, no variant or compatibility-version layer; 486/P5 is deferred. No runtime, Queue, proposal, or ABI changed.

- **M5 Td S109 P1:** made Project timing levels explicit: L1 is ordered functional boot without timing guarantee; L2 is implemented macro-scale proportional timing; L3 is the current documentation-driven constant/formula/range target; L4 hardware-real timing is prohibited from implementation.

- **M5 Td S108 P1:** established Project A's Core specification-driven instruction and transaction timing design: L1--L4 vocabulary, admitted capability universe, explicit L2 exception ledger, and a neutral Core-to-VM timing-contract boundary; no runtime or Queue change.

- **M5 Td S107 P1:** retired the completed tiered-fallback foundation from the active Queue, promoted the DeskPro physical-cycle program, and ordered CPU-to-board timing, device phases, Compaq fixed disk, CECG, BYOB lifecycle, and independent L3 re-audit with retained evidence tiers.

- **M5 Td S106 P1:** corrected the retained PCjs DeskPro HDC configuration fact to two generic AT Type-1 10.16 MB drives; the Compaq WD 40 MB non-qualification and all physical receivers remain unchanged.
