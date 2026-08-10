# Project Status

## Current Work

**M5 T316 S38 - INS/OUTS port string forms.** The coordinator admitted this
bounded ordinary-execution continuation in Coordinated Dual-Session Mode.

## M5 T316 S38 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Corrective; next unused subtask of the most recently closed numeric task, M5 T316. |
| Admission And Approval | The owner approved continuous execution of the Intel 80386DX architecture-completeness program in Coordinated Dual-Session Mode. This is the next linear bounded matrix slice after T316 S37. |
| Objective | Complete Intel INSB `6Ch`, INSW/INSD `6Dh`, OUTSB `6Eh`, and OUTSW/OUTSD `6Fh` string forms with single and F3 REP execution. |
| Non-goals | Ordinary IN/OUT, MOVS, CMPS, STOS, LODS, SCAS, general I/O privilege architecture changes, general or segment stack forms, broader prefix families, post-80386 behavior, 80387, production refactoring, and any shared port or `_kas_move_index` change without a demonstrated defect and caller sweep are outside this packet. |
| Reference Baseline | `61cc6ce0` / `vm-0-5-0316`, with T316 S37 closed, `main` equal to `origin/main`, and no active packet before admission. |
| Files And ABI Surface | Expected scope is a focused machine smoke and CMake registration, the ordinary-execution matrix, and this packet. `src/core/machine/cpu_instructions.c` changes only for a demonstrated defect. No public ABI change. |
| Applicable Rules | `docs/rules/EXECUTION.md`: one active packet, dual-session Instruction/report loop, actual-change coordinator review, linear corrective allocation, matrix audit, similar-issue sweep, and immediate push. `docs/rules/CODING.md`: preserve local code/test style and introduce no premature abstraction. Intel 80386 PRM INS/OUTS opcode, REP, operand/address attribute, prefix, exception, I/O permission, and interrupt behavior is the form authority. |
| Verification | Audit primary dispatch, port provider and memory paths, T302 I/O-string evidence, prefixes, profiles, and permission routes. Cover supported default forms; classify unsupported profiles as #UD without publication; 80386 `66h`, `67h`, and combined forms; fixed-ES INS destination under CS/FS overrides and DS-source OUTS overrides; DF, F3 counts zero/one/multiple, 16/32 count semantics, port width/DX, exact EIP/index/count/GPR/FLAGS and memory or port publication; pre-80386 attributes and 80386 LOCK #UD without state, memory, or port publication; protected INS ES write-limit and OUTS DS read-limit single restart/no publication and REP partial restart at the no-IDT diagnostic boundary; and pending-PIC single no-shadow plus REP one-primitive restartable progress. IOPL/TSS is evidence only if the existing route can express it. |
| Expected Markers | Focused smoke emits `M5:T316:S38:PORT-STRINGS:OK`; matrix evidence records every admitted opcode family and exact boundary disposition. |
| Asset Needs | None; deterministic CPU and local port fixture only. No guest media, external code, firmware, or research import. |
| Stop Conditions | Stop and report if a shared port or `_kas_move_index` change requires a covered caller sweep, if the existing port fixture cannot express an Intel behavior without a material harness change, or if a demonstrated defect expands beyond the declared forms. |
| Exit Criteria | Every admitted `6Ch`--`6Fh` form is classified and proven with no ordinary IN/OUT, broader I/O permission, other string-family, post-80386, or 80387 completeness claim. |
| Original Owner Request | Execute the complete Intel 80386 plan in dual-session mode against an Intel form--implementation--test matrix, repairing omissions and closing evidence without using Windows demand as the completeness boundary. |
| Similar-Issue Sweep | Audit INSB/INSW/OUTSB/OUTSW, `_p_input`, `_p_output`, `_kpa_test_mode`, `_kpa_test_iomap`, `_kas_move_index`, primary prefix routes, the port provider, and existing operand/address and I/O-string smokes. Classify all hits; ordinary IN/OUT and other string families are excluded by opcode/semantic boundary. |

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
| T316 S37 | Closed CMPS `A6`/`A7`: single/REPE/REPNE profiles plus `66`/`67`, DS-to-fixed-ES selection/DF/full FLAGS, #UD/LOCK, protected DS/ES #DF, and PIC no-shadow/restart coverage. No production change was needed; 52 governance/static checks and 166 current-gate tests passed. |
| T316 S36 | Closed SCAS `AE`/`AF`: single/REPE/REPNE profiles plus `66`/`67`, fixed ES/DF and full FLAGS, #UD/LOCK, protected ES #DF, and PIC no-shadow/restart coverage. No production change was needed; 52 governance/static checks and 165 current-gate tests passed. |
| T316 S35 | Closed LODS `AC`/`AD`: single/REP profiles plus `66`/`67`, source segments/DF, #UD/LOCK, DS #DF single/partial, and PIC no-shadow/restart coverage. No production change was needed; 52 governance/static checks and 164 current-gate tests passed. |
| T316 S34 | Closed STOS `AA`/`AB`: default plus `66`/`67`, REP/DF, fixed ES destination, #UD/LOCK, #DF single/partial, and PIC single/restartable coverage. Test-only; 52 governance/static checks and 163 current-gate tests passed. |
| T316 S33 | Closed MOVS `A4`/`A5`: single/REP default and `66`/`67`, segment/DF, #UD/LOCK, protected source/destination limit #DF full nonpublication including DS/ES cache, single no-shadow plus REP restartable partial IRQ. No production change was needed; 52 governance checks and 162 current tests passed. |
| T316 S32 | Closed `8C`/`8E` segment-register MOV: form/profile/`66`/`67`, #UD/LOCK, protected cache/null/accessed and #DF nonpublication, plus MOV SS shadow and non-SS no-shadow. No production change was needed; 52 static/governance checks and 161 current-gate tests passed. |
| T316 S31 | Closed primary non-segment GPR MOV `88`/`89`/`8A`/`8B`, `C6`/`C7`, and `B0`--`BF`: default, `66`/`67`, segment selection, #UD/LOCK, limit-#DF no-publication, and PIC no-shadow coverage. No production change was needed; 52 static/governance checks and 160 current-gate tests passed. |
| T316 S30 | Closed bounded moffs MOV `A0`--`A3`: default plus `66h`/`67h` forms, segment selection, #UD/LOCK and read/write nonpublication, and no IRQ shadow. No runtime change was needed; 52 static/governance checks and 159 current-gate tests passed. |
| T316 S29 | Closed Intel `98`/`99` default and `66h` sign extension with prefix, 80386 LOCK/nonpublication, and no-shadow coverage. The legacy LOCK-prefix divergence remains `TODO(Medium)`; no runtime change was needed. Documentation governance, 52 static checks, and 158 current-gate tests passed. |
| T316 S28 | Closed Intel accumulator XCHG `90`--`97`: `90` NOP-equivalent and `91`--`97` named AX/EAX exchanges, default and `66h` profile coverage, #UD/nonpublication and LOCK boundaries, and no IRQ shadow. No runtime change was needed; 52 static/governance checks and 157 current-gate tests passed. |
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
