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
| `LTR` (`0F 00 /3`); `_s_load_tr`, task descriptor helpers | Protected-mode CPL0 `r/m16` selector load. It requires an available GDT TSS of the admitted width, loads TR's selector/cache, and marks the descriptor busy without a task switch. Null/TI/type/busy/present failures use the Intel selector fault rules and do not alter TR or descriptor access/busy state. | In-scope correction, bounded to the existing admitted 16-bit TSS path. Do not add 32-bit TSS switching or task-gate behavior; prove selector/cache/busy commit ordering and retain task-switch regressions. |
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
| S3 | Selector-table and task-register forms: `SLDT`, `LLDT`, `STR`, and bounded 16-bit-TSS `LTR`. | Extend the same probe with fixed `r/m16` semantics, null/TI/type/present/busy selector cases, cache/busy-byte commit ordering, and retained task-switch checks. Stop before 32-bit TSS switching, gates, or delivery. |
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
