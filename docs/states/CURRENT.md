# Project Status

## Current Work

**Active: M5 T401.**
## M5 T401 S51 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner: user; standing approval to continue the four-profile CPU audit and fix confirmed small issues. |
| Objective | Audit 80186-plus INS/OUTS string-port forms `6Ch`-`6Fh`, consume the existing runtime/protection fixtures, and add direct four-profile lexical availability and 80386 attribute proof. |
| Non-goals | No port-provider, CPU execution/timing, Core/VM interface, DeskPro timing, or external-source change; no T401 closure. |
| Reference Baseline | `4ab13dc9` (S50 P2), T401 ledger, retained Intel references, `core-machine-port-strings-smoke`, `core-machine-80386-protected-io-timing-smoke`, and `core-machine-tss-iomap-port-smoke`. |
| Candidate Proposal | [Four-profile CPU instruction-correctness audit](../proposals/m5-four-profile-cpu-instruction-correctness-audit.md), 80186 string I/O form batch. |
| Files And ABI Surface | `tests/machine/core_machine_cpu_timing_preview_smoke.c`, `docs/states/CURRENT.md`, `docs/etc/evidence/t401-s1-four-profile-cpu-audit-ledger.md`; test/documentation only. |
| Applicable Rules | `ARCHITECTURE.md`: shared Core CPU/port transaction ownership stays unchanged; `CODING.md`: machine-owner tests; `DOCUMENT.md`/`EXECUTION.md`: lifecycle and evidence. |
| Verification | Build/run string-port, protected-I/O, TSS I/O-map and preview smokes; require their markers; run documentation governance and `git diff --check`. |
| Expected Markers | Existing port-string, protected-I/O and I/O-map markers plus `M5:T401:S51:STRING-IO-PREVIEW-PROFILES:OK`. |
| Asset Needs | None; no ROM, firmware, guest media or third-party source is read/imported. |
| Reporting Requirements | Report form/profile/attribute and permission evidence, production-discrepancy result, commits and nonphysical timing boundary. |
| Stop Conditions | Stop and transfer if a string I/O form exposes a port transaction, repeat, privilege or timing mechanism defect exceeding a bounded shared repair. |
| Exit Criteria | 8086 rejection, 80186/80286/80386 legal forms, REP and 80386 operand/address layouts are directly scanned; existing runtime/protection fixtures pass; ledger records conclusion. |
| Original Owner Request | Continue the CPU audit before later DeskPro L3 work; primary authority first, reference secondary, explicit transfers, direct repair of confirmed small issues. |
| Similar-Issue Sweep | Treat all four INS/OUTS opcodes together because they share string-port and privilege mechanisms; distinguish byte/word/dword and input/output directions through existing fixtures. |
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
| T401 S50 | Accepted: PUSH-immediate `68h`/`6Ah` audit closes four-profile availability and 80386 lexical attributes; no production discrepancy. [Ledger](../etc/evidence/t401-s1-four-profile-cpu-audit-ledger.md). |
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
