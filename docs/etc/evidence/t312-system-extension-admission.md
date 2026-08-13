# T312: Trace-Driven 80386 System-Extension Admission

## Authority

Intel 80386 Programmer's Reference Manual system, task-management, virtual-8086,
and debug-register chapters are authoritative. Bochs 2.6 read-only comparison
paths are `cpu/tasking.cc`, `cpu/protect_ctrl.cc`, `cpu/cpu.cpp`, and
`cpu/fetchdecode.cc`; PCjs 2.00.0 paths are
`machines/pcx86/modules/v2/x86.js`, `x86op0f.js`, and `x86help.js`. No source,
trace, guest image, or implementation is imported.

## Form And Consumer Matrix

| Candidate family | Existing decode/handler surface | Existing proof | Real VM/M5 consumer | S1 P0 disposition |
| --- | --- | --- | --- | --- |
| 32-bit TSS switch | 16-bit far-JMP task switch is implemented; 32-bit branches and task-call helpers are deferred. | `core-machine-task-switch-smoke` proves the retained 16-bit TSS path only. | None. The smoke is a focused proof, not a VM/M5 consumer. | No admission. |
| Task gate / nested return | Named task-gate and task-call paths are deferred; IRET has V86/task branches outside admitted T306 behavior. | No focused proof for an admitted 32-bit task-gate or nested return. | None. | No admission. |
| LDT breadth | `SLDT`/`LLDT` and private LDTR descriptor access exist for admitted T304 selector-table forms. | T304 descriptor-system proof fixes only its bounded forms. | None beyond those closed forms. | No admission. |
| Virtual-8086 | V86 condition branches are present around returns and I/O forms. | No admitted V86 probe. | None. Current Windows observation did not establish a V86 checkpoint. | No admission. |
| Debug/test registers | Decoder-facing CR/DR/TR selection names exist; admitted T304 behavior is CR0/CR2/CR3 only. | Product debugger uses copied core debug capability, not guest DR/TR instructions. | None. Debugger tests are not guest DR/TR consumers. | No admission. |
| Remaining system forms | `0F 00h`/`01h` handle only named T304 forms; later/reserved forms are rejected. | T304 focused probe retains legal and rejected forms. | None. | No admission. |

## Sweep And Result

The sweep covered instruction tables, `INS_0F_00`/`INS_0F_01`/control-register
paths, task/TSS helpers, V86 branches, diagnostic and trace hooks, CMake
registrations, focused probes, product debugger surfaces, TODO, Queue, and the
recorded Windows checkpoint evidence. T260 TSS-I/O and T261 task-switch tests
are retained architecture proof only; neither creates a present VM or M5
consumer for broader task semantics. No product trace or checkpoint reaches a
missing system-extension form.

Therefore T312 S1 P0 records **no implementation admission**. It does not
construct a trace framework, run a long guest image, add a decoder, or change
runtime behavior. A future packet requires an owner-supplied reproducible
checkpoint or trace naming one missing family and its first required form; it
must then freeze one family, one consumer, Intel sections, read-only comparison
paths, prepared-state proof, and explicit deferrals before implementation.

## S2 Corrective No-Admission Closure

`M5 T312 S1 P0: audit system extension admission` (`67e73ca8`) is the task's
occupied audit admission packet. The identifier gate therefore requires this
evidence closure to use T312 S2 Corrective, not a new T313 task. This is a
governance continuity fact only; it changes neither the matrix nor runtime.

The candidate-specific future evidence threshold is deliberately strict:

| Candidate | Existing handler/probe is insufficient because | Required future consumer evidence |
| --- | --- | --- |
| 32-bit TSS/task gate/nested return | T261 proves only a retained 16-bit focused far-JMP switch. | A bounded VM/M5 checkpoint or trace naming the first 32-bit task transfer form and state boundary. |
| LDT breadth | Existing selector-table support is bounded to T304. | A reproducible consumer requiring a named LDT selector form beyond that matrix. |
| V86 | Branches and TODOs are not a V86 execution checkpoint. | A trace that reaches a specific V86 entry, return, or I/O rule. |
| Debug/test registers | Product debug capability is not guest DR/TR execution. | A guest instruction trace naming one DR/TR form and its expected privilege/fault outcome. |
| Remaining system forms | Decoder names and rejected forms do not create a need. | A reproducible consumer for one named form, with a focused pre-fault contract. |

T312 is therefore withdrawn as an implementation candidate. It makes no claim
that the system-extension families are complete, does not create a T312
artifact, and does not activate T313 or M6.
