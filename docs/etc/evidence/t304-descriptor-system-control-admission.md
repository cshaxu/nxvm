# M5 T304 S1: Descriptor-Table And System-Control Admission Audit

## Scope And Authority

This record freezes the 80386 descriptor-table and system-control instruction
family before implementation: `SGDT`, `SIDT`, `LGDT`, `LIDT`, `SLDT`, `LLDT`,
`STR`, `LTR`, `SMSW`, `LMSW`, `CLTS`, and the 80386 `MOV` control-register
forms. It deliberately excludes debug/test registers, task switching, task
gates, virtual-8086, interrupt delivery, and any expansion of paging policy.

The authority is the *Intel 80386 Programmer's Reference Manual* (1986):
Chapters 4--7 and 10, the individual Chapter 17 entries `CLTS`,
`LGDT/LIDT`, `LLDT`, `LMSW`, `LTR`, `MOV -- Move to/from Special Registers`,
`SGDT/SIDT`, `SLDT`, `SMSW`, and `STR`, plus the Appendix A opcode map. The
matrix below distinguishes the 16-bit compatibility forms from 80386-only
control-register forms; an operand-size prefix never silently turns an
`r/m16` selector or MSW operand into an `r/m32` operand.

Read-only comparison used the owner-local Bochs 2.6 source set
`cpu/protect_ctrl.cc`, `cpu/crregs.cc`, and `cpu/fetchdecode.cc`, and the
owner-local PCjs 2.00.0 source set
`machines/pcx86/modules/v2/x86op0f.js`, `x86ops.js`, `segx86.js`, and
`cpux86.js`. No source was copied or translated. Both references agree with
Intel on memory-only table operands, fixed-width selector/MSW operands,
CPL-zero loads/control writes, and the separation of visible selector state
from descriptor caches. PCjs retains a historical non-register `MOV CR`
compatibility note; it is not an Intel requirement and is not admitted here.
No paired-step bridge is required.

## Form, Profile, Mode, And Commit Matrix

| Family / encoding / current path | Frozen 80386 contract | Current classification and focused proof |
| --- | --- | --- |
| `SGDT` / `SIDT` (`0F 01 /0,/1`); `INS_0F_01` | Both are unprivileged, memory-only six-byte stores in real and protected mode. Operand size selects `m16:24` (upper base byte zero) or `m16:32`; register ModRM is `#UD`. Destination access must succeed before any six-byte store is visible. | In-scope correction. `SGDT` has a useful base path; `SIDT` is incorrectly rejected whenever PE is set. Prove 16/32 storage layout, real/protected mode, register `#UD`, destination-limit/write fault preservation, and unchanged table state. |
| `LGDT` / `LIDT` (`0F 01 /2,/3`); `_s_load_gdtr`, `_s_load_idtr` | Memory-only six-byte loads. Operand size selects a 24-bit or 32-bit base paired with the 16-bit limit. Legal in real mode; in protected mode only CPL0 may commit the table register. Register ModRM is `#UD`; source access/CPL failure leaves the old table register intact. | In-scope correction. The retained 16-bit `LIDT` boot path and shared helpers are implementation bases only. Prove both widths, mode/CPL, source failure, and no partial limit/base commit. |
| `SLDT` / `STR` (`0F 00 /0,/1`); `INS_0F_00` | Protected-mode, unprivileged `r/m16` stores of the visible selector. Real mode is `#UD`; operand size does not widen a register destination. Destination access must preflight before the selector is stored. | In-scope correction. The current protected gate is correct, but register destinations follow `_GetOperandSize` and can write 32 bits. Prove memory/register 16-bit stores, real-mode `#UD`, CPL independence, and destination-fault nonmutation. |
| `LLDT` (`0F 00 /2`); `_s_load_ldtr` | Protected-mode CPL0 `r/m16` selector load. A null GDT selector invalidates LDTR; TI is `#GP(selector)`; non-LDT/type failure is `#GP(selector)`; non-present LDT is `#NP(selector)`. Candidate selector/cache validation completes before LDTR commits. | In-scope correction. The current helper has CPL/TI routing and generic cache loading, but null, type, present, selector error and cache atomicity are not family evidence. Prove all before considering LDT breadth. |
| `LTR` (`0F 00 /3`); `_s_load_tr`, task descriptor helpers | Protected-mode CPL0 `r/m16` selector load. It requires an available GDT 16-bit or 32-bit TSS descriptor, loads TR's selector/cache, and marks the descriptor busy without a task switch. Null/TI/type/busy/present failures use the Intel selector fault rules and do not alter TR or descriptor access/busy state. | In-scope correction, bounded to descriptor load/cache/busy-bit support for existing 16-bit and 32-bit TSS consumers. Do not add 32-bit TSS switching or task-gate behavior; prove selector/cache/busy commit ordering and retain task-switch regressions. |
| `SMSW` (`0F 01 /4`); `INS_0F_01` | Unprivileged in real and protected mode. It stores the 16-bit MSW image to `r/m16`; operand size does not widen this operand. It must not change CR0 or flags. | In-scope correction. Memory writes are currently word-sized, but a 32-bit register operand follows `_GetOperandSize`. Prove fixed word semantics, real/protected mode, and destination-failure preservation. |
| `LMSW` (`0F 01 /6`); `_s_load_cr0_msw` | `r/m16`, legal in real mode and CPL0-only in protected mode. It updates only the machine-status portion of CR0; once PE is set, LMSW cannot clear it. Source/CPL failure leaves CR0 unchanged. | Retained 286-compatible path, insufficient. Prove 16-bit source width, PE stickiness, retained non-MSW CR0 state, real/protected privilege, and source-fault atomicity without broadening paging policy. |
| `CLTS` (`0F 06`); `CLTS` | Clears CR0.TS, legal in real mode and CPL0-only in protected mode; flags and unrelated CR0 bits are unchanged. The 80386 profile must accept it, while the supported 80286 compatibility profile requires an explicit profile disposition rather than an accidental 80386-only gate. | In-scope correction. Current semantics handle real/CPL0 protected mode, but the metadata begins at 80386. Probe supported-profile admission/rejection and failure nonmutation before deciding whether to admit the 80286 compatibility form. |
| `MOV r32,CRx` / `MOV CRx,r32` (`0F 20 /r`, `0F 22 /r`); `_d_modrm_creg`, `MOV_R32_CR`, `MOV_CR_R32` | 80386-only, register-only, fixed 32-bit forms independent of operand-size prefixes. Real mode is legal; protected CPL other than zero is `#GP(0)`. The admitted CR set is CR0, CR2, and CR3; reserved CR numbers are `#UD`, with no destination/control-state mutation. Every write validates before committing. | In-scope correction, limited to the project's existing T258 paging envelope. Current read path omits CR3; write path omits CR2; CR0 accepts only the separately admitted PE/PG policy and CR3 only its existing base policy. S4 must prove the exact permitted forms and rejection/commit boundary, not expand paging, debug registers, or raw debugger mutation. |
| `0F 00 /4,/5` (`VERR`/`VERW`) and `0F 02/03` (`LAR`/`LSL`) | Already closed by T301 selector-family evidence. | Retained regression only; T304 must not reopen query semantics. |
| `0F 01 /5,/7`, debug/test-register MOV, 32-bit TSS switching, task gates, V86, delivery, paging-policy additions, later-CPU control registers | Not admitted by this family. | `#UD`, retained path, or explicit later-family deferral as applicable. A discovery requiring any of these ends the active batch and is reported rather than folded into T304. |

## Current Path Classification And Commit Boundaries

- `INS_0F` reads the extension opcode and ModRM before applying
  `core_machine_cpu_profile_allows_form`; S2 must preserve a profile rejection
  as a native `#UD` without table, descriptor, control, or destination change.
- `INS_0F_00` currently gates its entire group on protected mode. That is
  correct for `SLDT`/`STR`/`LLDT`/`LTR`, but its register-width behavior must
  be separated from the selector source width before a store commits.
- `INS_0F_01` shares table and MSW decoding. `SIDT` currently has a legacy
  protected-mode `#UD` branch, while the source/load paths can read or write
  limit and base as separate operations. T302 memory rules may be consumed,
  but the family probe must establish that table registers and six-byte stores
  have the required visible commit boundary.
- `_s_load_ldtr` and `_s_load_tr` delegate cache construction to the existing
  selector route. S3 must use candidate state and descriptor-byte observations
  to distinguish selector/type/present/busy faults from an accidental partial
  cache or busy-bit mutation.
- `_s_load_cr0_msw`, `_s_write_cr0_80386`, and `_s_write_cr3_80386` are the
  only admitted control-state writers. T304 may add exact instruction-form
  checks around them, but cannot turn this family into a new paging-policy or
  debugger mutation API.

## S2+ Batches And Focused Synthetic Probes

| Subtask | Bounded implementation | Focused synthetic probe and stop boundary |
| --- | --- | --- |
| S2 | Descriptor-table store/load forms: `SGDT`, `SIDT`, `LGDT`, and `LIDT`. | Add one prepared CPU/table-state probe for both operand sizes, memory-only rejection, real/protected CPL, base/limit layout, source/destination failure, and table-state atomicity. Stop before LDTR/TR or CR state. |
| S3 | Selector-table and task-register forms: `SLDT`, `LLDT`, `STR`, and bounded `LTR` descriptor load. | Extend the same probe with fixed `r/m16` semantics, null/TI/type/present/busy selector cases, 16-bit/32-bit TSS cache/busy-byte commit ordering, and retained task-switch checks. Stop before 32-bit TSS switching, gates, or delivery. |
| S4 | MSW/control subset: `SMSW`, `LMSW`, `CLTS`, and the already-admitted `MOV CR0/CR2/CR3` encodings. | Prove fixed-width operands, profile/mode/CPL gates, reserved-register and memory-form `#UD`, exact T258 CR0/CR3 policy, CR2 symmetry, and pre-fault destination/control-state preservation. Stop before paging-policy additions or debug/test registers. |
| S5 | Family sweep and closure evidence. | Re-run the T304 probe plus retained paging, task-switch, protected-return, control-transfer, and selector probes, then the normal gates/artifact and one bounded product observation. A product observation is record-only and cannot expand T304. |

The focused probe will use only prepared CPU, descriptor-table, and ordinary
memory state. It will contain no system image, long-start fixture, host
shortcut, second executor, or public raw-layout path. The eventual marker is
`M5:T304:DESCRIPTOR-SYSTEM:OK`.

## Similar-Issue Sweep And Deferrals

The audit used:

```powershell
rg -n -i "\\b(sgdt|sidt|lgdt|lidt|sldt|lldt|str|ltr|smsw|lmsw|clts|mov[_ ]?cr|gdtr|idtr|ldtr|tr)\\b" src/core/machine tests CMakeLists.txt docs/etc/evidence
rg -n "core_machine_cpu_profile_allows_form|CPU_INSTRUCTION_0F|INS_0F_00|INS_0F_01|_s_load_(gdtr|idtr|ldtr|tr)|_s_load_cr0_msw|_s_write_cr[03]_80386|_d_modrm_creg" src/core/machine
```

All production hits in the named family are classified above. Existing T257
80286 `LIDT` coverage, T258 paging coverage, T261 task-switch coverage, and
T301 selector-query coverage are retained regressions, not evidence that the
T304 matrix is complete. The remaining hits in debug/test register handlers,
task-switch and gate routes, delivery, V86, and broad paging behavior belong
to their explicitly deferred families. No current S1 finding requires an
architecture change, a second state owner, a public interface, or a bridge.

## S2 Descriptor-Table Evidence

`tests/machine/core_machine_descriptor_system_smoke.c` is the prepared-state
focused probe for `SGDT`, `SIDT`, `LGDT`, and `LIDT`. It uses the retained
single core CPU executor and ordinary memory only. It proves `m16:24` versus
`m16:32` six-byte layouts, real-mode table stores and loads, protected-mode
`SIDT`, protected CPL-zero loads, register-ModRM `#UD`, CPL-three load
`#GP(0)`, and source/destination limit failures that preserve the old GDTR and
IDTR values. The deliberate terminal fault diagnostics are asserted negative
paths.

Two narrow production corrections were required in `INS_0F_01`. `SIDT` no
longer has its non-architectural protected-mode `#UD` branch. The four table
forms now share `_d_modrm_table_memory`: it rejects a register ModRM before
the six-byte memory operand decoder can produce the prior internal `#CE(6)`.
Memory forms continue through the existing decoder and memory route; no new
executor, table-state owner, memory path, delivery behavior, or public
interface was introduced.

The S2 sweep revisited `INS_0F`, `core_machine_cpu_profile_allows_form`,
`INS_0F_01`, `_d_modrm_ea`, `_d_modrm_table_memory`, `_s_load_gdtr`,
`_s_load_idtr`, and the retained 80286 protected-mode test. `0F 00` selector
forms, MSW/control registers, debug/test registers, task paths, delivery,
paging policy, and V86 remain outside S2 exactly as frozen. The focused probe
emits `M5:T304:DESCRIPTOR-SYSTEM:OK`; retained
`M5:T257:S6:80286-PROTECTED-MODE:OK`,
`M5:T301:SEGMENT-SELECTOR:OK`, and
`M5:T303:CONTROL-TRANSFER:OK` also pass.

## S3 Selector-Table And Bounded Task-Register Evidence

The Intel 80386 PRM Chapter 17 entries `SLDT`, `STR`, `LLDT`, and `LTR`
remain authoritative for this batch.  The S1 read-only comparison paths were
rechecked: Bochs 2.6 `cpu/protect_ctrl.cc`, `cpu/segment_ctrl.cc`, and
`cpu/fetchdecode.cc`; PCjs 2.00.0 `machines/pcx86/modules/v2/x86op0f.js`,
`x86ops.js`, and `segx86.js`.  They agree with Intel that selector operands
remain `r/m16`, that `SLDT`/`STR` are protected-mode stores rather than
operand-size-dependent general-register writes, and that `LLDT`/`LTR` validate
their candidate before publishing their invisible cache.  No reference source
was copied or translated.

`tests/machine/core_machine_descriptor_system_smoke.c` now extends the
prepared-state focused synthetic probe with this frozen matrix:

- Real-mode `SLDT`, `STR`, `LLDT`, and `LTR` each reach native `#UD(0)` with
  LDTR/TR unchanged.  Protected user-CPL `SLDT` and `STR` store only their
  visible selector into a register destination, including under `66h`; the
  upper half of EAX is preserved.  Their memory forms store a word.
- `LLDT` accepts both register and `66h`-prefixed memory `r/m16` sources;
  a null GDT selector invalidates LDTR.  TI, non-LDT, and non-present sources
  produce respectively `#GP(selector)`, `#GP(selector)`, and `#NP(selector)`
  without changing the prior LDTR cache.
- `LTR` accepts an available, present GDT 16-bit or 32-bit TSS descriptor in
  this bounded batch. It commits the TR selector/cache and changes only that
  descriptor's access byte from available `0x81` or `0x89` to busy `0x83` or
  `0x8b`. Its null, TI, LDT-type, already-busy, and non-present cases produce
  the frozen `#GP(0)`, `#GP(selector)`, `#GP(selector)`,
  `#GP(selector)`, and `#NP(selector)` results. Each negative probe asserts
  the first fault, old TR cache, and, for a selected GDT descriptor, unchanged
  access byte. Fault-state descriptor observations use the allowed private
  test fixture physical-memory operation only; production remains behind the
  single core machine interface.

Two narrow production corrections support that evidence. `INS_0F_00` now
decodes and writes `SLDT`/`STR` as fixed two-byte `r/m16` operations for both
register and memory operands. No task switch, gate, delivery, paging, public
interface, or executor path changed.

The S3 similar-issue sweep revisited `INS_0F`, `INS_0F_00`,
`_s_load_ldtr`, `_s_load_tr`, `_ksa_load_sreg`, descriptor type macros, and
the retained T261 task-switch route. The T261 switch remains a separate
bounded task-switch regression; the LTR form evidence does not admit 32-bit
switching, task gates, delivery, LDT breadth, or any future task family. The
focused probe emits `M5:T304:DESCRIPTOR-SYSTEM:OK`; the retained
task-switch smoke emits `M5:T261:S2:TASK-SWITCH:OK`,
`M5:T261:S3:TASK-SWITCH:CORPUS:OK`, and `M5:T261:S5:SS-CACHE:OK`.

## S4 MSW And Admitted Control-Register Evidence

The Intel 80386 PRM Chapter 17 entries `CLTS`, `LMSW`, `SMSW`, and `MOV --
Move to/from Special Registers` are authoritative.  The S1 reference paths
were rechecked: Bochs 2.6 `cpu/protect_ctrl.cc`, `cpu/crregs.cc`, and
`cpu/fetchdecode.cc`; PCjs 2.00.0
`machines/pcx86/modules/v2/x86op0f.js`, `x86ops.js`, and `cpux86.js`.
They confirm fixed `r/m16` MSW operands, 80286-or-later `CLTS`, and 80386
register-only 32-bit control-register moves.  The project continues to admit
only CR0, CR2, and CR3 and retains its pre-existing CR0 PE/PG and aligned CR3
policy; no reference implementation was copied or translated.

The prepared-state focused probe now proves:

- `SMSW` is a two-byte store in real mode and at protected user CPL, including
  a `66h` register form that preserves EAX's upper half.  It does not alter
  CR0.  `LMSW` consumes fixed `r/m16`, accepts a protected CPL-zero prepared
  state, preserves PE once set, and changes the MSW TS bit without widening
  its source.
- `CLTS` clears only CR0.TS and leaves EFLAGS intact in real mode for both
  80286 and 80386 profiles and in protected CPL0.  It is native `#UD(0)` on
  80186 and `#GP(0)` at protected CPL3, with CR0/EFLAGS preserved.
- `MOV r32,CR0/CR2/CR3` and `MOV CR0/CR2/CR3,r32` are fixed-width,
  register-only 80386 forms.  The probe covers all three successful reads and
  writes, including newly symmetric CR2 write and CR3 read.  Reserved CR,
  memory ModRM, invalid CR0 policy input, and protected CPL3 paths reach the
  required `#UD` or `#GP(0)` before changing the general destination or any
  admitted control register.

Three narrow production corrections were required.  The `0F` metadata now
admits `CLTS` from the 80286 profile rather than treating it as 80386-only.
`SMSW` now always decodes and writes `r/m16`.  The existing control-register
decoder now exposes CR3 to the read form and commits CR2 on the write form.
The retained T258 paging smoke was updated to keep testing a reserved CR1
write instead of treating the newly admitted CR2 write as a paging-policy
failure.  No debug/test-register path, INVD/INVLPG behavior, paging policy,
delivery path, public interface, or executor changed.

The S4 sweep revisited `core_machine_cpu_instruction_metadata_get`, `INS_0F`,
`INS_0F_01`, `CLTS`, `_d_modrm_creg`, `MOV_R32_CR`, `MOV_CR_R32`,
`_s_load_cr0_msw`, `_s_write_cr0_80386`, and `_s_write_cr3_80386`, together
with the T258 focused paging probe.  Debug/test-register handlers, `0F 01
/5,/7`, 32-bit TSS work, task/gate paths, paging-policy changes, V86, and
delivery remain deferred exactly as frozen.  The T304 marker remains
`M5:T304:DESCRIPTOR-SYSTEM:OK`; T258 retains
`M5:T258:S2:I386-PAGING:OK` and `M5:T258:S3:I386-PAGING:CORPUS:OK`.

## S5 Retained-Path Correction

The S5 retained `core-machine-tss-iomap-port-smoke` first failed before its
I/O-map program could issue a port operation: `LTR` on the existing available
32-bit TSS selected at `0x28` produced `#GP(0x28)`. The direct cause was the
S3 restriction of the `SREG_TR` loader from the pre-existing generic
available-TSS predicate to the 16-bit-only predicate. S5 restores the generic
available-TSS check for `LTR` only. It does not modify task-switch, gate,
nested-task, or delivery code.

The focused descriptor-system probe now proves both legal `LTR` descriptor
loads: the existing 16-bit descriptor yields selector `0x20`, the expected TR
cache fields, and access byte `0x83`; the existing 32-bit descriptor yields
selector `0x40`, base `0x00000a00`, limit `0x67`, cache type
`TSS_32_BUSY`, and access byte `0x8b`. The retained type, busy, non-present,
and failure-atomicity cases remain. Re-running the focused and retained paths
produced `M5:T304:DESCRIPTOR-SYSTEM:OK`, `M5:T260:S3:TSS-IOMAP:CORPUS:OK`,
and all T261 task-switch markers. This restores existing 32-bit TSS consumers
without admitting 32-bit task switching.

## S5 Closure-Preparation Evidence

The one current artifact target is `vm-0-5-0304`; the generated
`build/output/nxvm_0_5_0304.exe` has SHA-256
`64516A1936456C77635C692D9120198D54A135F37EB931B529CC77921220F0BD`.

The final managed `cmake --build --preset current-gates-gcc` run passed all 51
static/governance targets and 133/133 current-gate CTests. The focused T304
probe emitted `M5:T304:DESCRIPTOR-SYSTEM:OK`. Retained direct probes emitted
the T258 paging markers, `M5:T260:S3:TSS-IOMAP:CORPUS:OK`, all three T261
task-switch markers, `M5:T293:S2:PROTECTED-RETURN-ATOMICITY:OK`,
`M5:T303:CONTROL-TRANSFER:OK`, and `M5:T301:SEGMENT-SELECTOR:OK`.
Documentation governance and `git diff --check` also pass.

One bounded 45-second owner-supplied product observation was attempted. The
NXVM process remained alive but had no discoverable main-window handle, so no
external media mount, guest command, or Setup checkpoint occurred. No guest
input was sent. The process was terminated and cleanup confirmed no residue.
This is a host-automation limitation retained for owner-controlled manual
observation, not a product or CPU conclusion; it is not retried in T304.

At its closure-preparation point, T304 remained pending coordinator acceptance.
The coordinator subsequently accepted and closed T304; this record is
historical closure-preparation evidence only: it did not set the project Idle, close
T304, change Queue state, or authorize a subsequent task.
