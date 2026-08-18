# Project Status

## Current Work

**Active: M5 T401.**

## M5 T401 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; M5 T401 S2. |
| Admission And Approval | Owner approved the four-profile CPU audit and its bounded shared repairs on 2026-08-17; S1 found a reproducible 80386 `0F 25` metadata/dispatch mismatch. |
| Objective | Correct the reserved `0F 25` 80386 metadata classification and prove metadata, lexical scan and execution all reject it while adjacent CR/DR/TR forms retain their current classification. |
| Non-goals | No new instruction support, CR/DR/TR behavior change, timing model, CPU profile fork, decoder rewrite, public ABI change, device work or physical/L3 claim. |
| Reference Baseline | T401 S1 ledger Batch B candidate; Intel 80386 PRM order 230985 Appendix A; current metadata, lexical scanner and `0F` dispatch table. |
| Candidate Proposal | [Four-profile CPU instruction-correctness audit and repair program](../proposals/m5-four-profile-cpu-instruction-correctness-audit.md). |
| Files And ABI Surface | `cpu_instructions.c`, a focused existing/new CPU smoke, T401 ledger and CURRENT only; private shared Core implementation, no public ABI. |
| Applicable Rules | Execution bounded corrective/actual-diff review; shared Core CPU ownership; coding/test rules; Intel primary-source requirement; documentation indexing. |
| Verification | Add direct metadata and lexeme checks for `0F 25`, execute it to the existing #UD delivery path, sweep `0F 20--24` and `0F 26` metadata, run focused CTest, documentation governance and diff check. |
| Expected Markers | Existing #UD delivery marker plus a new focused `M5:T401:S2:0F25-METADATA:OK` marker. |
| Asset Needs | None. No ROM, guest media, Microsoft component or external binary is needed. |
| Reporting Requirements | Record exact Intel encoding disposition, current owner, before/after metadata result, regression proof, adjacent-form sweep and retained nonphysical timing boundary. |
| Stop Conditions | Stop if the primary encoding disposition conflicts with the 80386 manual, the repair affects a different instruction family, or the necessary validation cannot remain at the shared metadata owner. |
| Exit Criteria | `0F 25` is invalid in metadata/lexeme/execution, all adjacent intended CR/DR/TR encodings retain their intended 80386 disposition, and focused proof/gates pass. |
| Original Owner Request | Continue the CPU audit after DeskPro timing work, repairing discovered defects instead of leaving them for later. |
| Similar-Issue Sweep | Inspect the entire `0F 20--26` metadata branch, its dispatch entries, CR/DR/TR handlers and all metadata/lexeme consumers for other range-based reserved-opcode admissions. |

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
| T401 S1 | Accepted: frozen CPU decoder-space ledger and Batch A system-control reconciliation found `0F 25` metadata/dispatch mismatch; S2 owns the bounded repair. [Ledger](../etc/evidence/t401-s1-four-profile-cpu-audit-ledger.md). |
| T400 | Closed: the HDC, CECG and CPU/DMA/BWAIT generic-reference ledger is exhausted; accepted logical repairs retain every DeskPro physical boundary. [Closure audit](../etc/evidence/t400-s1-task-closure-audit.md). |
| T399 | Closed: the five-batch reference-derived DeskPro L3 ledger is exhausted; B2/B3 logical contracts are accepted, B1/B4/B5 retain exact receivers, and both reference-derived and physical L3 are not ready. [Closure audit](../etc/evidence/t399-s5-task-closure-audit.md). |
| T398 | Closed: final exact 1986 DeskPro Model-40 audit is not-ready for L3; five physical/firmware boundaries retain explicit TODO receivers and require a fresh audit after repair. [Closure audit](../etc/evidence/t398-s2-task-closure-audit.md). |
| T397 | Closed: selected functional FDC/HDC/CECG/KBC-NMI/DMA routes and reset/order evidence retained; physical availability, phase and service facts transfer to explicit TODO receivers. Final DeskPro L3 may make only a ready/not-ready decision. [Closure audit](../etc/evidence/t397-s2-task-closure-audit.md). |
| T396 | Closed: finite Model-40 source/owner, six-domain, reset and callback-order ledger retained; all unsourced physical clock, phase, wait and device/firmware visibility facts transfer to the next DeskPro candidate. No physical scalar or L3 claim. [Closure audit](../etc/evidence/t396-s2-task-closure-audit.md). |
| T395 | Closed: private 12-to-64 bounded Core route growth with direct allocation/limit rollback, frozen-registration and order/decline proof. Transfers only the queued DeskPro board-level timing receiver. [Closure audit](../etc/evidence/t395-s2-task-closure-audit.md). |
| T394 | Closed: S1-S7 establish the complete C0 81-key qualification and Core copied-descriptor safety boundary, then transfer physical selection solely to the queued source-backed DeskPro board clock-plan contract. No physical clock or L3 claim. [Closure audit](../etc/evidence/t394-s7-physical-qualification-closure-audit.md). |
| T392 | Closed: S1-S5 exhaust copied C1, VM/session and FDC operation candidates, then repair the missed proposal archival and stale Queue entry; no endpoint or defect exists. Only a separately admitted generic immutable device-operation snapshot plus selected consumer contract can proceed. No Core/VM ABI, CPU timing, physical, board, firmware or L3 claim. [Corrective audit](../etc/evidence/t392-s5-closure-topology-corrective-audit.md). |

## Recent Governance

- **M5 Td S104 P1:** added the owner-authorized reference-derived DeskPro timing-bridge candidate ahead of CPU audit; it preserves physical-L3 not-ready and requires a fresh independent re-audit.

- **M5 Td S103 P1:** adopted task-scale convergence and coverage governance: coverage-bearing T work requires a durable universe, batch dispositions, completion predicate and coordinator reconciliation before whole-domain or L3 acceptance.
- **M5 Td S102 P1:** reconciled accepted D4 parity work into the T386 proposal and Queue title, defined the non-preallocated remaining functional receiver order, and confirmed the existing Core/VM/profile architecture authority needs no amendment.
- **M5 Td S101 P1:** added a compact execution-entry table without creating new paths, and made coordinator semantic review explicit after structural documentation-gate success.
- **M5 Td S100 P1:** archived the closed T382 proposal, queued Model-339 current-source re-audit before DeskPro, reconciled T380 S2 history, and made host/workspace path detection reject the external-assets form while retaining guest DOS-path examples.
