# Project Status

## Current Work

**Idle.** M5 T316 S53 is closed; the next 80386 matrix slice requires a
separately admitted packet.

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
| T316 S53 | Closed protected ARPL `63 /r`: all register ModRM/RPL/ZF outcomes, segment and 16/32 EA selection, profile/prefix/LOCK rejection, #GP handler-boundary, and STI-window PIC no-extra-shadow coverage passed. No production change was needed; 182 current-gate tests passed. |
| T316 S52 | Closed protected IRET `CF` CPL0-to-outer returns: 16/32 and `66`/`67` forms, full target cache/GPR/stack-image proof, selector/limit handler-boundary failures, and TSS-backed IF/PIC delivery passed. No production change was needed; 181 current-gate tests passed. |
| T316 S51 | Closed same-CPL IRET `CF`: four real profiles, `66` wide-frame and inert `67` 16-bit stack route, #UD/LOCK, protected selector/limit #DF, and IF-restoring versus IF-preserving PIC boundaries passed. No production change was needed; 180 current-gate tests passed. |
| T316 S50 | Closed software interrupts INT3 `CC`, INT imm8 `CD ib`, and INTO `CE`: four real-mode profiles, `66`/`67`/LOCK, IVT/IDT frame, DPL/VM86/fault, and pending-PIC boundaries passed. No production change was needed; 179 current-gate tests passed. |
| T316 S49 | Closed HLT `F4`: default and `66`/`67`/LOCK form behavior, protected/VM86 privilege rejection, and IF-qualified pending-IRQ wake behavior passed. No production change was needed; 178 current-gate tests passed. |
| T316 S48 | Closed CLI/STI `FA`/`FB`: profiles/`66`/`67`/LOCK, protected and ordinary VM86 IOPL boundaries, and exact CLI pending-IRQ inhibition versus STI one-instruction shadow coverage passed. No production change was needed; 177 current-gate tests passed. |
| T316 S47 | Closed PUSHF/PUSHFD and POPF/POPFD `9C`/`9D`: visible FLAGS/bit1, profiles/`66`/`67`/LOCK, protected and VM86 privilege boundaries, stack #DF atomicity, and PIC no-shadow coverage passed. Corrected legacy PUSHF FLAGS-image normalization and POPFD RF preservation; 176 current-gate tests passed. |
| T316 S46 | Closed legacy segment-register stacks `06`/`07`, `0E`, `16`/`17`, and `1E`/`1F`: default/`66`/`67`/LOCK, protected selector/cache and stack-limit #DF, plus POP SS shadow and POP ES/DS/PUSH no-shadow coverage passed. No production change was needed; 29 static/governance steps and 175 current-gate tests passed. |
| T316 S45 | Closed PUSH immediate `68`/`6A`: corrected the 80186 profile gate, `6A` sign extension, and LOCK rejection. Default/`66`/`67`, protected stack-limit #DF nonpublication, and PIC no-shadow coverage passed; 52 governance/static checks and 174 current-gate tests passed. |
| T316 S44 | Closed general-register PUSH/POP `50`--`5F`, `FF /6`, and `8F /0`: corrected 8086 PUSH SP source ordering, post-POP r/m effective-address timing, and LOCK rejection. Profile/prefix, protected stack/source/destination fault, and PIC no-shadow coverage passed; 52 governance/static checks and 173 current-gate tests passed. |
| T316 S43 | Closed ENTER `C8` / LEAVE `C9`: corrected the 80186 profile gate, ENTER display-chain count and post-display allocation base, and 16-bit LEAVE stack-address preflight. Default/`66`/`67`/LOCK, protected stack-fault, and PIC no-shadow coverage passed; 52 governance/static checks and 172 current-gate tests passed. |
| T316 S42 | Closed PUSHA 60 and POPA 61: 8086 rejection; 80186--80386 default plus 80386 66/67 and LOCK; exact stack image and original-SP/ignored-slot rules; protected partial-write/read-fault boundaries; and PIC no-shadow. Corrected the local 80186 profile guard. 52 static/governance checks and 171 current-gate tests passed. |
| T316 S41 | Closed LES/LDS `C4`/`C5`: four profiles plus `66`/`67` and LOCK, selector/cache atomicity, protected #DF, and PIC no-shadow coverage. No production change was needed; 52 static/governance checks and 170 current-gate tests passed. |
| T316 S40 | Closed CMC `F5`, CLC `F8`, STC `F9`, CLD `FC`, and STD `FD`: profiles plus `66`/`67` and LOCK, protected/VM86 preservation, and PIC no-shadow coverage. No production change was needed; 52 static/governance checks and 169 current-gate tests passed. |
| T316 S39 | Closed LAHF/SAHF `9E`/`9F`: profiles plus `66`/`67` and LOCK, protected/VM86 FLAGS preservation, and PIC no-shadow coverage. No production change was needed; 52 governance/static checks and 168 current-gate tests passed. |
| T316 S38 | Closed port strings `6C`--`6F`: single/REP profiles plus `66`/`67`, segment/DF, #UD/LOCK, protected DS/ES #DF, and PIC no-shadow/restart coverage. No production change was needed; 52 governance/static checks and 167 current-gate tests passed. |
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

- **M5 Td S65:** aligned active numeric-task progress retention with task-level closure consolidation, added narrow structural checker coverage (including retained-progress self-tests), and preserved the eight-row cap for task-level closures. Td work has no runtime or artifact change.

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
