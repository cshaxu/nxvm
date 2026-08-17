# Project Status

## Current Work

## M5 T386 S25 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; T386 remains open after accepted S24 logical-FDC receiver. |
| Admission And Approval | Owner approved on 2026-08-16 to rapidly close S24, perform compact proposal/Queue/architecture governance, then continue original DeskPro 386 development. Scope is the newly explicit external-ROM board-startup compatibility receiver; no timing/L3 claim or architecture exception. On 2026-08-16 the owner additionally approved one local-only classification of the first external-ROM `#UD`, solely to identify and repair its shared CPU owner under the Intel 80386 Programmer's Reference Manual; the classification instead identifies a documented-strict but original-80386-compatible `MOV CR` MOD-field behavior: strict 80386 remains the default, while a Model-40 declaration selects the bounded shared-Core silicon behavior; DR/TR remain strict. A separately identified valid protected-to-real-mode `MOV CR0` transition also needed a shared-Core repair. The owner then approved the source-backed D4 ROM-replacement, compatibility-RAM and write-protect state-machine repair. After its neutral BYOB replay remained at `#UD`, the owner approved one additional local-only `#UD` classification under the same Intel 80386 Programmer's Reference Manual boundary. No ROM bytes, disassembly, paths, hashes or derived source may enter Git. |
| Objective | Identify with a bounded external-ROM trace the earliest shared or Model-40 board owner that prevents the selected Model-40 ROM from reaching the self-contained 1.2-MB boot-sector consumer; repair only that owner and prove the same BYOB replay reaches the consumer without weakening S24 FDC behavior. The proven owners are the Model-40 D4 compatibility-RAM/ROM-replacement state, the bounded shared 80386 control-register capability, and the reusable PC/AT PIT-counter-1 to port-61h refresh wiring/reset state. Clock-domain calibration remains explicitly unowned by this S. |
| Non-goals | No firmware/media import; no decoding or committing firmware bytes, paths or hashes; no profile-local CPU fork; no HDC/CECG work; no flux/CRC/index/rotation/physical-CHRN semantics; no board waits, service durations, arbitration, generic variants, device-timing or L3 claim. Do not select an instruction-to-PIT ratio or derive a physical timing value. Do not add a test-only production API. Apart from the owner-approved initial and one additional `#UD` classifications, do not decode or derive external-ROM source. |
| Reference Baseline | Accepted S24 logical-FDC receiver `79e365af` / `4397da92` / `cd89901d` plus S24 closure governance; current developer artifact `vm-0-5-0389` SHA-256 `197BC89BD2476F10D34559DC4EB71A35EEF7F6C58DFA8ECFEE9F8F592817836D`; external-ROM observation is a transient owner-supplied test input under source policy. |
| Candidate Proposal | [DeskPro functional closure](../proposals/m5-deskpro-386-model40-device-functional-closure.md), external-ROM board-startup receiver following the S24 selected FDC logical receiver. |
| Files And ABI Surface | Core immutable-ROM lifecycle, bounded shared 80386 execution and reusable PC/AT PIT/port-B wiring only where their independently sourced contracts require it; VM Model-40 D4 compatibility-RAM/ROM-replacement/reset mapping; existing synthetic and BYOB smoke/evidence/CURRENT/proposal. No public Console/YAML or test-only ABI. |
| Applicable Rules | Task Reading Set; EXECUTION lifecycle/mechanism-defect/closure rules; ARCHITECTURE shared-owner/opaque-boundary invariants; CODING C11/cohesion/test-boundary rules; source policy transient-research/no-import rule; DOCUMENT at closure. |
| Verification | Preserve the S24 focused FDC/DMA smoke. Prove the D4 reset state, low replacement selection, write-protection state and reset-visible compatibility mapping with project-owned synthetic bytes. Use optional BYOB invocation with owner-supplied ROM and, when needed, the owner-supplied 1.2-MB DOS boot image only transiently; record neutral execution observations without firmware or guest-media source data; use the Intel 80386 Programmer's Reference Manual as the primary contract for the owner-approved single `#UD` classification; add focused regressions at each resolved production owner, including strict-default isolation and both Model-40/planar PC/AT refresh wiring, and prove the boot-sector marker through the FDC/DMA path or transfer the calibrated CPU-to-PIT prerequisite to the queued board-timing receiver. Run applicable ownership, specialized and current gates before acceptance. |
| Expected Markers | Retain S24 `M5:T386:S24:FDC-12MB-LOGICAL:OK`, `M5:T386:S24:FDC-DMA2-IRQ6:OK`, `M5:T386:S24:MODEL40-FDC-BINDING:OK`; P1 adds Core-only `M5:T386:S25:ROM-ALIAS-LIFECYCLE:OK`, which is explicitly not Model-40 decode proof; add synthetic D4 compatibility/reset and PC/AT refresh-clock markers; retain the S25 consumer marker only after the consumer is reached. |
| Asset Needs | Transient owner-supplied ROM and 1.2-MB DOS boot image only. No firmware, guest media, local path, vendor hash, source text, derived binary or third-party code enters Git. |
| Reporting Requirements | Report the first trace-backed failing boundary, owner decision and alternative-owner elimination; report FDC preservation, BYOB consumer result, focused/current gates, artifact identity, and HDC/CECG/timing transfer. Coordinator independently reviews all P changes before acceptance. |
| Stop Conditions | Stop for any external-ROM analysis beyond the owner-approved initial and one additional `#UD` classifications, a required physical timing inference, or a required test-only product operation. The unresolved calibrated CPU-to-PIT clock-domain relation transfers to the queued DeskPro board bus/device timing candidate; do not guess a ratio. |
| Exit Criteria | The same external ROM reaches the self-contained boot sector through production FDC/DMA, or a source-backed impossible/unsupported prerequisite is explicitly transferred with owner approval; one earliest production owner is proven, no profile-local CPU fork appears, S24 FDC proof remains green, and required gates/evidence complete without timing or L3 overclaim. |
| Original Owner Request | Owner-approved request, 2026-08-16: quickly close current S, perform small necessary proposal/Queue/architecture governance, then continue original DeskPro development. |
| Similar-Issue Sweep | Sweep reset-vector/ROM alias and immutable-map priority, 80386 exception delivery and diagnostic flow, Model-40 D4 startup mapping/control, unmapped board I/O and reset defaults, ROM composition, and both default/Model-40 isolation. |

## Current Technical Baseline

- **Current developer artifact:** T386 S23 `vm-0-5-0389` /
  `build/output/nxvm_0_5_0389.exe`, SHA-256
  `197BC89BD2476F10D34559DC4EB71A35EEF7F6C58DFA8ECFEE9F8F592817836D`.
  S24 preserves the artifact and retains its selected 1.2-MB logical-FDC proof
  in [S24 evidence](../etc/evidence/t386-s24-model40-12mb-fdc.md).
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
| T386 S24 | Accepted logical-FDC receiver: the fixed 1.2-MB raw-IMG geometry, selected 765A command/state/error/reset behavior and production DMA2/IRQ6 path are proven. An external ROM initializes but does not reach the boot-sector consumer; S25 owns trace-backed startup compatibility without a CPU pre-judgment. [Evidence](../etc/evidence/t386-s24-model40-12mb-fdc.md). |
| T386 S23 | Accepted: D4-SKEY uses the existing 8042 `D1h` output-port A20/reset owner, while a failed CPU double fault produces shutdown that only a D4-configured board consumes through the existing cold-reset lifecycle. Non-D4 terminal behavior remains isolated; FDC/HDC/CECG receivers and timing remain open. [Evidence](../etc/evidence/t386-s23-d4-skey-reset-arbitration.md). |
| T385 | Closed: the selected Model-40 80386DX-16 consumes the shared 80386 CPU/state/retirement owners without a DeskPro CPU fork; a clean current-source 251/251 gate passed, while functionality and board timing transfer unchanged. [Closure audit](../etc/evidence/t385-s1-task-closure-audit.md). |
| T384 | Closed: the Model-40 capability audit freezes its original Compaq system board/storage ledger and owner-selected color EGA, keyboard and external-ROM constraint; its shared-VADP IBM/Compaq personality boundary transfers CPU, functional and timing work without making a runnable or L3 claim. [Closure audit](../etc/evidence/t384-s2-task-closure-audit.md). |
| T383 | Closed: independent current-source replay accepts the frozen Model-339 deterministic L3 contract after T380's strict-start repair; 251/251 current gate and focused matrix are retained in [evidence](../etc/evidence/t383-s1-model339-current-source-reaudit.md). |
| T382 | Reclosed at S8: the prior S1--S7 closure is retained historically, while S8 corrects its Console 64-bit transport and final artifact baseline. [Corrective audit](../etc/evidence/t382-s8-reclosure-audit.md). |

## Recent Governance

- **M5 Td S102 P1:** reconciled accepted D4 parity work into the T386 proposal and Queue title, defined the non-preallocated remaining functional receiver order, and confirmed the existing Core/VM/profile architecture authority needs no amendment.
- **M5 Td S101 P1:** added a compact execution-entry table without creating new paths, and made coordinator semantic review explicit after structural documentation-gate success.
- **M5 Td S100 P1:** archived the closed T382 proposal, queued Model-339 current-source re-audit before DeskPro, reconciled T380 S2 history, and made host/workspace path detection reject the external-assets form while retaining guest DOS-path examples.