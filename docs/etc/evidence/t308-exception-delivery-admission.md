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

## S2 Same-CPL Error-Frame Evidence

S2 found that the existing `ExecFinal` to `_e_except_n` to
`_ser_int_protected_32_same` route already implements the admitted
same-CPL 32-bit behavior without a second dispatcher or a CPU correction.
`ExecFinal` retains the producer's `instruction_state.data.excode` while it
selects vector 13 for `#GP`, vector 11 for `#NP`, and vector 12 for `#SS`.
The selected 32-bit interrupt gate pushes that unchanged code as the low
dword, below saved EIP, zero-extended CS, and EFLAGS. Only after successful
entry does the core copy the delivered-exception diagnostic and clear the
terminal first-fault result.

The expanded `core-machine-interrupt-entry-smoke` focused synthetic probe
uses all three admitted producer classes:

| Producer class | Trigger | Required delivered frame and diagnostic |
| --- | --- | --- |
| T301 selector | `MOV DS, selector` with a non-present descriptor and `MOV SS, selector` with a non-present descriptor | `#NP(0018)` through vector 11 and `#SS(0018)` through vector 12; each frame has the original code at its lowest dword. |
| T304 descriptor/control | CPL3 `0F 01 F0` | `#GP(0000)` through vector 13 with the low error-code dword and a copied delivered diagnostic. |
| T305 IDT front end | `INT 30h` through an invalid IDT gate | `#GP(0182)` through vector 13; because the rejected `INT` is a fault, saved EIP is the instruction start, not its post-instruction address. |

At S2 the same probe forced the vector-13 delivery gate invalid, non-present,
to a non-present target CS, and onto an insufficient current SS range. In every
case it observed the original T304 `#GP(0000)` as the terminal first fault,
no delivered-exception record, unchanged EIP/ESP/EFLAGS and complete CS/SS
caches, plus unchanged target-code descriptor access byte. This is the
preflight-then-restore boundary for the admitted route; no failed delivery
partially publishes a frame, cache, or accessed descriptor.

The similar-issue sweep rechecked all three `ExecFinal` mappings and both
16-bit and 32-bit same-CPL planners. The 16-bit planner remains a retained
consumer and is not widened by S2. `#TS`, outer-CPL error frames,
hardware/NMI origin, double/triple-fault conversion, task and virtual-8086
paths, and paging remain deferred exactly as frozen in S1. The focused marker
is `M5:T308:S2:SAME-CPL-ERROR-DELIVERY:OK`; retained T301/T304/T305/T306/T307
probes remain required closure evidence.

## S3 Same-CPL #TS Delivery Evidence

Intel 80386 protected-mode exception delivery assigns `#TS` to vector 10.
For the already admitted 80386 path, `ExecFinal` now maps an exact terminal
`#TS` to vector 10 and reuses the existing `_e_except_n` to
`_ser_int_protected_32_same` route. The mapping keeps the producer's exact
`instruction_state.data.excode`; it does not add a dispatcher, normalize
`#TS` to `#GP` or `#SS`, or change bit-combined terminal failures.

The expanded `core-machine-call-gate-privilege-entry-smoke` focused synthetic
probe uses the existing T307 32-bit call-gate target-SS selector failure. A
TSS `SS0` selector with invalid RPL produces `#TS(0010)`. With a valid
same-CPL vector-10 32-bit gate, the delivered diagnostic records vector 10
and code `0010`; the copied guest frame at the current user stack is, in
ascending dword order, error code `00000010`, saved EIP `00000000`, saved CS
`0000001b`, and EFLAGS `00000202`. The handler is a bounded user-mode loop,
so its observation cannot introduce the privileged `HLT` fault that would
obscure the delivered result.

At S3 the same focused probe made the vector-10 gate invalid, non-present, and
preflight-failing through the current SS limit. Each case retained the
original terminal `#TS(0010)`, created no delivered-exception record, and
keeps EIP, ESP, EFLAGS, complete CS/SS caches, the target-code accessed byte,
and the candidate frame memory unchanged. This proves that failed vector-10
delivery follows the existing preflight-then-restore boundary.

The S3 similar-issue sweep covered all `_SetExcept_TS` producers and the
16-bit and 32-bit same-CPL consumers. The retained producers include selector
and task-register validation, 16/32-bit interrupt planners, protected return
checks, and the T307 call-gate target-stack checks. Only the exact 80386
terminal `#TS` enters the newly admitted vector-10 mapping. The 16-bit
same-CPL planner remains unchanged, while outer-CPL error frames,
hardware/NMI, task and virtual-8086 paths, and paging remained deferred. S6
later admits the bounded double-fault policy. The marker is
`M5:T308:S3:SAME-CPL-TS-DELIVERY:OK`; the retained T301/T304/T305/T306/T307
focused probes remain required evidence.

## S4 Retained Call-Gate Completion Evidence

The S3 focused-probe diff was rechecked specifically for completion semantics.
The retained T307 `cg_test_success(0u)` and `cg_test_success(2u)` cases use
the original `cg_run(..., 0, ...)` helper and therefore require
`CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT` after the existing kernel handler
halts. The S3 vector-10 `#TS` delivery case alone uses `cg_run_budget`: its
user-mode `EB FE` handler is intentionally non-privileged and bounded so that
the delivery observation does not introduce a later `HLT` `#GP`.

`main` executes both retained zero/two-parameter success cases, every
pre-existing T307 DPL, gate, parameter-source, target code/stack, and stack
boundary atomicity case, then all three S3 `#TS` cases. The direct focused
run emits both `M5:T307:CALL-GATE-PRIVILEGE-ENTRY:OK` and
`M5:T308:S3:SAME-CPL-TS-DELIVERY:OK`. No CPU, CMake, artifact, Queue, or
product behavior changed for this corrective evidence record.

## S5 Outer-CPL Error-Frame Evidence

The S5 producer audit covers every exact protected 80386 terminal exception
already admitted by `ExecFinal`: `#GP` maps to vector 13, `#NP` to vector 11,
`#SS` to vector 12, and `#TS` to vector 10. For each, an IDT 32-bit gate with
a nonconforming target code descriptor at a numerically lower DPL reaches the
existing `_ser_int_protected_32_outer` planner. Existing sources include T301
selector and stack loads (`#NP` and `#SS`), T304 descriptor/TSS validation
(`#GP`, `#NP`, and `#TS`), T306 return preflight (`#GP`, `#NP`, and `#SS`),
and T307 call-gate validation (`#GP`, `#NP`, `#SS`, and `#TS`). This is one
existing producer-to-consumer route, not a new fault origin.

The focused prepared-state proof selects the already retained T307 CPL3
call-gate DPL rejection, `#GP(0030)`, and installs an existing-style vector-13
32-bit interrupt gate targeting CPL0. Before S5, the outer planner rejected
its `error_frame` parameter with `#CE(0000)`, proving the missing admitted
consumer. The corrected planner retains its existing old/new TSS stack,
candidate CS/SS cache, target-limit, and descriptor-write preflight, and now
includes the error dword in the preflighted frame size. On success, the frame
at the new CPL0 stack is, in ascending dword order: `00000030`, saved EIP
`00000000`, saved CS `0000001b`, EFLAGS `00000202`, old ESP `00008800`, and
old SS `00000023`. The copied delivered diagnostic retains `#GP(0030)` and
the target CS/SS accessed bytes publish only after the preflight succeeds.

At S5 the same focused probe made vector 13 invalid and non-present, made its
target CS non-present, and reduces the selected new SS limit below the
six-dword frame. Each failure preserves the original terminal `#GP(0030)`,
does not record a delivered exception, and restores complete entry CS/SS
caches, EIP, ESP, EFLAGS, both candidate descriptor accessed bytes, and the
candidate frame memory. At S5 the bounded `ExecFinal` restoration was the sole
failed-delivery containment policy; S6 supersedes that disposition for
contributor pairs.

Intel 80386 PRM Chapter 9 requires the error code below the saved instruction
state for a protected-mode exception frame. The existing read-only comparison
remains Bochs 2.6 `cpu/exception.cc` and PCjs 2.00.0
`machines/pcx86/modules/v2/cpux86.js`; both model exception entry separately
from instruction decoding and introduce no semantic conflict. The marker is
`M5:T308:S5:OUTER-CPL-ERROR-DELIVERY:OK`. Hardware/NMI delivery, outer-CPL
fault producers beyond the admitted exact exceptions, task/V86 paths, and
paging remain deferred. S6 records the admitted double-fault policy.

## S6 Failed-Delivery Containment Evidence

Intel 80386 PRM Chapter 9 classifies `#TS`, `#NP`, `#SS`, and `#GP` as
contributory exceptions. A second contributory exception while delivering one
of those faults must become `#DF(0000)`. The S6 sweep classified every already
admitted `ExecFinal` producer and delivery-preflight failure accordingly:

| Original producer | Existing delivery-preflight failure | Intel disposition |
| --- | --- | --- |
| T301 selector and T304 descriptor/control `#GP`, `#NP`, or `#SS` | Invalid gate/target descriptor `#GP`, non-present gate/target `#NP`, or stack range `#SS` | `#DF(0000)` |
| T304/T307 TSS or target-stack `#TS` | Invalid or non-present vector gate, candidate target, or stack | `#DF(0000)` when the second exception is contributory |
| T305 same-CPL and T306 return preflight `#GP`, `#NP`, or `#SS` | Same existing 16/32-bit gate preflight failures | `#DF(0000)` |
| T307 outer-CPL call-gate `#GP(0030)` | Vector-13 gate preflight `#GP` | `#DF(0000)` |

`ExecFinal` now recognizes this exact 80386 contributory pair after its first
existing `_e_except_n` delivery attempt. It restores the original fault CPU,
sets `#DF(0000)`, and makes the vector-8 attempt through that same
`_e_except_n` route. No dispatcher, recovery engine, selector owner, stack
route, or public interface was added. A successful vector-8 gate creates the
ordinary error-code frame and copied delivered-exception diagnostic. If vector
8 preflight itself fails, the core restores the original machine state and
records a terminal `#DF(0000)` diagnostic without attempting a third delivery.
This is a bounded diagnostic disposition, not an assertion that the project
implements Intel shutdown/reset after a true triple fault.

The focused call-gate probe uses the already admitted CPL3 call-gate DPL
rejection, `#GP(0030)`, followed by an invalid vector-13 gate. With a valid
vector-8 32-bit gate it observes vector 8, code zero, and the six-dword outer
frame at the target stack in ascending order: `00000000`, `00000000`,
`0000001b`, `00000202`, `00008800`, `00000023`. With vector 8 invalid it
observes terminal `#DF(0000)`, no delivered record, and unchanged EIP, ESP,
EFLAGS, complete CS/SS caches, candidate descriptor access bytes, and frame
memory. The marker is `M5:T308:S6:DOUBLE-FAULT-CONTAINMENT:OK`.

Read-only comparison remains Intel 80386 PRM Chapter 9, Bochs 2.6
`cpu/exception.cc` and `cpu/fetchdecode.cc`, and PCjs 2.00.0
`machines/pcx86/modules/v2/cpux86.js` and `x86help.js`. Both references
separately model contributory-pair escalation and terminal triple-fault
handling; neither requires a different result for the admitted combinations.
The sweep also checked the 16-bit same-CPL and outer planners: no new producer
or delivery route is admitted. Page-fault combinations, hardware/NMI,
task/V86 paths, and an actual triple-fault shutdown/reset policy remain
deferred.

## S7 Closure Preparation Evidence

The sole current artifact target is `vm-0-5-0308`. Its generated executable is
`build/output/nxvm_0_5_0308.exe` with SHA-256
`A6BE95BB5C9647F72DB8C28982C8DEFB95CBDA9C9FE92F04F3B4EDC592BC646F`.
The build-tree copy has the same SHA-256. The artifact contains no new CPU or
test semantics beyond S1--S6.

The S7 retained sweep found three 80386 protected focused probes whose blank
error-vector fixtures had correctly changed from an original contributory
terminal to `#DF(0000)`: T302 operand/address, T303 control transfer, and
T261 task switching. Their helpers now select `#DF` only for the 80386
protected contributor-pair case; their 80286 and real-mode assertions retain
their original exception class. No production path changed in S7. This fixes
the probe callers before they read a captured CPU snapshot, and retains every
existing state assertion.

`cmake --build --preset current-gates-gcc` passed all 51 static/governance
targets and 137/137 current CTests. The direct retained intersection passed
the T308 S2, S3, S5, and S6 markers together with T301, T304, T305, T306,
T307, T261, T302, and T303 coverage. Documentation governance,
`verify-current-artifact-target`, and `git diff --check` passed.

One 45-second no-input, no-media product observation launched the 0308
artifact hidden. The process remained alive, but its host window handle was
zero and its title was empty. The observer stopped only that launched process.
No guest command, guest checkpoint, or product regression conclusion was
obtained; manual owner-controlled observation remains a verification-only
follow-up. T308 remains active pending coordinator acceptance. Deferred
boundaries remain page-fault combinations, hardware/NMI, task/V86 paths, and
actual triple-fault shutdown/reset.
