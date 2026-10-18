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
| V01 | Family membership, enum stability, DEFAULT/invalid admission, name, FPU-none board selection | CPU interface/lifecycle and Machine create; manual sections 1, 3, 7 | S3 implements native CPU membership/name and enum stability, rejects invalid profiles and keeps Machine admission closed; board/FPU selection remains unqualified |
| V02 | Reset native PSW/MD, FFFF:0000, segments, register-defined versus unspecified values, reset after both modes | CPU reset; manual sections 3, 7--8 | S3 native reset/entry and MD-clear reset vectors implemented; S23 adds actual BRKEM and native CALLN-handler reset abort plus dirty-context initialization of the hardware MD write-enable. S24 adds ordinary emulation without another reset state; reset/init remain at that existing sole owner. Complete Machine/board reset qualification remains V16 |
| V03 | Native primary slots 00--FF with every documented form and reserved disposition | CPU form metadata/handler table; manual section 12 and Appendix A | S12 unifies actual primary ModRM shape/minimum admission with pure scan before operand effects; nineteen constrained opcode families cover every ModRM and segment prefix, valid witnesses, truncation and fetch failures/wrap. No invented native INT6. Complete native slot/form disposition and legal instruction semantics remain unqualified |
| V04 | NEC extension slots 00--FF, bit test/set/clear/invert, packed BCD strings, nibble rotation, bit fields, mode-entry form | CPU NEC form owner; original section 12 encoding diagrams | S4 implements 10--1F/28/2A; S5 implements documented packed-BCD 20/22/26; S6 adds bit-field 31/33/39/3B through the same format/metadata/preview owner. S23 adds BRKEM FF vector admission at that same owner. Output-alias silicon, full prefix/bus contexts, undocumented inputs and timing remain unqualified |
| V05 | Segment, repeat/conditional-repeat and bus-lock prefixes, order/last override, interrupted restart and long malformed streams | CPU existing prefix/refresh owner; manual sections 6, 12 | S7 implements native carry comparison prefixes, one execution/preview recognizer, prefix-selection vectors and up-to-three-prefix interrupt restart; complete LOCK, long-stream and other mode/prefix qualification remains open |
| V06 | MOV/exchange/address/stack, byte registers, segment loads, PUSH-SP and PUSHA/POPA/ENTER/LEAVE variants where defined | CPU existing transfer/stack handlers; manual section 12 | S14 qualifies native GP stack/register-set/frame forms and seven-transfer POPR; S15 qualifies segment transfer/results and associated external inhibition. S16 qualifies ordinary byte/word MOV, XCH, LEA and XLAT results, all memory ModRM/default/override, GP aliases, immediates, wrap/lane/tail and data failures through unchanged handlers. SP/data-tail and store-BRK L2 grades remain explicit. Physical LOCK/interoperation, complete prefix/mode contexts and PSW/control frames retain their original receivers; complete V06/T2 is not inferred from result tests |
| V07 | ALU, decimal adjust, multiply/divide, sign extension, shift/rotate count and flags | CPU existing arithmetic/rotate handlers; manual section 12 | S11 fixes native shift/rotate. S17 qualifies existing native scalar ADD/ADDC/SUB/SUBC, OR/AND/XOR/CMP/TEST, INC/DEC/NEG/NOT and CVTBW/CVTWL, with independent complete CPU/defined-flag, every byte-pair, full sign-extension input, encoding/alias/segment/lane/tail and provider-failure proof. S18 qualifies native ADJBA/ADJBS and radix-ten CVTBD/CVTDB across complete AX domains and defined flags; packed ADJ4A/ADJ4S source/semantic ambiguities, non-ten conversions, mode, timing and undocumented contexts remain unqualified. S19 implements documented native signed quotient bounds and following-PC vector-0 return; complete native byte/word unsigned/signed and immediate multiplication/division result/alias/host-failure proof is recorded separately, without retiring general V11/time receivers |
| V08 | Near/far/conditional branches, loops, calls/returns, breakpoint/interrupt return | CPU control/entry helpers; manual sections 6, 12 | S20 qualifies existing native sixteen Jcc, LOOP variants/JCXZ, short/word relative and direct/indirect near/far JMP/CALL, and ordinary near/far RET with/without cleanup. Complete CPU/unchanged flags, target/return words, every legal ModRM, aliases/segment/wrap/overlap, actual CALL-to-RET and accepted-prefix host failure are proven separately. S22 adds ordinary-native software entry and RETI frame/return proof. Full priority, emulation-sensitive PSW/mode, prefixes/LOCK, asynchronous target-fetch and physical clocks retain original receivers |
| V09 | Block transfers/comparisons, port strings, REP/REPC/REPNC zero/first/continuation/terminal and interrupts | CPU existing repeated instruction owner; manual sections 6, 12 | S7 qualifies carry/zero repeat selection; S9 qualifies port strings; S10 qualifies MOVS/LODS/STOS and actual interrupt/IRET continuation. S25 qualifies all four native CMPS/SCAS byte/word result/defined-flag, count/DF/source/fixed-ES, canonical repeat, real wrap/lane/overlap and every observed host-refusal context with 942,920 inputs plus 800 actual IRQ/RETI routes per width. Distinct old prefix order/Intel/excess-prefix restart proof remains; weak operand/failure paths are removed. Complete long-prefix/mode/electrical timing and full general interoperation remain original receivers |
| V10 | PSW fixed/writable bits and MD, flag load/image, debug patches/entry, native versus emulation flag meanings | CPU owned architectural register image; manual sections 3, 8, 12 | S21 qualifies ordinary native PSW/AH/carry/direction/IE, native MD retention and public Debug/prepared-entry flags with primary undefined-bit masks. S22 qualifies every native RETI PSW word. S23 adds existing-loader MD restoration with one hardware write-enable and every RETEM/RETI/POP PSW input word. S24 qualifies the five mapped 8080 flags, fixed PSW bits, source-backed subtraction/AND/DAA differences, all POP PSW words and EI/DI. General step/entry/priority, loaded BRK sampling and physical timing remain original receivers |
| V11 | NMI/INT/BRK/divide/step priorities, acknowledge, shadows, native entry from emulation, nested native handlers | CPU existing entry/pending path; manual sections 6, 8 | S15 consumes S14's complete segment-transfer inhibition gap: both MOV directions and POP defer NMI/INT through the next instruction, with exact frame/IRET, chained transfers, IF/STI gating, step-shadow model and reset proof. PUSH/LDS/LES do not acquire this shadow. Store-BRK suppression remains explicit L2, not silicon-certified. S19 consumes only native divide vector 0: zero/overflow frame, next-PC/IRET and every operand/frame/vector provider failure, with unchanged accepted-prefix rollback. S22 repairs native CHKIND BRK5 to save following PC at the existing common owner and proves BRK3/immediate/conditional software frames, vectors, ordinary RETI and host-failure prefixes. CHKIND negative-domain signedness remains a retained comparator model, not source-complete instruction eligibility. S27 qualifies ordinary native/8080 NMI over enabled INT over BRK with 15,361 success inputs, exact single frames/pending state, real handler/RETI, retained INT after return, fresh-instruction re-stepping and accepted-prefix/ack/preview failures. Superseded steps clear only at the sole successful disabling real-entry commit; failed external entry preserves them. S28 repairs software-entry same-boundary priority once at successful real entry, with one private per-refresh completion fact consumed by debug and asynchronous admission. Both widths prove 3,457 native/8080 mode-call, taken/untaken, pending-source, flag, wrap and stack inputs plus every observed provider refusal; external requests remain pending until actual handler execution. Loaded-BRK/general arrival/nesting, full PSW/mode and physical timing retain original receivers; DE/BR retirement classification remains V15 |
| V12 | 8080 slots 00--FF, register mapping, arithmetic/flags, stack, memory and port operations | CPU same executor selected by MD; manual section 8 plus NEC-documented 8080 instruction contract | S24 implements 244 documented ordinary slots plus the existing constrained NEC escape at the same CPU executor/format owner. Both widths pass 4,328,948 inputs: mapped registers, ALU/DAA/rotate, every pair/PSW word, DS:BP stack, memory/ports, condition/flow, wrap/alias/preview and provider-refusal routes. Eleven ordinary reserved slots and 254 prohibited ED suffixes remain explicit diagnostics, never native fallback/NOP. Primary source/typo dispositions and independent oracle are in S24 evidence. Physical electrical sequencing/clocks and general priority/Debug/Machine remain V14--V16 receivers, not full CPU eligibility |
| V13 | BRKEM, CALLN, RETEM, native IRET reentry, RESET abort and prohibited emulation nesting | CPU mode/entry owner; manual section 8 figures 8-4--8-7 and section 12 | S23 implements BRKEM/CALLN/RETEM and MD-write enable at existing entry/return/executor owners; every selector/PSW, code/native-stack wrap and observed host refusal is checked. NMI/INT/CALLN native RETI reentry, RETEM relock, reset and prohibited nesting pass. S24 adds actual BRKEM -> ordinary 8080 stack/ALU/port/call program -> CALLN/RETI -> RETEM and EI/HLT/RETI routes. General BRK/priority, full prefix/alias contexts and physical time remain original receivers; no Machine eligibility |
| V14 | HLT/WAIT/LOCK, physical address wrapping, even/odd word lanes and split transfers, fetch versus observational reads | CPU bus/segment owner plus existing public provider; manual sections 4--6, 12 | S8 qualifies native lanes/20-bit width with a pinned-source L2 contiguous native data-tail model; S9 qualifies scalar/string I/O; S13 repairs actual fetch offsets at the common owner with preview/failure/wrap witnesses. S24 qualifies mapped 8080 byte/word address wrap, stack order, immediate-byte I/O and native/emulation HLT wake/entry/return, including disabled INT wake without acknowledgement. V30 electrical emulation microsequence/prefetch/standby hold time remains unqualified, not inferred from discrete 8080AF clocks. S26 corrects native POLL's inherited first deadline to 7 clocks (NEC 2 + 5n), retaining five-clock later samples at the existing wait owner; 35 pin/flags/preview/wrap/failure/reset/actual IRQ-RETI/8080-isolation cases pass both widths. Logical sampling does not qualify electrical POLL/WAIT/LOCK/physical timing or the reserved-port provider model, which remain original receivers |
| V15 | Prefetch-qualified instruction clocks, operand transfers, branch/repeat/entry terms, maximum bounded run and waits | CPU timing selector/model; manual section 12 introductory assumptions and individual forms | V30 source/L2 origins, actual-retirement/preview/max-query consistency; no inherited Intel fallback. S19/S22 qualify DE/BR frame results. S29 uses the existing accepted software-entry fact to restore completed-instruction observation instead of an unretired-fault marker; ordinary HALT-wake failure cannot publish a delivered event, with all-profile contrast and existing native/8080 success/failure vectors. S30 connects the complete native data-transfer nominal-clock source subfamily (NEC 12-4..12-13) at the sole selector with 24,160 actual-execution cases per width, appended nominal-source origin and no Intel fallback. S31 extends that sole selector with fixed scalar ALU/unary/CY-DIR-IE nominal rows, independently checked in the existing 917,460 ALU inputs and execution seam of 406,800 flag inputs; prefixes/conversions and fifteen adjacent exclusions remain non-eligible. No new interface/state or old Intel timing change. S32 adds original stack/scalar-port nominal rows, with 3,356 existing stack inputs, PSW/segment execution seams and 160 scalar-port cases; missing level-one/mixed-alignment/register-alias/prefix rows remain zero/unattributed at this same V15 receiver, not fabricated per-transfer slopes. S33 adds ordinary native branch/loop/JMP/CALL/RET nominal clocks through the existing pure decision and selector, with all 463,982 semantic inputs, no target observations, original rejection of far-register forms and retained unknown mixed/prefix timing. S34 adds all seven shift/rotate one/CL/immediate nominal rows and the source-known unpacked-decimal/radix-ten/CVTBW conversions, using original CL and decoded cimm rather than prefetch length. All 20356 shift, 817152 decimal, 917460 ALU and 24160 transfer inputs retain their proof; CVTWL and multiply/divide data-dependent ranges, packed semantic ambiguities, generalized radices and prefixes remain unknown. S35 adds every source-known ordinary NEC data nominal row: sixteen CL/immediate bit forms, ROL4/ROR4 and even-count BCD. The six existing proof units retain full semantics/failures and add 6720 bit, 4608 nibble, 31608 BCD seam and 5130 wrapped timing contrasts; all four field ranges and odd-count BCD rounding remain explicitly zero/unattributed. No executor/public/Intel path changes. S36 adds all fourteen ordinary string primitive/canonical-repeat nominal rows using the existing repeat phase recorder and original count/index/port parity. The three real string proof owners retain every old assertion, all 942920 comparison inputs and source-unknown extra-prefix dispositions; Block expands both endpoint parity axes and directly proves same-address NMI/INT reentry restarts setup at the sole successful V30 entry commit. No new public contract/state, Intel body or execution semantics. All total-time eligibility/maxima remain unallocated and Machine stays gated; prefix/alias/8080/other families and full bus/prefetch/dynamic/entry/runtime/max accounting retain the original receiver; selected-SX system-event metadata candidates have a separate TODO/source-L2 receiver outside V30 |
| V16 | Neutral Machine validation, one clock/attachment axis, physical retirement eligibility and board vector 0x30 | Machine configuration/scheduler/retirement; T1 public contract audit | Synthetic board public-route tests then original-set instruction checkpoints; not yet implemented |
| V17 | Old Intel 8086/8088/186/286/386/486 behavior and consumer deployment | Existing Shared units and four PC Apps | S36 final x64/x86 all-target builds and 678/678 whole-repository units pass, including 191 X86 Core tests and 498 strict rows per width. Eight PC 0564 releases, 14 selected receiving integrations plus restored-default identity per width and all ten PE/hashes pass; MyNES/INIs/media unchanged. Fresh whole external integration/gameplay is not claimed |

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

S3 replaces the remaining ordinal candidates with explicit private Intel/NEC
capability decisions and adds native reset/PSW and representative compatible
16-bit execution/preview at the existing CPU owner. This does not qualify
all primary forms, NEC extensions, emulation, entry/repeat, timing or Machine.
The owner additionally approved the sole external Board profile switch's
equivalent default branch and its manifest; it adds no PC V30 admission.
See [native family evidence](evidence/t2-s3-native-progress.md). No complete
runtime mechanism row is promoted solely from representative instruction tests.

S4 implements the complete eighteen-suffix bit/nibble subfamily at the same
metadata/execution owner, using primary NEC encodings and original synthetic
vectors. Its approved NEC-space enum is appended; old Intel values and dispatch
are preserved. Pure/execution preview, register/high-byte/alias, addressing,
flags and provider failure paths are checked. Unknown instruction-space values
are now rejected at metadata ownership rather than inheriting a valid primary
form. See [family evidence](evidence/t2-s4-nec-bit-nibble.md). This closes only
the named V04 subfamily, not the remaining NEC, prefix, bus or timing universe.

S5 implements all three packed BCD string operations for documented valid
decimal inputs and one-to-254 digit counts, preserving indices/count and fixed
ES destination with source overrides. An independent decimal oracle, all valid
byte pairs and lengths, the primary example stream, overlap/wrap and accepted
write-prefix failures provide owned proof. The NEC format owner now distinguishes
implicit and ModRM forms across decode/preview/execution; no second executor
or public layout change exists. See [BCD family evidence](evidence/t2-s5-packed-bcd.md).
Odd final nibbles/flags and undocumented input behavior are not silicon-exact
claims. Bit fields, mode/emulation, prefixes, full bus and timing retain their
original complete receiving rows; this subfamily does not close V04 or T2.

S6 implements the four INS/EXT encodings with one/two-word masked transfers,
existing register decode and logical bus operations, original-input capture and
post-transfer field/index update. A bitwise oracle exhausts offsets/lengths and
register combinations; actual manual example streams and every read/write
failure position provide separate proof. [Bit-field evidence](evidence/t2-s6-bit-fields.md)
distinguishes documented operations from AX/AL/AH output-alias source-order
model and still-unqualified physical bus/prefix cases. The FFFF-start word
limitation is now specifically inventoried under V14, not fixed by a parallel
instruction bus. No existing V03/V05--V16 receiving obligation is retired.

S7 implements native REPC/REPNC and existing zero-condition comparison repeat
through one prefix recognizer and one comparison-result predicate. All original
operand handlers, one-iteration execution, interrupt entry and IRET remain the
only production routes. [Conditional-repeat evidence](evidence/t2-s7-conditional-repeat.md)
records count/initial-flag/DF/termination vectors, segment/repeat selection,
first/partial read failure and actual NMI/INT-to-IRET continuation, including
the documented three-prefix retention. Non-comparison carry targets remain
diagnostic, not undocumented-silicon claims. This is a V05/V09 subfamily,
not retirement of their entire block/port/prefix/mode domain or any V14/V15 row.

S8 implements native memory transfer at the common logical/physical owners,
not an INS/EXT workaround. [Memory-transfer evidence](evidence/t2-s8-memory-transfer.md)
records primary lane/address facts and the exact reference-backed L2 segment-
tail model, 20-bit wrapping, code versus data behavior, pair/stack/bit-field
integration and every relevant split/fetch/observation failure position. The
formerly failing FFFF bit-field case is restored with provider-address proof.
This consumes the V14 native memory subfamily; no I/O, full prefetch/timing,
mode, full V14 or complete CPU claim follows from it.

S9 consumes the native I/O transfer subfamily through existing scalar and
string port owners. [Port-transfer evidence](evidence/t2-s9-port-transfer.md)
records documented byte/word lanes, sixteen-bit address model boundaries,
all twelve forms, REP/DF/index/segment contexts, split/memory/provider failure
positions and actual NMI/INT-to-IRET continuation. Device completion occurs
once per accepted cycle; no failed instruction replays an accepted prefix.
Mode/8080, electrical timing and unqualified prefix behavior retain their
original receivers; this batch does not retire all V09/V14 or T2.

S10 qualifies the existing native MOVS/LODS/STOS byte/word mechanism without
production changes. [Data-block evidence](evidence/t2-s10-block-transfer.md)
records independent full-memory/CPU expectations, actual provider addresses,
REP/DF/segment/count/tail and overlap contexts, every applicable failure
position and native interrupt/IRET continuation. The test does not copy a
production interpreter or weaken an existing assertion. The complete ledger
still retains mode/8080, timing, native full-form and real-board requirements;
S8's segment-tail L2 model is not promoted by these synthetic tests.

S11 repairs native Group-2 count ownership rather than copying an ALU.
[Shift/rotate evidence](evidence/t2-s11-shift-rotate.md) records full CL/imm8
counts 0--255, defined flags versus masked undefined values, register aliases,
memory/segment/odd-tail and complete provider failure positions. Intel count
behavior and existing handlers remain unchanged. V30 timing still exits as
unallocated before Intel timing-count rules; this does not retire all V07,
mode/8080, Machine or board requirements.

S12 reuses existing format/minimum predicates at actual native primary dispatch,
removing the placeholder-ModRM admission gap without handler-specific guards.
[Form-admission evidence](evidence/t2-s12-form-admission.md) records every
ModRM for nineteen constrained opcode families with five segment selections,
actual rejected-form effect boundaries, valid witnesses and code-fetch failure/
wrap contexts. Existing NEC admission and Intel dispatch order remain unchanged.
The baseline reserved-memory read is repaired; complete legal native semantics,
all slot dispositions, mode/8080, electrical timing and Machine/board proof
remain explicit receiving obligations. This does not retire all V03 or T2.

S12's source sweep found a V04/V14 fetch receiver: NEC suffix-at-FFFF lookahead
passed unmasked `eip + 1` to native logical translation. S13 now reproduces
that failure through actual INS execution and normalizes the starting offset
at the sole code-read owner, retaining S8's existing span/provenance/lane
mechanisms and data-tail model. [Fetch evidence](evidence/t2-s13-code-fetch.md)
records 38 primary/NEC streams, all short segment selections, odd/tail starts,
three code bases, pure preview, retained-cache continuation and every relevant
fetch/observation refusal. The caller-specific primary clamp is removed;
no new cache, reader, interface or persistent state exists. This consumes
the logical native fetch-start/span subfamily, not all V04/V14, physical
prefetch timing, 8080/mode, complete native semantics or Machine/board proof.

S14 consumes the native GP-stack/register-set/frame subfamily, not all V06.
[Stack evidence](evidence/t2-s14-stack.md) records primary NEC seven-transfer
POPR behavior, retained source-graded SP aliases and data-tail models, complete
PREPARE lexical levels, independent full CPU/memory and physical transfer
expectations, and accepted-prefix failure effects. The existing memory fixture
is extracted for two actual CPU test consumers, not duplicated or turned into
a production framework. Native segment-transfer inhibition remains a complete
V06/V11 receiver; PSW/control frames, mode/8080, timing and Machine/board proof
remain open.

S15 consumes S14's segment-transfer/inhibition receiver at the three existing
instruction owners, not the generic segment-cache loader. [Segment evidence](evidence/t2-s15-segment.md)
records both MOV directions, segment PUSH/POP, LDS/LES, independent complete
CPU/storage and transfer/failure expectations, NMI/INT/step boundaries, exact
word-wrapped frames, IRET, chains, IF/STI and reset. Identical MOV load decode
branches are removed. Native store-BRK behavior reuses the existing shadow as
an explicit L2 model; general BRK/entry/mode/PSW qualifications remain V10/V11.
The full original domain and all other native/timing/Machine/board obligations
remain unchanged.

S16 qualifies the ordinary native MOV/XCH/LEA/XLAT result and data-order
subfamily without production changes. [Transfer evidence](evidence/t2-s16-transfer.md)
records 55,200 explicit input vectors, pure previews, all memory ModRM and
short segment choices, byte/word alias preservation, EA wrap, immediate and
lookup values, and every applicable data-provider failure. Actual callbacks
prove read-before-write and accepted-prefix effects, not physical bus-lock
exclusion or clocks. `_m_read_rmw`'s existing lock-signal admission is still
486-only; native BUSLOCK/arbitration/prefetch remains the complete V05/V14/V15
receiver, not an implicitly certified property of a successful XCH value.
All original native/entry/mode/time/Machine/board requirements remain.

S17 consumes the native scalar ALU/sign-extension subfamily through unchanged
arithmetic/operand owners. [ALU evidence](evidence/t2-s17-alu.md) records
917,460 success vectors plus every relevant provider failure, complete CPU and
defined flags, all ordinary ModRM/segment/width/immediate forms and aliases.
Exhaustive byte pairs and sign-extension domains complement independent word
boundary inputs, not a copied production flag implementation. The CPU-local
fixture can restart its CPU/probe state while retaining explicitly initialized
RAM; original prepare callers still clear all storage. Production and all ten
receiving EXEs are unchanged. Remaining decimal/multiply/divide, prefix/mode,
physical clock/entry, Machine and board/game members are not retired by this
result batch. The separately accepted T564 RGB import resolves only the old
host palette-capacity TODO, not actual board-colour publication.

S18's [unpacked decimal evidence](evidence/t2-s18-decimal.md)
qualifies ADJBA/ADJBS and documented radix-ten CVTBD/CVTDB through unchanged
handlers: 817,152 success inputs plus every applicable fetch/preview refusal,
complete defined CPU results and exact absence of data transfers. The primary
ADJ4A page prints a contradictory low-nibble comparison and marks V affected;
ADJ4S's arbitrary-input CY/AC ordering is also unresolved. Those packed forms
remain at V07, not silently equated with Intel or assigned an invented model.
Generalized conversion radices, full prefix/mode and physical timing remain
their original receivers; no instruction count or matrix retires them.

Remaining implementation estimates are provisional until each S freezes
its actual forms. S19's [multiply/divide evidence](evidence/t2-s19-multiply-divide.md)
records the complete native arithmetic form batch and bounded vector-0
receiver: documented -127/-32767 quotient bounds and following-PC software
interrupt return are repaired at existing owners. Independent wide arithmetic,
complete CPU/defined flags, exact memory/phase and accepted-prefix failures,
and actual IRET cover all frozen forms. Undefined overflow results retain the
old rollback model; full entry/step/mode, prefixes/LOCK, physical clocks and
neutral Machine/board/game remain original receivers.

Further implementation estimates are provisional until each S freezes
its actual forms: native family/NEC forms about +900--2200 CPU source lines;
8080/mode-entry about +600--1400; timing/Machine about +400--1000; ROM-free CPU
and Machine vectors about +700--1600 across those batches. Existing verified
mechanisms are reused, not reproduced to meet a guessed line count. No Lib,
Emulator or X86 IBMPC source change is justified by the native batch. The narrow
approved X86 NXVM default branch is the sole consumer compatibility edit. These
estimates are not approval for new public functions, layouts or callbacks.

S1 rechecked source admission/reset/form/preview/entry/time seams and rendered
the NEC repeat-prefix and emulation-return pages. Original text clocks assume
prefetched instruction bytes; adding bus/prefetch terms requires an explicit
source/L2 accounting model, not charging the same fetch twice. REPC/REPNC
must inspect comparison results rather than a copied Intel ZF predicate.
Native return from emulation restores the architectural MD from the saved
frame; an App-owned parallel mode flag would be a second owner.
