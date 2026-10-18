# T2 S24 Ordinary 8080AF Execution

Admitted from clean `35c6242ad`. This record is in progress, not acceptance.
The complete sixteen-field active packet is in Current. The T2 ledger's V12
ordinary table is this batch; physical bus retirement/time and board admission
remain separate receivers. No third-party implementation or protected payload
is imported.

## Primary Sources And Decisions

The NEC October 1986 V20/V30 user manual, SHA-256
`FABBFCD19D4A2FCDAAC9F0BC06B00BF0A6A4BEEDAFBD3FF16ACCD075A374EA4C`,
sections 8 and 9 defines 8080AF compatibility, register/flag mapping, DS data
including the independent BP stack, native BCU I/O, mode transitions and
standby. S23 already pins and qualifies the native frame/MD-write owner.

Additional original-manual copies were acquired on 2026-10-18 only into
ignored research:

- NEC 1979 Microcomputer Catalog, original printed p.183 (PDF 182) and AF
  instruction tables from printed p.193: [copy](https://bitsavers.trailing-edge.com/components/nec/_dataBooks/1979_NEC_Microcomputer_Catalog.pdf),
  SHA-256 `21600D61AE01FACC3D174A8A8E74668C619938E3EA2CE374473389274B81B1C5`.
  AF is explicitly fully compatible with the industry-standard 8080A. Do not
  substitute the distinct older NEC 8080A or undocumented 8085/Z80 behavior.
- Intel 8080 Assembly Language Programming Manual, 1975 Rev B:
  [copy](https://altairclone.com/downloads/manuals/8080%20Programmers%20Manual.pdf),
  SHA-256 `DCECFA12AC1955BE330C93E5F78BBF4E25E180769D45F3204075534CB8CD8CB2`.
  Chapter 2 defines ordinary register/data/ALU/flow/stack/port operations.
  Printed pp.15--16 define sequential DAA and actual low-nibble carry; pp.18--19
  explicitly show subtraction AC as carry, not x86 auxiliary borrow; pp.22--25
  define the five PSW bits, high/low push and low/high pop, XTHL and SPHL.
- Intel 8080/8085 Assembly Language Programming Manual, May 1981,
  9800301-04: [copy](https://archive.decromancer.ca/bitsavers.org/pdf/intel/ISIS_II/9800301-04_8080_8085_Assembly_Language_Programming_Manual_May81.pdf),
  SHA-256 `C2F17D613159B624CF7226B0B9200192A49FF172147D88F56D50F45E94691643`.
  Printed p.1-12 (PDF 22) clarifies 8080 AND AC as OR of input bit 3, unlike
  8085's always-one AC. This later primary clarification supersedes Rev B's
  stale logical-AND flag prose. RIM/SIM in its combined table are 8085-only.

Rendered primary DAA, AND clarification, AF identity/full opcode/cycle tables
and V30 standby pages were inspected;
relevant extracted instructions were checked against the existing native ALU,
logical bus, port, mode and standby owners before implementation.

Rev B printed p.23 contains a POP PSW prose typo assigning the flags byte
to SP+1. Its adjacent worked example explicitly loads flags C3 from SP=2C00
and A=FF from SP+1=2C01, consistently with PUSH PSW's layout on that page.
The implementation/test use that defined layout, not the contradictory sentence.
The complete 65,536-word POP PSW matrix checks A, fixed/reserved flag bits and
unchanged upper PSW fields. No silicon behavior is inferred from a typo.

## Resulting Ownership

One private opcode-form classifier/length function serves CPU execution and
context-aware preview. The old control-only owner is replaced, not retained
as a second interpreter. The raw public profile/byte scanner remains native,
because it has no mode context. Emulation architectural state remains in the
existing PSW, GPRs and segments; there is no new stored mode, register file,
queue, host loop, clock or public interface.

The existing native arithmetic routines own result/SZP/carry. Small mapped
operations preserve the other PSW fields and implement the documented 8080
AC differences. The two data bytes use the existing logical-memory provider;
the 8080 logical address wraps per byte. Stack byte order is source-backed;
the precise V30 electrical microsequence/prefetch overlap is not qualified
by these semantic tests and remains V14/V15. Port addresses are the immediate
eight-bit value on the native V30 BCU, not discrete-8080 pin mirroring.

Reserved ordinary slots are 08/10/18/20/28/30/38/CB/D9/DD/FD. ED is the NEC
escape already qualified in S23. Thus 244 ordinary documented slots plus
one constrained NEC escape are classified, with no undocumented NOP aliases.
The catalogue's 8080AF clock counts are not borrowed as V30 timing. Native
BCU electrical sequencing, queue overlap and source-graded clocks remain
V14/V15; the unchanged Machine admission continues to reject this profile.

## Verification And Actual Change

The independent test table transcribes all 256 slots; a separate integer/
boolean oracle checks complete CPU state and typed real-memory transfer plans.
It does not call the production format or arithmetic helpers. Finite matrices
cover all flag combinations, byte-pair/carry ALU operands, DAA/rotate/unary
flags, wrapped code/data/stack, code/data and vector/data aliases, preview
purity, and refusal of each observed fetch/read/write/port transfer. Actual
entry/return and interrupt/standby routes will be recorded after execution.

The final ordinary matrix passes 4,328,948 success inputs per width, plus
the deep preview/accepted-prefix/provider-refusal routes. It includes all
256 slot dispositions; 8 ALUs through register/M/immediate for every byte
pair/carry; all 32 mapped flag combinations; unary/DAA/rotate flags; all
65,536 pair inputs and POP PSW words; every immediate port; wrap/physical
code-data and vector-data aliases. Actual BRKEM -> ordinary program -> CALLN
-> native RETI -> RETEM preserves the independent stacks and upper registers.
EI's following instruction, DI, native/emulation HLT, NMI/INT enabled entry,
disabled-INT wake without acknowledgement/mode change and RETI resume pass.

Final original x64/x86 Release Ninja/ccache all-target builds and complete
repository unit suites pass 674/674 per width, including 187 X86 Core units,
in 306.73/136.84 seconds. Ordinary matrices take 7.09176/8.18136 seconds;
existing S23 328,192-input mode matrices take 2.03784/1.17283 seconds. Both
specialized targets pass with 498 strict rows. Types, dependency, ownership,
negative, manifest and whitespace gates pass. Old CPU warning exemptions are
unchanged; this is not a claim of exemption-free strict compilation.

Eight current PC 0564 EXEs are rebuilt/deployed. Each width passes 14 selected
four-profile receiving integrations plus one restored-default identity check.
DeskPro groups finish in 122.81/175.90 seconds within existing budgets. Both
original graphs are restored to default; receiving loops recompile zero CPU
objects. All ten PE/width/name/size/hash identities pass; MyNES 0044 and INIs/
media/ROMs remain unchanged. [Results](t2-s24-results.json) and
[ten identities](t2-s24-artifacts.json) retain exact proof. Production is
frozen at CPU implementation SHA-256
`878F9649308C189A6BA11AE46A6C8FC22F9BE21BF6D240A6D2442118F1474F59`;
the context header and CPU lifecycle C file retain the S23 hashes. Changed
Shared source/test/manifest raw working bytes equal the staged blobs and use
LF, so the committed manifests describe the actual canonical bytes.

No fresh complete external/native integration suite, full V30 timing,
runnable MyRaidenII, game, manual desktop or RDP qualification is claimed.
No Lib/Emulator, App production, public API/enum/layout, new stored CPU state,
worker, clock, scheduler or interpreter loop is added.

Self-review found a lexical-component omission before final acceptance: a
three-byte ordinary instruction has one opcode and one word operand, not
three components. Context preview now reports two; the NEC two-opcode escape
retains its existing two/three counts. The independent unit checks both byte
and component counts. This does not change execution, but it does complete
the existing lexical contract. The initial x64 complete 674-unit run passed
in 302.25 seconds; its old source is not final qualification. The partially
running x86 suite was deliberately stopped at its verified owned process
tree for this source correction, not due to a timeout. Initial logs remain
under the ignored `t2-s24-initial-*` names. Fresh final builds and full suites
were completed for both widths, using the same original graphs and budgets.

Counted code is the CPU C implementation, test registration and three directly
changed/new CPU tests, measured by `git diff --numstat 35c6242ad -- <paths>`.
Production +394/-23/net +371 and tests/registration +653/-4/net +649 give
code +1,047/-27/net +1,020. Manifests separately add 5/remove 4/net +1;
records and artifacts are excluded. Both totals are below the admitted
estimates because the existing ALU/bus/entry/standby and memory-plan fixture
are reused. The positive increase implements a whole previously absent
documented execution table, not a duplicate path. Nine new private helpers
own opcode-form selection, mapped byte/pair references, M operand access,
wrapped data words, BP stack order, mapped flag commit and condition tests.
The two old private control length/execution functions are replaced in place.

## Similar-Issue Sweep And Review

Queries over `src/x86-core` and `test/x86-core` for
`NEC_EMULATION_CONTROL|emulation_control_length` return no obsolete owner.
Execution and context preview share the single mode format owner; the raw
mode-less public scanner deliberately remains native. Native arithmetic,
register references, logical reads/writes, `_p_input/_p_output`, HLT/CLI/STI,
real entry/return and ExecFinal retain their existing ownership. No direct
RAM/port shortcut, native instruction fallback or test-only production entry
is introduced. `_s_read_ds/_s_write_ds` legacy stubs are not used.

The native-vs-8080 differences are explicit: DS:BP layout and 16-bit byte
wrapping, accumulator/register mapping, auxiliary carry, mapped/fixed PSW
bits and absolute 8080 targets. Native Intel behavior is not generalized into
NEC behavior. Only the existing standby wake selector receives the sourced
V30 exception for disabled INT; vector delivery still requires IF. The actual
mode program and all four standby source/enable combinations per mode prove
the existing shared entry/return path rather than independent flag mutation.
The standalone table's ordinary initial states are synthetic CPU-owner vectors;
they are not claimed to replace that actual mode-route proof.

The test guards are narrowed only for the newly implemented documented forms:
all eleven ordinary reserved slots and all 254 prohibited ED suffixes still
fail diagnostically, with no native exception or data side effect. Other
Intel/native units, field ownership, build qualifiers and old budgets remain.
No source outside the one CPU C file changes. The new unit uses its own
CPU-local support, not an App or outer test fixture.

Shared implementation P1 `ceab5712c` and NXVM receiving P2 `1153273ac` are
pushed. The MyRaidenII evidence P preserves the complete sixteen-field active
packet and final results/identities for coordinator actual-change review.
Executor self-review finds no remaining admitted-batch defect; this does not
close T2 or promote its original physical timing/Machine/board/game receivers.
