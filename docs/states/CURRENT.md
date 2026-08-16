# Project Status

## Current Work

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation -- M5 T386 S22; latest open T386 retains accepted S21 progress. |
| Admission And Approval | Owner approved on 2026-08-16 to continue the original DeskPro 386 functional plan after the small S21 planning/proposal/Queue governance work. Scope is this bounded first D4/platform functional receiver; no exception to Core/VM ownership or source policy. |
| Objective | Close the source-backed D4 memory-parity diagnostic and IOCHK functional mechanism: reusable Core memory-parity events carry the fault location to their selected owner; the Model-40 D4 memory carrier latches byte-lane diagnostic state, publishes selected IOCHK/NMI through the existing D4 platform owner, clears IOCHK on the documented covered-memory write, and implements diagnostic/control-register read/write/reset behavior. |
| Non-goals | No D4-SKEY A20 interception, shutdown/program/power reset arbitration, physical reset pulse or bus timing, memory-cycle wait timing, firmware execution, generic variant schema, physical-media work, ROM/media import, or Board/L3 claim. Do not use a test-only injection API or reuse IBM planar port-B semantics. |
| Reference Baseline | Accepted T386 S21 `7439b99d`; artifact remains `vm-0-5-0389` SHA-256 `B0B70FA0C0A304BA9D8E1F2ADECC0E32A547DABA3FE274513B5C9DFEBA8C9483`. Primary Compaq D3PE Processor Descriptions, 1987-01-05, inspected transiently from the owner-approved 1986 technical corpus; S4, S16 and S21 evidence constrain the existing source. |
| Candidate Proposal | `docs/proposals/m5-deskpro-386-model40-device-functional-closure.md`, current S21 matrix D4/platform row. |
| Files And ABI Surface | Expected: shared Core memory-parity callback/configuration owner and its callers/tests; `src/vm/profile/model40/model40.{c,h}` and Model-40 composition only as needed for the D4 carrier; focused Core/VM Model-40 tests and CMake registration; T386 evidence/index/history/CURRENT. No Console/YAML ABI or new public test operation. |
| Applicable Rules | Task Reading Set; EXECUTION lifecycle, mechanism-defect, similar-sweep, artifact and closure rules; ARCHITECTURE shared-owner/opaque-boundary invariants; CODING C11/ASCII/cohesion/test-boundary rules; source policy transient-research/no-import rule; DOCUMENT at closure. |
| Verification | Add focused regressions that corrupt selected normal-memory parity through the existing retained internal memory mechanism, prove lane status/IOCHK/NMI mask and release, control-write diagnostic clear, first covered-memory-write IOCHK clear, reset restoration, and IBM planar isolation. Run affected strict-owner/static gates, focused tests, serial current gate, documentation gate, diff check and actual-change review. |
| Expected Markers | `M5:T386:S22:D4-PARITY-DIAGNOSTIC:OK`; `M5:T386:S22:D4-IOCHK-CLEAR:OK`; `M5:T386:S22:MEMORY-PARITY-OWNER:OK`. |
| Asset Needs | Transient primary technical reference only. No firmware, guest-media, local asset path, vendor hash, source text, derived binary or third-party code enters Git. |
| Reporting Requirements | Confirm the mechanism scope; report source-backed owner decision, Core/VM surface, regression/gate result, artifact identity and SKEY/reset transfer. Coordinator independently reviews all P changes before acceptance. |
| Stop Conditions | Stop for a need to expose a test-only production API, a conflict with existing planar parity, a required physical timing inference, or an unproven memory-write/IOCHK relationship. Return SKEY/reset arbitration and physical timing to later T386/board receivers. |
| Exit Criteria | One shared location-bearing parity event mechanism has one Core owner; Model-40 D4 uses it without an IBM parity fork; primary-defined diagnostic/control/IOCHK/reset behavior has regressions; selected non-D4 behavior remains unchanged; required gates pass; evidence records source boundary and remaining transfers. |
| Original Owner Request | Owner-approved request, 2026-08-16: continue the original DeskPro 386 development plan after S21 planning/proposal/Queue governance. |
| Similar-Issue Sweep | Sweep all Core memory-parity configuration/callback users, planar/D4 port-B/NMI paths, Model-40 D4 mappings and tests; disposition every location-bearing callback, reset and clear path. |
## Current Technical Baseline

- **Current developer artifact:** T386 S20 `vm-0-5-0389` /
  `build/output/nxvm_0_5_0389.exe`, SHA-256
  `B0B70FA0C0A304BA9D8E1F2ADECC0E32A547DABA3FE274513B5C9DFEBA8C9483`.
  Built from the accepted S20 source after its 275-test serial current-gate; the
  constrained BYOB/backbone proof is retained in
  [S20 evidence](../etc/evidence/t386-s20-model40-byob-profile.md).
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
| T386 S21 | Accepted: current Model-40 matrix truthfully separates the four remaining functional clusters from physical/timing debt, corrects S20 as a fixed backbone rather than generic variants, and transfers the cross-product contract to its queued post-DeskPro candidate. [Evidence](../etc/evidence/t386-s21-model40-current-functional-matrix.md). |
| T385 | Closed: the selected Model-40 80386DX-16 consumes the shared 80386 CPU/state/retirement owners without a DeskPro CPU fork; a clean current-source 251/251 gate passed, while functionality and board timing transfer unchanged. [Closure audit](../etc/evidence/t385-s1-task-closure-audit.md). |
| T384 | Closed: the Model-40 capability audit freezes its original Compaq system board/storage ledger and owner-selected color EGA, keyboard and external-ROM constraint; its shared-VADP IBM/Compaq personality boundary transfers CPU, functional and timing work without making a runnable or L3 claim. [Closure audit](../etc/evidence/t384-s2-task-closure-audit.md). |
| T383 | Closed: independent current-source replay accepts the frozen Model-339 deterministic L3 contract after T380's strict-start repair; 251/251 current gate and focused matrix are retained in [evidence](../etc/evidence/t383-s1-model339-current-source-reaudit.md). |
| T382 | Reclosed at S8: the prior S1--S7 closure is retained historically, while S8 corrects its Console 64-bit transport and final artifact baseline. [Corrective audit](../etc/evidence/t382-s8-reclosure-audit.md). |
| T381 | Closed: S1 P1 `9b6a86c6` delivered frozen YAML profile selection and the true zero-session Console; corrective S2 restored its task-level closure state without changing runtime evidence or the T381 artifact. [Evidence](../etc/evidence/t381-s1-console-yaml-profiles.md). |
| T380 | Closed: strict Model-339 startup, 512 KB FDC/BDA repair, input/FDC current-gate recovery, and the owner-replayed DOS 5/HIMEM no-XMS path are verified. It does not claim DOS completion or 5170 L3 closure. [Evidence](../etc/evidence/t380-s2-model339-start-and-current-gate-recovery.md). |
| T378 | Closed at `42f0402a`: production ingress proof and virtual-time ownership are corrected without a test-only API; Model-339 re-audit remains the sole L3 decision receiver. [Evidence](../etc/evidence/t378-s1-native-ingress-boundary-repair.md). |

## Recent Governance

- **M5 Td S101 P1:** added a compact execution-entry table without creating
  new paths, and made coordinator semantic review explicit after structural
  documentation-gate success.
- **M5 Td S100 P1:** archived the closed T382 proposal, queued Model-339
  current-source re-audit before DeskPro, reconciled T380 S2 history, and made
  host/workspace path detection reject the external-assets form while retaining
  guest DOS-path examples.
- **M5 Td S99 P1:** compacted documentation gates, closure evidence, artifact
  identity, build hygiene, and recorder containment within Execution; design
  authorities remain unchanged.
- **M5 Td S98 P1:** made coordinator-review inputs, exit criteria, and the
  executor-report boundary explicit in Role cycle.
- **M5 Td S97 P1:** added explicit single-/separate-session review navigation
  without changing the lifecycle requirements.
- **M5 Td S96 P1:** compressed duplicate Role cycle prose into references to
  its retained authorities without changing lifecycle requirements.
- **M5 Td S95 P1:** removed duplicate M5 technical narrative from Queue while
  retaining all shared-context, candidate, and required proposal-index links.
- **M5 Td S94 P1:** locks the DeskPro 386 Model 40 and PC/XT 5160-268
  baselines, establishes capability-ledger then functional-before-timing
  closure for all three machines, and retains external ROMs outside Git.
