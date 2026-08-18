# Project Status

## Current Work

**Active: M5 T402.**
## M5 T402 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New |
| Admission And Approval | Owner: user; explicit continuing authorization to implement DeskPro 386 L3 timing and hardware gaps with original-source, reference-derived, then labelled generic tiers; user also directs direct correction of confirmed small issues. |
| Objective | Freeze and reconcile the original 1986 DeskPro D4 memory-control functional matrix against the Model-40 profile; implement every confirmed bounded mapping/control discrepancy without changing Core/VM interface ownership. |
| Non-goals | No physical D4 DRAM/ISA/cache timing scalar, L3 claim, firmware/media import, generic AT substitution, copied third-party source or public Core/VM ABI change. |
| Reference Baseline | `0e13e676` (T401 closure); D3PE 1986 primary material in approved external research archive; T386/T390 D4 evidence; local read-only 86Box `compaq_386.c` as Tier 2 only. |
| Candidate Proposal | [DeskPro D4 memory-controller reconciliation](../proposals/m5-deskpro-386-d4-memory-controller-reconciliation.md). |
| Files And ABI Surface | Model-40 profile D4 owner, focused D4 regression(s), T402 evidence/index/history/CURRENT; Core interfaces only if a demonstrated shared owner requires it. |
| Applicable Rules | Original source controls board facts; Tier 2 is labelled and cannot override primary material; preserve Core/VM boundary and explicit physical timing transfer. |
| Verification | Build/run focused D4 map and parity smokes plus any new matrix smoke; static owner sweep; full current CTest, documentation governance and diff review. |
| Expected Markers | `M5:T402:S1:D4-MATRIX` plus focused functional mapping/control markers and an explicit physical-transfer statement. |
| Asset Needs | Read-only approved research materials only; no ROM, media, trace or third-party source enters NXVM. |
| Reporting Requirements | Report the complete D4 member matrix, each evidence tier, every confirmed repair and every retained physical receiver. |
| Stop Conditions | Stop and transfer a member if neither original material nor a bounded labelled Tier-2 reference determines it, or if it requires a new timing owner/interface. |
| Exit Criteria | Every selected D4 functional member is conforming, fixed, explicitly Tier-2-derived, rejected or transferred; no physical or L3 overclaim. |
| Original Owner Request | Implement DeskPro 386 L3 timing and all hardware gaps using original sources, then reference implementations, then explicitly labelled generic fallback; do not stop merely for lack of physical hardware. |
| Similar-Issue Sweep | Search all Model-40 D4 mapping/control registrations, aliases, reset paths, memory queries and focused smokes; classify each production hit and named D4 register/window variant. |

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
| T401 | Closed: bounded four-profile CPU semantic/source-timing audit; every frozen form is proven, repaired, nonphysical-observed, rejected or transferred, without an x87, physical CPU, board or L3 claim. [Closure audit](../etc/evidence/t401-s78-task-closure-audit.md). |
| T400 | Closed: the HDC, CECG and CPU/DMA/BWAIT generic-reference ledger is exhausted; accepted logical repairs retain every DeskPro physical boundary. [Closure audit](../etc/evidence/t400-s1-task-closure-audit.md). |
| T399 | Closed: the five-batch reference-derived DeskPro L3 ledger is exhausted; B2/B3 logical contracts are accepted, B1/B4/B5 retain exact receivers, and both reference-derived and physical L3 are not ready. [Closure audit](../etc/evidence/t399-s5-task-closure-audit.md). |
| T398 | Closed: final exact 1986 DeskPro Model-40 audit is not-ready for L3; five physical/firmware boundaries retain explicit TODO receivers and require a fresh audit after repair. [Closure audit](../etc/evidence/t398-s2-task-closure-audit.md). |
| T397 | Closed: selected functional FDC/HDC/CECG/KBC-NMI/DMA routes and reset/order evidence retained; physical availability, phase and service facts transfer to explicit TODO receivers. Final DeskPro L3 may make only a ready/not-ready decision. [Closure audit](../etc/evidence/t397-s2-task-closure-audit.md). |
| T396 | Closed: finite Model-40 source/owner, six-domain, reset and callback-order ledger retained; all unsourced physical clock, phase, wait and device/firmware visibility facts transfer to the next DeskPro candidate. No physical scalar or L3 claim. [Closure audit](../etc/evidence/t396-s2-task-closure-audit.md). |
| T395 | Closed: private 12-to-64 bounded Core route growth with direct allocation/limit rollback, frozen-registration and order/decline proof. Transfers only the queued DeskPro board-level timing receiver. [Closure audit](../etc/evidence/t395-s2-task-closure-audit.md). |
| T394 | Closed: S1-S7 establish the complete C0 81-key qualification and Core copied-descriptor safety boundary, then transfer physical selection solely to the queued source-backed DeskPro board clock-plan contract. No physical clock or L3 claim. [Closure audit](../etc/evidence/t394-s7-physical-qualification-closure-audit.md). |

## Recent Governance

- **M5 Td S104 P1:** added the owner-authorized reference-derived DeskPro timing-bridge candidate ahead of CPU audit; it preserves physical-L3 not-ready and requires a fresh independent re-audit.

- **M5 Td S103 P1:** adopted task-scale convergence and coverage governance: coverage-bearing T work requires a durable universe, batch dispositions, completion predicate and coordinator reconciliation before whole-domain or L3 acceptance.
- **M5 Td S102 P1:** reconciled accepted D4 parity work into the T386 proposal and Queue title, defined the non-preallocated remaining functional receiver order, and confirmed the existing Core/VM/profile architecture authority needs no amendment.
- **M5 Td S101 P1:** added a compact execution-entry table without creating new paths, and made coordinator semantic review explicit after structural documentation-gate success.
- **M5 Td S100 P1:** archived the closed T382 proposal, queued Model-339 current-source re-audit before DeskPro, reconciled T380 S2 history, and made host/workspace path detection reject the external-assets form while retaining guest DOS-path examples.
