# Project Status

## Current Work

**Idle. M5 T263 is closed; the next instruction-family admission requires a
separate failing corpus and packet.**

## T263 Admission Packet

### Original Request

Admit remaining 286/386 instruction families only one family at a time, from
a reproducible failing corpus.  Each family must complete a contract,
core-only implementation, and project-owned corpus before the shared T263
artifact is rebuilt.  This task must not become a claim of complete 286 or
386 support.

### S1 Gap Audit And First Admission

The first admitted family is the **80286 `ARPL r/m16,r16` form** (`63 /r`),
not the broader 0F opcode map.  It is a concrete present defect:

| Evidence | Current result | Required result |
| --- | --- | --- |
| `core_machine_cpu_instruction_metadata_get(PRIMARY, 63h, /r)` | Declares `minimum_cpu = 80286` | Metadata and execution gate agree |
| `ARPL_RM16_R16()` | Enters only for `cpu_profile >= 80386`; 80286 falls through to `#UD` | 80286 protected mode executes the same admitted 16-bit form |
| Existing profile smoke | Proves only 80186 rejection; no 80286 positive case | New corpus proves 80286 positive and lower-profile `#UD` negative cases |

S2 may change only the CPU decoder/executor path and its metadata if needed.
It admits protected-mode 16-bit register and memory `r/m16,r16` forms, with
the architectural RPL comparison/update and `ZF` result.  Real-mode use,
32-bit operands/addressing, VM/firmware handling, debugger bypasses, and any
other descriptor/control-transfer family remain deferred.  The existing
protected-mode selector/memory route remains the sole state owner and
execution route.

S3 must add a project-owned core corpus covering a 80286 positive register
case, a memory destination case, both `ZF` outcomes, 80186 and 8086 `#UD`
gates, an admitted prefix/ModR/M boundary, and no partial destination mutation
on rejected execution.  It must retain T257--T261 protected-mode/paging/TSS
corpora and the existing DOS, FDD/HDD, Console, debugger, and current GCC/
CTest matrix.  A behavior-changing closure rebuilds only
`build/output/nxvm_0_5_0263.exe` and records its source commit and SHA-256.

### Requirements And Owners

| Requirement | Owner | Evidence / completion condition |
| --- | --- | --- |
| One admitted family | `core/machine` CPU | `63 /r` only; no 0F-table expansion |
| Correct profile gate | CPU instruction metadata and executor | 80286 positive; 80186/8086 `#UD` |
| Correct side effects | CPU register/memory/FLAGS route | RPL replacement only when source RPL is higher; `ZF` exact |
| No second path | `core_machine_run()` | No VM, firmware, platform, host, or debugger involvement |
| Retained behavior | Existing products and devices | Focused corpus plus current-gates regression |

### Similar-Issue Sweep

Defect class: metadata says an instruction is admitted for a CPU profile while
the executor imposes a stricter, undocumented profile threshold.  S1 searched
the tracked primary/0F metadata table, corresponding opcode handlers, and
the profile/protected-mode corpora using `rg` for `minimum_cpu`, profile gates,
and `UndefinedOpcode`.  The confirmed production hit is `ARPL`; the many
other unimplemented 0F, gate, task-return, and 32-bit forms are already
explicitly deferred by T257--T262 or require a distinct failing corpus.
S2/S3 must repeat the query after the fix and record every new production hit
as fixed, not applicable, or deferred in `TODO.md`.

### Rules, Commands, And Stop Conditions

Applicable: local module boundaries; core owns CPU state and the sole guest
executor; CPU-profile metadata must match decoder/executor behavior; no global
or TLS current object; task artifact revision equals task number; one active
subtask; and the execution-policy similar-issue sweep.

Planned focused commands are `cmake --preset mingw-gcc-x64`, the focused
ARPL smoke target, and `cmake --build --preset current-gates-gcc -j 4`.
Expected closure markers are `M5:T263:S2:ARPL:OK` and
`M5:T263:S3:ARPL:CORPUS:OK`.  Stop and split if correct ARPL behavior requires
CPL transition, IDT delivery, LDT, task switching, a second executor, or a
VM/firmware/host shortcut.

### S1 Evidence

The evidence above is source-reproducible: the instruction metadata at
`src/core/machine/cpu_instructions.c:5959-5962` assigns 80286, while
`ARPL_RM16_R16()` at `src/core/machine/cpu_instructions.c:8644-8670` gates
execution at 80386.  `tests/machine/cpu_profile_gate_smoke.c:145-157` covers
only the 80186 negative case.  Existing T257 protected-mode code already
provides the GDT/selector route needed to make an owned 80286 ARPL corpus;
it does not need new VM integration.

### S1/S2 Closure

S1 selected one reproducible mismatch, recorded the owner, scope, corpus
requirements, similar-issue sweep, and stop conditions.  S2 changed only
`ARPL_RM16_R16()` so its minimum CPU gate matches the existing 80286 metadata.
It did not change ARPL's RPL/ZF algorithm, selector access, the metadata table,
or another instruction handler.

### S3 Closure

`core-machine-arpl-smoke` now proves the admitted 80286 protected-mode
register and ES-prefixed memory forms, both `ZF` outcomes, 8086/80186 profile
rejection, real-mode rejection without an AX mutation, and the frozen metadata
minimum.  The historical CPU-profile gate also had a stale expected-fault
assertion: it required `TYPE_STATUS_OK` for intentional `#UD`.  It now checks
the established `TYPE_STATUS_FAULT`/`CORE_MACHINE_STOP_FAULT` outcome and is
registered in the current matrix, preserving its existing 8086/80186/80286
negative gates without adding instruction behavior.

The similar-issue sweep repeated the exact `63h` metadata/handler query across
tracked production CPU source and profile corpus: the only `ARPL` execution
gate is now `>= 80286`, matching metadata.  Other 286/386 candidates remain
separate, explicitly unadmitted T263 families and require their own failing
corpus; they were not altered or implicitly enabled.

`current-gates-gcc` passed **99/99** CTest cases, all static/ownership gates,
FDD/HDD boot, DOS prompt/`MEM`, Console, and debugger regression.  The current
artifact is `build/output/nxvm_0_5_0263.exe`, SHA-256
`CDECA028180652317C2EDC9C872B8BDB0F49A5465C506A6921BC2A00DB0CAF02`.

## T262 Admission Packet

### Original Request

Establish a real but deliberately bounded present-FPU path in `core/machine`.
The FPU is per-machine state; the existing CPU ESC decoder is its only entry.
The default remains CPU `80386`, FPU `none`.  `none` continues to consume legal
`D8h`--`DFh` encodings, while `CR0.EM`/`CR0.TS` report `#NM` and `FWAIT`
reports `#NM` only for `TS && MP`.  A present but unadmitted form stops with
the explicit internal FPU-unsupported diagnostic, never `#UD`.

### S1 Contract

T262 admits **only `CORE_MACHINE_FPU_PROFILE_8087`** as the first present
profile.  The existing `80287` and `80387` enum values remain valid frozen
configuration values, but they gain no execution claim in this task: they
continue to return the explicit FPU-unsupported outcome until a later
profile-specific admission.  This avoids falsely calling an 8087 subset a
387 implementation merely because the default CPU profile is 80386.

`core_machine_fpu` is the sole state owner.  S2 adds a resettable control
word, status word, eight tagged stack registers, TOP, and pending-unmasked-
exception state.  The value representation and every arithmetic operation
remain owned C data/algorithms: host floating-point state, a VM/firmware
helper, TLS/current-object state, or a second executor are forbidden.

| Concern | T262 admission | Deferred / deterministic result |
| --- | --- | --- |
| CPU entry | Existing `D8h`--`DFh` ESC dispatch plus `FWAIT` | No alternate decoder or VM path |
| Present profile | Exact 8087 selection, CPU minimum 8086 | 80287/80387 remain explicit FPU-unsupported; FPU=none consumes legal ESC |
| Initial state | `FNINIT`/`FINIT` reset control/status/tag/TOP/pending state | Environment save/restore, 287/387 environment differences |
| Data forms | Finite normalized `m32real` load/store through the core memory route | 64/80-bit, integer, BCD, denormal, NaN, infinity, and memory-format breadth |
| Arithmetic | Register `FADD`, `FSUB`, `FMUL`, `FDIV` on the admitted stack forms | Transcendentals, remainder, compare, and unadmitted ModR/M forms |
| Exceptions | Stack underflow/overflow and divide-by-zero set owned FPU status; a deliberately unmasked pending exception is observed by `FWAIT` as retained `#MF` | Full x87 exception delivery, precision/rounding breadth, and host FP exception use |
| Control | `FLDCW m16` is admitted only to make the unmasked-exception/FWAIT corpus guest-visible | Arbitrary control/environment manipulation |

Every admitted form has metadata declaring minimum CPU (`8086`) and minimum
FPU (`8087`), plus an exact-8087 dispatch gate.  S2 must reject an unsupported
present form without consuming a second path, and must preserve the existing
`EM`/`TS` precedence before FPU dispatch.  S3's core-only corpus covers
FPU=none ESC consumption, `EM`/`TS`/`MP` `#NM`, `FNINIT`, stack
load/arithmetic/store, `FWAIT`, a stack fault, an unmasked divide-by-zero
outcome, and CPU/FPU profile gates.  The retained `DOS MEM` regression proves
that `DB E3` no longer becomes `#UD`.

### Constraints And Stop Conditions

T262 changes only `core/machine` and its project-owned probes.  It may not
modify VM firmware, platform, Console, debugger, boot flow, or the default
FPU=none profile.  Stop and split the work if the corpus requires host
floating-point state, 287/387-specific behavior, complete IEEE handling,
environment save/restore, protected-mode FPU delivery, or any second CPU/FPU
execution path.  T262 produces `build/output/nxvm_0_5_0262.exe` only after
the implementation and closure subtasks complete.

### S1 Evidence

The pre-S2 audited implementation had one per-machine `core_machine_fpu` skeleton
bound into the CPU execution context, one `FPU_ESCAPE` decoder for `D8h`--
`DFh`, existing `FPU=none` consumption, and existing `EM`/`TS`/`MP` smoke
coverage.  Present profiles then all returned FPU-unsupported; no old FPU
operation, host shortcut, or duplicate execution path existed.

### S2/S3 Closure

S2 replaced that skeleton with the exact-8087 state owner: control/status
words, tagged eight-entry stack, TOP, pending unmasked exception, and finite
`m32real` conversion/arithmetic all live in `core/machine/fpu.*`.  CPU retains
ESC decoding and guest-memory access; it invokes the FPU only after the
existing `EM`/`TS` gate.  `FWAIT` now reports retained `#MF` for a pending
unmasked exception.  No `float`, `double`, TLS/current-object state, VM
provider, firmware shortcut, or second execution path was introduced.

S3 adds `core-machine-fpu-8087-smoke`, proving load/add/store, stack
overflow plus `FNINIT` reset, unmasked divide-by-zero plus `FWAIT`, exact
profile gates, and deterministic unsupported outcomes.  The retained ESC
smoke proves FPU=none consumption and `EM`/`TS`/`MP` behavior.  `current-gates-gcc`
passed **97/97** current CTest cases, all static/ownership gates, FDD/HDD
boot, DOS prompt/`MEM`, Console, and debugger regression.  The artifact is
`build/output/nxvm_0_5_0262.exe`, SHA-256
`95AD517F6E045E9E936595FE5F7FBADBEC0CD1AED8531233D755F01F32331899`.

## Historical T261 Admission Packet

This retained record describes the T261 closure point. References below to a
then-current artifact or to T262 being unstarted are historical evidence, not
the current project state.

### Original Request

Implement the smallest real hardware task-switch closure only in
`core/machine`: save task A, load task B, transition busy descriptors, and
preserve observable faults. The one existing `core_machine_run()` executor
remains the only guest path. VM, firmware, platform, and product code must not
participate. The task produces `nxvm_0_5_0261.exe` only after the completed
task closure.

### S1 Contract

T261 admits exactly one entry: a CPL0, 16-bit far `JMP` to a GDT-resident,
present, **available 16-bit TSS**. This applies to the 80286 profile and to
the 80386 profile while it executes the same 16-bit TSS layout. A task gate,
far `CALL`, `IRET` task return, nested tasks, backlink changes, and `NT`
changes are not admitted.

| Concern | T261 admission | Deferred / deterministic result |
| --- | --- | --- |
| State owner/path | CPU/TR/descriptors/TSS memory in `core/machine`; far-JMP decoder -> one task-switch helper -> existing executor | VM/firmware/host context, a second executor, or a task-specific loop |
| Source/target | Current TR is a valid busy 16-bit TSS; target is GDT, RPL 0, DPL 0, present, available 16-bit TSS | null/LDT selector, wrong type, privilege mismatch, not-present or busy target faults before task-state mutation |
| Layout | 16-bit TSS offsets through `0x2b`: IP, FLAGS, AX/CX/DX/BX/SP/BP/SI/DI, ES/CS/SS/DS, and LDTR selector | 32-bit TSS/CR3, FS/GS, I/O map, task switch under paging, LDT loading |
| Save/load | Save task A's admitted 16-bit state to its TSS; load task B's state through normal selector validation. The 80386 profile follows its architected 16-bit-TSS register widening rule, explicitly asserted by the corpus. | 32-bit task state or an implicit host snapshot |
| Busy/flags | Far-JMP clears old A busy, sets B busy, leaves B link and `NT` unchanged, and sets CR0.TS | CALL/INT/IRET busy/link/NT semantics |
| Atomicity/fault | Validate both descriptors, both TSS bounds, and target selectors before any save, descriptor busy write, or CPU/TR commit. A pre-commit fault retains its original copied diagnostic and leaves A live. | mid-switch paging/segment-load recovery; these require another admission |
| Faults | invalid selector/type/DPL/RPL/busy -> `#GP(selector)`; not present -> `#NP(selector)`; insufficient old or target TSS -> `#TS(selector)` | protected-IDT delivery beyond the retained T259 route |

The project-owned `core-machine-task-switch-smoke` begins as a pre-S2
baseline: it establishes protected mode, `LTR` task A, then attempts the
admitted far-JMP form under both profiles and proves the current intentional
unsupported disposition while TR remains task A. S2 turns that same fixture
into the task-A -> task-B positive corpus, then adds the admitted fault cases.
Its focused markers are
`M5:T261:S1:TASK-SWITCH:BASELINE:OK`,
`M5:T261:S2:TASK-SWITCH:OK`, and
`M5:T261:S3:TASK-SWITCH:CORPUS:OK`.

### Rules, Evidence, And Stop Conditions

Applicable rules: core has no VM/VDM dependency; state remains single-owner;
all descriptor and TSS access uses the existing checked core memory route; no
global/TLS current object; provider topology is frozen before reset; platform
does not mutate guest state; no third-party source import, runtime dependency,
or protected guest asset. S1 examines Intel's system-programming task model and
the local Bochs 2.6 compatibility checkout only as a behavioral reference; it
does not import or transliterate either implementation.

S2 may change only the core far-JMP/TSS path and existing descriptor/TSS
helpers. S3 runs the focused corpus, T257--T260, paging, retained DOS/FDD/HDD,
Console/debugger, and current gates. Stop and split for 32-bit TSS state,
paging-time atomicity, task gates, far CALL, nested/IRET task return, LDT,
CPL1/2, V86, any VM/firmware/host shortcut, another executor, or a retained
NXVM UX change.

**Similar-issue sweep plan.** The defect class is a system-segment control
transfer reaching a stub or a bypassed TSS state transition. Before closure,
search all `_ser_*_tss`, `_ser_*_task_gate`, `_s_read_tss`, `_s_write_tss`,
`_s_load_tr`, `_IsDescTSS*`, and descriptor write sites in tracked source,
tests, CMake, and task records. Every production hit must be the shared T261
owner path, explicitly deferred, or recorded in `TODO.md` with an admission.

### S1 Result

S1 is complete. `core-machine-task-switch-smoke` establishes protected mode,
loads task A through `LTR`, and executes the selected far-JMP-to-TSS form under
both 80286 and 80386 profiles. Before S2 it deterministically reaches the
existing `#CE` stub while TR remains A; its marker is
`M5:T261:S1:TASK-SWITCH:BASELINE:OK`. The focused target and retained T259/T260
protection smokes pass. The task used the Intel system-programming task model
as primary architectural reference and the local Bochs 2.6 checkout only to
cross-check the 16-bit TSS limit and non-nested far-JMP busy/NT model; neither
source is imported or a runtime dependency.

### S2 Result

S2 is complete. The old `_s_write_tss` stub now uses the one checked core
logical-memory route. `_ser_jmp_far_tss()` is the only admitted task-switch
owner: it prevalidates the current busy 16-bit TSS, target available 16-bit
TSS, GDT write access, TSS read/write ranges, B's null LDTR requirement, and
B's CPL0 code/data selectors before it changes state. It then saves A's
dynamic 16-bit state, clears A busy, sets B busy, installs B's segment/TR
caches and register state, clears FS/GS for the retained 16-bit layout, and
sets CR0.TS. The TSS LDTR field is consumed only as a required null selector;
hardware LDT switching is explicitly deferred rather than being approximated.

The upgraded core-only fixture proves A -> B under both profiles: A writes
back its post-far-JMP IP and AX, B restores AX and writes a guest marker, the
two descriptor busy values exchange, TR becomes B, and the 80386 profile
widens a 16-bit-TSS general register to `FFFF2222h`. No VM, firmware,
platform, product, or host context participates.

### S3 Result

S3 is complete. The core-only corpus now proves that an invalid target
selector retains `#GP(selector)`, a not-present target retains
`#NP(selector)`, a busy target retains `#GP(selector)`, and a target whose
16-bit TSS limit ends before `0x2b` retains `#TS(selector)`. Each case leaves
the live TR on task A. The current retained protected-IDT mechanism may make a
subsequent failed delivery observable in trace output, but the checked
first-fault diagnostic remains the original task-switch fault.

The required similar-issue sweep found no second production task-switch path:
far-JMP-to-TSS is the sole admitted owner; `LTR` and the T260 I/O-map reader
remain their own existing contracts; task-gate and far-CALL-to-TSS helpers are
explicit deferred stubs. The deferred set remains 32-bit TSS switching,
paging-time task-switch atomicity, LDT loading, task gates, far CALL, and
nested/IRET task return.

### S4 Initial Closure

T261 is closed. At that closure point the official artifact target was `vm-0-5-0261`; its
developer artifact is `build/output/nxvm_0_5_0261.exe`, SHA-256
`05DBCEA9B0CF7D025715A334E17D43C8DBE57CFE61C27230B2A50D0A8E531132`.
`current-gcc` built that target with GCC, and `current-gates-gcc` passed all
static gates plus 95/95 CTest cases. The retained DOS/FDD/HDD, Console,
debugger, CGA/EGA, ATA, RTC, T257--T260, and focused task-switch paths are
therefore verified against the same source revision.

### S5 Corrective Scope

Post-closure review found that the admitted task-B SS selector was validated
as writable data but materialized with `SREG_DATA`. That is a real core CPU
state-classification defect: stack operations can lose their existing distinct
`#SS` limit path. S5 changes only the task-switch cache classification and its
core-only corpus. It must retain the same descriptor validation, TSS memory
route, busy-state transition, one executor, and all product behavior.

The focused corpus must assert task B's live `SS.sregtype == SREG_STACK` and
execute a bounded task-B `POP` across an SS limit to retain `#SS(0)`. The
similar-issue sweep covers all task-switch segment-cache assignments and
existing stack cache producers; direct segment-load paths are not rewritten
unless they exhibit the same incorrect classification. Rebuild the same T261
artifact identity, run retained protected-mode and current GCC gates, record a
replacement SHA-256, then return status to Idle. T262 remains unstarted.

### S5 Corrective Closure

S5 is complete. The shared task selector validator still applies the one
writable-data selector contract, but now receives the target segment class:
task B's SS cache is materialized as `SREG_STACK`; DS and ES remain
`SREG_DATA`. The 80286 and 80386 cases each switch successfully to B, then B's
`POP` crosses the isolated zero-limit SS descriptor and records `#SS(0)` while
TR remains B. The focused marker is `M5:T261:S5:SS-CACHE:OK`.

The similar-issue query was
`rg -n "_s_task_cache_descriptor\\(|_s_task_validate_data_selector\\(|data\\.ss\\s*=|&cpu_state\\.data\\.ss|\\.ss\\.sregtype|sregtype\\s*=\\s*SREG_DATA" src tests CMakeLists.txt docs TODO.md`.
Its production hits are the corrected task-switch assignments, the existing
`SREG_STACK` CPU initialization and direct SS loaders, normal stack access
helpers, and intentional FS/GS data classifications. No second misclassified
SS cache producer remains.

The retained protection CTests passed 5/5. `current-gcc` rebuilt the official
artifact and `current-gates-gcc` passed all static gates plus 95/95 CTest
cases. The replacement T261 artifact remains
`build/output/nxvm_0_5_0261.exe`, SHA-256
`5EBC07BF8F5FABB21F7DB1901D05778F2EC27CE9FF9AC1F3222EE583E440CE58`.
T262 remains unstarted.

### S5 P2 Evidence Correction

The prior S5 closure recorded a stale build: the source and artifact timestamps
prove that neither the focused smoke nor `nxvm_0_5_0261.exe` incorporated the
S5 marker and SS-cache correction. P2 changes no runtime source. It must
force a fresh build of `core-machine-task-switch-smoke` and `vm-0-5-0261`, run
the focused smoke with S2/S3/S5 markers visible, run `current-gates-gcc`,
replace the same task artifact, and record its new SHA-256 before T261 may
return to Idle.

The clean rebuild also exposed a gate-only omission: eight media-backed CTest
entries were registered but absent from `run-current-smokes` dependencies.
P2 corrects that one CMake list relationship so every registered current smoke
is built before CTest executes it; it does not change guest runtime behavior.

### S5 P2 Closure

P2 is complete. A clean-first build rebuilt both
`core-machine-task-switch-smoke` and `vm-0-5-0261` from current S5 source; the
focused executable emitted all three required markers: S2, S3, and
`M5:T261:S5:SS-CACHE:OK`. The rebuilt executable and developer artifact have
later timestamps than the S5 source and the artifact SHA-256 is now
`5EBC07BF8F5FABB21F7DB1901D05778F2EC27CE9FF9AC1F3222EE583E440CE58`.

`PROJECT_CURRENT_ALL_SMOKE_TARGETS` now derives one de-duplicated current
build dependency list from base and media smoke lists, so registered media
CTest entries cannot be Not Run after a clean build. The clean gate exposed
the DOS video fixture's inadequate 500,000-instruction bound; its deterministic
probe budget is now 1,500,000 and it reaches `INT10=200`, `F2=0`, and the DOS
prompt without any runtime change. The corrected `current-gates-gcc` passed
all static gates and 95/95 CTest cases. T262 remains unstarted.

## T260 Admission Packet

### Original Request

Implement TSS I/O permission checking only in `core/machine`, on the T259
16-bit CPL3-to-CPL0 gate/outer-`IRET` path. When `CPL > IOPL`, guest IN/OUT
access must use the loaded TSS map, apply the result to every byte of the port
span, and turn denial into observable `#GP(0)` through T259's protected-IDT
delivery or original-fault retention. CPL0 and `CPL <= IOPL` retain their
current direct port behavior. The task excludes task switching, LDT, all
32-bit frames/gates, CPL1/2, V86, host/firmware port shortcuts, and another
executor. Its artifact is fixed as `nxvm_0_5_0260.exe`.

### S1 Contract

The shared 16-bit protected execution path remains common to 80286 and 80386,
but the I/O bitmap itself is an **80386 32-bit TSS** facility. This is a
hardware distinction, not a scope expansion:

| Concern | T260 admission | Deferred / deterministic result |
| --- | --- | --- |
| Owner/path | `_kpa_test_mode` -> `_kpa_test_iomap` before the one existing port executor; `core_machine_run()` only | VM/profile/firmware/host port bypass or second executor |
| 80286 / 16-bit TSS | `CPL <= IOPL` retains existing access; `CPL > IOPL` receives `#GP(0)` because no I/O-map exists | no invented 80286 bitmap |
| 80386 TSS | A loaded busy 32-bit TSS is a read-only `ESP0`/`SS0` source for the already-admitted 16-bit gate and an I/O-map source | no task switch, context save, I/O bitmap mutation, or 32-bit gate/frame |
| Stack bridge | For the retained 16-bit gate, `ESP0` must fit in 16 bits and `SS0` must be valid; otherwise `#TS` before a frame is committed | 32-bit stack/frame semantics |
| Bitmap | Read the 16-bit map base at TSS offset `0x66`; bit `0` allows and bit `1` denies. Every bit for port `port..port + width - 1` must be readable and clear. A missing, truncated, or overflowing span denies with `#GP(0)` | V86 and unbounded map interpretation |
| I/O forms | Existing immediate/DX IN/OUT and string I/O reach the same width-aware check | new I/O opcodes or host APIs |
| Fault | A denied access does not reach the port provider; it is a `#GP(0)`, then uses T259's one protected delivery attempt and copied diagnostic. Failed delivery retains the original terminal fault | firmware/product fault consumption |

T260 S1 uses a core-owned test port provider and one 80386 prepared state with
a 32-bit TSS, 16-bit CPL3 code/data segments, and the existing IDT `#GP` gate.
The corpus must prove allowed IN and OUT reach that provider, denied IN and OUT
do not, denied access reaches the `#GP` handler and produces a copied
diagnostic, a one-byte map truncation denies a word span, and 80286/IOPL bypass
cases retain their stated behavior.

### Rules, Evidence, And Stop Conditions

Applicable rules: core owns CPU/TR/TSS/ports/diagnostics; core does not depend
on VM/VDM; no global/TLS current state; providers are frozen before reset;
platform never mutates guest state; no protected media or external runtime
dependency. S2 may change only the core CPU privilege/port boundary and the
T259 stack-source helper needed for a 32-bit TSS. S3 adds a core-only port/TSS
corpus and retains T257--T259, paging, DOS boot, FDD/HDD, Console, debugger,
and current gates. The expected focused marker is
`M5:T260:S3:TSS-IOMAP:CORPUS:OK`.

### S4 Closure

S4 is complete. `current-gcc` selects `vm-0-5-0260` and generated
`build/output/nxvm_0_5_0260.exe`, SHA-256
`D04F3F90E74F1CA3B47158C7E85F8981FCD53BD69E652ED17134FFDABEDA5D08`.
`current-gates-gcc` passed its static/ownership checks and **94/94** current
CTest cases. T260 admits only the tested 80386 32-bit-TSS I/O-map behavior
plus the retained 80286/IOPL deterministic cases; task switching, generic
protected interrupt delivery, LDT, 32-bit frames/gates, CPL1/2, V86, and
broader 386 compatibility remain deferred.

Stop and split if the corpus needs task/busy-TSS switching, LDT, call/task/trap
gates, a 32-bit frame/gate, CPL1/2, V86 semantics, a VM/firmware/host port
shortcut, a second executor, or a Console/debugger/boot UX change.

### S2 Implementation Result

S2 is complete. `_kpa_test_iomap()` now owns the one bitmap decision before
the retained port executor. It rejects unsupported width, V86, non-80386
profiles, absent/non-32-bit busy TR, overflowing port spans, a truncated TSS
map-base field, missing bitmap bytes, and any set bit with `#GP(0)`. It reads
the TSS only through the existing core descriptor/memory route. `_p_input`,
`_p_output`, `_p_ins`, and `_p_outs` still share `_kpa_test_mode()`, so no I/O
form gained a bypass.

The retained 16-bit gate stack transition now accepts a loaded 32-bit busy TSS
as a read-only `ESP0`/`SS0` source. `ESP0` must fit the admitted 16-bit stack
frame; otherwise it returns `#TS(0)` before changing the frame. The existing
16-bit TSS path is unchanged. No VM, firmware, platform, or product source was
changed.

### S3 Corpus Result

S3 is complete. `core-machine-tss-iomap-port-smoke` is a core-only prepared
state corpus, registered in `PROJECT_CURRENT_SMOKE_TARGETS`. It proves an
80386 CPL3 program can perform an allowed byte `IN` and `OUT`, while bitmap
denied `IN`, bitmap denied `OUT`, and a word operation crossing a truncated
bitmap boundary enter the T259 `#GP(0)` gate without reaching the port
provider. It also proves `CPL <= IOPL` bypasses the bitmap under both the 80386
and 80286 profiles. The target emits
`M5:T260:S3:TSS-IOMAP:CORPUS:OK`.

**Similar-issue sweep plan.** The defect class is a protected low-privilege
port operation that bypasses a single permission decision. Before closure,
search all `_p_input`, `_p_output`, `_p_ins`, `_p_outs`, and direct
`core_machine_port_execute_*` production references. Every hit must be the
single core guard or be explicitly outside guest execution; the focused corpus
must show both provider-reached and provider-not-reached outcomes.

## T259 S1 Admission Packet

S1 is complete. T259 admits only a common 80286/80386 **16-bit**
protected-mode subset:
CPL3 software `INT imm8` through a DPL-permitted 16-bit interrupt gate into
CPL0, a read-only 16-bit TSS `SS0:SP0` stack switch after `LTR`, and the paired
outer `IRET` back to CPL3. TSS here is only a stack-field source: task switch,
I/O bitmap, LDT, call/task/trap gates, 32-bit frames, CPL1/2, V86, hardware
INTR/NMI delivery, and double/triple-fault machinery remain forbidden.

`core/machine` remains the sole owner of CPU, descriptor, IDT, TR, stack, and
diagnostic state; `core_machine_run()` remains the only executor. Every
delivered exception must leave a copied core diagnostic event before its gate
transfer. An invalid or unhandled delivery remains the existing terminal
`STOP_FAULT` with first-fault diagnostic. Firmware, VM composition, platform,
and product code may not emulate or consume either path.

The S1 corpus is one CPL0 -> CPL3 -> CPL0 -> CPL3 round trip: CPL0 configures
GDT, 16-bit IDT, and 16-bit TSS; outer `IRET` enters CPL3; CPL3 executes
`INT 30h`; the gate switches to `SS0:SP0`; CPL0 marks guest memory and `IRET`s
back; CPL3 marks guest memory, then returns through another DPL3 gate to a
CPL0 `HLT`. A DPL-rejected CPL3 software interrupt must enter the configured
`#GP` gate, leave a copied core diagnostic, and then stop in CPL0. An absent
or invalid exception gate remains terminal `STOP_FAULT` with the original
first-fault diagnostic. Existing code establishes the precise gaps:
`_ksa_load_sreg` omits code-segment privilege checks and `_ser_ret_far_outer`
is a stub; no VM, firmware, or host shortcut is admitted.

| S1 concern | Required T259 result |
| --- | --- |
| Selector rules | Nonconforming code transition validates `CPL`, `RPL`, `DPL`, Present, and limit before mutable CPU state changes. |
| IDT gate | Accept only present 16-bit interrupt gates; software INT checks gate DPL, CPU faults do not. |
| Privilege stack | Validate TSS/`SS0:SP0` and target stack before committing an outer-to-inner frame. |
| IRET | Support only the paired 16-bit inner-to-outer frame; invalid frames fault without partial segment/stack mutation. |
| Diagnostics | Retain first terminal fault and add a copied delivered-exception observation/count for successful gate delivery. |
| Evidence | New core-only target emits `M5:T259:S2:PROTECTED-PRIVILEGE:OK` and `M5:T259:S3:PROTECTED-PRIVILEGE:CORPUS:OK`; S4 runs current gates and records the 0.5.0259 SHA. |

S2 stops and requests a split if this corpus requires a 32-bit frame/gate,
generic hardware interrupt delivery, call/task/trap gates, task switch, TSS
I/O bitmap, LDT, a second executor, host/firmware mutation, or a Console,
debugger, or boot UX change. The artifact identity is fixed now:
`nxvm_0_5_0259.exe`.

### S2 Result

`core/machine` now owns the admitted 16-bit gate transfer, TSS `SS0:SP0`
stack switch, outer `IRET`, and `#GP` delivery attempt. A successful delivery
records a copied `last_delivered_exception` plus count; a delivery failure
restores and reports the original fault rather than replacing it with a
secondary gate fault. The new `core-machine-protected-privilege-smoke` covers
the round trip and a DPL-rejected `INT 32h` delivered through `#GP` for both
80286 and 80386 profiles. It is now part of the current CTest matrix.

### S3 Result

The focused target must emit both `M5:T259:S2:PROTECTED-PRIVILEGE:OK` and
`M5:T259:S3:PROTECTED-PRIVILEGE:CORPUS:OK`. It proves one 80286 outer-ring
round trip, then runs the DPL-rejection / delivered-`#GP` case under both 80286
and 80386 profiles. The retained T257 corpus supplies adjacent invalid-selector,
non-present code/stack, same-CPL `IRET`, protected `LIDT`, configured-gate, and
pre-286/386 profile-rejection coverage. Both focused CTest entries passed; the
new corpus emits its S2/S3 markers and no firmware, VM, platform, or host-memory
shortcut is present.

**Similar-issue sweep.** The defect class is a protected `#GP` losing its
original diagnostic when a bounded IDT-delivery attempt fails. The S3 query is
`rg -n "_e_except_n\\(|_ser_int_protected\\(|record_delivered_exception|VCPUINS_EXCEPT_GP" src tests CMakeLists.txt`.
Its only production delivery site is `ExecFinal`; the remaining real-mode IVT
path is explicitly not a protected delivery route. The focused T259 smoke and
retained T257 negative cases lock the two dispositions: successful protected
delivery records an event, and unavailable delivery preserves the original
terminal fault.

### S4 Closure

`current-gates-gcc` passed all 93/93 CTest cases and its static ownership,
artifact-truthfulness, and dependency gates. `current-gcc` built the current
target `vm-0-5-0259` and copied
`build/output/nxvm_0_5_0259.exe`; SHA-256 is
`61FEF63CD57ED1FD46D1B5A2B49C24538830FCAF3EDF0D35EF5D09CAE8FC283A`.
The next task is required to preserve this baseline.

## T258 Closure Record

### Original Request

On top of T257's GDT-only, 16-bit CPL0 protected-mode path, admit one real,
verifiable 80386 CPL0 4 KiB paging path. It includes only page-directory/page-
table walks, `MOV r32,CR0`, `MOV r32,CR2`, `MOV CR0,r32`, `MOV CR3,r32`, and
core diagnostic `#PF`. The target artifact is `nxvm_0_5_0258.exe`.

### Frozen Scope

| Concern | T258 admission | Deferred / forbidden |
| --- | --- | --- |
| Owner and path | `core/machine` CPU, segment translation, physical memory route, and diagnostics; existing `core_machine_run()` only | VM/profile/firmware/platform paging or fault shortcuts; second executor |
| Address translation | logical -> T257 segment cache -> linear -> two-level 4 KiB page walk -> core physical route | host MMU, RAM pointers, PSE/PAE/V86/long mode, TLB cache |
| Control forms | 80386 CPL0 `MOV r32,CR0`, `MOV r32,CR2`, `MOV CR0,r32`, `MOV CR3,r32`; `CR2` is fault-written only | guest write `CR2`, all other CR forms, 80286/80186 forms |
| CR0 / CR3 | only PE and PG may be changed by T258; PG requires PE; CR3 must be page-directory aligned | opening MP/EM/TS/ET semantics, silent CR3 normalization |
| Page checks | Present, 4 KiB address, core-routed A/D updates, CR2 and P/W/U `#PF` diagnostic | CPL3 user/supervisor faults, supervisor write-protect fault, protected IDT delivery |
| CPL0 semantics | CPL0 may access supervisor and read-only mappings as 80386 permits; non-present fetch/read/write faults are proven | treating `RW=0` or `US=0` as a CPL0 fault; general privilege model |
| Fault result | copied `STOP_FAULT` / first-fault diagnostic at the original instruction | guest `#PF` handler, IDT/task/call gate, double/triple fault |

`RW=0` is **not** a CPL0 write-protect failure on the 80386: `CR0.WP` is not
part of this CPU. T258 proves that supervisor behavior rather than fabricating
a write-protect `#PF`; actual user write protection waits for T259's CPL3
corpus.

### S1 Result

S1 is complete. The retained executor already had one core-owned two-level
4 KiB walker, but its generic control-register decoder allowed guest writes
outside T258's contract and fault rollback discarded a newly recorded `CR2`.
T258 therefore keeps the single walker and executor, narrows `MOV CRx`, and
retains page-fault `CR2` in the copied core diagnostic. The similar-issue
sweep also found VM debugger raw control-register mutation; it is a separate
debug-boundary debt recorded in `TODO.md`, not a second paging path.

### S2 Result

S2 is complete. `MOV r32,CR0` now permits only CR0/CR2 reads, while guest
writes permit only CR0 and aligned CR3. CR0 changes are limited to PE/PG, PG
cannot be set before PE, and guest CR2 writes reject as `#UD`. The existing
page walker remains the sole logical-to-physical path. `ExecFinal` now copies
the fault-written CR2 into the retained first-fault snapshot before rollback.
The focused target passed its valid mapping, fetch/data/stack, A/D, page-fault,
control-form, CPU-profile, and reset checks.

### S3 Result

S3 is complete. `core-machine-80386-paging-smoke` emits both the S2 and S3
markers after proving valid code/data/stack mappings, PDE/PTE A/D updates,
non-present fetch/read/write faults with retained CR2 and original point,
control-form rejection, profile gates, and cold-reset clearing. The final
`current-gates-gcc` run passed all 92 CTest cases, including retained DOS,
FDD/HDD, input, video, Console, and debugger coverage.

### S4 Result

T258 maps exactly to artifact version `0.5.0258`. The current target is
`vm-0-5-0258`; `build/output/nxvm_0_5_0258.exe` SHA-256 is
`51AAF534434F0943AE3BCBB4AA4A56C3ED1A815B79C47BC4FEC4B6DD02B8F62C`.
The runtime implementation was verified from source commit `2a6442e`; the
artifact-target configuration and this closure record are the following S4
commit. The next task must establish a new complete packet before source work.

### S1 Audit And Requirement Map

| Requirement | Current evidence | S1 disposition / planned evidence |
| --- | --- | --- |
| Page walk | `_kma_physical_linear` already reads PDE/PTE through core physical memory, sets A/D, and records `CR2`/`#PF` | Narrow and prove it with `core-machine-80386-paging-smoke`. |
| `CR0` / `CR3` writes | `_d_modrm_creg` exposes CR0/2/3 and `MOV_CR_R32` writes the selected storage directly | Replace with explicit form-specific validation: CR2 write `#UD`; CR0 mask and PE-before-PG; aligned CR3 only. |
| Profile gate | 0F `20h`--`26h` are metadata-gated at 80386 | Corpus proves 80386 positive and 80286/80186 `#UD` negatives. |
| Fault boundary | `ExecFinal` records a first fault then stops; T257 has separately disabled protected IDT delivery | Prove `#PF` retains original point, CR2, P/W/U code, and `STOP_FAULT`. |
| Reset | CPU cold reset zeroes CR0/CR2/CR3 before retained reset-vector setup | Corpus proves PG and CR3 clear after cold reset. |

The similar-issue sweep covers all tracked production references to
`_kma_physical_linear`, `_d_modrm_creg`, `MOV_R32_CR`, `MOV_CR_R32`,
`VCPU_CR0_PG`, `cr2`, `cr3`, and `VCPUINS_EXCEPT_PF`; each hit is either the
single owner path, an explicit negative gate, or deferred to T259--T261.

### S1 Rules, Verification, And Stop Conditions

Applicable rules: `core` has no VM dependency; no global/TLS current object;
all page-table accesses use the checked core physical route; profile selection
is frozen before reset; platform never mutates guest state; no protected media
or external runtime dependency. The planned focused target is
`core-machine-80386-paging-smoke`, with S2 marker
`M5:T258:S2:I386-PAGING:OK` and S3 marker
`M5:T258:S3:I386-PAGING:CORPUS:OK`.

S2 stops and requests a split if the corpus needs CPL3, protected IDT delivery,
TSS, task switch, a second executor, VM/firmware mutation, host-side page-table
access, or a Console/debugger/boot UX change. S3 must run the focused corpus,
T257 and real-mode corpora, FDD/HDD/DOS, Console/debugger, CGA/EGA, ATA, RTC,
and `current-gates-gcc`. S4 records the T258-to-`0.5.0258` artifact mapping and
SHA-256 before closure.

| Closure | Evidence |
| --- | --- |
| T249--T251 | Copied input, presentation, and cancellable wait contracts are closed with artifacts `0.5.0248`--`0.5.0250`. |
| T252 | Composition owns the sole run-handle teardown sequence; artifact `0.5.0251` and 86/86 current CTest pass. |
| T253 | ATA PIO sector-count progression is controller-owned; artifact `0.5.0252` and 87/87 current CTest pass. |
| T254 | Bounded digital CGA `640x200x2` is VADP-owned; artifact `0.5.0253` and 89/89 current CTest pass. |
| T255 | Machine-profile admission contract is closed; design/governance only, no artifact. |
| T256 | Core-owned Level 1 rational device clocks are closed; artifact `0.5.0254` and 90/90 current CTest pass. |
| T258 | Bounded 80386 CPL0 paging baseline is closed; artifact `0.5.0258` and 92/92 current CTest pass. |
| T259 | Bounded 16-bit protected privilege and `#GP` IDT delivery are closed; artifact `0.5.0259` and 93/93 current CTest pass. |
| T260 | 80386 32-bit-TSS I/O-map allow/deny is closed through the real CPL3 corpus; artifact `0.5.0260` and 94/94 current CTest pass. |
| T261 | Bounded 16-bit-TSS far-JMP task switching is closed through the core-only positive and fault corpus; artifact `0.5.0261` and 95/95 current CTest pass. |
| T262 | Exact-8087 finite `m32real` load/store/arithmetic, status/stack state, and pending-exception `FWAIT` are closed through a core-only corpus; artifact `0.5.0262` and 97/97 current CTest pass. |
| T263 | 80286 protected-mode `ARPL r/m16,r16` now matches its frozen metadata and is covered by register, ES-memory, profile-gate, and rejection-atomicity corpus; artifact `0.5.0263` and 99/99 current CTest pass. |

The next instruction family must establish a complete active packet before implementation.

## Current Technical Baseline

- **T263 artifact identity:** `current-gcc` and
  `verify-current-artifact-target` select `vm-0-5-0263`; static/ownership
  checks and 99/99 CTest cases passed. Artifact `nxvm_0_5_0263.exe` SHA-256:
  `CDECA028180652317C2EDC9C872B8BDB0F49A5465C506A6921BC2A00DB0CAF02`.
- **T243--T246:** core owns checked physical memory, bounded `#UD`
  transitions, immutable ROM mapping, and atomic real-mode entry plans. T247
  verifies the current artifact target and full gate over that boundary.
- **Product boundary:** `nxvm.exe` is the retained runnable product. `mantle`,
  `dos`, and `nxvdm.exe` remain future architecture commitments; they are not
  current runtime or release products.

Completed implementation detail, artifact history, and rationale are in
[M5 History](../history/m5.md) and Git history. When a task is active, its
packet, the roadmap, and the M5 closure checklist are the current operational
authorities.

## Recent Governance Updates

- **M5 Td S20:** aligned the five-component architecture.
- **M5 Td S21:** added source-distribution notices and release-record gates.
- **M5 Td S22:** aligned roadmap and trusted external-research boundaries.
- **M5 Td S23:** compacted completed M5 records into history, corrected the
  historical baseline, clarified notice provenance, and removed an untracked
  temporary error file. This documentation task changes no runtime behavior.
- **M5 Td S24:** fixed the time/device ownership vocabulary: core owns guest
  ticks and generic PIT/PIC/DMA mechanics; VM owns PC/AT CMOS/RTC and BIOS time
  semantics; composition owns the bounded product pump and host pacing. This
  documentation task changes no runtime behavior or active-task scope.
- **M5 Td S25:** reconciled completed M5 baseline references to T252 S3 and
  removed stale task scheduling from the completed T248 record. This
  documentation task changes no runtime behavior or active-task scope.
- **M5 Td S26:** split the unstarted 80386 route so T258 is CPL0 paging only;
  protected privilege/IDT delivery, TSS I/O permissions, task switching, FPU,
  and remaining instruction families now have independent admission tasks.
  This documentation task changes no runtime behavior or active-task scope.
- **M5 Td S27:** compacted completed T248--T257 task records into M5 history,
  removed the completed T248 ledger entry, and clarified idle-state authority.
  This documentation task changes no runtime behavior or active-task scope.
- **M5 Td S28:** replaced independent artifact revisions with task-identity
  revisions. The T257 current artifact is reissued as `0.5.0257`; future
  numeric task artifacts must use their task number exactly.
- **M5 Td S29:** aligned the architecture overview with the task-identity
  artifact rule. This documentation task changes no runtime behavior or
  active-task scope.

## Milestone State

| Milestone | State | Current authority |
| --- | --- | --- |
| M0--M4 | Closed | [Roadmap](roadmap.md) and [history](../history/README.md) |
| M5 | Open | [M5 closure checklist](m5-closure-checklist.md) |
| M6--M8 | Not started | [Roadmap](roadmap.md) |
| M9 onward | Research queue | [Roadmap](roadmap.md) |

## Operational Reading Order

1. [Documentation Guide](../README.md)
2. [Roadmap](roadmap.md)
3. [M5 closure checklist](m5-closure-checklist.md)
4. [Architecture overview](../architecture/overview.md) and
   [contracts](../architecture/contracts.md)
5. [Requirements](../requirements/)
