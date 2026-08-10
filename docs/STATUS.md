# Project Status

## Current Work

**M5 T316 S28 - accumulator XCHG forms.** The coordinator admitted this
bounded ordinary-execution continuation in Coordinated Dual-Session Mode.

## M5 T316 S28 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Corrective; next unused subtask of the most recently closed numeric task, M5 T316. |
| Admission And Approval | The owner approved continuous execution of the Intel 80386DX architecture-completeness program in Coordinated Dual-Session Mode. This is the next linear bounded matrix slice after T316 S27. |
| Objective | Close the Intel 80386 accumulator-XCHG opcode slice `90h`--`97h`: establish focused evidence for `91h`--`97h` exchanges between AX/EAX and each named general register, and explicitly prove `90h`'s architecturally equivalent no-op behavior without treating it as a separate data-exchange implementation. |
| Non-goals | No new CPU abstraction, decoder, or runtime feature; no x87 implementation; no post-80386 behavior; no memory-XCHG, `86h`/`87h` r/m coverage beyond S27; no broad prefix-family claim. |
| Reference Baseline | `bf52dba4` / `vm-0-5-0316`, with T316 S27's r/m XCHG matrix closed and current `main` equal to `origin/main` before admission. |
| Files And ABI Surface | Expected test/build/evidence scope: `tests/machine/core_machine_xchg_smoke.c`, `CMakeLists.txt` only if a distinct target is necessary, `docs/etc/evidence/t316-ordinary-execution-matrix.md`, and this active packet. `src/core/machine/cpu_instructions.c` changes only for a demonstrated defect. No public ABI change. |
| Applicable Rules | `docs/rules/EXECUTION.md`: one active packet, dual-session Instruction/report loop, actual-change coordinator review, linear corrective allocation, matrix audit, similar-issue sweep, immediate push. `docs/rules/CODING.md`: preserve local instruction/test style and introduce no premature helper. Intel 80386 PRM XCHG/NOP, operand-size, prefix, interrupt, and exception behavior is the form authority. |
| Verification | Audit every `90h`--`97h` table route and handler. Add focused vectors for default 16-bit exchange on 8086/80186/80286/80386; 80386 `66h` 32-bit exchange; partial-register/high-half preservation; EIP and EFLAGS preservation; every named register form; `90h` no-op; lower-profile `66h` rejection without publication; LOCK rejection without publication; and pending-IRQ delivery proving no interrupt shadow. Build and run the focused smoke, documentation governance, `git diff --check`, then the complete current-gate preset before closure. |
| Expected Markers | Focused smoke emits `M5:T316:S28:XCHG-ACC:OK`; matrix evidence records every `90h`--`97h` form and the exact boundary dispositions. |
| Asset Needs | None; deterministic CPU fixture only. No guest media, external code, firmware, or research import. |
| Stop Conditions | Stop and report if the form audit requires a shared helper change whose existing callers lack focused coverage, if Intel behavior conflicts with the retained profile boundary, or if fixing a demonstrated defect expands into another form family. |
| Exit Criteria | Each declared opcode/form is complete or explicitly classified with authority; its result/no-op behavior, width, profile/prefix rejection, FLAGS/EIP, and IRQ boundary have focused proof; no demonstrated runtime defect remains unaddressed; matrix wording no longer leaves accumulator XCHG partial; required gates pass; coordinator independently reviews all changed artifacts before closure. |
| Original Owner Request | Execute the complete Intel 80386 plan in dual-session mode against an Intel form--implementation--test matrix, repairing omissions and closing evidence without using Windows demand as the completeness boundary. |
| Similar-Issue Sweep | Audit the primary table entries and all `XCHG_*_EAX` routes for `90h`--`97h`, plus prefix LOCK/profile dispatch. S27 already owns `86h`/`87h`; no source defect is presumed. If a defect is found, search the full accumulator-XCHG handler group and classify every hit. |

## Current Technical Baseline

- **Current task artifact:** `current-gcc` and
  `verify-current-artifact-target` select `vm-0-5-0316` / `build/output/nxvm_0_5_0316.exe`.
  T316 S5 owns this developer artifact; its source commit is finalized only by
  the coordinator's subsequent acceptance commit.
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
| T316 S26 | Closed Intel LEA `8D /r` coverage without a runtime change: 8086--80386 default 16-bit form, all 80386 `66`/`67` combinations in real and protected mode, #UD/profile/LOCK boundaries, null-DS no-read semantics, and no IRQ shadow. Its focused smoke joins the current gate. |
| T316 S24 | Closed the bounded Intel 80386 `LSS`/`LFS`/`LGS` (`0F B2/B4/B5`) matrix with dedicated current-gate coverage: real/protected 16/32-bit memory far pointers, 80386/80286 and register-direct profile dispositions, source-fault non-publication, and the SS-only one-instruction IRQ shadow. No runtime or artifact change was needed. Documentation governance, 52 static gates, and 154 current-gate tests passed. |
| T314 | Closed the EGA CRTC 13h bound, registered the deterministic xasm smoke, restored six historical target-local strict GCC sets while retaining three selected libraries, and added the CRTC static closure verifier; the two inherited-governance debts remain. Its 0315 artifact, 52 gates, 147 CTests, and S2/S3/S6 markers passed. |
| T313 | Completed construction-failure atomicity and startup-failure visibility: single RAM allocation, transactional port/controller assembly, visible session start failure, and explicit debugger mappings. Its accepted artifact, 51 gates, 145 CTests, and S2--S6 focused markers passed. |
| T307 | Completed 80386 32-bit CPL3-to-CPL0 IDT and call-gate entry with TSS32 target stacks, preflighted atomic publication, and Intel-correct target-SS `#TS` classification. Its 0307 artifact, 51 gates, 137 CTests, and 9 focused/retained probes passed; product observation remained host-window limited before guest input. |
| T308 | Completed protected 32-bit same/outer error delivery and bounded contributory `#DF` containment through the existing route. Its 0308 artifact, 51 gates, and 137 CTests passed; triple-fault shutdown/reset remains deferred. |
| T309 | Completed the form-level 80386 audit and bounded trace-design record without runtime or artifact change; it selected T310 through evidence rather than a completeness claim. |
| T310 | Completed the admitted `0F` integer bit/data forms: `SETcc`, `MOVZX`/`MOVSX`, bit test/modify, `SHLD`/`SHRD`, `BSF`/`BSR`, and two-operand `IMUL`. Its 0310 artifact, 51 gates, and 143 CTests passed. |
| T311 | Completed the bounded non-PAE paging family: CPL3 U/S/R/W, CPL0 `CR0.WP`, exact producer #PF/CR2 diagnostics, and atomic one-boundary cross-page access. Its 0311 artifact, 51 gates, and 143 CTests passed; TLB/`INVLPG` remains unadmitted without a consumer. |
| T312 | Closed as a withdraw/no-op after audit found no real VM/M5 consumer for the candidate 32-bit system-extension families. It made no runtime change or artifact; future admission requires the per-family consumer evidence recorded in [T312 evidence](etc/evidence/t312-system-extension-admission.md). M6 candidates remain unadmitted. |

## Recent Governance

- **M5 Td S64:** requires an actual-change review before a task or subtask is
  accepted: reports, test summaries, and diff statistics are evidence indexes,
  not substitutes for reading the relevant changed code, build, test, and
  documentation artifacts. In dual-session mode the coordinator owns that
  independent review. Td work has no runtime or artifact change.

- **M5 Td S63:** made Intel 80386 PRM form audits a per-candidate admission
  input and exit gate. A bounded task can close only its declared matrix slice;
  a family cannot close while any in-scope form is partial, missing, or
  unclassified. Td work has no runtime or artifact change.

- **M5 Td S62:** recorded the owner-approved Intel 80386DX
  architecture-completeness program before M6, replaced the closed T314 Queue
  candidate with its ordered family sequence, scoped 80386-to-x87 coupling
  without admitting an 80387, and recorded the required reuse and coverage
  discipline. Td work has no runtime or artifact change.

- **M5 Td S61:** restored Queue dependency order by placing residual M5 work
  before M6 mantle candidates; removed stale package/index detail; and changed
  historical evidence to name closure artifacts rather than current state.

- **M5 Td S60:** routed every DOS 5-bound smoke contract through the explicit
  M1 FDD image while retaining the current MS-DOS 6.22 FDD and EWIN31 Setup
  HDD for their applicable checks. Fresh configuration restored 145/145 current
  gates without weakening old assertions or claiming broad DOS 6.22 support.

- **M5 Td S59:** established the repository-relative current-media root,
  approved-media identities, and protected-media change controls. Its initial
  DOS 5 contract mismatch was classified and corrected by S60.

- **M5 Td S58:** made immediate remote push a cross-mode change-discipline
  requirement. Every successful task or subtask commit must be pushed before
  acceptance, closure, or completed-result reporting; a push failure is a
  reportable delivery failure unless the owner records a bounded deferral.

- **M5 Td S57:** completed the direct 80386 package-close audit. It found no
  T313-level runtime or boundary remediation; corrected stale lifecycle and
  recent-closure records in place; and recorded T301--T312 package completion
  before any separate M6 admission. See the
  [package-close audit](etc/evidence/m5-direct-80386-package-close-audit.md).
