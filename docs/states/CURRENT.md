# Project Status

## Current Work

**Active: M5 T375 S19.**

| Task | Compact progress |
| --- | --- |
| T375 S18 | Accepted P1 `8a4487e1`: explicit virtual-time publication emits a distinct core trace event; persistent replay and Model-339 physical timing remain open. [Evidence](../etc/evidence/t375-s18-external-time-trace.md). |

## M5 T375 S19 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Project owner approved continuing the ordered L3 program, the Type-3 Model-339 aftermarket 1.44 MB compatibility route, and 86Box/MAME/PCjs only as secondary behavior references. The owner also approved use of local/external ROMs outside Git. This S admits no ROM, guest media, or external code. |
| Objective | Establish the source-labelled FDC service-time inputs and reference-model crosswalk needed to replace the current one-readiness-tick pending-command/pending-completion progression with elapsed-time scheduling at the existing FDC owner. |
| Non-goals | No claim that a 1.44 MB drive was factory-installed in Model 339; no unnamed replacement controller or drive; no import or transliteration of 86Box code; no physical waveform or IBM board scalar; no raw-IMG Deleted/Control-Mark/Scan repair; no final Model-339 L3 conclusion. |
| Reference Baseline | Accepted T374 functional FDC closure, T375 S1 timing inventory, T375 S17 virtual-time batches, and T375 S18 external-time trace. Current `fdc.c` advances one pending command or completion per readiness timeline tick. |
| Candidate Proposal | [IBM PC/AT 5170 board and device phase-timing closure](../proposals/m5-5170-board-phase-timing-closure.md). |
| Files And ABI Surface | `docs/etc/evidence/t375-s19-fdc-service-time-crosswalk.md`, `docs/etc/README.md`, and this packet only for P1. No production ABI or runtime surface changes in this evidence S. A later S owns any elapsed-time FDC scheduler and its focused proof. |
| Applicable Rules | `docs/rules/EXECUTION.md` lifecycle and P review; `docs/rules/DOCUMENT.md` authority/index topology; `docs/etc/operations/policy/source-policy.md` external-source containment. Architecture and coding rules are not triggered because P1 changes no code. |
| Verification | Inspect every command service path and external 86Box v6.0 revision `4fef696a` FDC timer call without importing code; verify document links/index and run `tools/Verify-DocumentationGovernance.ps1 -RepositoryRoot .`. |
| Expected Markers | `Documentation governance verification passed.` and `M5:T375:S19:FDC-SERVICE-CROSSWALK:OK` recorded in the evidence review. |
| Asset Needs | Read-only external 86Box source at `O:\repos.external\86box-source`; no firmware, ROM, disk image, guest program, trace, or external binary is used or retained. |
| Reporting Requirements | Report the crosswalk result, exact non-admissions, pushed P1, governance result, and whether the next S may implement an elapsed-time scheduler. |
| Stop Conditions | Stop and return for owner direction if a required Model-339 board/controller/drive identity is missing for a proposed scalar, if the evidence would require importing protected material, or if a candidate conflicts with the fixed no-factory-1.44-MB boundary. |
| Exit Criteria | Durable evidence distinguishes primary component facts from 86Box behavior, identifies every existing pending transition and its timing input or transfer, records the eligible subsequent scheduler scope, passes documentation governance, and contains no external source/media. |
| Original Owner Request | Continue the current task toward full L3 accuracy; use 86Box/MAME/PCjs as reference behavior only where authoritative manuals give a range or no deterministic timing; keep the 1.44 MB route a documented aftermarket upgrade, with assets external to Git. |
| Similar-Issue Sweep | Inspect FDC command acceptance, reset, seek/recalibrate, transfer/DRQ, completion/IRQ6, Ready polling, DMA2 handoff, and the HDC deferred-service contrast. Transfer all unmodelled mechanical/rotational and raw-format semantics instead of concealing them in a generic delay. |

## Current Technical Baseline

- **Current developer artifact:** T369 S4 `vm-0-5-0369` /
  `build/output/nxvm_0_5_0369.exe`; its SHA-256 and source commit are
  recorded in the T369 S4 closure audit.
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
| T375 S18 | Accepted P1 `8a4487e1`: explicit virtual-time publication emits a distinct core trace event; persistent replay and Model-339 physical timing remain open. [Evidence](../etc/evidence/t375-s18-external-time-trace.md). |
| T374 | Closed at `f742433c`: S1--S19 complete the selected Model-339 functional closure and preserve raw-IMG 765 Deleted/Control-Mark/Scan as explicit TODO debt. Board/device timing, final Model-L3, DeskPro 386, PC/XT and Windows 3.1 remain open. [Closure audit](../etc/evidence/t374-s19-task-closure-audit.md). |
| T373 | Closed at `06246a8e`: S1--S4 freeze the three-machine capability ledger and receivers. **5170, DeskPro Model 40 and PC/XT 5160-268 L3 are not ready.** [Closure audit](../etc/evidence/t373-s4-task-closure-audit.md). |
| T372 | Closed at `3f56c72c`: Model 339 has selected logical ownership and deterministic ordering, but lacks selected-device functional closure and source-backed board/phase timing. [Closure audit](../etc/evidence/t372-s8-task-closure-transfer-audit.md). |
| T370 | Closed at `77a73c04`: selected Model-339 device-service ownership is reconciled; unavailable duration transfers to phase refinement. [Closure audit](../etc/evidence/t370-s5-planar-cga-transfer-audit.md). |
| T369 | Closed at `08a64bea`: accepted 5170 bus-stage input, logical 80286 DMA handoff and selected FDC/PIC visibility; board/device timing remains open. [Closure audit](../etc/evidence/t369-s4-pcat-bus-stage-closure-audit.md). |
| T368 | Closed at `4da84be8`: source-labelled 80286 successful-retirement timing closes; CPU waits, bus/device timing and IBM 5170 L3 remain open. [Closure audit](../etc/evidence/t368-s7-80286-retirement-closure-audit.md). |
| T367 | Closed at `f60d87ea`: VM owns concrete machine selection and CPU/timing contract binding; no CPU-timing, bus, device or L3 receiver closes. [Binding evidence](../etc/evidence/t367-s2-vm-profile-contract-binding.md). |

## Recent Governance

- **M5 Td S94 P1:** locks the DeskPro 386 Model 40 and PC/XT 5160-268
  baselines, establishes capability-ledger then functional-before-timing
  closure for all three machines, and retains external ROMs outside Git.
