# Project Status

## Current Work

**Active: M5 T404.**
## M5 T404 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approved the T404 proposal and ongoing in-scope implementation, commits and pushes on 2026-08-18. Scope: Batch A public catalog/profile/variant/firmware/media contract audit and bounded repair. No exceptions. |
| Objective | Consume Batch A of the T404 ledger: reconcile the public YAML catalog, console-to-session construction, three named profiles, declared overrides, firmware manifest and startup-media constraints; repair one shared product-boundary defect if the audit establishes it. |
| Non-goals | No board or physical timing/L3 claim; no ROM/media import or committed bytes; no generic free-form machine builder; no device-controller functional Batch B work. |
| Reference Baseline | T404 S1 ledger; T402/T403 accepted functional repairs; current `session_catalog`, console and session-factory/session implementation. |
| Candidate Proposal | [T404 proposal](../proposals/m5-current-product-device-profile-capability-closure.md); Batch A in [ledger](../etc/evidence/t404-s1-current-public-device-capability-ledger.md). |
| Files And ABI Surface | Inspect `src/vm/product/session_catalog.*`, `console.*`, and `src/vm/composition/session/*`; a repair may alter only the VM product/session contract and its owned product tests. Preserve Core/VM boundary and opaque public session ownership. |
| Applicable Rules | `docs/rules/EXECUTION.md`, `docs/rules/DOCUMENT.md`, `docs/rules/ARCHITECTURE.md`, `docs/rules/CODING.md`, `docs/design/ARCHITECTURE.md`, `docs/design/CODING.md`, and source policy for any research/import (none planned). |
| Verification | Focused catalog/profile/session regressions; full `ctest --test-dir build/t400-current-gate --output-on-failure` after code; documentation governance gate before acceptance. |
| Expected Markers | Existing product markers remain; added Batch A regression/evidence names `M5:T404:S2:*:OK` if a repair is admitted. |
| Asset Needs | None. BYOB firmware remains external and manifest-only; no asset is read, copied or committed. |
| Reporting Requirements | Record the complete Batch A before/after disposition, source ownership, actual-change review, verification, and explicit transfer of unresolved controller/media behavior to Batch B. |
| Stop Conditions | Stop and transfer any unsupported profile variant, firmware/media lifecycle fact, physical/timing behavior, external asset, or change that would broaden the public surface. |
| Exit Criteria | Every Batch A public path has a declared disposition and direct regression or named transfer; any found shared defect is repaired at its earliest VM product/session owner; no Core/VM interface breach. |
| Original Owner Request | Implement DeskPro 386 L3 and every hardware gap using primary sources, labelled reference implementations, or generic skeletons when needed; continue the CPU audit afterwards; do not block useful implementation on missing perfect references. |
| Similar-Issue Sweep | Audit all three named profiles and both YAML and direct option paths for generic overrides, firmware-manifest leakage, and startup media replacement; document every production hit and its disposition. |
## Current Technical Baseline

- **Current developer artifact:** T404 S2 P1 `vm-0-5-0404` /
  `build/output/nxvm_0_5_0404.exe`, SHA-256
  `A790623CF0FE337350161C5EA74613C8DC3F419A05F6ECFFBDCB3ADA110B4D86`.
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
| T404 S1 | Accepted: frozen public device/profile/catalog/media ledger defines Batch A/B coverage and every functional/physical disposition before migration. [Ledger](../etc/evidence/t404-s1-current-public-device-capability-ledger.md). |
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
