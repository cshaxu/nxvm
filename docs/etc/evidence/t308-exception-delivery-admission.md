# M5 T308 S1: 32-Bit Exception-Delivery Consistency Admission

## Scope And Authority

T308 audits only the already admitted protected-mode 32-bit error delivery
surface. The CPU retains one instruction executor, one checked stack and memory
route, and core-owned selector/cache state. This task does not add guest
recovery, a new fault origin, a second delivery route, task or virtual-8086
return, paging policy, product behavior, or a public interface.

Intel's *80386 Programmer's Reference Manual* (1986), Chapter 9, "Interrupts
and Exceptions", its exception and double-fault entries, and Chapter 6
protected-mode descriptor rules are the semantic authority for exceptions, IDT
gates, error codes, and fault nesting. Read-only behavior comparison uses
Bochs 2.6 `cpu/exception.cc`, `cpu/fetchdecode.cc`,
`cpu/protect_ctrl.cc`, `cpu/segment_ctrl.cc`, and `cpu/call_far.cc`, plus PCjs
2.00.0 `machines/pcx86/modules/v2/cpux86.js`, `x86ops.js`, `x86op0f.js`, and
`segx86.js`. Both retain an exception dispatcher distinct from instruction
decoding; no reference conflict requires a paired-step bridge. These references
are inspection-only; no source is copied.

## Current Producer And Consumer Matrix

| Family | Retained 32-bit producer facts | Current consumer and disposition |
| --- | --- | --- |
| T301 selector loads and queries | Segment type, privilege, limit, and present failures produce `#GP`, `#SS`, or `#NP` before cache/accessed publication. | `ExecFinal` recognizes `#GP`, `#NP`, and 80386 `#SS` for one protected IDT attempt; a successful delivery records a copied delivered-exception diagnostic. |
| T304 descriptor/system controls | `LLDT`/`LTR` and admitted TSS validation retain `#GP`, `#NP`, and `#TS` classifications. | `#TS` is not currently assigned an IDT vector in `ExecFinal`; it remains a terminal first-fault diagnostic. This is an explicit T308 input, not a silent normalization. |
| T305 IDT front ends and existing faults | IDT limit/type/DPL failures use `#GP(vector * 8 + 2)`; non-present gate or target code uses `#NP`; stack checks use `#SS`; target-stack/TSS checks may use `#TS`. Same-CPL 32-bit entry can compose an error-code dword. | `_e_except_n` re-enters the existing protected gate route with `software_origin=false` and `is_exception=true`. The same-CPL 32-bit planner consumes `instruction_state.data.excode` as a dword error code. |
| T306 protected returns | Same-CPL and outer `IRET`/`RETF` preflight selectors, frames, target limits, flags, and stack candidates before commit; retained negative cases include `#GP`, `#NP`, and `#SS`. | The common terminal/delivery boundary applies with no return-specific delivery path. Existing probes prove failed-return cache, stack, flags, and accessed-byte preservation. |
| T307 IDT and call-gate privilege entry | CPL3-to-CPL0 IDT/call-gate validation produces `#GP`, `#NP`, `#SS`, and Intel-correct `#TS(SS selector)` before new-stack/frame/cache publication. | T307 focused probes prove planner classification and atomicity. `#TS` remains terminal today; no error-frame or hardware-NMI integration is claimed. |

The direct dispatch in `ExecFinal` maps only a sole `#GP` to vector 13, a sole
`#NP` to vector 11, and on 80386 a sole `#SS` to vector 12. It snapshots the
original fault CPU and code, attempts `_e_except_n` once, records a delivered
exception only after that attempt succeeds, and otherwise restores the
original fault state before recording a terminal first-fault diagnostic. It
does not currently map `#TS` to vector 10, compose a second fault, or synthesize
`#DF`/triple fault.

## Error-Code, Priority, And Atomicity Contract

The frozen input contract is that producer-selected `excode` reaches a
successful existing 16/32-bit gate planner unchanged. IDT gate-validation
errors use the normalized `vector * 8 + 2` code. For a 32-bit same-CPL planner,
the error code is the low dword below the saved EIP/CS/EFLAGS frame. The
existing 32-bit outer planner explicitly rejects `error_frame` with `#CE`; it
therefore cannot yet deliver an admitted error source across a privilege stack
switch.

Existing T301--T307 planners preflight descriptor access, candidate caches,
old/new stack ranges, and target limits before descriptor accessed-byte or CPU
state publication. The retained focused probes cover representative `#GP`,
`#NP`, and `#SS` delivery and planner rejection paths. T308 must extend that
evidence by producer class rather than weaken these atomicity contracts.

The current priority boundary is deliberately finite: one original fault gets
one IDT delivery attempt. A failure of that attempt restores and reports the
original fault; no recursive handler invocation or double-fault conversion is
implemented. This preserves a deterministic terminal diagnostic, but is not an
80386 double-/triple-fault policy. S1 found no existing product consumer that
requires recovery behavior merely to preserve diagnostic evidence. Any S2+
proposal for `#DF` must first demonstrate a specific already admitted delivery
failure whose architectural priority cannot be represented by this bounded
terminal outcome.

## Planned Focused-Probe Batches

| Batch | Bounded work and proof | Stop boundary |
| --- | --- | --- |
| S2 | Make the accepted `#GP`/`#NP`/`#SS` 32-bit same-CPL error-frame path explicit across producer classes; prove normalized code, frame order, successful delivered diagnostic, and failed-delivery original-state preservation. | No outer privilege switch, `#TS`, hardware/NMI, or recursive policy. |
| S3 | Audit and, only if required by Intel and a focused reproducer, admit one `#TS` delivery classification path or explicitly retain terminal `#TS` with a precise compatibility rationale. | No task switch, task gate, or generic recovery engine. |
| S4 | Decide whether an existing outer-CPL error producer requires the currently absent 32-bit error frame; if proven, add only that preflighted frame and its error-code evidence. | No new fault origin, hardware/NMI integration, or paging. |
| S5 | Revisit bounded failed-delivery containment after S2--S4. Add `#DF` or triple-fault behavior only with an Intel-defined, focused existing-family reproducer. | No invented guest recovery or reset policy. |

## Similar-Issue Sweep And Queue Hygiene

The S1 sweep searched the retained CPU producer/consumer helpers, diagnostic
provider, and focused probes for `#GP`, `#NP`, `#SS`, `#TS`, error-code, IDT,
and delivered-exception paths. All production hits are classified in the matrix
above. Unimplemented task-gate, task-switch, virtual-8086, paging, and new
hardware-origin paths remain outside the task packet rather than hidden
exceptions to the sweep.

Queue hygiene removed the completed operand/address/stack, control-transfer,
descriptor/system-control, interrupt/exception-entry, protected-return, and
privilege-transition candidates. The first remaining candidate is the current
exception-delivery consistency closure; later candidates are unchanged and
remain unnumbered.
