# Project Status

## Current Work

**Active: M5 T404.**
## M5 T404 S6 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approved ongoing T404 implementation, acceptance and push on 2026-08-18. Scope: complete Batch B reconciliation and T404 closure decision. |
| Objective | Exhaust the frozen Batch B controller/media/display/input universe by reconciling every public route to direct current regression, accepted T404 repair, named retained physical/reference receiver, or a newly admitted bounded repair. |
| Non-goals | No physical timing or L3 completion claim; no new generic builder, guest media/ROM import, source import, or unsupported device addition. |
| Reference Baseline | T404 S1 ledger; S2 catalog contract; S3/S4 media lifecycle evidence; S5 host-input evidence; retained T386 selected-device functional matrix and T403 KBC closure. |
| Candidate Proposal | [T404 proposal](../proposals/m5-current-product-device-profile-capability-closure.md); [ledger](../etc/evidence/t404-s1-current-public-device-capability-ledger.md). |
| Files And ABI Surface | Documentation/evidence, `CURRENT.md`, task history, Queue/proposal only if the T404 completion predicate is met. No code change unless the audit finds a bounded functional defect. |
| Applicable Rules | `docs/rules/EXECUTION.md`, `docs/rules/DOCUMENT.md`, and the existing architecture/coding evidence boundaries; source policy is not triggered. |
| Verification | Inspect actual source/test registrations and prior evidence; run documentation governance; rerun full current CTest gate for any code repair. |
| Expected Markers | `M5:T404:S6:BATCH-B-RECONCILIATION:OK`; a closure audit only if every ledger row is exhausted or transferred. |
| Asset Needs | None. |
| Reporting Requirements | Record the complete batch matrix, test/evidence owner, residual receiver, actual-change review and task-level completion decision. |
| Stop Conditions | If any public route lacks direct proof, admitted repair, unavailable/removed result, or named receiver, retain T404 open and admit only the earliest bounded owner repair. |
| Exit Criteria | Every Batch B row is explicitly reconciled; T404 closes only when the S1 completion predicate is met, retaining all physical/L3 receivers in Queue/TODO/evidence. |
| Original Owner Request | Implement DeskPro 386 L3 and all hardware gaps using primary sources, clearly labelled references, or generic skeletons when needed; do not block useful progress on missing perfect references. |
| Similar-Issue Sweep | Search all public session catalog/Console/session interface entry points and current controller/display/input target registrations for unclassified routes. |
## Current Technical Baseline

- **Current developer artifact:** T404 S5 P1 `vm-0-5-0404` /
  `build/output/nxvm_0_5_0404.exe`, SHA-256
  `291DF614D24CF5EE4AB0C65188CBCF1815084978A94FC9C700CBCED171115469`.
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
| T404 S5 | Accepted: VM-public host input accepts only mapped key/mouse events; unknown kinds reject before ingress without a guest-side effect. [Evidence](../etc/evidence/t404-s5-public-host-input-contract.md). |
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
