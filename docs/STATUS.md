# Project Status

## Current Work

**No active task.** M5 T316 S25 is closed; the next 80386 matrix slice requires
a separately admitted packet.

## M5 T316 S25 Closure Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Sequential bounded implementation; M5 T316 S25 follows accepted S24 within the owner-approved 80386DX ordinary-execution program. |
| Admission And Approval | Owner approved continuous T316 80386 matrix execution; S24 is accepted at `8ad66329`. Coordinator admits only this adjacent LES/LDS slice. |
| Objective | Complete Intel `LES`/`LDS` (`C4`/`C5`) memory far-pointer loads in legacy default 16-bit and 80386 `66h` 32-bit operand-size forms: selector and GPR offset publication, profile disposition, and source/selector fault atomicity. |
| Non-goals | Do not admit LSS/LFS/LGS, FS/GS prefix consumers, MOV/POP segment-register families, `LEA`, generic MOV/XCHG, interrupt-shadow refactoring, exception-delivery expansion, x87, or post-80386 forms. |
| Reference baseline | Accepted T316 S24 at `8ad66329`; Intel 80386 PRM LES/LDS forms; existing `_e_load_far` caller audit and focused corpus. |
| Files And ABI Surface | `src/core/machine/cpu_instructions.c` only if a demonstrated LES/LDS defect requires it; one owned focused smoke and CMake registration; T316 matrix/evidence and Status closure records. No public ABI or artifact change. |
| Applicable rules | One CPU decoder/executor route; audit `_e_load_far` callers before shared change; preserve local style; test owner-bound forms; Intel authority and no imported external source; dual-session coordinator independently reviews actual changes. |
| Verification | Audit `LES_R32_M16_32`, `LDS_R32_M16_32`, decoder metadata/profile disposition, `_d_modrm_ea`, `_m_read_rm`, `_e_load_far`, `_s_load_sreg`, and all `_e_load_far` callers. Prove C4/C5 memory-only far pointers in real and controlled protected mode, legacy default 16-bit and `66h` 32-bit operand size; assert GPR offset, ES/DS selector, EIP, and EFLAGS. Prove register-direct #UD; prove 8086/80286 legacy default-16 acceptance and `66h` #UD below 80386; prove source or selector fault non-publication and no maskable-IRQ shadow. Run focused, documentation governance, current gates, diff, commit, and push. |
| Expected markers | New `M5:T316:S25:LES-LDS:OK`; retained S1--S24 and current-gate markers pass. |
| Asset needs | None beyond governed current-gate assets. |
| Stop conditions | Stop if correct proof needs a shared selector/far-load/PIC/exception-delivery redesign, if a helper change lacks caller coverage, or if decoder/ABI ownership changes. |
| Exit criteria | Every declared C4/C5 form is proven for size, memory-only encoding, profile, publication, declared fault atomicity, and absence of an SS-only IRQ shadow; all scan hits are disposed; broader segment/operand families remain explicit partial; required gates and delivery pass. |
| Original owner request | Execute the complete 80386 program in Coordinated Dual-Session Mode against Intel form--implementation--test matrix evidence without using Windows demand as a scope filter. |
| Similar-issue sweep | Scan `LES_R32_M16_32|LDS_R32_M16_32|LSS_R32_M16_32|LFS_R32_M16_32|LGS_R32_M16_32|_e_load_far|_d_modrm_ea|_m_read_rm|_s_load_sreg|flagMaskInt|0xC4|0xC5` across source, tests, CMake, and records; classify each production hit as covered, fixed, deferred, or out of scope. |

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
