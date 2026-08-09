# M5 T306 S1: Protected Return Family Admission Audit

## Scope and Authority

T306 admits only the 80386 protected-mode `IRET` and `RETF` return family:
same-CPL and outer-privilege returns, 16-bit and 32-bit operand-size frames,
selector/cache validation, checked-stack validation, flags handling, and
failure-before-commit behavior. It retains the single core CPU executor and
the existing checked stack and logical-memory routes.

The semantic authority is the Intel *80386 Programmer's Reference Manual*
(1986), `IRET` and `RET` instruction entries and its protected-mode return,
stack, and privilege-transition descriptions. Read-only comparison identities
are Bochs 2.6 (`cpu/ctrl_xfer16.cc`, `cpu/ctrl_xfer32.cc`, `cpu/iret.cc`,
`cpu/segment_ctrl.cc`, `cpu/segment_ctrl_pro.cc`, and `cpu/fetchdecode.cc`)
and PCjs 2.00.0 (`machines/pcx86/modules/v2/x86ops.js`, `x86help.js`,
`segx86.js`, and `cpux86.js`). They are behavior-location references only;
no source is imported or copied.

Deferred: task and nested-task returns, task and call gates, general task
switching, virtual-8086 returns, new fault origins, reset/triple-fault policy,
and paging-policy work.

## Current Owner and Path Inventory

| Area | Current core route | S1 classification |
| --- | --- | --- |
| IRET decode and protected dispatch | `_e_iret`, `IRET` opcode handler in `src/core/machine/cpu_instructions.c` | In scope: ordinary protected return currently ends in `#CE`; 16-bit outer dispatch exists. |
| Outer 16-bit IRET | `_ser_iret_protected_outer_16` | Retained implementation baseline; requires matrix and atomicity evidence before any shared-helper change. |
| RETF decode and width selection | `RETF_I16`, `RETF`, `_e_ret_far` | Retained T303 same-CPL coverage; in scope for outer 16/32 alignment. |
| Same-CPL RETF | `_ser_ret_far_same` | Retained T303 behavior, including 16/32 operand-width forms. |
| Outer RETF | `_ser_ret_far_outer` | In scope: currently rejects any width other than 16-bit with `#CE`. |
| Shared frame access | `_s_test_ss_pop`, `_s_peek_ss_pop`, `_kec_pop`, `_kec_push` | Single checked stack route; preserve it and audit all preflight/commit boundaries. |
| Descriptor/cache validation | `_ksa_load_sreg`, `_kma_test_logical`, candidate cache helpers | Existing sole core owner; no second selector/cache path is admitted. |
| Prior consumers | T293 protected-return atomicity, T303 control transfer, T304 descriptor-system, T305 32-bit gate entry, T260/T261 TSS/task retained checks | Intersections retained as regression evidence; task semantics remain out of scope. |

## Frozen Form and Commit Matrix

The selector value is always 16 bits. In a 32-bit return frame it occupies a
32-bit stack slot, whose upper bits are not selector identity. `66h` changes
the return operand/frame width. `67h` does not change the implicit return-frame
width; SS stack-address size independently selects SP or ESP address/wrap
semantics.

| Form | Frame in stack order | Current state | T306 disposition |
| --- | --- | --- | --- |
| Protected same-CPL IRET, 16-bit | IP, CS, FLAGS | ordinary protected branch terminates in `#CE` | S2 implement and probe. |
| Protected same-CPL IRET, 32-bit | EIP, CS dword slot, EFLAGS | ordinary protected branch terminates in `#CE` | S2 implement and probe. |
| Protected outer IRET, 16-bit | IP, CS, FLAGS, SP, SS | `_ser_iret_protected_outer_16` exists | S4 retain, audit, and strengthen focused proof. |
| Protected outer IRET, 32-bit | EIP, CS dword slot, EFLAGS, ESP, SS dword slot | no admitted planner | S4 implement and probe. |
| Protected same-CPL RETF, 16/32-bit | IP/EIP, CS selector slot | T303 covered | Retain; rerun with T306 atomicity intersections. |
| Protected outer RETF, 16-bit | IP, CS, SP, SS | `_ser_ret_far_outer` admits 16-bit only | S3 audit/align and probe. |
| Protected outer RETF, 32-bit | EIP, CS dword slot, ESP, SS dword slot | explicit `#CE` rejection | S3 implement and probe. |
| Real-mode IRET/RETF | Existing 16/32 real return routes | retained outside protected admission | Regression-only. |
| V86, NT/task return, gates | Existing rejection/branch behavior | deferred | Do not broaden in T306. |

For each protected form, focused synthetic probes must establish first-fault
precedence and preserve entry EIP, flags, stack pointer/address width, CPL,
CS/SS selector and cache fields, and descriptor accessed/busy bytes until all
required frame, selector, privilege, present/type, target-limit, and new-stack
checks have succeeded. Successful outer returns may publish descriptor access
updates only with the final architectural state commit. Exact EFLAGS writable
mask and privilege constraints remain an S2/S4 focused-proof item; S1 does not
infer them from the current helpers.

## Producer and Consumer Intersections

T305's 32-bit IDT planner publishes an optional error-code dword below the
saved EIP/CS/EFLAGS return portion. A handler must adjust that error code before
an `IRET`; T306 must not normalize or reinterpret the producer frame. T293's
16-bit outer-return atomicity remains the retained failure baseline. T303's
same-CPL `RETF` coverage remains the retained same-level control-transfer
baseline. T304's descriptor and T260/T261 task checks guard descriptor-cache
and TSS consumers without admitting task-return behavior.

## Proposed Batches and Stop Boundaries

| Batch | Work | Focused synthetic proof | Stop boundary |
| --- | --- | --- | --- |
| S2 | Same-CPL protected IRET, 16/32 frame planning and atomic commit | Frame order, `66h`, SS address size, target/cache/limit failures, flags and unchanged state | No outer return, task, V86, or fault-origin expansion. |
| S3 | Outer RETF alignment for 16/32 frames, including immediate stack adjustment | New SS/SP validation, selector/cache/target failures, commit ordering | No IRET outer frame or CPL delivery work. |
| S4 | Outer IRET 16/32 planner and flags/CPL return rules | Full frames, fault precedence, stack/cache/flags atomicity | No task return, gate, V86, or exception-delivery change. |
| S5 | Family closure | Retained T293/T303/T304/T305/T260/T261 probes and full governance | One bounded product observation only after all focused work. |

Any evidence that the required behavior needs a second executor, second state
owner, host shortcut, public layout exposure, task-return semantics, or a
change to fault/exception delivery stops T306 for coordinator direction.

## Similar-Issue Sweep and S1 Result

The S1 sweep searched `cpu_instructions.c`, focused machine probes, CMake
registrations, and T293--T305 evidence for `IRET`, `RETF`, return-frame,
outer-return, `_ser_iret`, `_ser_ret_far`, and checked-stack helpers. The only
production gaps admitted by this packet are ordinary protected IRET and
32-bit outer RETF/IRET planning. Same-CPL RETF and real-mode routes are retained;
V86, nested-task, and general task paths are deferred rather than silently
folded into this family.

S1 changes no CPU behavior, probe, CMake artifact target, Queue entry, or
product observation.
