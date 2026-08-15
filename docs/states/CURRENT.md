# Project Status

## Current Work

**Active: M5 T376 S2.**

| Task | Compact progress |
| --- | --- |

## M5 T376 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner-approved continuous M5 L3 implementation; accepted T376 S1 selects VM FDD/media-save as the sole sidecar and paired-persistence owner. |
| Objective | Implement the optional raw-IMG same-basename sidecar lifecycle: bounded versioned metadata, mount validation, FDD address-mark state and failure-preserving raw-plus-sidecar persistence. |
| Non-goals | No new image/container format, guest-media/ROM commit, generic filesystem API, test-only media operation, FDC Deleted/Scan command implementation, guessed timing, or final L3 decision. |
| Reference Baseline | T376 S1 owner/lifecycle inventory, frozen core media address-mark ABI, existing VM FDD raw-image lifecycle and `vm_machine_media_save_atomically` behavior. |
| Candidate Proposal | [8272A raw-IMG sidecar fidelity closure](../proposals/m5-8272a-img-sidecar-fidelity-closure.md). |
| Files And ABI Surface | `vm/machine/fdd.*`, `vm/machine/media_save.*`, VM media regression and CMake registration, plus evidence/index/current/history. Core public media ABI must stay unchanged. |
| Applicable Rules | One VM FDD owner, failure-atomic media-pair lifecycle, raw IMG payload-only compatibility, no test-only public contract, existing core provider and T375 cadence preservation. |
| Verification | Focused VM media regression proves absent/valid/malformed/stale sidecar, mark persistence, read-only rejection, mount rollback and paired-save failure preservation; current GCC build/test and documentation governance pass. |
| Expected Markers | `M5:T376:S1:RAW-IMG-SIDECAR-INVENTORY:OK`; `M5:T376:S2:RAW-IMG-SIDECAR-LIFECYCLE:OK`. |
| Asset Needs | Tests generate only ignored temporary raw/sidecar files; no external ROM, guest media or third-party source is imported or committed. |
| Reporting Requirements | Report schema, raw-only behavior, exact commit/rollback boundary, changed owner surface, focused proof and retained command receivers. |
| Stop Conditions | Stop for owner direction if portable paired replacement cannot retain recovery semantics, if schema needs sector payload/second format, or if the frozen provider ABI cannot express the required lifecycle. |
| Exit Criteria | FDD provider advertises/address-marks only for valid mounted sidecar-or-default mark state; ordinary raw IMG remains usable without a sidecar; all specified failure paths preserve guest-visible old media and focused regressions pass. |
| Original Owner Request | Continue through L3 before Windows; retain pure IMG compatibility, avoid pure test APIs, and keep ROM/media external to Git. |
| Similar-Issue Sweep | Inspect every FDD init/reset/finalize/replace/insert/remove/provider route, all media-save callers and VM media tests; classify HDD as single-file non-sidecar and leave it unchanged. |

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
| T376 S1 | Accepted P1 `0896739e`: raw-IMG sidecar inventory establishes VM FDD/media-save as the sole metadata and paired-persistence owner; core media ABI and T375 cadence stay unchanged. [Evidence](../etc/evidence/t376-s1-raw-img-sidecar-inventory.md). |
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
