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
