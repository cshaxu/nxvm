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
| S3 | Preserve the retained outer RETF code-descriptor eligibility after the shared cache-helper correction | Conforming-code rejection and unchanged state | No outer IRET work. |
| S4 | Outer RETF alignment for 16/32 frames, including immediate stack adjustment | New SS/SP validation, selector/cache/target failures, commit ordering | No IRET outer frame or CPL delivery work. |
| S5 | Outer IRET 16/32 planner and flags/CPL return rules | Full frames, fault precedence, stack/cache/flags atomicity | No task return, gate, V86, or exception-delivery change. |
| S6 | Family closure | Retained T293/T303/T304/T305/T260/T261 probes and full governance | One bounded product observation only after all focused work. |

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

## S2 Same-CPL IRET Evidence

S2 adds `_ser_iret_protected_same` on the existing core executor. It preflights
the complete 6-byte or 12-byte frame with `_s_test_ss_pop`, then peeks rather
than pops the return IP/EIP, CS slot, and FLAGS/EFLAGS. It validates the
same-CPL selector/type/DPL/present conditions, prepares a candidate code cache,
checks the candidate target limit, and applies the protected FLAGS mask before
the descriptor accessed-byte write and final CS/EIP/EFLAGS/SP-or-ESP commit.
The ordinary protected branch in `_e_iret` selects this route only for a
non-VM, non-NT same-CPL return. Existing VM, nested-task, and outer-return
branches remain unchanged.

The candidate code-cache helper now accepts code descriptors generally. The
S2 return route supplies the Intel same-CPL conforming/non-conforming DPL
checks before invoking it. P2 re-audited all six existing callers: the call
gate, 16-bit interrupt gate, 32-bit same-CPL gate, and outer IRET callers
already make their non-conforming eligibility explicit; `_ser_ret_far_outer`
now does so as well, after its retained new-SS validation and before candidate
cache preparation. No caller outside the S2 same-CPL IRET route gains a new
admitted descriptor form.

`core-machine-protected-iret-smoke` proves default 32-bit IRET, `66h` 16-bit
IRET, `67h` retaining the 32-bit frame, a same-CPL conforming-code return, and
a 16-bit SS address size with a 16-bit operand frame. It also proves `#NP` for
a non-present CS and `#GP` for code type, DPL, and target-limit failures plus
`#SS` for an unreadable complete frame. Every negative case checks the first
diagnostic and preserves EIP, EFLAGS, ESP, CS cache, SS cache, and the code
descriptor access byte. A CPL3 return additionally proves that the frame's
IOPL and IF bits remain masked when CPL exceeds IOPL while permitted arithmetic
flags restore.

The retained T293 outer-return atomicity probe now includes a conforming-code
CS frame for both 16-bit outer RETF and outer IRET. It requires the retained
`#GP(selector)` delivery, unchanged entry flags/CS/SS cache and descriptor
access byte, and the exact delivered exception record. The pre-existing
same-CPL and outer 16-bit success/failure paths remain in that probe.

## S4 Outer RETF Evidence

S4 extends `_ser_ret_far_outer` for both operand widths. It reads the 16-bit
frame as IP, CS, parameter area, SP, SS and the 32-bit frame as EIP, CS dword
slot, parameter area, ESP, SS dword slot. `66h` selects that width; `67h` does
not alter it. The old stack route checks the return IP/EIP and CS words before
the new-SP/SS peeks validate their own accesses. The retained new-SS-before-CS
validation order remains intact.

After all candidate CS, SS, privilege, present, and target-limit checks pass,
descriptor accessed bytes are written and the new SS, CS, EIP, CPL, and stack
pointer publish together. The new SS B bit independently chooses whether the
returned pointer plus `imm16` updates ESP or only SP; neither the operand-size
prefix nor `67h` changes that choice.

The focused return probe adds 80386 outer RETF immediate forms for 16-bit
operand size, 32-bit operand size, and `67h` with a 32-bit operand frame. It
proves both target-stack B-bit outcomes and emits
`M5:T306:S4:OUTER-RETF:OK`. A 32-bit-frame non-present new SS case proves
`#SS(0x30)` delivery without early user CS/SS/ESP or descriptor-access publish.
T293 remains the retained outer-16 atomicity marker; T303 and T305 remain
retained transfer and delivery intersections. Outer IRET is unchanged.

## S5 Outer IRET Evidence

S5 replaces the retained 16-bit-only outer-Iret planner with
`_ser_iret_protected_outer(context, byte)`. It reads the 16-bit frame as IP,
CS, FLAGS, SP, SS and the 32-bit frame as EIP, CS dword slot, EFLAGS, ESP, SS
dword slot. `66h` selects the frame width; `67h` does not. The source SS stack
address-size remains independent, while the returned SS B bit selects whether
the final stack pointer write updates ESP or only SP.

The planner first checks the full old-stack frame and peeks every frame field.
It preserves the retained CS descriptor/type/present/RPL/DPL checks before the
new SS selector/cache validation. Candidate code and stack caches, including
the return-target limit check, complete before either descriptor accessed byte
or CS, SS, EIP, EFLAGS, CPL, or stack state is published. The established
EFLAGS reserved-bit mask is retained; no VM, task, nested-task, or exception
delivery policy is admitted.

The S5 additions to the focused atomicity probe cover 80386 outer IRET with a
16-bit frame, a default 32-bit frame, and `67h` with the same 32-bit frame.
They check CS/SS CPL 3 caches, EFLAGS restoration, and both target-SS B-bit
outcomes. A 32-bit non-present target SS case proves delivered `#SS(0x30)`
without early state or descriptor-access publication; a 32-bit target-limit
case proves delivered `#GP(0)` with the unaccessed target-code descriptor
unchanged. The probe emits `M5:T306:S5:OUTER-IRET:OK`.

The S5 similar-issue sweep covered `_e_iret`, the outer-return planner,
`_s_test_ss_pop`, `_s_peek_ss_pop`, candidate code/stack cache preparation,
and T293/T303/T305 retained probes. Same-CPL IRET, outer RETF, real-mode,
VM/task branches, gates, and paging retain their previously frozen behavior.

## S6 Outer IRET EFLAGS Privilege Evidence

The Intel 80386 `IRET` protected-mode rules use the privilege state before the
return: IOPL changes only at old CPL 0, IF changes only when old CPL is no
greater than the old IOPL, and RF remains an ordinary restorable 80386 EFLAGS
bit. VM is unaffected by ordinary protected returns; a frame requesting VM is
routed to the retained virtual-8086 branch rather than this planner. The
16-bit frame form additionally cannot supply high EFLAGS bits.

`_ser_iret_protected_outer` now creates an explicit commit mask from the old
CPL and old IOPL: reserved and VM bits always preserve entry state; nonzero
old CPL also preserves IOPL; and an old CPL greater than old IOPL preserves
IF. RF is deliberately absent from that mask. The same-CPL IRET sweep found
the same erroneous nonzero-CPL RF mask and removes it there as the identical
Intel rule; its IOPL, VM, and IF conditions remain otherwise unchanged.
Real-mode, current-V86, returned-V86, and nested-task EFLAGS assignments use
different frozen paths and are not changed.

Read-only Bochs 2.6 `cpu/iret.cc` confirms the old-CPL/old-IOPL IF and IOPL
conditions and the ordinary RF restore mask; PCjs 2.00.0
`machines/pcx86/modules/v2/x86help.js` remains the read-only IRET path
comparison. Neither source is copied. The Intel 80386 Programmer's Reference
Manual remains semantic authority.

The focused probes now cover a CPL0-to-CPL3 outer 32-bit frame that restores
IF, IOPL, and RF, a 16-bit representative that restores IF and IOPL, and a
CPL1-to-CPL3 32-bit frame where incoming IF and IOPL remain preserved while RF
restores. VM stays clear in every ordinary protected-frame result, matching
its explicit non-restoration rule. The retained same-CPL CPL3 probe now proves
RF restoration while IOPL and IF remain masked. Failure probes retain the
pre-commit CS/SS/EFLAGS/access-byte assertions. The marker remains
`M5:T306:S5:OUTER-IRET:OK` because S6 narrows its existing EFLAGS contract.

## S7 Closure-Preparation Evidence

The one current artifact target is `vm-0-5-0306`. The generated
`build/output/nxvm_0_5_0306.exe` has SHA-256
`276A1ECE78FF66FCD2746C1DF9171E37716F203F36F9BAECC9ACB06DCBDF2DE3`.

The final managed `cmake --build --preset current-gates-gcc` run passes all
51 static/governance targets and 135/135 current-gate CTests. Focused return
probes emit `M5:T306:S2:SAME-CPL-IRET:OK`,
`M5:T306:S4:OUTER-RETF:OK`, and `M5:T306:S5:OUTER-IRET:OK`; the latter retains
the S6 EFLAGS privilege cases. Retained probes emit
`M5:T293:S2:PROTECTED-RETURN-ATOMICITY:OK`,
`M5:T303:CONTROL-TRANSFER:OK`, `M5:T304:DESCRIPTOR-SYSTEM:OK`,
`M5:T305:INTERRUPT-ENTRY:OK`, `M5:T260:S3:TSS-IOMAP:CORPUS:OK`, and the T261
task-switch markers. Documentation governance and `git diff --check` pass.

One bounded owner-supplied product observation ran the 0306 artifact without
media or guest input for ten seconds. The process remained alive but exposed
main-window handle zero, so host automation could not safely discover a guest
surface. It was terminated and cleanup found no process residue. No guest
command, Setup checkpoint, or CPU conclusion was obtained. This is a
host-automation limitation retained for owner-controlled manual observation;
it is not retried in T306.

At its closure-preparation point, T306 remained pending coordinator acceptance.
The coordinator subsequently accepted and closed T306; this is historical
closure-preparation evidence only: it did not set the project Idle, close
T306, change Queue state, or authorize T307. Deferred boundaries remain task
and nested-task returns, task/call gates, virtual-8086 returns, reset or
triple-fault policy, paging-policy expansion, new fault origins, and public
ABI changes.

The retained T293 outer-return atomicity, T303 control-transfer, and T305
interrupt-entry probes remain required regression consumers. S2 does not
admit outer RETF/IRET, task/V86 returns, gates, or exception-delivery changes.
