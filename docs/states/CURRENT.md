# Project Status

## Current Work

**Active: M5 T406 S1.**

## M5 T406 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New |
| Admission And Approval | Owner approved continued DeskPro L3/hardware-gap implementation using original sources first, read-only reference implementations second, and explicitly labelled generic IBM AT skeletons when no higher tier yields a mechanism; commits and pushes to `master` are standing-authorized, reaffirmed 2026-08-18. |
| Objective | Implement the Td S105 first receiver: a generic-AT virtual-time cadence for native keyboard serial bytes in the existing Core KBC owner, bound only by the Model-40 composition. |
| Non-goals | No original Compaq rate, physical/L3 claim, host-time coupling, firmware/media import, new Core/VM bypass, second scheduler/queue, command-response semantic change, FDC/HDC/CECG work, or user-facing configuration. |
| Reference Baseline | Td S105 tiered receiver ledger; T403 functional KBC closure; T405 host-build baseline. PCjs/86Box are read-only mechanism references only; actual cadence is labelled `generic-at`. |
| Candidate Proposal | [DeskPro tiered timing fallback foundation](../proposals/m5-deskpro-tiered-timing-fallback-foundation.md). |
| Files And ABI Surface | `src/core/machine/kbc.[ch]`, Core machine configuration/storage, Model-40 session composition, KBC/Model-40 tests, `docs/history/M5-T406-deskpro-generic-at-keyboard-serial-cadence.md`, `docs/etc/evidence/`, `docs/states/CURRENT.md`, and `docs/states/QUEUE.md` at closure. Existing public native-byte API retains its signature and route. |
| Applicable Rules | Architecture: Core owns reusable KBC behavior and VM composes the profile binding; no reverse dependency or duplicate path. Coding: C11/type facade, one owner and project-owned test boundary. Source policy: references read-only, no source/firmware/media import. |
| Verification | Focused KBC controller cadence proof; Model-40 composition proof; full `current-gate`; documentation governance; actual diff and similar-issue sweep over KBC serial submit/drain/reset call sites. |
| Expected Markers | `M5:T406:S1:KBC-SERIAL-CADENCE:OK` and `M5:T406:S1:CLOSURE-AUDIT:OK` |
| Asset Needs | None. No ROM, guest media, local asset path, binary, hash catalogue, or third-party source import. |
| Reporting Requirements | Record the generic-AT tier, PCjs/86Box configuration limits, cadence input/output/reset/cancellation contract, zero-cadence compatibility, affected caller sweep, gates, artifact identity/hash, and remaining physical/L3 transfer. |
| Stop Conditions | Stop and transfer if the mechanism requires a new VM-to-Core input path, host elapsed time, a fixed original-Compaq value, or modification of the KBC command/FIFO/IRQ contract beyond serial release scheduling. |
| Exit Criteria | The existing KBC serial queue releases no more than one native byte per configured virtual-time cadence, zero preserves current behavior, reset clears cadence state, Model-40 selects a labelled generic-AT cadence, all focused/full gates pass, artifact `vm-0-5-0406` is verified, and the remaining physical/L3 boundary is retained. |
| Original Owner Request | Implement DeskPro 386 L3-level timing and all hardware gaps: original sources first, reference implementations second, generic IBM AT skeletons if necessary; label all three levels and keep moving. |
| Similar-Issue Sweep | Sweep `core_machine_kbc_submit_native_byte(s)`, serial drain, KBC reset, machine peripheral advancement, and both generic/Model-40 construction paths; fix same-owner omissions or record a bounded transfer. |
## Current Technical Baseline

- **Current developer artifact:** T406 S1 P1 `vm-0-5-0406` /
  `build/output/nxvm_0_5_0406.exe`, SHA-256
  `81357370B706709088DAB0CD5F5AE42AA15006A9DD95F665DCEBE486D9B35C4E`.
  [T405 x86 GCC evidence](../etc/evidence/t405-s1-x86-gcc-host-build-compatibility.md) records 286/286 current-gate passes on both x86 and x64 hosts; this remains no L3 claim.
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
| T405 | Closed: x86 GCC host build accepts the current product after pointer-width and validated-media-offset repairs; x86 and x64 current gates both pass 286/286, with no L3 claim. [Evidence](../etc/evidence/t405-s1-x86-gcc-host-build-compatibility.md). |
| T404 | Closed: every frozen public profile/catalog/media/controller/display/input route has direct functional proof or a named physical receiver; no timing or L3 claim. [Closure audit](../etc/evidence/t404-s6-batch-b-functional-reconciliation.md). |
| T403 | Closed: shared keyboard serial endpoint prevents accepted break loss under full 8042 output; functional KBC flow is complete while physical keyboard/8042/DeskPro timing remains transferred. [Closure audit](../etc/evidence/t403-s2-keyboard-flow-control-closure-audit.md). |
| T402 | Closed: D3PE-first functional D4 matrix accepted; the Model-40 control aperture repair is reference-labelled where primary material is incomplete, while cache/DRAM/ISA physical timing and L3 remain transferred. [Closure audit](../etc/evidence/t402-s2-d4-functional-closure-audit.md). |
| T401 | Closed: bounded four-profile CPU semantic/source-timing audit; every frozen form is proven, repaired, nonphysical-observed, rejected or transferred, without an x87, physical CPU, board or L3 claim. [Closure audit](../etc/evidence/t401-s78-task-closure-audit.md). |
| T400 | Closed: the HDC, CECG and CPU/DMA/BWAIT generic-reference ledger is exhausted; accepted logical repairs retain every DeskPro physical boundary. [Closure audit](../etc/evidence/t400-s1-task-closure-audit.md). |
| T399 | Closed: the five-batch reference-derived DeskPro L3 ledger is exhausted; B2/B3 logical contracts are accepted, B1/B4/B5 retain exact receivers, and both reference-derived and physical L3 are not ready. [Closure audit](../etc/evidence/t399-s5-task-closure-audit.md). |
| T398 | Closed: final exact 1986 DeskPro Model-40 audit is not-ready for L3; five physical/firmware boundaries retain explicit TODO receivers and require a fresh audit after repair. [Closure audit](../etc/evidence/t398-s2-task-closure-audit.md). |

## Recent Governance

- **M5 Td S105 P2:** restored a usable tiered DeskPro work path, recorded the five-receiver ledger and admitted next target T406 for a labelled generic-AT KBC serial-delivery mechanism; this is not a physical/L3 acceptance.

- **M5 Td S104 P1:** added the owner-authorized reference-derived DeskPro timing-bridge candidate ahead of CPU audit; it preserves physical-L3 not-ready and requires a fresh independent re-audit.

- **M5 Td S103 P1:** adopted task-scale convergence and coverage governance: coverage-bearing T work requires a durable universe, batch dispositions, completion predicate and coordinator reconciliation before whole-domain or L3 acceptance.
- **M5 Td S102 P1:** reconciled accepted D4 parity work into the T386 proposal and Queue title, defined the non-preallocated remaining functional receiver order, and confirmed the existing Core/VM/profile architecture authority needs no amendment.
- **M5 Td S101 P1:** added a compact execution-entry table without creating new paths, and made coordinator semantic review explicit after structural documentation-gate success.
- **M5 Td S100 P1:** archived the closed T382 proposal, queued Model-339 current-source re-audit before DeskPro, reconciled T380 S2 history, and made host/workspace path detection reject the external-assets form while retaining guest DOS-path examples.
