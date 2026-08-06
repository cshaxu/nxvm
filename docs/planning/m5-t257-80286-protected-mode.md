# M5 T257: Constrained 80286 Protected-Mode Baseline

**Status:** S4 active.

## Original Request

Provide one real, verifiable 16-bit 80286 protected-mode path in
`core/machine`: guest code enters PE with real instructions, resolves GDT
descriptors, loads admitted code/data/stack selectors, and makes same-privilege
far control transfers. Descriptor/table access must use core memory routing;
the existing `core_machine_run()` remains the only executor. The task allocates
developer artifact revision `0.5.0255` at S4.

## Scope And Frozen Decisions

| Concern | T257 admission | Explicit deferral |
| --- | --- | --- |
| CPU form | `80286` profile only; `LGDT`, `LMSW`, MOV segment loads, 16-bit far JMP/CALL/RET | 386 `MOV CRx`, 32-bit operands/addressing, paging, CR3, V86 |
| Tables | GDT only, 8-byte 286 code/data descriptors through core memory | LDT/LLDT, task register, TSS, task switch, call/task gates |
| Privilege | CPL=RPL=DPL=0; nonconforming 16-bit present code/data/stack descriptors | rings 1--3, conforming code, IOPL/CPL model, expand-down, 32-bit/granular descriptors |
| Faults | copied core `STOP_FAULT` diagnostic records `#GP`, `#NP`, or `#SS` mask and selector-derived code | IDT gate delivery, guest exception handlers, double/triple fault semantics |
| Mode transition | real-mode `LMSW` sets PE; on 80286 PE remains set until cold reset | clearing PE to return to real mode |
| Product | profile chooses CPU; VM composition, firmware, platform, Console, and debugger retain their paths | BIOS or host-assisted protected-mode setup; Win3.x/Win95 claim |

An architecturally visible fault outcome is not guest exception delivery. Until a
separate IDT-gate task exists, a protected-mode validation failure stops the
core executor at the original instruction and exposes its existing diagnostic
to the runner/debugger. It never fabricates a guest handler transfer.

## S1 Current-State Audit

| Existing surface | Evidence | T257 disposition |
| --- | --- | --- |
| Per-machine CPU and segment-cache storage | `src/core/machine/cpu_instructions.h`, `t_cpu_data_sreg`; context is owned by `core_machine` | Retain; no second CPU state. |
| Logical-to-linear segment checks | `cpu_instructions.c` `_kma_linear_logical` | Retain and prove CS/DS/SS cache behavior through corpus. |
| GDT descriptor read/write | `_ksa_read_gdt`, `_ksa_read_xdt`, `_ksa_load_sreg` | Retain only GDT reads; descriptor accessed-bit write is not part of the first contract. |
| `LGDT` / `LMSW` decode | `INS_0F_01`, `_s_load_gdtr`, `_s_load_cr0_msw` | Verify 286 profile gates and 24-bit table base; reject 386-only forms. |
| Far transfer | `_e_jmp_far`, `_e_call_far`, `_e_ret_far` | Narrow to same-CPL nonconforming 16-bit code descriptors. |
| Exception finalization | `ExecFinal` records fault, then retains a legacy `#GP` delivery attempt | S2 must make the admitted validation path stop with the original fault outcome, not enter legacy IDT-gate logic. |
| Profile gate | instruction metadata distinguishes 80286/80386 forms | Extend corpus to prove 80186 rejects 286 system forms and 80286 rejects 386-only forms; retained 8086 `0F` remains `POP CS`. |

## Corpus Contract

The project-owned corpus starts in real mode and uses a core-created machine
with the `80286` profile. It installs a GDT pseudo-descriptor and GDT bytes in
ordinary guest memory, executes `LGDT`, executes `LMSW` to set PE, then uses
ordinary guest instructions to load `DS` and `SS` and far-transfer to a
nonconforming 16-bit code selector. The protected code writes a marker through
its data selector and makes one same-CPL far CALL/RET. Its result is observed
through copied core memory and CPU/fault diagnostics.

Negative cases use a null or out-of-limit selector, a non-present code/data
descriptor, and an invalid stack selector. They must stop with the expected
fault class and error code without modifying the prior segment cache or
creating an IDT handler transfer. The corpus also proves `80186` faults on the
admitted 286 form and that `80286` faults on a 386-only form. The 8086 has a
real `POP CS` instruction at primary opcode `0F`; its byte stream cannot be
reclassified as `#UD` without breaking retained 8086 semantics. Its negative
evidence is therefore that it never enters the `0F` system-instruction decoder
or gains PE.

## Similar-Issue Sweep

Defect class: a protected-mode control/selector path bypasses core memory,
accepts a 386-only form on the 80286 profile, mutates partial segment state on
failure, or routes an admitted validation fault through an unapproved guest
exception path. S1 queries:

```text
rg -n "_IsProtected|_s_load_(gdtr|idtr|ldtr|tr|cr0_msw)|_ksa_(read|write)_xdt|_e_(jmp|call|ret)_far|VCPUINS_EXCEPT_(GP|NP|SS)" src tests
rg -n "CORE_MACHINE_CPU_PROFILE_(80286|80386)|INS_0F_0[01]|MOV_CR" src tests
```

Every production hit is either part of the admitted GDT/CPL0 path, explicitly
deferred, or must be prevented from affecting the T257 corpus. S2 records the
post-fix result and a focused regression.

## Rules, Evidence, And Stop Conditions

Applicable rules: one core-owned CPU/executor; `core` has no VM dependency;
all descriptor/table access uses the core memory interface; profile selection
is frozen before reset; no global/TLS current object; platform never mutates
guest state; no protected guest media or external runtime dependency. Required
closure evidence is the focused corpus, CPU profile gate, retained real-mode
corpus, FDD/HDD boot, DOS prompt, Console/debugger, CGA/EGA, ATA, RTC, and the
full current GCC/CTest gate.

Stop and split if the corpus needs paging, a privilege transition, a TSS,
task-switch, LDT, call/interrupt gate, guest exception delivery, a new run
loop, or altered retained NXVM UX.

## S1 Exit

S1 closes only after the current path is classified against this contract and
the exact corpus/fault observations are recorded. It does not claim a working
protected-mode path or create an artifact.

**S1 result:** the project-owned corpus demonstrated that the retained
executor already has one usable GDT-only, 16-bit, CPL0 path: real-mode `LGDT`
and `LMSW`, `DS`/`SS` loads, code-cache fetch, data-segment writes, and same-CPL
far CALL/RET all reached the expected protected-mode halt boundary. The audit
also found two concrete gate defects: metadata excluded `0F 01 /6` (`LMSW`) at
the 80286 profile, and the `0F` decoder treated every pre-286 profile as 8086
`POP CS`, thereby accepting a 286 system byte sequence on 80186. S2 corrects
those gates, makes protected validation faults diagnostic-only, and expands the
corpus with non-present/stack and 386-negative cases.

## S2 Result

The focused `core-machine-80286-protected-mode-smoke` passes with one real
GDT-only protected path: `LGDT`, `LMSW`, `DS`/`SS` loads, code fetch through
the protected segment cache, data writes, and same-CPL far CALL/RET. It also
proves `#GP(0018)`, `#NP(0008)`, and `#SS(0010)` diagnostics without protected
IDT delivery. `80186` rejects `0F 01`, while `80286` rejects 386-only
`MOV CR0`; true 8086 `0F` remains `POP CS`.

The corpus is registered in `PROJECT_CURRENT_SMOKE_TARGETS`. S3 now runs the
full retained GCC/CTest matrix and records the exact result before the artifact
revision is advanced.

## S3 Result

`current-gates-gcc` passed with **91/91** current CTest cases. This includes
the T257 corpus, retained real-mode corpus, DOS prompt and `MEM` samples,
FDD/HDD boot, keyboard and mouse paths, CGA/EGA paths, ATA, RTC, and retained
Console/debugger coverage. S4 may now advance the truthful current artifact to
`0.5.0255`, record its SHA-256, and close the task.
