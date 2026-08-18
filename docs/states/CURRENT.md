# Project Status

## Current Work

**Active: M5 T404.**
## M5 T404 S3 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approved the T404 proposal and ongoing in-scope implementation, commits and pushes on 2026-08-18. Scope: Batch B media/controller/display/input functional reconciliation and bounded shared-owner repair. No exceptions. |
| Objective | Consume the Batch B fixed/removable-media lifecycle slice: make the session owner enforce that fixed HDD media is startup-only and removable FDD media cannot change while running; retain controller/display/input coverage for the subsequent complete Batch B reconciliation. |
| Non-goals | No controller register/timing/L3 claim; no media or ROM import; no Console-only workaround; no public Core/VM ABI expansion. |
| Reference Baseline | T404 S1 ledger, S2 catalog contract, current `vm_session_insert_fdd`/`vm_session_insert_hdd` paths, Model-40 startup HDC binding, and existing console lifecycle check. |
| Candidate Proposal | [T404 proposal](../proposals/m5-current-product-device-profile-capability-closure.md); [ledger](../etc/evidence/t404-s1-current-public-device-capability-ledger.md). |
| Files And ABI Surface | `src/vm/composition/session/session.*` and owned product/machine tests; preserve public function signatures, Core/VM boundary, and Model-40 private startup HDC path. |
| Applicable Rules | `docs/rules/EXECUTION.md`, `docs/rules/DOCUMENT.md`, `docs/rules/ARCHITECTURE.md`, `docs/rules/CODING.md`, `docs/design/ARCHITECTURE.md`, `docs/design/CODING.md`; source policy is not triggered because no external source or asset is used. |
| Verification | Focused session/console media lifecycle tests, current full CTest gate, documentation governance gate, and actual-diff review. |
| Expected Markers | Existing session product markers remain; new focused lifecycle regression records `M5:T404:S3:MEDIA-LIFECYCLE:OK`. |
| Asset Needs | None. Tests use temporary project-owned synthetic disk images only; no firmware, guest media, or asset archive access. |
| Reporting Requirements | Record every public FDD/HDD insertion path, startup/publication/running disposition, before/after test proof, similar-issue sweep, and deferred controller/display/input Batch B rows. |
| Stop Conditions | Stop and transfer any controller semantic, physical timing, firmware/media byte, unsupported profile behavior, or interface expansion outside the lifecycle boundary. |
| Exit Criteria | Fixed HDD replacement is unavailable after session publication; removable FDD insertion is rejected while running by its owner; startup construction remains functional for every supported profile; all known public insertion callers are dispositioned. |
| Original Owner Request | Implement DeskPro 386 L3 and all hardware gaps using primary sources, labelled reference implementations, or a generic IBM AT skeleton when necessary; do not block useful implementation on missing perfect references. |
| Similar-Issue Sweep | Search all production and test callers of `vm_session_insert_fdd`, `vm_session_insert_hdd`, Model-40 startup HDD insertion, and Console insert commands; fix shared-owner hits or record an explicit disposition. |
## Current Technical Baseline

- **Current developer artifact:** T404 S3 P1 `vm-0-5-0404` /
  `build/output/nxvm_0_5_0404.exe`, SHA-256
  `BDB5436DC88B22A7CCC15E701AE2CC07BC26D85829B7FDB2C38D566D5D9CF32A`.
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
| T404 S2 | Accepted: Batch A reconciles all public catalog-to-session profile paths; unsupported default CPU/FPU catalog variants now reject before selection, and Batch B retains media/controller function. [Evidence](../etc/evidence/t404-s2-public-catalog-session-contract.md). |
| T403 | Closed: shared keyboard serial endpoint prevents accepted break loss under full 8042 output; functional KBC flow is complete while physical keyboard/8042/DeskPro timing remains transferred. [Closure audit](../etc/evidence/t403-s2-keyboard-flow-control-closure-audit.md). |
| T402 | Closed: D3PE-first functional D4 matrix accepted; the Model-40 control aperture repair is reference-labelled where primary material is incomplete, while cache/DRAM/ISA physical timing and L3 remain transferred. [Closure audit](../etc/evidence/t402-s2-d4-functional-closure-audit.md). |
| T401 | Closed: bounded four-profile CPU semantic/source-timing audit; every frozen form is proven, repaired, nonphysical-observed, rejected or transferred, without an x87, physical CPU, board or L3 claim. [Closure audit](../etc/evidence/t401-s78-task-closure-audit.md). |
| T400 | Closed: the HDC, CECG and CPU/DMA/BWAIT generic-reference ledger is exhausted; accepted logical repairs retain every DeskPro physical boundary. [Closure audit](../etc/evidence/t400-s1-task-closure-audit.md). |
| T399 | Closed: the five-batch reference-derived DeskPro L3 ledger is exhausted; B2/B3 logical contracts are accepted, B1/B4/B5 retain exact receivers, and both reference-derived and physical L3 are not ready. [Closure audit](../etc/evidence/t399-s5-task-closure-audit.md). |
| T398 | Closed: final exact 1986 DeskPro Model-40 audit is not-ready for L3; five physical/firmware boundaries retain explicit TODO receivers and require a fresh audit after repair. [Closure audit](../etc/evidence/t398-s2-task-closure-audit.md). |
| T397 | Closed: selected functional FDC/HDC/CECG/KBC-NMI/DMA routes and reset/order evidence retained; physical availability, phase and service facts transfer to explicit TODO receivers. Final DeskPro L3 may make only a ready/not-ready decision. [Closure audit](../etc/evidence/t397-s2-task-closure-audit.md). |
| T396 | Closed: finite Model-40 source/owner, six-domain, reset and callback-order ledger retained; all unsourced physical clock, phase, wait and device/firmware visibility facts transfer to the next DeskPro candidate. No physical scalar or L3 claim. [Closure audit](../etc/evidence/t396-s2-task-closure-audit.md). |

## Recent Governance

- **M5 Td S104 P1:** added the owner-authorized reference-derived DeskPro timing-bridge candidate ahead of CPU audit; it preserves physical-L3 not-ready and requires a fresh independent re-audit.

- **M5 Td S103 P1:** adopted task-scale convergence and coverage governance: coverage-bearing T work requires a durable universe, batch dispositions, completion predicate and coordinator reconciliation before whole-domain or L3 acceptance.
- **M5 Td S102 P1:** reconciled accepted D4 parity work into the T386 proposal and Queue title, defined the non-preallocated remaining functional receiver order, and confirmed the existing Core/VM/profile architecture authority needs no amendment.
- **M5 Td S101 P1:** added a compact execution-entry table without creating new paths, and made coordinator semantic review explicit after structural documentation-gate success.
- **M5 Td S100 P1:** archived the closed T382 proposal, queued Model-339 current-source re-audit before DeskPro, reconciled T380 S2 history, and made host/workspace path detection reject the external-assets form while retaining guest DOS-path examples.
