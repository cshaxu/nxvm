# Project Status

## Current Work

**M5 T260 S1 active: freeze the TSS I/O-permission-map contract and core-only
port corpus.**

## T260 Admission Packet

### Original Request

Implement TSS I/O permission checking only in `core/machine`, on the T259
16-bit CPL3-to-CPL0 gate/outer-`IRET` path. When `CPL > IOPL`, guest IN/OUT
access must use the loaded TSS map, apply the result to every byte of the port
span, and turn denial into observable `#GP(0)` through T259's protected-IDT
delivery or original-fault retention. CPL0 and `CPL <= IOPL` retain their
current direct port behavior. The task excludes task switching, LDT, all
32-bit frames/gates, CPL1/2, V86, host/firmware port shortcuts, and another
executor. Its artifact is fixed as `nxvm_0_5_0260.exe`.

### S1 Contract

The shared 16-bit protected execution path remains common to 80286 and 80386,
but the I/O bitmap itself is an **80386 32-bit TSS** facility. This is a
hardware distinction, not a scope expansion:

| Concern | T260 admission | Deferred / deterministic result |
| --- | --- | --- |
| Owner/path | `_kpa_test_mode` -> `_kpa_test_iomap` before the one existing port executor; `core_machine_run()` only | VM/profile/firmware/host port bypass or second executor |
| 80286 / 16-bit TSS | `CPL <= IOPL` retains existing access; `CPL > IOPL` receives `#GP(0)` because no I/O-map exists | no invented 80286 bitmap |
| 80386 TSS | A loaded busy 32-bit TSS is a read-only `ESP0`/`SS0` source for the already-admitted 16-bit gate and an I/O-map source | no task switch, context save, I/O bitmap mutation, or 32-bit gate/frame |
| Stack bridge | For the retained 16-bit gate, `ESP0` must fit in 16 bits and `SS0` must be valid; otherwise `#TS` before a frame is committed | 32-bit stack/frame semantics |
| Bitmap | Read the 16-bit map base at TSS offset `0x66`; bit `0` allows and bit `1` denies. Every bit for port `port..port + width - 1` must be readable and clear. A missing, truncated, or overflowing span denies with `#GP(0)` | V86 and unbounded map interpretation |
| I/O forms | Existing immediate/DX IN/OUT and string I/O reach the same width-aware check | new I/O opcodes or host APIs |
| Fault | A denied access does not reach the port provider; it is a `#GP(0)`, then uses T259's one protected delivery attempt and copied diagnostic. Failed delivery retains the original terminal fault | firmware/product fault consumption |

T260 S1 uses a core-owned test port provider and one 80386 prepared state with
a 32-bit TSS, 16-bit CPL3 code/data segments, and the existing IDT `#GP` gate.
The corpus must prove allowed IN and OUT reach that provider, denied IN and OUT
do not, denied access reaches the `#GP` handler and produces a copied
diagnostic, a one-byte map truncation denies a word span, and 80286/IOPL bypass
cases retain their stated behavior.

### Rules, Evidence, And Stop Conditions

Applicable rules: core owns CPU/TR/TSS/ports/diagnostics; core does not depend
on VM/VDM; no global/TLS current state; providers are frozen before reset;
platform never mutates guest state; no protected media or external runtime
dependency. S2 may change only the core CPU privilege/port boundary and the
T259 stack-source helper needed for a 32-bit TSS. S3 adds a core-only port/TSS
corpus and retains T257--T259, paging, DOS boot, FDD/HDD, Console, debugger,
and current gates. The expected focused marker is
`M5:T260:S3:TSS-IOMAP:CORPUS:OK`.

Stop and split if the corpus needs task/busy-TSS switching, LDT, call/task/trap
gates, a 32-bit frame/gate, CPL1/2, V86 semantics, a VM/firmware/host port
shortcut, a second executor, or a Console/debugger/boot UX change.

**Similar-issue sweep plan.** The defect class is a protected low-privilege
port operation that bypasses a single permission decision. Before closure,
search all `_p_input`, `_p_output`, `_p_ins`, `_p_outs`, and direct
`core_machine_port_execute_*` production references. Every hit must be the
single core guard or be explicitly outside guest execution; the focused corpus
must show both provider-reached and provider-not-reached outcomes.

## T259 S1 Admission Packet

S1 is complete. T259 admits only a common 80286/80386 **16-bit**
protected-mode subset:
CPL3 software `INT imm8` through a DPL-permitted 16-bit interrupt gate into
CPL0, a read-only 16-bit TSS `SS0:SP0` stack switch after `LTR`, and the paired
outer `IRET` back to CPL3. TSS here is only a stack-field source: task switch,
I/O bitmap, LDT, call/task/trap gates, 32-bit frames, CPL1/2, V86, hardware
INTR/NMI delivery, and double/triple-fault machinery remain forbidden.

`core/machine` remains the sole owner of CPU, descriptor, IDT, TR, stack, and
diagnostic state; `core_machine_run()` remains the only executor. Every
delivered exception must leave a copied core diagnostic event before its gate
transfer. An invalid or unhandled delivery remains the existing terminal
`STOP_FAULT` with first-fault diagnostic. Firmware, VM composition, platform,
and product code may not emulate or consume either path.

The S1 corpus is one CPL0 -> CPL3 -> CPL0 -> CPL3 round trip: CPL0 configures
GDT, 16-bit IDT, and 16-bit TSS; outer `IRET` enters CPL3; CPL3 executes
`INT 30h`; the gate switches to `SS0:SP0`; CPL0 marks guest memory and `IRET`s
back; CPL3 marks guest memory, then returns through another DPL3 gate to a
CPL0 `HLT`. A DPL-rejected CPL3 software interrupt must enter the configured
`#GP` gate, leave a copied core diagnostic, and then stop in CPL0. An absent
or invalid exception gate remains terminal `STOP_FAULT` with the original
first-fault diagnostic. Existing code establishes the precise gaps:
`_ksa_load_sreg` omits code-segment privilege checks and `_ser_ret_far_outer`
is a stub; no VM, firmware, or host shortcut is admitted.

| S1 concern | Required T259 result |
| --- | --- |
| Selector rules | Nonconforming code transition validates `CPL`, `RPL`, `DPL`, Present, and limit before mutable CPU state changes. |
| IDT gate | Accept only present 16-bit interrupt gates; software INT checks gate DPL, CPU faults do not. |
| Privilege stack | Validate TSS/`SS0:SP0` and target stack before committing an outer-to-inner frame. |
| IRET | Support only the paired 16-bit inner-to-outer frame; invalid frames fault without partial segment/stack mutation. |
| Diagnostics | Retain first terminal fault and add a copied delivered-exception observation/count for successful gate delivery. |
| Evidence | New core-only target emits `M5:T259:S2:PROTECTED-PRIVILEGE:OK` and `M5:T259:S3:PROTECTED-PRIVILEGE:CORPUS:OK`; S4 runs current gates and records the 0.5.0259 SHA. |

S2 stops and requests a split if this corpus requires a 32-bit frame/gate,
generic hardware interrupt delivery, call/task/trap gates, task switch, TSS
I/O bitmap, LDT, a second executor, host/firmware mutation, or a Console,
debugger, or boot UX change. The artifact identity is fixed now:
`nxvm_0_5_0259.exe`.

### S2 Result

`core/machine` now owns the admitted 16-bit gate transfer, TSS `SS0:SP0`
stack switch, outer `IRET`, and `#GP` delivery attempt. A successful delivery
records a copied `last_delivered_exception` plus count; a delivery failure
restores and reports the original fault rather than replacing it with a
secondary gate fault. The new `core-machine-protected-privilege-smoke` covers
the round trip and a DPL-rejected `INT 32h` delivered through `#GP` for both
80286 and 80386 profiles. It is now part of the current CTest matrix.

### S3 Result

The focused target must emit both `M5:T259:S2:PROTECTED-PRIVILEGE:OK` and
`M5:T259:S3:PROTECTED-PRIVILEGE:CORPUS:OK`. It proves one 80286 outer-ring
round trip, then runs the DPL-rejection / delivered-`#GP` case under both 80286
and 80386 profiles. The retained T257 corpus supplies adjacent invalid-selector,
non-present code/stack, same-CPL `IRET`, protected `LIDT`, configured-gate, and
pre-286/386 profile-rejection coverage. Both focused CTest entries passed; the
new corpus emits its S2/S3 markers and no firmware, VM, platform, or host-memory
shortcut is present.

**Similar-issue sweep.** The defect class is a protected `#GP` losing its
original diagnostic when a bounded IDT-delivery attempt fails. The S3 query is
`rg -n "_e_except_n\\(|_ser_int_protected\\(|record_delivered_exception|VCPUINS_EXCEPT_GP" src tests CMakeLists.txt`.
Its only production delivery site is `ExecFinal`; the remaining real-mode IVT
path is explicitly not a protected delivery route. The focused T259 smoke and
retained T257 negative cases lock the two dispositions: successful protected
delivery records an event, and unavailable delivery preserves the original
terminal fault.

### S4 Closure

`current-gates-gcc` passed all 93/93 CTest cases and its static ownership,
artifact-truthfulness, and dependency gates. `current-gcc` built the current
target `vm-0-5-0259` and copied
`build/output/nxvm_0_5_0259.exe`; SHA-256 is
`61FEF63CD57ED1FD46D1B5A2B49C24538830FCAF3EDF0D35EF5D09CAE8FC283A`.
The next task is required to preserve this baseline.

## T258 Closure Record

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

### S2 Result

S2 is complete. `MOV r32,CR0` now permits only CR0/CR2 reads, while guest
writes permit only CR0 and aligned CR3. CR0 changes are limited to PE/PG, PG
cannot be set before PE, and guest CR2 writes reject as `#UD`. The existing
page walker remains the sole logical-to-physical path. `ExecFinal` now copies
the fault-written CR2 into the retained first-fault snapshot before rollback.
The focused target passed its valid mapping, fetch/data/stack, A/D, page-fault,
control-form, CPU-profile, and reset checks.

### S3 Result

S3 is complete. `core-machine-80386-paging-smoke` emits both the S2 and S3
markers after proving valid code/data/stack mappings, PDE/PTE A/D updates,
non-present fetch/read/write faults with retained CR2 and original point,
control-form rejection, profile gates, and cold-reset clearing. The final
`current-gates-gcc` run passed all 92 CTest cases, including retained DOS,
FDD/HDD, input, video, Console, and debugger coverage.

### S4 Result

T258 maps exactly to artifact version `0.5.0258`. The current target is
`vm-0-5-0258`; `build/output/nxvm_0_5_0258.exe` SHA-256 is
`51AAF534434F0943AE3BCBB4AA4A56C3ED1A815B79C47BC4FEC4B6DD02B8F62C`.
The runtime implementation was verified from source commit `2a6442e`; the
artifact-target configuration and this closure record are the following S4
commit. The next task must establish a new complete packet before source work.

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
| T258 | Bounded 80386 CPL0 paging baseline is closed; artifact `0.5.0258` and 92/92 current CTest pass. |
| T259 | Bounded 16-bit protected privilege and `#GP` IDT delivery are closed; artifact `0.5.0259` and 93/93 current CTest pass. |

The next task must establish a complete active packet before implementation.

## Current Technical Baseline

- **T259 artifact identity:** `current-gcc` and
  `verify-current-artifact-target` select `vm-0-5-0259`; static/ownership
  checks and 93/93 CTest cases passed. Artifact `nxvm_0_5_0259.exe` SHA-256:
  `61FEF63CD57ED1FD46D1B5A2B49C24538830FCAF3EDF0D35EF5D09CAE8FC283A`.
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
- **M5 Td S29:** aligned the architecture overview with the task-identity
  artifact rule. This documentation task changes no runtime behavior or
  active-task scope.

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
