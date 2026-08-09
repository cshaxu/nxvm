# M5 T305 S1: 32-Bit Interrupt And Exception-Entry Admission Audit

## Scope And Authority

This audit freezes the 80386 protected-mode admission for 32-bit IDT
interrupt and trap gates: software `INT`, `INT3`, and `INTO`; PIC/NMI-facing
hardware interrupts; and entry for already-admitted faults. It is an
entry-only family. `IRET`, outer returns, call/task gates, task switching,
virtual-8086, paging-policy changes, public interfaces, and product behavior
are expressly outside this task.

The authority is the *Intel 80386 Programmer's Reference Manual* (1986),
Chapter 9, "Interrupts and Exceptions", its `INT n`, `INT 3`, and `INTO`
instruction entries, and the protected-mode descriptor/selector rules in
Chapter 6. In particular, the IDT gate type selects the protected entry-frame
width; an instruction operand-size prefix does not substitute for the gate
type. The source of the saved return EIP differs by event class: software
interrupts are traps after their instruction, while a fault retains its
architecturally defined fault EIP and error code.

Read-only comparison used Bochs 2.6 `cpu/exception.cc`, `cpu/event.cc`, and
`cpu/soft_int.cc`, plus PCjs 2.00.0
`machines/pcx86/modules/v2/x86ops.js`, `x86help.js`, and `cpux86.js`.
Both preserve separate software-trap, external-interrupt, and fault entry
origins before a shared protected-mode dispatcher, and distinguish a trap
return address from a fault return address. They are corroborating behavior
references only; no reference source was copied, translated, or imported.
No paired-step bridge is needed because this admission has no unresolved
Intel/reference conflict.

## Current Path Inventory

| Current path | Classification | S1 conclusion |
| --- | --- | --- |
| `_ser_int_real` | Retained regression boundary | Existing real-mode 16/32 frame path is not changed by T305. |
| `_ser_int_protected_16` | Retained 16-bit implementation base | It validates a 16-bit interrupt gate and builds an existing 16-bit frame. It is not evidence for 32-bit gates or trap-gate IF behavior. |
| `_ser_int_protected` when `byte == 4` | In-scope correction | The TODO-marked route mixes 32-bit gates with V86/task branches and reaches `#CE` on ordinary paths. Its selection by instruction byte instead of gate type is not an admissible 32-bit implementation. |
| `_e_int3`, `_e_int_n`, `_e_into` | In-scope front ends | They retain instruction decoding and software origin/DPL handling. Protected `INTO` currently returns `#UD`; T305 must admit it when OF is set. |
| `_e_intr_n`, `ExecInt` | In-scope front end | PIC/NMI event selection remains core-owned, but protected hardware entry currently returns `#UD`. No new PIC or executor path is permitted. |
| `_e_except_n`, `ExecFinal` | In-scope bounded fault delivery | Existing diagnostics attempt one protected `#GP` or `#NP` delivery and restore the original fault if delivery fails. This establishes the retained containment boundary, not 32-bit frame evidence. |
| T257/T259/T263 focused probes | Retained regressions | Their 16-bit IDT, privilege, delivered `#GP`/`#NP`, and atomicity checks must continue to pass; they do not prove this 32-bit family. |

All named paths execute in the single `core_machine_cpu_execution_context`.
PIC selection enters through the existing core event route; neither the audit
nor later batches admit a second executor, an external delivery owner, or a
VM/platform callback.

## Form, Profile, Mode, And Commit Matrix

| Event / form | Frozen admission | Required validation and visible commit boundary | Current classification |
| --- | --- | --- | --- |
| Real-mode `INT n`, `INT3`, `INTO`, external interrupt, and retained real faults | Retain the current real-mode route, including its existing 16/32 compatibility frame selection. | This family adds no real-mode behavior. Retained probes guard against regression only. | Retained / deferred from T305 implementation. |
| Protected software `INT n` and `INT3` to a 32-bit interrupt or trap gate on the 80386 profile | Admitted. Software origin checks gate DPL; target code selector must be non-null GDT code, present, compatible with the admitted same-CPL scope, and able to execute the target offset. | Validate IDT limit, gate type/present/DPL, candidate CS/cache, target limit, and the complete old-stack write range before publishing CS/EIP/EFLAGS/ESP or an accessed descriptor byte. Interrupt gates clear IF; trap gates preserve IF; both clear TF. | In-scope correction. |
| Protected `INTO` with OF clear / set | OF clear has no entry. OF set is the same software-origin vector 4 admission as `INT`, including DPL and gate behavior. | A no-entry form preserves all state. A rejected OF-set entry preserves the old instruction-visible state. | In-scope correction: OF-set PE currently `#UD`. |
| Protected external IRQ/NMI to a 32-bit interrupt or trap gate | Admitted through the retained PIC/NMI selection route. Hardware origin bypasses software gate-DPL checking; it still validates gate, target CS, target limit, present state, and stack capacity. | Gate type controls IF semantics exactly as above. Event acknowledgement/state publication must occur only on a successful entry plan. | In-scope correction: `_e_intr_n` currently returns `#UD` in PE. |
| Already-admitted protected faults, including existing `#GP`/`#NP` delivery and existing fault sources with an established error code | Admitted only as entry consumers of their existing fault classification and error-code policy. T305 does not add fault origins or change paging policy. | A 32-bit gate pushes a dword error code after the return frame. The original fault register/error state remains the input until a successful plan publishes the new context. | In-scope correction / bounded to existing fault producers. |
| 16-bit interrupt/trap gates under protected mode | Retained. Existing 16-bit interrupt-gate behavior remains a regression boundary; a trap-gate correction, if needed for shared planning, must not change its established frame layout. | Gate type, not an opcode prefix, decides frame width. Any shared preflight must preserve current 16-bit success and fault observations. | Reusable base only. |
| 80286 or earlier profile encountering a 32-bit gate | Not admitted. It is an invalid protected gate form and must take the architecture's gate-validation rejection before any 32-bit frame attempt. | S2 must prove the project-native selector/error result and no state mutation; it must not turn descriptor validation into an opcode-profile `#UD`. | Deferred verification inside S2. |
| Protected CPL change, stack switch, and outer `IRET` | Not admitted in T305. | Same-CPL entry may be implemented only with complete preflight. Any required new privilege stack/selector transition stops the batch and belongs to the queued 32-bit privilege-transition/return families. | Explicit deferral. |
| Task gate, call gate, task switch, V86, paging-policy expansion | Not admitted. | Preserve existing rejection/deferred behavior; do not route these paths through a new partial 32-bit entry implementation. | Explicit deferral. |

For a successful same-CPL 32-bit gate, the architectural stack image is, from
the new ESP upward: optional error-code dword, saved EIP dword, zero-extended
saved CS selector dword, and saved EFLAGS dword. `INT3`, `INT n`, and an OF-set
`INTO` save the address after their respective instruction; a fault uses its
already-established return EIP. A 16-bit gate retains a word frame. The gate
type, not `66h`, selects this frame width.

## Bounded Batches And Focused Synthetic Probes

| Batch | Bounded implementation | Focused synthetic probe and stop boundary |
| --- | --- | --- |
| S2 | Build a candidate/commit 32-bit same-CPL IDT gate entry for `INT` origin, with both `INTGATE_32` and `TRAPGATE_32`. | Prepared IDT/GDT/stack states prove IDT limit, invalid type, DPL, present, target-CS type/present/limit, gate-controlled IF/TF, exact 32-bit stack order, and no cache/stack/flags/accessed-byte mutation on each failed validation. Stop before hardware origin, `INTO`, or new fault containment. |
| S3 | Connect the validated entry plan to `INT3`, `INT n`, OF-set `INTO`, and the existing PIC/NMI event front end. | Prove trap return EIP, OF-clear no-entry, software DPL versus hardware DPL bypass, interrupt/trap IF behavior, NMI/PIC selection, and successful event publication through the single core route. Stop before CPL change or fault recursion. |
| S4 | Use the same plan for already-admitted fault entry and define a bounded failed-delivery containment result. | Prove existing error-code preservation for prepared `#GP`/`#NP` and any already-established non-policy fault source, first-fault restoration if entry fails, and the exact terminal/double-fault containment observation. Do not implement `IRET`, task gates, reset/triple-fault policy, or new exception delivery machinery. |
| S5 | Family closure only. | Re-run the T305 focused synthetic probe and retained T257/T259/T263, PIC, selector, protected-return, and task-switch regressions, then normal gates/artifact and one bounded product observation. |

The eventual focused probe uses prepared CPU, IDT/GDT, stack, and PIC state
only. It contains no guest image, long-start fixture, host shortcut, public
raw-layout access, or second execution path. Its marker is reserved until a
later implementation batch.

## Similar-Issue Sweep And Deferrals

The audit used the following bounded sweep:

```powershell
rg -n "_ser_int_|_e_(int|intr|except)|Exec(Int|Final)|INT3|INT_I8|INTO|IDT|INTGATE|TRAPGATE|#(GP|NP)" src/core/machine tests/machine CMakeLists.txt
rg -n "_s_read_idt|_ksa_load_sreg|_kma_test_access|_kec_push|core_machine_cpu_profile_allows_form" src/core/machine
```

Every production hit is classified in the inventory or matrix. The 16-bit
protected entry, selector/cache helpers, stack primitives, PIC event source,
and terminal diagnostic route are retained dependencies. Outer returns,
inter-privilege entry, task/call gates, task switching, V86, reset/triple-fault
policy, and paging-policy work are deferred to their queued owners. The
current generic 32-bit entry's V86/task branches are neither a second path nor
a permitted implementation base for this task.

No audit finding requires a public interface change, a second state owner, or
a reference bridge. T305 S1 changes no CPU behavior, CMake target, artifact,
Queue entry, or product observation.

## S2 Same-CPL 32-Bit Gate Evidence

`_ser_int_protected` now reads the IDT entry before choosing an entry width.
`INTGATE_16` continues through the retained 16-bit helper, while only an
80386 `INTGATE_32` or `TRAPGATE_32` reaches the new
`_ser_int_protected_32_same` planner. The instruction's decoded operand size
is validated as an existing instruction form but does not select the frame:
a `66h INT imm8` to a 32-bit gate still pushes the required three dwords.

The planner is intentionally limited to the S2 software `INT imm8`, same-CPL,
non-conforming-code case. It validates the IDT range, gate type, present bit,
software DPL, target selector/type/present state, target offset, and the full
old-stack frame range before descriptor access, frame writes, CS/EIP, or flags
are published. A successful interrupt gate clears IF and TF; a trap gate
preserves IF and clears TF. The frame at new ESP is saved EIP, zero-extended
CS, then saved EFLAGS. `INT3` with a 32-bit operand form, `INTO`, external
interrupts, protected fault delivery, V86, inter-privilege entry, task gates,
and 16-bit trap gates remain deferred and do not enter this planner.

`tests/machine/core_machine_interrupt_entry_smoke.c` is the prepared-state
focused synthetic probe. It proves both 32-bit gate types, gate-type rather
than `66h` frame width, exact dword frame order, interrupt/trap IF behavior,
TF clearing, and successful target-code accessed-byte publication. Its IDT
limit/type/DPL/present, target code type/present/limit, and stack-limit cases
assert the first fault plus unchanged CS selector/base/limit, EIP, ESP,
EFLAGS, and descriptor access byte. The marker is
`M5:T305:INTERRUPT-ENTRY:OK`.

The S2 sweep revisited `_ser_int_protected`, `_ser_int_protected_16`,
`_ser_int_protected_32_same`, `_e_int3`, `_e_into`, `_e_int_n`, `_e_intr_n`,
`_e_except_n`, IDT/selector preparation, stack preflight, and the retained
T257/T259 probes. No hit requires a second executor, state owner, public
interface, PIC change, or exception-delivery rewrite. The retained 16-bit
protected-mode and privilege/atomicity probes pass; hardware, `INT3`, `INTO`,
fault, and outer/CPL-transition semantics remain S3/S4 or later-family work.

## S3 Software And Hardware Front-End Evidence

The planner now receives an explicit origin classification. `INT n`, `INT3`,
and OF-set `INTO` are software origins and apply the gate DPL check; OF-clear
`INTO` remains a no-entry form. Retained PIC and NMI selection are external
origins, so they bypass only that software gate-DPL check while retaining the
same gate, selector, target-limit, old-stack, frame, and commit validation.
Fault entry remains separately marked and rejected by this batch; it is not
silently reclassified as hardware delivery.

`ExecInt` remains the sole core event route. PIC selection first uses the new
internal `core_machine_pic_peek_interrupt` operation, runs the existing
planner, and acknowledges the PIC only after the planner leaves no exception.
NMI likewise clears its pending bit only after successful entry. A rejected
event therefore keeps its PIC IRR state or NMI pending state, respectively;
no new controller, callback, or host route is introduced. The existing debug
trap source remains deferred rather than being admitted incidentally with
external IRQ/NMI handling.

The T305 focused synthetic probe extends its prepared states with `INT3`,
OF-set and OF-clear `INTO`, and actual shared-PIC/NMI selection. It proves
trap-gate return EIP for the software forms, OF-clear no entry, an external
event from CPL3 through a DPL-zero 32-bit gate, successful PIC ISR publication
and NMI clear, plus rejected PIC/NMI preservation. Its CPL3 external handler
uses a bounded non-privileged loop solely to observe entry; it is not a guest
fixture. The retained S2 rejection cases continue to prove frame/cache/flag
atomicity. The marker remains `M5:T305:INTERRUPT-ENTRY:OK`.

The S3 sweep revisited `_e_int3`, `_e_into`, `_e_int_n`, `_e_intr_n`,
`ExecInt`, `core_machine_pic_scan_interrupt`,
`core_machine_pic_peek_interrupt`, `core_machine_pic_get_interrupt`, the
NMI state fields, and the focused PIC lifecycle checks. `IRET`, outer or CPL
stack transitions, fault recursion, task/call gates, V86, paging, and public
interfaces remain outside S3. No production hit requires another executor or
an expanded controller ownership contract.

## S4 Existing Fault Entry And Containment Evidence

The frozen same-CPL 32-bit planner now accepts the existing exception-entry
flag. It preflights sixteen stack bytes for a fault gate, retains the existing
fault error code, and publishes the dword image as error code, saved EIP,
saved CS, then saved EFLAGS. `ExecFinal` retains its single protected delivery
attempt and original-fault restoration rule, while adding the already-existing
exact `#SS` consumer to the same `#GP`/`#NP` vector selection on the admitted
80386 profile only. The retained 80286/16-bit terminal `#SS` behavior is not
reclassified. A rejected delivery restores the original CPU state and first-fault diagnostic before the
existing terminal stop; it does not publish an IDT target, stack write,
accessed byte, or delivered-exception record.

The focused prepared-state probe directly produces existing protected `#GP`,
`#NP(0018)`, and `#SS(0018)` sources and verifies their 32-bit gates, retained
error code, return EIP, selector, flags, four-dword frame, and target
descriptor access. The `#GP` failed-gate case verifies the bounded terminal
containment: original first-fault diagnostics remain observable, no delivered
exception is recorded, and CS cache, EIP, ESP, EFLAGS, and descriptor access
are unchanged. The direct `#NP`/`#SS` setup clears fixture TF before its
selector load, because debug-trap entry is intentionally deferred; this is
only a prepared-state isolation measure and does not change a fault origin.

The S4 sweep revisited `_e_except_n`, `ExecFinal`,
`_ser_int_protected_32_same`, `_s_test_ss_push`, `_kec_push`, and the existing
`MOV Sreg` loader route. No loader change was needed: the initial apparent
`#UD` occurred after a successful `MOV EAX, imm32` with fixture TF set, before
the selector-load instruction. Retained T301 selector coverage and the T305
S2/S3 front-end proof pass with the direct fault cases. `IRET`, fault-origin
expansion, recursive/double-fault policy, reset/triple-fault policy, task and
call gates, CPL transitions, V86, paging, and public interfaces remain
deferred.

## S5 Closure-Preparation Evidence

The one current artifact target is `vm-0-5-0305`. The generated
`build/output/nxvm_0_5_0305.exe` has SHA-256
`53005149E1C157646D37BB54F9D3B68468122A41050706396E8DD7EFFC7412E6`.

The final managed `cmake --build --preset current-gates-gcc` run passes all
51 static/governance targets and 134/134 current-gate CTests. The focused T305
probe emits `M5:T305:INTERRUPT-ENTRY:OK`. Retained direct probes emit
`M5:T301:SEGMENT-SELECTOR:OK`, `M5:T293:S2:PROTECTED-RETURN-ATOMICITY:OK`,
`M5:T304:DESCRIPTOR-SYSTEM:OK`, `M5:T260:S3:TSS-IOMAP:CORPUS:OK`, and the
T261 task-switch markers. Documentation governance and `git diff --check`
pass.

One bounded owner-supplied product observation ran the 0305 artifact without
guest input for ten seconds. The process was alive and responding but exposed
main-window handle zero, so host automation could not safely discover a guest
surface. It was terminated and cleanup found no process residue. No media was
mounted and no guest command, Setup checkpoint, or CPU conclusion was
obtained. This is a host-automation limitation retained for owner-controlled
manual observation; it is not retried in T305.

T305 remains active and pending coordinator acceptance. This record is
closure-preparation evidence only: it does not set the project Idle, close
T305, change Queue state, or authorize a subsequent task. The deferred
boundary remains 32-bit `IRET`, outer/CPL transitions, call/task gates, task
switching, V86, reset/triple-fault policy, paging-policy expansion, and new
fault origins.
