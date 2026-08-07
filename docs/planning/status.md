# Project Status

## Current Work

**M5 T258 S2 active: implement the bounded core-only 80386 CPL0 paging
baseline.**

### Original Request

On top of T257's GDT-only, 16-bit CPL0 protected-mode path, admit one real,
verifiable 80386 CPL0 4 KiB paging path. It includes only page-directory/page-
table walks, `MOV r32,CR0`, `MOV r32,CR2`, `MOV CR0,r32`, `MOV CR3,r32`, and
core diagnostic `#PF`. The target artifact is `nxvm_0_5_0258.exe`.

### Frozen Scope

| Concern | T258 admission | Deferred / forbidden |
| --- | --- | --- |
| Owner and path | `core/machine` CPU, segment translation, physical memory route, and diagnostics; existing `core_machine_run()` only | VM/profile/firmware/platform paging or fault shortcuts; second executor |
| Address translation | logical -> T257 segment cache -> linear -> two-level 4 KiB page walk -> core physical route | host MMU, RAM pointers, PSE/PAE/V86/long mode, TLB cache |
| Control forms | 80386 CPL0 `MOV r32,CR0`, `MOV r32,CR2`, `MOV CR0,r32`, `MOV CR3,r32`; `CR2` is fault-written only | guest write `CR2`, all other CR forms, 80286/80186 forms |
| CR0 / CR3 | only PE and PG may be changed by T258; PG requires PE; CR3 must be page-directory aligned | opening MP/EM/TS/ET semantics, silent CR3 normalization |
| Page checks | Present, 4 KiB address, core-routed A/D updates, CR2 and P/W/U `#PF` diagnostic | CPL3 user/supervisor faults, supervisor write-protect fault, protected IDT delivery |
| CPL0 semantics | CPL0 may access supervisor and read-only mappings as 80386 permits; non-present fetch/read/write faults are proven | treating `RW=0` or `US=0` as a CPL0 fault; general privilege model |
| Fault result | copied `STOP_FAULT` / first-fault diagnostic at the original instruction | guest `#PF` handler, IDT/task/call gate, double/triple fault |

`RW=0` is **not** a CPL0 write-protect failure on the 80386: `CR0.WP` is not
part of this CPU. T258 proves that supervisor behavior rather than fabricating
a write-protect `#PF`; actual user write protection waits for T259's CPL3
corpus.

### S1 Result

S1 is complete. The retained executor already had one core-owned two-level
4 KiB walker, but its generic control-register decoder allowed guest writes
outside T258's contract and fault rollback discarded a newly recorded `CR2`.
T258 therefore keeps the single walker and executor, narrows `MOV CRx`, and
retains page-fault `CR2` in the copied core diagnostic. The similar-issue
sweep also found VM debugger raw control-register mutation; it is a separate
debug-boundary debt recorded in `TODO.md`, not a second paging path.

### S1 Audit And Requirement Map

| Requirement | Current evidence | S1 disposition / planned evidence |
| --- | --- | --- |
| Page walk | `_kma_physical_linear` already reads PDE/PTE through core physical memory, sets A/D, and records `CR2`/`#PF` | Narrow and prove it with `core-machine-80386-paging-smoke`. |
| `CR0` / `CR3` writes | `_d_modrm_creg` exposes CR0/2/3 and `MOV_CR_R32` writes the selected storage directly | Replace with explicit form-specific validation: CR2 write `#UD`; CR0 mask and PE-before-PG; aligned CR3 only. |
| Profile gate | 0F `20h`--`26h` are metadata-gated at 80386 | Corpus proves 80386 positive and 80286/80186 `#UD` negatives. |
| Fault boundary | `ExecFinal` records a first fault then stops; T257 has separately disabled protected IDT delivery | Prove `#PF` retains original point, CR2, P/W/U code, and `STOP_FAULT`. |
| Reset | CPU cold reset zeroes CR0/CR2/CR3 before retained reset-vector setup | Corpus proves PG and CR3 clear after cold reset. |

The similar-issue sweep covers all tracked production references to
`_kma_physical_linear`, `_d_modrm_creg`, `MOV_R32_CR`, `MOV_CR_R32`,
`VCPU_CR0_PG`, `cr2`, `cr3`, and `VCPUINS_EXCEPT_PF`; each hit is either the
single owner path, an explicit negative gate, or deferred to T259--T261.

### S1 Rules, Verification, And Stop Conditions

Applicable rules: `core` has no VM dependency; no global/TLS current object;
all page-table accesses use the checked core physical route; profile selection
is frozen before reset; platform never mutates guest state; no protected media
or external runtime dependency. The planned focused target is
`core-machine-80386-paging-smoke`, with S2 marker
`M5:T258:S2:I386-PAGING:OK` and S3 marker
`M5:T258:S3:I386-PAGING:CORPUS:OK`.

S2 stops and requests a split if the corpus needs CPL3, protected IDT delivery,
TSS, task switch, a second executor, VM/firmware mutation, host-side page-table
access, or a Console/debugger/boot UX change. S3 must run the focused corpus,
T257 and real-mode corpora, FDD/HDD/DOS, Console/debugger, CGA/EGA, ATA, RTC,
and `current-gates-gcc`. S4 records the T258-to-`0.5.0258` artifact mapping and
SHA-256 before closure.

| Closure | Evidence |
| --- | --- |
| T249--T251 | Copied input, presentation, and cancellable wait contracts are closed with artifacts `0.5.0248`--`0.5.0250`. |
| T252 | Composition owns the sole run-handle teardown sequence; artifact `0.5.0251` and 86/86 current CTest pass. |
| T253 | ATA PIO sector-count progression is controller-owned; artifact `0.5.0252` and 87/87 current CTest pass. |
| T254 | Bounded digital CGA `640x200x2` is VADP-owned; artifact `0.5.0253` and 89/89 current CTest pass. |
| T255 | Machine-profile admission contract is closed; design/governance only, no artifact. |
| T256 | Core-owned Level 1 rational device clocks are closed; artifact `0.5.0254` and 90/90 current CTest pass. |
| T257 | Bounded 80286 GDT/CPL0 protected-mode baseline is closed; artifact `0.5.0257` and 91/91 current CTest pass. |

The next task must establish a complete active packet before implementation.

## Current Technical Baseline

- **T257 artifact identity correction:** `current-gcc` and
  `verify-current-artifact-target` select `vm-0-5-0257`; static/ownership
  checks and 91/91 CTest cases passed. Artifact `nxvm_0_5_0257.exe` SHA-256:
  `E69FC24A8E1113E4A7AED552F9C5B944372F4FCF73FFB0D669CC14B6C375A1F1`.
- **T243--T246:** core owns checked physical memory, bounded `#UD`
  transitions, immutable ROM mapping, and atomic real-mode entry plans. T247
  verifies the current artifact target and full gate over that boundary.
- **Product boundary:** `nxvm.exe` is the retained runnable product. `mantle`,
  `dos`, and `nxvdm.exe` remain future architecture commitments; they are not
  current runtime or release products.

Completed implementation detail, artifact history, and rationale are in
[M5 History](../history/m5.md) and Git history. When a task is active, its
packet, the roadmap, and the M5 closure checklist are the current operational
authorities.

## Recent Governance Updates

- **M5 Td S20:** aligned the five-component architecture.
- **M5 Td S21:** added source-distribution notices and release-record gates.
- **M5 Td S22:** aligned roadmap and trusted external-research boundaries.
- **M5 Td S23:** compacted completed M5 records into history, corrected the
  historical baseline, clarified notice provenance, and removed an untracked
  temporary error file. This documentation task changes no runtime behavior.
- **M5 Td S24:** fixed the time/device ownership vocabulary: core owns guest
  ticks and generic PIT/PIC/DMA mechanics; VM owns PC/AT CMOS/RTC and BIOS time
  semantics; composition owns the bounded product pump and host pacing. This
  documentation task changes no runtime behavior or active-task scope.
- **M5 Td S25:** reconciled completed M5 baseline references to T252 S3 and
  removed stale task scheduling from the completed T248 record. This
  documentation task changes no runtime behavior or active-task scope.
- **M5 Td S26:** split the unstarted 80386 route so T258 is CPL0 paging only;
  protected privilege/IDT delivery, TSS I/O permissions, task switching, FPU,
  and remaining instruction families now have independent admission tasks.
  This documentation task changes no runtime behavior or active-task scope.
- **M5 Td S27:** compacted completed T248--T257 task records into M5 history,
  removed the completed T248 ledger entry, and clarified idle-state authority.
  This documentation task changes no runtime behavior or active-task scope.
- **M5 Td S28:** replaced independent artifact revisions with task-identity
  revisions. The T257 current artifact is reissued as `0.5.0257`; future
  numeric task artifacts must use their task number exactly.

## Milestone State

| Milestone | State | Current authority |
| --- | --- | --- |
| M0--M4 | Closed | [Roadmap](roadmap.md) and [history](../history/README.md) |
| M5 | Open | [M5 closure checklist](m5-closure-checklist.md) |
| M6--M8 | Not started | [Roadmap](roadmap.md) |
| M9 onward | Research queue | [Roadmap](roadmap.md) |

## Operational Reading Order

1. [Documentation Guide](../README.md)
2. [Roadmap](roadmap.md)
3. [M5 closure checklist](m5-closure-checklist.md)
4. [Architecture overview](../architecture/overview.md) and
   [contracts](../architecture/contracts.md)
5. [Requirements](../requirements/)
