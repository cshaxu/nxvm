# Project Status

## Current Work

**Active: M5 T401.**
## M5 T401 S49 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner: user; approval is the continuing instruction to complete the four-profile CPU audit and directly fix confirmed small issues. This narrow corrective replaces an over-strong accepted lexical-coverage claim without changing production semantics. |
| Objective | Correct the S44 LEA attribute component counts and add executable four-profile lexeme proof for S46 XCHG, S47 GPR MOV, and S48 segment MOV ModR/M forms; correct the current-state statement and record the correction in the T401 ledger. |
| Non-goals | No CPU execution/timing change, Core/VM interface change, DeskPro hardware/timing claim, external-source import, or task-level T401 closure. |
| Reference Baseline | `6df7c0f9` (S48 P2), current T401 four-profile ledger and existing `core-machine-cpu-timing-preview-smoke` harness. |
| Candidate Proposal | [Four-profile CPU instruction-correctness audit](../proposals/m5-four-profile-cpu-instruction-correctness-audit.md), decoder/ModR/M byte-consumption batch. |
| Files And ABI Surface | `tests/machine/core_machine_cpu_timing_preview_smoke.c`, `docs/states/CURRENT.md`, and `docs/etc/evidence/t401-s1-four-profile-cpu-audit-ledger.md`; test and documentation only, no public or production ABI surface. |
| Applicable Rules | `ARCHITECTURE.md`: shared Core ownership and no profile fork remain unchanged; `CODING.md`: test belongs to machine owner and preserves C style; `DOCUMENT.md` and `EXECUTION.md`: exact packet, ledger, actual-change review and acceptance lifecycle. |
| Verification | Configure/build `core-machine-cpu-timing-preview-smoke`; run it and require its S49 marker; run `tools/Verify-DocumentationGovernance.ps1`; inspect `git diff --check` and actual diff. |
| Expected Markers | `M5:T401:S49:MODRM-DATA-MOVE-PREVIEW-PROFILES:OK`; documentation governance pass. |
| Asset Needs | None; no ROM, firmware, guest media, external code, or asset is read or imported. |
| Reporting Requirements | Report the corrected evidence boundary, P1/P2 commits, focused verification and retained nonphysical timing boundary. |
| Stop Conditions | Stop and transfer if lexeme behavior contradicts the existing execution-audit conclusion or requires a decoder/production repair beyond a bounded correction. |
| Exit Criteria | Actual test calls cover all stated legacy profile forms plus 80386 operand/address and FS/GS selector forms; S48 current claim is corrected; ledger records the result; focused build/test and documentation gate pass. |
| Original Owner Request | Continue CPU audit before further DeskPro L3 timing work; use authoritative evidence where available, reference implementations only secondarily, retain unsupported/unknown work explicitly, and fix confirmed small issues directly. |
| Similar-Issue Sweep | Reviewed S44 through S48 after the focused executable exposed stale S44 component counts and marker-only gaps in S46 through S48; S49 supplies direct calls for the complete local set. |

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
| T401 S48 | Accepted: segment-register MOV audit closes four-profile selector and protection execution coverage; lexical proof is corrected by active S49. [Ledger](../etc/evidence/t401-s1-four-profile-cpu-audit-ledger.md). |
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
