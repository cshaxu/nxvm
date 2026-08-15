# Project Status

## Current Work

**Active: M5 T378 S1.**

| Task | Compact progress |
| --- | --- |
| T377 | Closed at `1cdd5110`: Model-339 is not L3-ready until the native input/global-gate repair and independent re-audit complete. [Audit](../etc/evidence/t377-s1-model339-final-l3-audit.md). |

## M5 T378 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New |
| Admission And Approval | Owner approved continuous implementation through L3 before Windows and explicitly approved production-native input rather than test-only APIs; T377 closed at `1cdd5110` with this Queue-head repair receiver. |
| Objective | Reconcile the native Win32 keyboard/mouse production-ingress proof with its actual layered transport (rather than force a forbidden direct call), repair the real virtual-time/composition dependency inversion, and preserve the Model-339 IRQ1-only input selection. |
| Non-goals | No test-only injection API, generic AT/clone expansion, AUX/IRQ12 enablement in Model-339, ROM/media work, physical timing claim, Model-339 L3 decision, or Windows work. |
| Reference Baseline | T377 negative audit and repair proposal; current keyboard/AUX ingress and dependency-gate scripts; Model-339 composition/input contracts. Initial source review proves Win32 already submits through the platform host-input sink into composition's input source, while the static tests are stale literal checks. |
| Candidate Proposal | [Model-339 preclosure input and global-gate repair](../proposals/m5-model339-preclosure-gate-repair.md). |
| Files And ABI Surface | Native Win32 input adapter, existing production transport/composition boundary, focused tests/CMake evidence/history/current/queue status; no public test-only ABI. |
| Applicable Rules | One production ingress owner, platform adapters cannot mutate guest state directly, static verification must prove the layered production route rather than prescribe a forbidden direct call, dependency flow toward neutral capability, no raw cross-module mutable state, validation/reset/cancellation sweep, and no second input truth source. |
| Verification | Inspect all native keyboard/mouse handlers and ingress callers; add/repair source or behavioral proof for the actual layered route; run focused input/Model-339 tests, keyboard/AUX/dependency static gates, full current gate, documentation governance and diff review. |
| Expected Markers | Existing transport/model markers plus passing keyboard-ingress, AUX-mouse and dependency-DAG gates; no artificial test-only marker. |
| Asset Needs | None; no ROM, guest media, external code or external runtime. |
| Reporting Requirements | Record ingress caller/handler/rollback matrix, Model-339 IRQ1-only retention, AUX product-boundary retention, dependency repair reasoning, full-gate result and remaining receiver. |
| Stop Conditions | Stop for owner direction if repair needs a new public API, changes Model-339 device selection, alters guest-visible timing/ABI, imports material, or exposes another unowned ingress mechanism. |
| Exit Criteria | All three failing gates pass without a direct platform-to-core input bypass; focused reset/cancellation and Model-339 selection regressions pass; remaining L3 decision transfers only to the queued independent re-audit. |
| Original Owner Request | Continue through L3 before Windows; use native/production input rather than pure test APIs, and do not accept Model-339 while a real production route is incomplete. |
| Similar-Issue Sweep | Search all Win32/Linux/native keyboard/mouse handlers, transport ingress operations, direct controller injections, platform time/composition includes and static-gate references; classify every hit. |

## Current Technical Baseline

- **Current developer artifact:** T376 S4 `vm-0-5-0376` /
  `build/output/nxvm_0_5_0376.exe`; its SHA-256 and source commit are
  recorded by this S4's acceptance record.
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
| T377 | Closed at `1cdd5110`: final audit rejects premature Model-339 L3 acceptance; its shared input/global-gate repair and independent re-audit successors are queued. [Audit](../etc/evidence/t377-s1-model339-final-l3-audit.md). |
| T376 | Closed at `991060a4`: raw-IMG sidecar persistence, selected 8272A command fidelity and malformed-sidecar rejection are proven; physical duplicate-CHRN representation remains explicitly outside pure raw-IMG support. [Closure audit](../etc/evidence/t376-s6-task-closure-audit.md). |
| T375 | Closed at `07484727`: board/device phase work accepts source-backed service and logical-order contracts, transfers raw-IMG fidelity and source-exhausted physical board phases, and **does not accept Model-339 L3**. [Closure audit](../etc/evidence/t375-s26-task-closure-transfer-audit.md). |
| T374 | Closed at `f742433c`: S1--S19 complete the selected Model-339 functional closure and preserve raw-IMG 765 Deleted/Control-Mark/Scan as explicit TODO debt. Board/device timing, final Model-L3, DeskPro 386, PC/XT and Windows 3.1 remain open. [Closure audit](../etc/evidence/t374-s19-task-closure-audit.md). |
| T373 | Closed at `06246a8e`: S1--S4 freeze the three-machine capability ledger and receivers. **5170, DeskPro Model 40 and PC/XT 5160-268 L3 are not ready.** [Closure audit](../etc/evidence/t373-s4-task-closure-audit.md). |
| T372 | Closed at `3f56c72c`: Model 339 has selected logical ownership and deterministic ordering, but lacks selected-device functional closure and source-backed board/phase timing. [Closure audit](../etc/evidence/t372-s8-task-closure-transfer-audit.md). |
| T370 | Closed at `77a73c04`: selected Model-339 device-service ownership is reconciled; unavailable duration transfers to phase refinement. [Closure audit](../etc/evidence/t370-s5-planar-cga-transfer-audit.md). |

## Recent Governance

- **M5 Td S94 P1:** locks the DeskPro 386 Model 40 and PC/XT 5160-268
  baselines, establishes capability-ledger then functional-before-timing
  closure for all three machines, and retains external ROMs outside Git.
