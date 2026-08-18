# Project Status

## Current Work

**Active: M5 T403.**
## M5 T403 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New |
| Admission And Approval | Owner: user; continuing authorization to implement DeskPro 386 L3 timing and all hardware gaps using original-source, reference-derived, then labelled generic tiers; direct repairs, normal commits and pushes are authorized. |
| Objective | Freeze the shared keyboard/8042 producer-consumer matrix and repair atomic native-sequence admission, break/typematic state and CPU-output flow control so congestion cannot discard a valid break. |
| Non-goals | No physical DeskPro keyboard wire/FIFO timing claim, exact 8042 firmware or capacity claim, vendor firmware/media import, new host ABI, platform-side guest mutation, Core/VM reversal or L3 claim. |
| Reference Baseline | `7c516481` (T402 closure); queue proposal; retained T351 keyboard/KBC lifecycle evidence; IBM PC/AT and Intel 8042 material for observable controller contract; local references only as labelled ordering guidance. |
| Candidate Proposal | [Keyboard serial endpoint and 8042 flow-control repair](../proposals/m5-keyboard-serial-endpoint-and-8042-flow-control.md). |
| Files And ABI Surface | Core KBC/keyboard private owner, VM session ingress retry behavior only if endpoint semantics require it, focused Core/VM/product regressions, T403 evidence/index/history/CURRENT; no profile-specific Core path. |
| Applicable Rules | Core owns shared keyboard state/output arbitration; VM maps host transitions only; atomic admission prevents partial state publication; preserve 60h/64h, IRQ1/IRQ12 and command/AUX contracts; source tiers and no physical overclaim. |
| Verification | Producer/consumer inventory; focused Set-1/Set-2/translation/break/typematic/endpoint-full/output-full/reset/command/AUX tests; VM ingress and DOS keyboard regressions; full current CTest, documentation governance and actual-diff review. |
| Expected Markers | `M5:T403:S1:KBC-FLOW` plus sequence-atomicity, break-before-typematic and ordered-output markers; explicit physical transfer. |
| Asset Needs | No new asset. Read-only primary/reference documents only; no ROM, firmware, media or third-party source enters NXVM. |
| Reporting Requirements | Record the complete producer/consumer matrix, one owner for every pending byte/state transition, source tier, regression proof and retained physical timing receiver. |
| Stop Conditions | Stop and transfer if a required behavior needs unverified firmware emulation, a physical parameter, a Core/VM public interface change, or an ordering decision not supported by the selected generic/primary contract. |
| Exit Criteria | Every selected keyboard/8042 producer/consumer and lifecycle variant is fixed, conforming, explicitly reference-derived or transferred; valid mapped breaks cannot be lost through CPU-output congestion; no physical/L3 claim. |
| Original Owner Request | Implement DeskPro 386 L3 timing and all hardware gaps: original sources first, then reference implementations, then labelled generic skeletons; do not stop merely because no hardware is available. |
| Similar-Issue Sweep | Search all KBC native-input, typematic, command-response, AUX, output-read/status/IRQ, reset, VM request and platform host-ingress paths; classify every production hit and relevant focused/product regression. |
## Current Technical Baseline

- **Current developer artifact:** T395 S1 P4 `vm-0-5-0395` /
  `build/output/nxvm_0_5_0395.exe`, SHA-256
  `5DBD4B50762BAD5E393CD8EFA8A0CB7E06FD9A1304A44F8993CC64915A99D4A6`.
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
| T402 | Closed: D3PE-first functional D4 matrix accepted; the Model-40 control aperture repair is reference-labelled where primary material is incomplete, while cache/DRAM/ISA physical timing and L3 remain transferred. [Closure audit](../etc/evidence/t402-s2-d4-functional-closure-audit.md). |
| T401 | Closed: bounded four-profile CPU semantic/source-timing audit; every frozen form is proven, repaired, nonphysical-observed, rejected or transferred, without an x87, physical CPU, board or L3 claim. [Closure audit](../etc/evidence/t401-s78-task-closure-audit.md). |
| T400 | Closed: the HDC, CECG and CPU/DMA/BWAIT generic-reference ledger is exhausted; accepted logical repairs retain every DeskPro physical boundary. [Closure audit](../etc/evidence/t400-s1-task-closure-audit.md). |
| T399 | Closed: the five-batch reference-derived DeskPro L3 ledger is exhausted; B2/B3 logical contracts are accepted, B1/B4/B5 retain exact receivers, and both reference-derived and physical L3 are not ready. [Closure audit](../etc/evidence/t399-s5-task-closure-audit.md). |
| T398 | Closed: final exact 1986 DeskPro Model-40 audit is not-ready for L3; five physical/firmware boundaries retain explicit TODO receivers and require a fresh audit after repair. [Closure audit](../etc/evidence/t398-s2-task-closure-audit.md). |
| T397 | Closed: selected functional FDC/HDC/CECG/KBC-NMI/DMA routes and reset/order evidence retained; physical availability, phase and service facts transfer to explicit TODO receivers. Final DeskPro L3 may make only a ready/not-ready decision. [Closure audit](../etc/evidence/t397-s2-task-closure-audit.md). |
| T396 | Closed: finite Model-40 source/owner, six-domain, reset and callback-order ledger retained; all unsourced physical clock, phase, wait and device/firmware visibility facts transfer to the next DeskPro candidate. No physical scalar or L3 claim. [Closure audit](../etc/evidence/t396-s2-task-closure-audit.md). |
| T395 | Closed: private 12-to-64 bounded Core route growth with direct allocation/limit rollback, frozen-registration and order/decline proof. Transfers only the queued DeskPro board-level timing receiver. [Closure audit](../etc/evidence/t395-s2-task-closure-audit.md). |

## Recent Governance

- **M5 Td S104 P1:** added the owner-authorized reference-derived DeskPro timing-bridge candidate ahead of CPU audit; it preserves physical-L3 not-ready and requires a fresh independent re-audit.

- **M5 Td S103 P1:** adopted task-scale convergence and coverage governance: coverage-bearing T work requires a durable universe, batch dispositions, completion predicate and coordinator reconciliation before whole-domain or L3 acceptance.
- **M5 Td S102 P1:** reconciled accepted D4 parity work into the T386 proposal and Queue title, defined the non-preallocated remaining functional receiver order, and confirmed the existing Core/VM/profile architecture authority needs no amendment.
- **M5 Td S101 P1:** added a compact execution-entry table without creating new paths, and made coordinator semantic review explicit after structural documentation-gate success.
- **M5 Td S100 P1:** archived the closed T382 proposal, queued Model-339 current-source re-audit before DeskPro, reconciled T380 S2 history, and made host/workspace path detection reject the external-assets form while retaining guest DOS-path examples.
