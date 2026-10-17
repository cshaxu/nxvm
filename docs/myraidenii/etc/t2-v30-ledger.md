# T2 V30 Convergence Ledger

## Frozen Domain And Completion

Source baseline: `7f7d23938`, NEC V20/V30 October 1986 manual pinned by T1.
The target is the original 16-bit V30, not V20 bus width, V33/DX, 80286 protected
mode or an Intel CPU with a substituted name. All runtime rows below are
unimplemented/unqualified at S1. This is the complete receiving domain, not
a claim that all instruction encodings have already been semantically audited.

The opcode domain has three complete 256-slot spaces: native primary, native
NEC 0F extension, and 8080 emulation. Every slot must receive a sourced legal,
prefix/escape, reserved/undefined or model-required disposition. Legal slots
expand into every documented register/memory width, ModRM extension, immediate
and control variant. Reserved slots are not automatically Intel #UD or NOP.
Prefix handling remains part of the same form owner, not a fourth interpreter.

Each form's proof key includes mode, opcode/subopcode, addressing form, operand
width, effective segment, prefix class and the relevant outcome/bus context.
Required values distinguish zero/sign/carry/overflow/parity/auxiliary carry,
boundary shift counts, zero/one/multiple repetitions, branch outcomes and
divide success/overflow/zero. Undefined flags are masked only with source
justification. Unknown facts remain explicitly non-eligible or require a
reviewed L2 model; they do not shrink the documented form/mode universe.

Accept a row only with its source, implemented sole path and executed vectors
in both widths. T2 exit requires no unimplemented documented row, a complete
slot/form disposition, source-graded timing and neutral Machine admission,
old Intel regressions, original board-path checkpoints and current affected
PC artifacts. Instruction/time budgets never count as coverage. A diagnostic
which reaches further cannot substitute for the complete form/state ledger.

## Mechanism Batches And Proof Owners

| ID | Complete mechanism / necessary contexts | Source and current owner | Receiving proof / disposition |
| --- | --- | --- | --- |
| V01 | Family membership, enum stability, DEFAULT/invalid admission, name, FPU-none board selection | CPU interface/lifecycle and Machine create; manual sections 1, 3, 7 | Family/reset batch; preserve old values and reject future/invalid integers; not yet implemented |
| V02 | Reset native PSW/MD, FFFF:0000, segments, register-defined versus unspecified values, reset after both modes | CPU reset; manual sections 3, 7--8 | Public reset/entry vectors; no invented register constants; not yet implemented |
| V03 | Native primary slots 00--FF with every documented form and reserved disposition | CPU form metadata/handler table; manual section 12 and Appendix A | Shared CPU native table-driven vectors and matching preview; not yet implemented |
| V04 | NEC extension slots 00--FF, bit test/set/clear/invert, packed BCD strings, nibble rotation, bit fields, mode-entry form | CPU NEC form owner; original section 12 encoding diagrams | Every defined suffix/form/length and undefined disposition; no copied external decoder; not yet implemented |
| V05 | Segment, repeat/conditional-repeat and bus-lock prefixes, order/last override, interrupted restart and long malformed streams | CPU existing prefix/refresh owner; manual sections 6, 12 | Decode/execution/preview equivalence and bounded safe-point tests; no assumed Intel 15-byte silicon rule; not yet implemented |
| V06 | MOV/exchange/address/stack, byte registers, segment loads, PUSH-SP and PUSHA/POPA/ENTER/LEAVE variants where defined | CPU existing transfer/stack handlers; manual section 12 | Register/stack/bus ordering, alias and wrap vectors; not yet qualified for V30 |
| V07 | ALU, decimal adjust, multiply/divide, sign extension, shift/rotate count and flags | CPU existing arithmetic/rotate handlers; manual section 12 | Full operand/flag boundaries, divide fault saved-PC and undefined flag masks; not yet qualified |
| V08 | Near/far/conditional branches, loops, calls/returns, breakpoint/interrupt return | CPU control/entry helpers; manual sections 6, 12 | Taken/not-taken, stack frame, segment wrap and target-failure observations; not yet qualified |
| V09 | Block transfers/comparisons, port strings, REP/REPC/REPNC zero/first/continuation/terminal and interrupts | CPU existing repeated instruction owner; manual sections 6, 12 | Register/memory/port effects, repeat flags and restart point; not yet implemented/qualified |
| V10 | PSW fixed/writable bits and MD, flag load/image, debug patches/entry, native versus emulation flag meanings | CPU owned architectural register image; manual sections 3, 8, 12 | PUSHF/POPF/IRET/BRKEM/CALLN/RETEM transitions and safe debug writes; no second mode flag; not yet implemented |
| V11 | NMI/INT/BRK/divide/step priorities, acknowledge, shadows, native entry from emulation, nested native handlers | CPU existing entry/pending path; manual sections 6, 8 | Exact vector/frame/return/restart and failed provider tests, independent of board PIC; not yet implemented/qualified |
| V12 | 8080 slots 00--FF, register mapping, arithmetic/flags, stack, memory and port operations | CPU same executor selected by MD; manual section 8 plus NEC-documented 8080 instruction contract | Complete emulation table and ROM-free vectors, not a Z80 substitute; not yet implemented |
| V13 | BRKEM, CALLN, RETEM, native IRET reentry, RESET abort and prohibited emulation nesting | CPU mode/entry owner; manual section 8 figures 8-4--8-7 and section 12 | Native->8080->native round trips and interrupted emulation, stack alias/failure; not yet implemented |
| V14 | HLT/WAIT/LOCK, physical address wrapping, even/odd word lanes and split transfers, fetch versus observational reads | CPU bus/segment owner plus existing public provider; manual sections 4--6, 12 | Bus provenance/effects, wait/interrupt/reset safe points and no partial replay; not yet qualified |
| V15 | Prefetch-qualified instruction clocks, operand transfers, branch/repeat/entry terms, maximum bounded run and waits | CPU timing selector/model; manual section 12 introductory assumptions and individual forms | V30 source/L2 origins, actual-retirement/preview/max-query consistency; no inherited Intel fallback; not yet implemented |
| V16 | Neutral Machine validation, one clock/attachment axis, physical retirement eligibility and board vector 0x30 | Machine configuration/scheduler/retirement; T1 public contract audit | Synthetic board public-route tests then original-set instruction checkpoints; not yet implemented |
| V17 | Old Intel 8086/8088/186/286/386/486 behavior and consumer deployment | Existing Shared units and four PC Apps | Whole repo-only x64/x86 units, required affected integration and eight current PC EXEs; existing baseline passes, post-change proof required |

Unit locations remain inside `test/x86-core/cpu` and `test/x86-core/machine`,
not MyRaidenII unit files covering Shared internals. Board-owned real-set
checkpoint harnesses belong to `test/myraidenii/integration`; earlier CPU
diagnostics must not call that coverage complete before it actually runs.

## Existing Ordinal Inventory

Query over all C/H in CPU and Machine:

```text
rg -n '(cpu_profile|profile)\s*(>=|<=|<|>)\s*(X86_CORE_CPU_PROFILE_|metadata.minimum_cpu)' src/x86-core/cpu src/x86-core/machine --glob '*.c' --glob '*.h'
```

At the frozen source baseline: 306 matching lines, comprising 297 instruction
unit lines, one instruction header, six timing-model and two Machine lines.
The matching line count is not the number of bugs, expressions or needed edits.
The complete candidate class dispositions, before implementation, are:

| Candidate class | Disposition and unique implementation receiver |
| --- | --- |
| 83 identical `>=80386` branches whose two arms are only `_adv;` | Remove the meaningless branch at the instruction owner; preserve exactly one existing advancement and surrounding handler/table style. No new helper or behavior. |
| Other 80386 generation decisions, including operand/address width, descriptor/cache/debug/entry paths | Explicit Intel-family capabilities, not new-V30-is-later; native-form batch must inspect each use before V30 admission. |
| 80186 decisions and minimum-CPU metadata comparison | Do not equate V30 with the entire 80186 set. One family/form admission owner must describe actual NEC-compatible forms and true differences. |
| 80286 decisions, double fault and protected/saved FLAGS paths | Protected architecture is Intel membership only; V30 real/native/MD paths need their own justified behavior, not a generic early-Intel alias. |
| Lexeme validity/profile range and Machine validation range | Explicit membership at admission; retain DEFAULT's existing meaning and reject unknown enums. Do not register an incomplete CPU as an eligible Machine. |
| Early-Intel exception/repeat restart and priority comparisons | Entry/mode batch qualifies V30 explicitly; preserve existing Intel ordering, not blanket inversion of `>=` checks. |
| Six timing-model ordinal matches and selector fall-through outside this regex | V30 timing owner cannot inherit Intel source IDs/grades; use necessary approved origin values and honest ineligible cases until implemented. |

Other sources requiring review although absent from the exact ordinal query:
CPU reset/base/name/profile binding; prefix enums and public early-8086 helper;
CPU bus/cache `==80486SX` checks; metadata space selection; live debug/entry
patches; Machine retirement key and time admission. Equality guards already
specific to 486 are retained unless a documented V30 behavior requires change.
No broad V30-as-8086 helper or public capability registry is justified.

## Batches And Prospective Code Cost

S1 is the current audit, not a Shared source change. Next bounded source work
can remove the 83 proven no-op forks before family admission, preserving the
single handler path. The estimated delta is approximately +0/-581 instruction
lines (seven net lines per fork); actual diff must be counted before delivery.
That removal is not resolution of the other 223 ordering candidates.

S2 implements that exact preparation: 83 forks removed, CPU +86/-667/net
-581 and existing gate/negative coverage +6/-0, total net -575 code lines.
Both full repo-only unit suites pass 655/655 after the recorded unchanged
x86 timeout/retry; eight current PC artifacts are refreshed. No V30 row is
accepted as implemented by this preparation. See the
[complete delivery](evidence/t2-s2-advancement-consolidation.md).

Full V30 work is larger and estimates remain provisional until each S freezes
its actual forms: native family/NEC forms about +900--2200 CPU source lines;
8080/mode-entry about +600--1400; timing/Machine about +400--1000; ROM-free CPU
and Machine vectors about +700--1600 across those batches. Existing verified
mechanisms are reused, not reproduced to meet a guessed line count. No Lib,
Emulator, X86 IBMPC or X86 NXVM source change is currently justified. These
estimates are not approval for new public functions, layouts or callbacks.

S1 rechecked source admission/reset/form/preview/entry/time seams and rendered
the NEC repeat-prefix and emulation-return pages. Original text clocks assume
prefetched instruction bytes; adding bus/prefetch terms requires an explicit
source/L2 accounting model, not charging the same fetch twice. REPC/REPNC
must inspect comparison results rather than a copied Intel ZF predicate.
Native return from emulation restores the architectural MD from the saved
frame; an App-owned parallel mode flag would be a second owner.
