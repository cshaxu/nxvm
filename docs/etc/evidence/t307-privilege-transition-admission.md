# M5 T307 S1: Privilege Transition Family Admission Audit

## Scope and Authority

T307 admits only 80386 protected-mode entries from CPL 3 to CPL 0 through a
32-bit interrupt or trap gate and a 32-bit call gate. The paired return
boundary is the accepted T306 outer 32-bit `IRET`/`RETF` work: this task owns
the matching TSS `SS0:ESP0` source, new-stack frame, selector/cache, privilege,
and preflight-then-commit entry behavior. It retains the single core CPU
executor, checked logical stack route, and core-owned selector/cache state.

The semantic authority is the Intel *80386 Programmer's Reference Manual*
(1986), protected-mode interrupt/trap-gate, call-gate, TSS, stack-switch, and
return descriptions. Read-only comparison identities are Bochs 2.6
(`cpu/exception.cc`, `cpu/call_far.cc`, `cpu/segment_ctrl_pro.cc`,
`cpu/iret.cc`, and `cpu/ctrl_xfer32.cc`) and PCjs 2.00.0
(`machines/pcx86/modules/v2/segx86.js`, `x86ops.js`, and `x86help.js`). They
are behavior-location references only; no source is imported or copied.

Deferred: task and nested-task entry/return, task gates, general task
switching, virtual-8086 entries, new fault origins, reset/triple-fault policy,
paging-policy work, product UX, public ABI, and source import.

## Current Path Inventory

| Area | Current core route | S1 classification |
| --- | --- | --- |
| IDT gate dispatch | `_ser_int_protected` | Retained profile/IDT/type/DPL front end; selects 16-bit route or 32-bit same-CPL route. |
| 16-bit inner entry | `_ser_int_protected_16` | Retained intersection: validates selector/TSS/new SS, uses a 16-bit frame and `_s_test_stack_frame_16`. |
| 32-bit same-CPL gate | `_ser_int_protected_32_same` | Retained T305 path; explicitly rejects target CPL lower than source CPL with `#CE`. |
| Call-gate dispatch | `_ser_call_far_call_gate` | Retained 16-bit gate/frame path; accepts only operand byte 2 and uses `_s_test_stack_frame_16`. |
| TSS reads | `_s_read_tss`, TR cache | Reusable checked logical TSS route; existing 32-bit TSS branches read offsets 4/8 but truncate or reject ESP above 16 bits for old 16-bit consumers. |
| New-stack validation | `_ksa_prepare_stack_sreg`, `_s_test_stack_frame_16` | Reusable selector/cache validation; no 32-bit target-stack frame preflight helper exists. |
| Return pairing | T306 outer `IRET`/`RETF` planners | Retained consumer boundary: 32-bit outer frames exist; T307 must produce their matching entry layout without changing return code. |
| Focused evidence | T305 interrupt-entry, T288 call-gate, T260 TSS I/O-map, T261 task-switch probes | Retained coverage for same-CPL gates, 16-bit call-gate entry, 32-bit TR/TSS loading, and task consumers; no CPL3-to-CPL0 32-bit entry probe exists. |

## Frozen Admission Matrix

| Family | Profile and mode | Source and target | Admitted 32-bit form | Required frame and flags | Current disposition |
| --- | --- | --- | --- | --- | --- |
| Software `INT n`, `INT3`, `INTO` | 80386, protected, non-V86 | CPL 3 to nonconforming CPL 0 code | 32-bit IDT interrupt/trap gate | On new SS0 stack: EIP, padded CS, EFLAGS, old ESP, padded old SS; interrupt gate clears IF and TF, trap gate clears TF | S2 implement. Software gate DPL applies; retain existing front ends. |
| Hardware IRQ/NMI and established fault delivery | 80386, protected, non-V86 | CPL 3 to CPL 0 | 32-bit IDT interrupt/trap gate | Same frame, with error code preceding EIP only for already admitted error-code sources | S4 integrate only after common planner; hardware bypasses software DPL. No new fault origin. |
| Far `CALL` through gate | 80386, protected, non-V86 | CPL 3 to nonconforming CPL 0 code | 32-bit call gate with a 32-bit target offset | New SS0 frame holds padded old SS, old ESP, padded old CS, old EIP; gate parameter count chooses dword copies between stack-pair and return pair | S3 implement zero-count first, then bounded count/copy evidence if required by the frozen gate form. |
| 16-bit IDT/call-gate intersection | 80286/80386, protected | Existing inner transition | Existing 16-bit gate/frame/TSS behavior | 16-bit frame and old TSS stack route | Retain and regression-test only; do not rewrite during S2/S3. |
| Same-CPL 32-bit IDT gate | 80386, protected | CPL unchanged | Existing T305 planner | EIP, padded CS, EFLAGS; no new stack | Retain; shared helper changes require T305 regression. |
| Real mode, V86, task/task gate, direct far transfer | Any deferred mode/family | Out of admission | None | None | Retain existing behavior or rejection; no implementation in T307. |

For the ordinary non-error 32-bit IDT entry, the lowest address after the
stack switch contains saved EIP, padded CS, EFLAGS, old ESP, then padded old
SS. An already admitted error-code source places its error code below that
five-dword frame. The call-gate frame uses the corresponding old-stack pair
and return pair, with gate-count parameter copies between them. Operand-size
prefixes do not narrow a 32-bit gate frame; target SS B independently selects
ESP or SP addressing/publication. The selector identity in every padded slot
remains 16 bits.

## Validation and Commit Contract

Before descriptor accessed-byte writes, target state, new-stack writes, IF/TF
changes, pending-event publication, or CPL change, the admitted planner must
validate: IDT or call-gate bounds/type/present and software DPL where
applicable; target CS selector/type/nonconforming DPL/present/limit; valid busy
32-bit TR; checked TSS `ESP0` and `SS0`; target SS selector/RPL/DPL/writable/
present/cache; every source parameter read; and every target-stack frame write.

Any failure must leave source CS/SS caches, EIP/EFLAGS, SP/ESP, CPL, target
descriptor accessed bytes, target stack contents, and event/pending state
unpublished. Existing error-code and terminal-delivery containment policies are
retained; S4 may only connect already admitted sources after S2 proves the
common planner.

## Batches and Stop Boundaries

| Batch | Work | Focused synthetic proof | Stop boundary |
| --- | --- | --- | --- |
| S2 | 32-bit IDT interrupt/trap common CPL3-to-CPL0 planner with TSS32 SS0:ESP0 and exact non-error frame | Software INT and trap/interrupt flag forms; frame/SS B/selector/cache/TSS/atomic negative matrix | No hardware/fault origin, call gate, parameter copy, outer return change, or V86/task path. |
| S3 | 32-bit call-gate CPL3-to-CPL0 planner | 32-bit target offset, zero-count frame, then bounded parameter-count copy and failure preservation | No direct far transfer, task gate, or return implementation. |
| S4 | Existing hardware/NMI and established error-code source integration | Hardware DPL bypass, error-code six-dword frame, rejection publication and retained T305 diagnostics | No new fault source or recursive/double-fault policy. |
| S5 | Family closure | T307 focused probes, retained T305/T306/T304/T260/T261, current gates, artifact, and one bounded observation | No follow-on task until coordinator acceptance. |

Stop and report any need for a second executor/state owner, a VM/platform
dependency, a public raw layout, an exception-delivery policy expansion, or
task/V86/paging semantics.

## Similar-Issue Sweep and S1 Result

The S1 sweep covered `_ser_int_protected`, `_ser_int_protected_16`,
`_ser_int_protected_32_same`, `_ser_call_far_call_gate`, TSS read and stack
frame helpers, selector/cache preparation, interrupt front ends, T306 outer
returns, T305/T288/T260/T261 focused probes, descriptor constants, and 80386
read-only comparison paths.

The only admitted production gaps are the 32-bit inner-privilege IDT and
call-gate planners plus their checked 32-bit new-stack support. The existing
16-bit path, same-CPL 32-bit planner, TSS I/O-map consumers, outer returns,
and task/V86 paths are classified as retained or deferred above. S1 changes no
CPU behavior, CMake artifact identity, Queue entry, or product observation.

## S2 Common Planner Evidence

S2 adds `_ser_int_protected_32_outer` and reaches it only from the existing
32-bit IDT planner after the gate target is known to be a lower-DPL
nonconforming code segment. It accepts only an ordinary interrupt or trap
gate, a non-V86 protected source, and a valid busy 32-bit TR. The planner
reads `ESP0` and `SS0` through the existing checked TSS route, validates the
candidate code and stack caches and every five-dword destination slot, then
writes descriptor accessed bytes and publishes the new stack, CPL, frame, CS,
EIP, and gate-controlled IF/TF state. Thus no checked memory or selector
operation remains after publication begins.

The new `core-machine-idt-privilege-entry-smoke` focused synthetic probe
records the following S2 matrix:

| Case | Expected result |
| --- | --- |
| CPL3 software `INT` through a DPL3 32-bit interrupt gate | CPL0 kernel CS/SS, `ESP0 - 20`, five dwords `EIP`, padded CS, EFLAGS, old ESP, padded SS; IF and TF clear. |
| CPL3 software `INT` through a DPL3 32-bit trap gate | The same frame and transition; TF clears while IF remains set. |
| Target SS B clear | The planner uses 16-bit stack addressing for all five dword writes while preserving the normal frame layout. |
| CPL3 software `INT` through a DPL0 gate | `#GP(vector * 8 + 2)` before CS/SS, EIP, ESP, EFLAGS, or accessed-byte mutation. |
| Existing external PIC origin through a DPL0 gate | Delivery bypasses the software DPL check and reaches the CPL0 interrupt-gate frame; the retained PIC state changes only after successful planner completion. |
| Non-present gate target CS | `#NP(CS)` before candidate-stack or frame publication, including both target descriptor accessed bytes. |
| Non-present TSS `SS0` descriptor | `#SS(SS0)` before all candidate-cache, frame, register, or accessed-byte publication. |

The sweep revisited the 16-bit entry planner, same-CPL 32-bit planner, TSS
I/O-map consumer, selector/cache preparation, checked stack helpers, PIC
front-end, and T306 return planners. S2 changes only the lower-DPL 32-bit IDT
route and adds a 32-bit target-stack preflight helper. It intentionally rejects
error-code frames in this new route until the admitted S4 integration; it does
not change hardware/NMI front ends, call gates, task/V86 paths, or return
semantics.

## S3 Call-Gate Planner Evidence

Intel's 80386 call-gate count is a dword count for a 32-bit call gate. S3
therefore admits the bounded architectural range (the descriptor's five-bit
count, zero through 31) rather than treating a nonzero count as an internal
error. `_ser_call_far_call_gate_32` accepts only a protected, non-V86,
present 32-bit call gate whose DPL admits the source CPL/RPL and whose target
is a present nonconforming code segment at a lower CPL. It reads all selected
old-stack dwords before publishing the new stack, validates every `4 + count`
new-stack dword slot, then writes the candidate stack/code accessed bytes and
publishes the new SS/ESP/CPL, old SS/ESP pair, parameters, old CS/EIP pair,
and target CS/EIP.

The resulting low-to-high new-stack layout is saved EIP, padded CS, parameter
zero through parameter `count - 1`, old ESP, and padded old SS. The parameter
order is unchanged from the old stack. The retained 16-bit call-gate planner
continues to accept only its matching 16-bit descriptor/frame form; a gate
type and operand-size mismatch now reports `#GP(selector)` with selector RPL
bits clear instead of falling through with a non-architectural RPL-bearing
code.

`core-machine-call-gate-privilege-entry-smoke` proves the zero-count and
two-dword forms, including the target CS/SS caches, return continuation EIP,
and frame contents. Its negative cases prove DPL `#GP`, non-present `#NP`,
wrong-gate-type `#GP`, and a source-parameter stack `#SS`; each asserts old
CS/SS, EIP, ESP, EFLAGS, and both target descriptor accessed bytes remain
unchanged. The retained `core-machine-call-gate-smoke` continues to prove the
existing 16-bit gate consumer.

The S3 sweep reviewed far-CALL descriptor dispatch, both call-gate types,
selector diagnostic creation, checked old-stack reads, target-stack preflight,
TSS32 reads, selector/cache preparation, and T306 outer-return consumers. It
does not alter IDT, error-code, PIC/NMI, task, V86, paging, or public-interface
paths.

## S4 Call-Gate Preflight Evidence

S4 makes the TSS-provided `SS0` preflight explicit in the 32-bit call-gate
planner: selector null/TI/RPL mismatch remains `#TS(SS0)`, while the selected
descriptor remains subject to the Intel-required `#TS(SS0)` type/DPL and
`#SS(SS0)` not-present outcomes. The candidate stack-cache helper remains the
only cache constructor after those checks, so no parallel segment-cache
semantics were added.

The call-gate focused probe now directly proves, for every runnable negative
case, the first fault and unchanged entry CS/SS cache, EIP, ESP, EFLAGS/CPL,
and target CS/SS accessed bytes: target CS wrong type (`#GP`), target CS not
present (`#NP`), target SS not present (`#SS`),
and the target new-stack frame boundary (`#SS(0)`). The existing software DPL,
gate type/present, and old parameter-source checks remain alongside them.

At the S4 observation point, the prepared RPL-mismatched TSS `SS0` case emitted
the Intel-correct `#TS(SS0)`, but did not retain a terminal observation under a
one-instruction budget. S5 identified that the planner's return boundary, not
TS delivery policy, caused that observation gap; its explicit precheck now
keeps `#TS(SS0)` observable with the unchanged-state assertions. The retained
16-bit call-gate probe was rerun and showed no matching implementation defect.

## S5 Call-Gate Stack Fault Classification

Intel 80386 and the read-only Bochs 2.6 `cpu/call_far.cc` call-gate path agree
that a TSS-provided new SS descriptor which is not a writable data segment or
whose DPL does not match the target CPL raises `#TS(SS selector)`, not `#GP`.
S5 moves that rule into `_ser_check_call_gate_stack_sreg`, used by both the
32-bit S3 planner and the retained 16-bit privilege call-gate branch. Null,
TI, and RPL failures remain `#TS`; not-present remains `#SS`. Both callers
then invoke the existing `_ksa_prepare_stack_sreg` cache constructor, so the
classification change does not create a second cache implementation.

The prepared 32-bit target-SS type/DPL and selector-mismatch cases now retain
`#TS(SS0)` as the first fault under a one-instruction budget, with the same
unchanged-state and accessed-byte assertions as the other negative cases. The
earlier non-terminal observation was caused by the planner's return boundary,
not a reason to alter TS delivery; no T308 delivery implementation is needed
from this classification result.

## S6 Evidence Consistency

S6 made no CPU, probe, or build change. It preserves S4 as the historical
one-instruction observation and records that S5's explicit planner precheck
made the Intel-correct `#TS(SS0)` classification observable with the focused
unchanged-state assertions. It is not an unresolved delivery limitation or a
follow-on task input.

## S7 Closure Preparation

The T307 closure artifact identity is `vm-0-5-0307`; its executable is
`nxvm_0_5_0307.exe` with SHA-256
`933A06EAED336A79A7855A8B5D9A93C53DBDB1CB1600994E3B27CA141D9235DA`.

`cmake --build --preset current-gates-gcc` completed all 51 static/governance
targets and 137 of 137 CTests. The selected focused and retained run also
completed 9 of 9 CTests: the T307 IDT privilege-entry and call-gate
privilege-entry probes, plus retained T304 descriptor-system, T305
interrupt-entry, T306 protected-IRET and protected-return-atomicity, T288
call-gate, T260 TSS-I/O-map-port, and T261 task-switch probes. Documentation
governance and `git diff --check` pass for this preparation.

One bounded product observation launched the 0307 executable hidden with no
arguments, guest input, or media. After ten seconds the host process was still
alive but had `MainWindowHandle=0`; the observer stopped only that launched
process. This is a host-automation limitation, not a guest observation or
checkpoint, and the attempt was not retried.

At its closure-preparation point, T307 remained pending coordinator acceptance.
The coordinator subsequently accepted and closed T307. Deferred boundaries
remain error-code and hardware-NMI integration, task and virtual-8086 paths,
paging policy, and any new fault-delivery origin.
