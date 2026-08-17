# Project Status

## Current Work

## M5 T386 Progress

- **S25 accepted:** `f3701196` and `baf0a8eb` complete the external-ROM
  startup compatibility receiver.  Generic immutable-ROM aliases, bounded
  original-80386 control-register compatibility, D4 mapping/reset and shared
  PC/AT refresh wiring are proven without a profile-local CPU path.
  The optional owner-supplied replay reports only that the self-contained
  consumer is not reached.  CPU-to-PIT calibration transfers to the queued
  DeskPro board bus/device timing candidate; no timing or L3 claim follows.
  [Evidence](../etc/evidence/t386-s25-rom-alias-mechanism.md).
- **Next admission:** continue T386's selected-device functional matrix with
  the next unclosed receiver.  Current S25 packet is closed; no implementation
  work proceeds until the next approved packet is installed.
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