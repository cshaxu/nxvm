# Project Status

## Current Work

**Active: M5 T376 S1.**

| Task | Compact progress |
| --- | --- |

## M5 T376 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New |
| Admission And Approval | Owner-approved continuous M5 L3 implementation; T375 is closed and Queue places raw-IMG sidecar fidelity next before the final 5170 audit. |
| Objective | Audit the existing raw-IMG/FDC/media-provider routes and produce the implementation-ready, owner-local sidecar lifecycle and command-gap contract for T376. |
| Non-goals | No sidecar implementation, new image/container format, guest-media/ROM import, test-only media API, generic filesystem API, guessed FDC timing, or final 5170 L3 decision. |
| Reference Baseline | T374 establishes address-mark metadata distinct from payload and ordinary raw IMG limits; T375 supplies the FDC byte/seek timing ledger; the approved raw-IMG sidecar proposal defines required capabilities. |
| Candidate Proposal | [8272A raw-IMG sidecar fidelity closure](../proposals/m5-8272a-img-sidecar-fidelity-closure.md). |
| Files And ABI Surface | Evidence/index/current/history only in S1; inspect `core/machine/media_interface.*`, `vm/machine/fdd.*`, `vm/machine/media_save.*`, FDC routes and their tests without changing production behavior. |
| Applicable Rules | Execution lifecycle, architecture single-owner/failure boundary, coding no-test-only-contract rule, source policy, and raw-IMG/sidecar proposal limits. |
| Verification | Record exact repository sweep commands and dispositions for provider ABI, FDD mount/save/remove, media atomic-save helper, FDC command dispatcher, and focused media/FDC tests; documentation governance passes. |
| Expected Markers | `M5:T375:S26:TASK-CLOSURE-TRANSFER:OK`; `M5:T376:S1:RAW-IMG-SIDECAR-INVENTORY:OK`. |
| Asset Needs | No external media or ROM is used, copied, or committed; existing source and tests are the sole S1 inputs. |
| Reporting Requirements | State the one proposed production owner, state/rollback boundary, raw-only behavior, every implementation receiver, and any material objection before code work. |
| Stop Conditions | Stop for owner direction if the existing provider cannot support an atomic media-pair lifecycle without a second format or if command requirements require a timing change outside accepted T375 rules. |
| Exit Criteria | Indexed inventory/contract maps all relevant owner routes and proposed implementation subtasks, preserves raw IMG as payload-only and identifies no unowned/two-owner lifecycle path. |
| Original Owner Request | Continue through L3 before Windows; retain pure IMG compatibility, avoid pure test APIs, and use 86Box/MAME/PCjs only as bounded secondary sources when authority is insufficient. |
| Similar-Issue Sweep | Search all production and test uses of address-mark capability, provider query/read/write/format/flush, FDD insert/remove/save, atomic media save, and FDC Deleted/Scan dispatch; classify each hit. |

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
| T375 | Closed at `07484727`: board/device phase work accepts source-backed service and logical-order contracts, transfers raw-IMG fidelity and source-exhausted physical board phases, and **does not accept Model-339 L3**. [Closure audit](../etc/evidence/t375-s26-task-closure-transfer-audit.md). |
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
