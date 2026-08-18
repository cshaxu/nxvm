# Project Status

## Current Work

**Active: M5 T405 S1.**

## M5 T405 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New T405 S1. |
| Admission And Approval | Owner approved continued DeskPro 386 L3 and hardware-gap work, Queue/proposal updates, normal repairs, commits and pushes to `master` without repeated confirmation; standing approval reiterated 2026-08-18. This first queued candidate is admitted before the distinct 8088/PC/XT program. |
| Objective | Establish reproducible x86 GCC host-build provenance, configure an isolated 32-bit build, and determine whether the current product and current-gate suite can execute as a 32-bit host process. |
| Non-goals | No guest CPU semantic or profile change; no L3 claim; no toolchain, ROM, guest media, or third-party binary import; no 32-bit release promise; preserve the 64-bit build and artifact. |
| Reference Baseline | T404 functional product-capability closure; current developer artifact `vm-0-5-0404` at the recorded SHA-256; proposal `m5-32bit-gcc-host-build-compatibility.md`. |
| Candidate Proposal | `docs/proposals/m5-32bit-gcc-host-build-compatibility.md`. |
| Files And ABI Surface | `docs/states/CURRENT.md`, `docs/history/M5-T405-32bit-gcc-host-build-compatibility.md`, and indexed evidence only unless an observed host-width defect requires separately packeted owner-first repair. No public ABI change is planned. |
| Applicable Rules | Documentation and execution rules apply to admission and evidence; architecture and coding rules apply to any later portability repair. Host/toolchain remains an external development input, never a product dependency. |
| Verification | Record compiler version/target/path, CMake and Ninja versions, configuration command, generated executable architecture, build result, current-gate result, bounded profile session checks, 64-bit-width static sweep, and documentation governance. |
| Expected Markers | `M5:T405:S1:X86-GCC-PROVENANCE:OK`; later outcome marker is `M5:T405:S1:X86-GCC-COMPATIBILITY:{PASS|LIMITED}:OK`. |
| Asset Needs | No ROM, guest media, firmware, Microsoft material, or repository asset. The external installed x86 GCC is inspected only and is not copied or committed. |
| Reporting Requirements | Retain exact tool identity, target triple, generator, commands, executable architecture, test counts, profile-check bounds, defect/limitation dispositions, and explicit pass-versus-unavailable-toolchain distinction in indexed evidence. |
| Stop Conditions | Stop and record a bounded limitation if a reproducible GCC x86 target is unavailable, a dependency lacks a legal x86 build, a protected/machine-local asset is required, or a repair would alter a closed L3 contract. |
| Exit Criteria | A clean isolated x86 GCC configuration has either passed the required build/current-gate/profile checks with a source-backed compatibility decision, or every observed failure is bounded and fixed, explicitly rejected at its owner boundary, or transferred to a separately admitted repair. |
| Original Owner Request | Continue implementation: DeskPro 386 L3-level timing and every hardware gap; use primary sources first, reference implementations second, a labelled generic IBM AT skeleton only when necessary, and TODO only where no workable reference exists. Continue CPU audit after these prerequisites. |
| Similar-Issue Sweep | Host-width class: inspect tracked production source, tests, build descriptions and relevant task records with `rg -n -i "uintptr_t|size_t|ptrdiff_t|\\(u?int(32|64)_t\\)|%[0-9]*[duxX]|sizeof" src tests CMakeLists.txt CMakePresets.json`; classify pointer narrowing, allocation/capacity conversion, file offsets, formatting, ABI layout and dependency-architecture hits. No defect is presumed at admission. |
## Current Technical Baseline

- **Current developer artifact:** T404 S5 P1 `vm-0-5-0404` /
  `build/output/nxvm_0_5_0404.exe`, SHA-256
  `291DF614D24CF5EE4AB0C65188CBCF1815084978A94FC9C700CBCED171115469`.
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
| T404 | Closed: every frozen public profile/catalog/media/controller/display/input route has direct functional proof or a named physical receiver; no timing or L3 claim. [Closure audit](../etc/evidence/t404-s6-batch-b-functional-reconciliation.md). |
| T403 | Closed: shared keyboard serial endpoint prevents accepted break loss under full 8042 output; functional KBC flow is complete while physical keyboard/8042/DeskPro timing remains transferred. [Closure audit](../etc/evidence/t403-s2-keyboard-flow-control-closure-audit.md). |
| T402 | Closed: D3PE-first functional D4 matrix accepted; the Model-40 control aperture repair is reference-labelled where primary material is incomplete, while cache/DRAM/ISA physical timing and L3 remain transferred. [Closure audit](../etc/evidence/t402-s2-d4-functional-closure-audit.md). |
| T401 | Closed: bounded four-profile CPU semantic/source-timing audit; every frozen form is proven, repaired, nonphysical-observed, rejected or transferred, without an x87, physical CPU, board or L3 claim. [Closure audit](../etc/evidence/t401-s78-task-closure-audit.md). |
| T400 | Closed: the HDC, CECG and CPU/DMA/BWAIT generic-reference ledger is exhausted; accepted logical repairs retain every DeskPro physical boundary. [Closure audit](../etc/evidence/t400-s1-task-closure-audit.md). |
| T399 | Closed: the five-batch reference-derived DeskPro L3 ledger is exhausted; B2/B3 logical contracts are accepted, B1/B4/B5 retain exact receivers, and both reference-derived and physical L3 are not ready. [Closure audit](../etc/evidence/t399-s5-task-closure-audit.md). |
| T398 | Closed: final exact 1986 DeskPro Model-40 audit is not-ready for L3; five physical/firmware boundaries retain explicit TODO receivers and require a fresh audit after repair. [Closure audit](../etc/evidence/t398-s2-task-closure-audit.md). |
| T397 | Closed: selected functional FDC/HDC/CECG/KBC-NMI/DMA routes and reset/order evidence retained; physical availability, phase and service facts transfer to explicit TODO receivers. Final DeskPro L3 may make only a ready/not-ready decision. [Closure audit](../etc/evidence/t397-s2-task-closure-audit.md). |

## Recent Governance

- **M5 Td S104 P1:** added the owner-authorized reference-derived DeskPro timing-bridge candidate ahead of CPU audit; it preserves physical-L3 not-ready and requires a fresh independent re-audit.

- **M5 Td S103 P1:** adopted task-scale convergence and coverage governance: coverage-bearing T work requires a durable universe, batch dispositions, completion predicate and coordinator reconciliation before whole-domain or L3 acceptance.
- **M5 Td S102 P1:** reconciled accepted D4 parity work into the T386 proposal and Queue title, defined the non-preallocated remaining functional receiver order, and confirmed the existing Core/VM/profile architecture authority needs no amendment.
- **M5 Td S101 P1:** added a compact execution-entry table without creating new paths, and made coordinator semantic review explicit after structural documentation-gate success.
- **M5 Td S100 P1:** archived the closed T382 proposal, queued Model-339 current-source re-audit before DeskPro, reconciled T380 S2 history, and made host/workspace path detection reject the external-assets form while retaining guest DOS-path examples.
