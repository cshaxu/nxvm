# M5 T303 S1: Control-Transfer Admission Audit

## Scope And Authority

This S1 record freezes only the 80386 same-privilege control-transfer family
before implementation: near and far `JMP`/`CALL`/`RET`/`RETF`, conditional
relative branches, and `LOOP`/`JCXZ` forms. The authority is the *Intel 80386
Programmer's Reference Manual* (1986), Sections 3.6, 6.3, 7.3, 9.1, 17.1, and
17.2 and the individual `CALL`, `JMP`, `RET`, `Jcc`, `LOOP`, and `JCXZ`
entries.

Read-only comparison used Bochs 2.6 (the September 2, 2012 SVN snapshot)
`cpu/ctrl_xfer16.cc`, `cpu/ctrl_xfer32.cc`, `cpu/ctrl_xfer_pro.cc`,
`cpu/protect_ctrl.cc`, and `cpu/fetchdecode.cc`; and PCjs 2.00.0
`machines/pcx86/modules/v2/x86ops.js`, `x86op0f.js`, `x86help.js`,
`segx86.js`, and `x86mods.js`. The comparison was behavioral only: no source
was copied or translated. It agrees with the Intel distinctions among operand
size, address size, stack-address size, target validation, and far selector
validation. No bridge is required for admission.

## Form, Profile, And Mode Matrix

| Family / encoding / current path | 80386 form and mode contract | Classification and later focused proof |
| --- | --- | --- |
| `Jcc rel8` (`70h`--`7Fh`) and `JMP rel8` (`EBh`); `_e_jcc`, `_kec_jmp_near` | Legal in real mode and protected non-V86 mode. The displacement is signed 8-bit; operand size selects the resulting IP/EIP width. A non-taken conditional branch does not validate or commit the target. | In-scope correction. Prove 16/32 code defaults, `66h`, taken/not-taken code-limit behavior, and EIP/flags preservation on a taken-target fault. |
| `Jcc rel16/rel32` (`0F 80h`--`8Fh`); `_e_jcc` | `rel16` is retained on earlier profiles; `rel32` is 80386-only and selected by operand size. Legal in real and protected non-V86 mode. `67h` has no branch-target-width meaning. | In-scope correction. Prove 16/32 forms, pre-80386 `0F` rejection, and no prefix leakage. |
| `LOOPNZ`/`LOOPZ`/`LOOP`/`JCXZ` (`E0h`--`E3h`); `_e_loopcc`, `JCXZ_REL8` | Legal in real and protected non-V86 mode. The signed displacement is 8-bit. Address size chooses CX versus ECX; operand size chooses the resulting IP/EIP width. `LOOP*` decrements the selected count before its branch condition; `JCXZ` does not modify it. | In-scope correction. Prove all condition/DF-independent forms, `66h`/`67h` crosses, count width, wrap, and the Intel-defined count state if a taken target faults. |
| Near relative `CALL` (`E8h`), near relative `JMP` (`E9h`); `CALL_REL32`, `JMP_REL32` | `rel16` is retained; `rel32` is 80386-only and selected by operand size. Legal in real and protected non-V86 mode. A call pushes a return offset of the operand width; SS stack-address size remains independent. | In-scope correction. Prove 16/32 defaults and overrides, target code-limit checks before return-frame commit, and stack preservation on failure. |
| Near indirect `CALL`/`JMP` (`FF /2`, `FF /4`); `INS_FF`, `_e_call_near`, `_e_jmp_near` | Legal r/m16 or r/m32 form selected by operand size; `67h` selects only the memory effective address. Legal in real and protected non-V86 mode. | In-scope correction. Prove register and memory targets, 16/32 effective-address forms, source-read failure, and target-limit pre-commit behavior. |
| Far immediate `CALL`/`JMP` (`9Ah`, `EAh`); `CALL_PTR16_32`, `JMP_PTR16_32`, `_e_call_far`, `_e_jmp_far` | Pointer is ptr16:16 or ptr16:32 by operand size; selector remains 16-bit. Real mode loads a real-mode CS:IP/EIP pair. Protected mode admits only accessible same-CPL nonconforming code and accessible conforming code without a CPL change. | In-scope correction. Prove real-mode forms and protected same-CPL code-descriptor, present, privilege, limit, selector/cache, and failure-before-visible-commit rules. |
| Far indirect `CALL`/`JMP` (`FF /3`, `FF /5`); `INS_FF` | Memory-only ptr16:16/ptr16:32 form; a ModRM register encoding is `#UD` without target, cache, stack, or flag mutation. Address size selects only the pointer memory access. | In-scope correction. Prove memory-only rejection, both pointer widths, 16/32 effective addresses, and source-read/selector/target failures. |
| Near `RET`/`RET imm16` (`C3h`, `C2h`); `_e_ret_near`, `_kec_ret_near` | Return offset width follows operand size; immediate adjustment remains 16-bit; SS stack-address size is independent. Legal in real and protected non-V86 mode. Target code limit must be checked before the visible pop and parameter adjustment commit. | In-scope correction. Current `_kec_ret_near` pops before `_s_test_cs`; S3 must first prove the observable failure state, then make invalid-target returns atomic for EIP and SP/ESP. |
| Far `RETF`/`RETF imm16` (`CBh`, `CAh`); `_e_ret_far`, `_kec_ret_far`, `_ser_ret_far_same` | Return offset width follows operand size; selector is popped in the same operand-width stack slot; immediate adjustment remains 16-bit. This task admits only real-mode returns and protected same-CPL returns. | In-scope correction. Prove stack-frame reading, code selector/type/present/limit validation, cache and SP/ESP commit order, and `#GP`/`#NP` selectors. |
| Protected far transfer to call gate/task gate/TSS; `_ser_call_far_call_gate`, `_ser_jmp_far_call_gate`, task paths | Not admitted. | Deferred to privilege/call-gate or task-switch work; retain as non-regression paths only. |
| Protected outer-privilege `RETF`, `IRET`, interrupt/exception delivery, V86, paging policy | Not admitted. | Deferred to protected-return/exception, V86, or paging work. T303 must not change their helpers. |

`66h` and `67h` are 80386-only prefixes. The 8086/80186/80286 paths retain
their established 16-bit forms; only 80386 accepts the 32-bit operand/address
forms. The matrix intentionally does not claim V86 legality: it is deferred
with the task's V86 boundary rather than treated as a generic `#UD` rule.

## Current Path Classification And Commit Boundary

- `_kec_jmp_near` and `_kec_jmp_far` normalize the selected target width,
  validate CS or a candidate CS cache, then publish EIP or CS:EIP. They are the
  retained single core route, but need the matrix probes above.
- `_kec_call_near` validates the target before delegating its return push.
  `_kec_call_far` preflights the total return frame and validates a candidate
  CS cache before its two pushes. S3/S4 must still prove the exact late stack
  failure boundary rather than infer whole-instruction rollback.
- `_e_ret_far` already peeks and validates the return frame before dispatching
  same-CPL versus outer return. Its same-CPL path is in scope; the outer path
  is expressly deferred.
- `_kec_ret_near` is the sole admitted production candidate found in this
  sweep: it consumes the return word/dword before checking the resulting
  target against CS. The S3 probe must demonstrate the first fault and then
  require no EIP, SP/ESP, cache, descriptor, or flags mutation on that target
  validation failure.
- `_e_loopcc` updates the selected count before testing a taken target. Intel's
  architected fault-state boundary for a taken out-of-limit loop target is a
  focused S2 question; no conclusion is inferred from the current helper.
- Decoder fetch, ModRM/SIB, logical memory, and stack primitive behavior are
  shared but belong to T302. T303 may consume their frozen semantics and add
  control-transfer regressions only; it must not reopen their implementation.

## Implementation Batches And Focused Synthetic Probes

| Subtask | Bounded implementation | Focused synthetic probe and stop boundary |
| --- | --- | --- |
| S2 | Conditional relative, short/near `JMP`, and loop family. | Add `core_machine_control_transfer_smoke` with 16/32 CS, operand/address-size crosses, conditions, profile rejection, code limits, count rules, and target-fault state. Stop before calls, returns, and selectors. |
| S3 | Near relative and indirect `CALL`/`JMP`, plus near `RET`. | Extend the same probe with return offset widths, r/m source forms, stack-address-size crossings, return target faults, and invalid-RET atomicity. Stop before far selectors/cache. |
| S4 | Real-mode and protected same-CPL far `CALL`/`JMP`/`RETF`. | Add direct/indirect pointer-width, memory-only `#UD`, code-descriptor/type/present/privilege/limit, return-frame, and cache/stack commit probes. Stop before outer return, gates, tasks, or delivery. |
| S5 | Family sweep and closure evidence. | Re-run the focused probe and retained protected-return/call-gate/task-switch regressions, then gates, artifact, and the one owner-supplied Setup observation. A new observation is recorded only; it never widens T303. |

The future probe is prepared CPU/descriptor state only: no system image,
long-start fixture, or host shortcut. Every negative case records the first
fault and asserts the exact field that may commit. Its later marker is
`M5:T303:CONTROL-TRANSFER:OK`.

## Similar-Issue Sweep And Deferrals

The audit used:

```powershell
rg -n "_kec_(call|jmp|ret)|_e_(call|jmp|jcc|loop|ret)|CALL_|JMP_|RET|RETF|LOOP|JCXZ|INS_FF" src/core/machine/cpu_instructions.c
rg -n "protected-return|call-gate|task-switch|real-mode.*386" CMakeLists.txt tests/machine
```

The production hits are classified in the matrix. Existing
`core_machine_protected_return_atomicity_smoke`,
`core_machine_call_gate_smoke`, and `core_machine_task_switch_smoke` remain
retained regressions, but only the same-CPL return portion is T303 work. The
remaining outer-return, gate, and task behavior stays with its existing owner.
No current disagreement among Intel and the read-only comparison requires a
bridge, new executor, state owner, public interface, or host-side behavior.

## S2 Relative, Jump, And Loop Evidence

`tests/machine/core_machine_control_transfer_smoke.c` is the T303 focused
synthetic probe and is registered in `PROJECT_CURRENT_SMOKE_TARGETS` as
`core-machine-control-transfer-smoke`. It constructs only a private prepared
CPU/descriptor state; it does not use a guest image, long-start fixture, or
host shortcut. Its marker is `M5:T303:CONTROL-TRANSFER:OK`.

The probe proves all sixteen taken short and near `Jcc` predicates without
changing EFLAGS; short `JMP`; near `JMP` and `Jcc` in 16- and 32-bit code
defaults with the alternate operand size; and the real-mode 80386
`66h 0F 84 rel32` form.
It proves that `LOOPNZ`, `LOOPZ`, `LOOP`, and `JCXZ` retain their predicate
rules, and that `67h` selects CX while preserving the upper ECX half in a
32-bit code segment. It also proves `67h` selects ECX for `JECXZ`, that the
80386 `0F 84` form is rejected as `#UD` on the 80286 profile, and that a
not-taken conditional branch does not validate its otherwise out-of-limit
target.

Two deliberately terminal protected `#GP` cases have no installed IDT: a
taken `Jcc` target beyond CS and a taken `LOOP` target beyond CS. The probe
asserts that their first fault is `#GP`, their EIP and EFLAGS return to the
instruction entry state, and the `LOOP` ECX value also returns to its entry
state. The expected terminal diagnostics are therefore evidence of those
negative paths, not renderer or delivery work.

The implementation correction is limited to `_e_loopcc`. It now computes the
selected CX/ECX decrement locally, validates a taken target through the
existing `_kec_jmp_near` route, and publishes the count only after that route
succeeds. A non-taken loop still publishes its decrement. This matches the
Intel-defined restart boundary and the read-only Bochs 2.6 control-transfer
path; PCjs 2.00.0 independently confirms the address-size selection of the
loop/JCXZ count register. No Jcc/JMP helper, decoder, exception-delivery path,
stack primitive, public interface, or artifact target changed.

The S2 sweep revisited `_e_jcc`, `_e_loopcc`, `_kec_jmp_near`, `JMP_REL8`,
`JMP_REL32`, all `70h`--`7Fh` handlers, all `0F 80h`--`8Fh` handlers,
`LOOPNZ_REL8`, `LOOPZ_REL8`, `LOOP_REL8`, `JCXZ_REL8`, the primary/`0F`
profile-form gate, and the current-smoke registration. The remaining
`CALL`/`RET` and far selector/cache hits remain S3/S4 exactly as frozen in the
matrix. A temporary malformed 16-bit-CS `66h 0F 84` probe encoding was fixed
before acceptance; it was fixture data, not a production decoder defect.

Focused commands passed: `core-machine-control-transfer-smoke` emitted its
marker; retained `core-machine-operand-address-smoke` emitted
`M5:T302:OPERAND-ADDRESS-STACK:OK`; and retained
`core-machine-real-mode-386-address-smoke` emitted
`M5:T287:S24:REAL-MODE-386-ADDR32:OK`. S2 has no artifact, full-gate run, or
Setup observation; those remain family-close work.

## S3 Near Call, Jump, And Return Evidence

S3 extended the same focused synthetic probe without creating a second CPU
path. It covers relative `CALL` with each 16/32 code-default and operand-size
cross, matched near `RET` width, and a 32-bit SS stack-address-size return
with both 16-bit and 32-bit return offsets plus `RET imm16` adjustment. It
also covers `FF /2` and `FF /4` register and memory targets in both operand
widths, retaining T302's effective-address coverage rather than reopening its
helpers.

The admitted failure boundary is now explicit: direct `CALL`, indirect `JMP`,
and near `RET` to a target outside CS each produce the focused terminal
`#GP` path with entry EIP, flags, and applicable SP/ESP preserved. The RET
probe confirmed the S1 candidate without changing production code: although
`_kec_ret_near` uses the shared pop helper before target validation, the single
core instruction transaction restores the architecturally visible entry state
on the first target-validation fault. This agrees with the Intel `CALL`,
`JMP`, and `RET` entries and the read-only Bochs 2.6
`cpu/ctrl_xfer16.cc`/`cpu/ctrl_xfer32.cc` speculative return path. PCjs 2.00.0
was retained as the independent read-only reference identity from S1; no source
was copied or translated.

The S3 similar-issue sweep revisited `_kec_call_near`, `_kec_jmp_near`,
`_kec_ret_near`, `_e_call_near`, `_e_jmp_near`, `_e_ret_near`, `CALL_REL32`,
`JMP_REL32`, `RET`, `RET_I16`, and `INS_FF`. Far selector/cache forms remain
exclusively S4; no far, privilege, exception-delivery, stack-helper, public
interface, CMake artifact, or product-path code changed in S3. The focused
probe and retained T302 operand/address and real-mode address probes pass;
full gates, artifact construction, and Setup observation remain family-close
work.

## S4 Far Control-Transfer Evidence

S4 extends the focused synthetic probe with real-mode far immediate `JMP` and
`CALL`/`RETF`, plus protected same-CPL far immediate `JMP` and `CALL`/`RETF`
in both 16- and 32-bit pointer widths. Protected `RETF` frames now prove the
nonconforming DPL/RPL rejection as `#GP(selector)` and the non-present case as
`#NP(selector)`.

Two local production corrections were required. `CALL ptr16:32` now decodes
its architectural six-byte immediate rather than consuming eight bytes, so the
saved return EIP is the first byte after the far pointer. Protected same-CPL
`RETF` now rejects a nonconforming return code descriptor whose DPL differs
from the selector RPL before the generic code-cache loader, which intentionally
does not perform that transfer-specific privilege check. No outer return,
gate, task, delivery, public interface, or artifact path changed.

## S5 Corrective Evidence

The far-indirect `FF /3` and `FF /5` forms now decode the effective address at
the offset width before their memory-only check. Legal 16:16 and 16:32 memory
pointers reach the same protected far `CALL`/`JMP` routes as immediate forms;
ModRM register encodings now terminate as architectural `#UD` rather than the
decoder's former internal `#CE(6)`.

The same-CPL `RETF` negative probes retain first-fault `#GP(20)` for the DPL
mismatch and `#NP(28)` for the non-present descriptor. They compare entry and
post-fault EIP, ESP, EFLAGS, CS selector/base/limit and executable/conforming/
default-size cache fields. A private fixture-memory observation is necessary
after a terminal fault because the public memory API correctly rejects that
lifecycle state; it verifies the candidate descriptor access byte remains
`0xBA` and `0x1A`, respectively. No production interface or second memory path
was added.

## S6 Far-Indirect Closure Evidence

The focused probe now directly covers real-mode memory-indirect `FF /3` far
`CALL` with its `RETF` continuation and `FF /5` far `JMP`, both through a
ptr16:16 stored at the real-mode default data address. The protected register
encodings for the same groups retain first-fault `#UD` and preserve entry EIP,
ESP, EFLAGS, and CS selector/base/limit. Operand-size-crossing breadth outside
the admitted ptr16:16 real-mode path remains deferred with the task's broader
mode matrix; no outer return, gate, task, delivery, or production change was
needed for S6.

## S7 Closure-Preparation Evidence

The current artifact target is `vm-0-5-0303`; the retained executable is
`nxvm_0_5_0303.exe` with SHA-256
`099CFB4DB65D791FD32703ACBDF737655258AB4A00836C4CA99764553FC01F24`.

The focused control-transfer probe emitted `M5:T303:CONTROL-TRANSFER:OK`.
Retained checks emitted `M5:T302:OPERAND-ADDRESS-STACK:OK`,
`M5:T287:S24:REAL-MODE-386-ADDR32:OK`,
`M5:T293:S2:PROTECTED-RETURN-ATOMICITY:OK`,
`M5:T288:S2:CALL-GATE-16:OK`, and the three retained task-switch markers
`M5:T261:S2:TASK-SWITCH:OK`, `M5:T261:S3:TASK-SWITCH:CORPUS:OK`, and
`M5:T261:S5:SS-CACHE:OK`. Expected negative fault diagnostics in those
focused probes are asserted paths, not product failures.

`current-gates-gcc` completed its 51 static/governance targets and all
132/132 CTests. Documentation governance and `git diff --check` also passed.

The one bounded owner-supplied product observation did not reach a guest
command or Setup checkpoint: after 45 seconds the NXVM process was alive, but
host automation found no main-window handle. The process was then stopped; no
guest input, guest stdout/stderr observation, or process residue was used as
evidence. This is a host-automation limitation retained for owner-controlled
manual observation, not a CPU or product regression and not a reason for a
retry or repair in T303.

T303 remains active pending coordinator acceptance. This record is closure
preparation only; it neither closes T303 nor advances the Queue.
