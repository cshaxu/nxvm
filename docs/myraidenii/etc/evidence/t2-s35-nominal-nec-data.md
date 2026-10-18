# T2 S35 NEC Extension Data Nominal Clocks

Baseline `016fa8f84`, after accepted S34. This consumes the complete ordinary
NEC extension data nominal-clock batch at original V04/V15, not complete
V30 physical timing, Machine admission, board or game qualification.

## Primary Source And Whole-Batch Disposition

NEC October 1986 V20/V30 User's Manual, SHA-256
`FABBFCD19D4A2FCDAAC9F0BC06B00BF0A6A4BEEDAFBD3FF16ACCD075A374EA4C`.
[Original source](https://www.dosdays.co.uk/media/nec/NEC-V20-V30-Users-Manual.pdf).
Printed 12-21..24, 12-42..46 and 12-81..98 (PDF 66..69, 87..91,
126..143), all 27 pages, were rendered and visually inspected. The source
assumes prefetched instruction bytes and zero waits. No manual payload,
external implementation, ROM or media is imported.

| Ordinary NEC data member | Source-known nominal clocks / retained unknown |
| --- | --- |
| TEST1/CLR1/SET1/NOT1 CL, register | 3 / 5 / 4 / 4 |
| Same CL forms, byte or even-word memory | 12 / 14 / 13 / 18 |
| Immediate-index variants | Corresponding CL row plus one |
| Odd-word memory | TEST1 adds four; CLR1/SET1/NOT1 add eight |
| ROL4 register / memory | 25 / 28 |
| ROR4 register / memory | 29 / 33 |
| ADD4S/SUB4S/CMP4S, original even CL | 7+19*(CL/2) |
| Same BCD forms, odd CL | Source defines n as half the digit count without rounding; zero/unattributed, no invented ceiling |
| INS register, even / odd | Source ranges 31..117 / 35..113, no exact input-to-clock selection; zero/unattributed |
| INS immediate, even / odd | Source ranges 67..87 / 75..103; zero/unattributed |
| EXT register, even / odd | Source ranges 26..55 / 34..59; zero/unattributed |
| EXT immediate, even / odd | Source ranges 21..44 / 25..52; zero/unattributed |

All sixteen bit suffixes, two nibble suffixes, three BCD suffixes and four field
suffixes receive a disposition. Prefix clocks, field ranges, odd BCD rounding,
entry/mode/electrical/8080 and full elapsed-time accounting remain the original
ledger receivers, not transferred away to declare the CPU complete. Existing
BCD odd-tail and invalid-input semantics are retained, not newly certified by
the even-count clock formula.

## Ownership And Actual Path

Only the existing private V30 timing selector changes. It consumes existing
suffix bytes, decoded memory alignment and original CL, then uses the existing
publication boundary. The opcode recorder already wraps native 16-bit fetch;
no decoder repair is necessary. Existing primary form keys remain unchanged;
NEC keys add the suffix in the high sixteen bits of the existing lib_u32
opaque key and preserve low 0F and the memory bit. No new public declaration,
enum, layout, helper, persistent state, CPU executor, operand/target read,
Intel clock fallback, Lib or Emulator change is introduced. Nominal rows
include their documented operand work; callback counts are not a timing slope.

V30 remains source-time-unallocated, maximum zero and rejected by neutral
Machine admission. An exact nominal subtotal is not complete elapsed time.

## Proof And Initial Test Correction

The original bit/nibble test retains its complete CPU/defined-flag, RAM,
instruction-size, truncation, prefix, invalid-form and bus-failure assertions.
Adding even-memory parity extends bit cases from 4,704 to 6,720, including
register aliases, CL/immediate width and segment-override unknowns. All 4,608
nibble inputs gain source-clock/key checks, totaling 11,328 bit/nibble success
inputs. One test-local result helper shares the identical metadata assertions
at those two real execution seams; no test framework or production helper.
The even-memory write expectation correctly counts one word callback, while
the original odd word still counts two; this does not change CPU behavior.

The existing BCD execution seam now checks clocks, keys, inputs, no extra
observations and unknown prefix/rounding. Its 31,608 successful seam inputs
are 30,000 valid operand pairs, 1,524 count/direction cases, 75 prefix/count
cases and nine wrap/overlap cases. The six-instruction original example and
all 24 provider-failure attempts plus nine invalid/nondigit cases remain.
Bit-field tests retain all semantics and directly prove that every source
range stays unknown. Failure paths cannot publish completed nominal timing.

The original 38-form/five-segment/three-base/nine-start wrapped-fetch matrix
adds 5,130 timing comparisons against the same actual instruction at the
nonwrapped reference address. This relocation contrast is not an independent
clock oracle; the bit/nibble and BCD seams independently supply source rows.
Existing fetch/cache/provider failures retain all original state assertions
and add absence of failed timing publication. Observation counters reset
after existing preview; retirement adds no observational reads.

Both ALU and Transfer previously used TEST1 as an unknown-clock placeholder.
It is now legitimately allocated. The first replacement used register INS
0F31C0: the transfer fixture's AL=78h is outside the existing field-register
four-bit domain. The first x64 focused group therefore passed 8/9 and stopped
at `V30:NOMINAL-TRANSFER:EXCLUDED item=6 stop=1 faults=1`, cases=24130.
No full suite ran on that failed candidate. Both timing-only placeholders
now use valid immediate INS 0F39C100, original CL=0 and length=1. Field clocks
remain source-unknown. No production admission, semantics, assertion or
budget is weakened. The original failed log is retained as
`build/myraidenii/t2-s35-initial-focused-x64.log`. Final focused groups pass.
All 917,460 ALU and 24,160 Transfer cases remain.

## Similar-Issue And Code-Quality Review

Reviewed all ordinary NEC suffix/form/width/count classes, even/odd decoded
EA, actual wrapped opcode recording, prefix/mode admission, source-form keys,
failure publication, all timing-only exclusions, preview/max consumers and
the neutral Machine gate. No key collides with existing native primary/group
keys: high-word suffix is zero only for primary forms, and each allocated NEC
suffix is nonzero. No fresh operand read or parallel time owner is needed.
BRKEM and software/asynchronous entry retain a distinct entry owner and are
not misclassified as ordinary data. Existing Intel selectors and physical
time mechanisms remain unchanged.

Reproducible sweep queries include:

- `rg -n '0x0fu|suffix|odd_penalty|oldcpu.data.cl|form_id' src/x86-core/cpu/cpu_timing.c`
- `rg -n 'source_timing_unallocated|observations|completed_observation_valid|0x39u' test/x86-core/cpu/cpu_v30*smoke.c`
- `rg -n '_s_read_cs|instruction_offset|opcodes' src/x86-core/cpu/cpu_instructions.c`
- `rg -n 'V30|timing_maximum_ticks' src/x86-core/machine src/x86-core/cpu`

The sole production selector and original execution/data owners are retained.
Only a bounded test-local metadata helper is new. Six existing units are
extended; no new source/test registration, fixture state or framework.
Existing warning exceptions are unchanged, not presented as an exception-free
strict build. Types, dependency, manifest and specialized gates are required.

## Counted Delta And Qualification

Against baseline, `git diff --numstat` over the seven changed C files:
production +24/-3/net +21; tests +78/-14/net +64; total code
+102/-17/net +85. Manifests separately +7/-7/net zero. Production estimate
+35..65/-5..12 and tests +105..165/-20..45 are reduced by extending existing
selection/proof seams rather than adding registration or new infrastructure.

Shared source/test delivery is `1945350e2`; NXVM receiving publication is
`89a7c466e`. Both complete target-separated deliveries are pushed.

Both original cached Ninja all-target builds pass. Both nine-unit focused
groups and final full repository units pass 678/678 per width, including
191 X86 Core tests, zero failures/skips. Full units take 316.38 seconds x64
and 129.40 seconds x86. Specialized gates pass 498 retained strict rows and
the global Types audit covers 1,388 inputs. Historical warning exceptions
remain unchanged. All original ALU, transfer, flag, shift, decimal, stack,
port, control, completion and interrupt contrasts remain in the full suite.

Eight current PC 0564 EXEs are rebuilt/deployed. Fourteen selected receiving
integrations plus restored-default identity pass per width. DeskPro groups
take 121.22/159.97 seconds. Both original graphs are default; profile switches
and restores recompile zero CPU objects. All ten deployed PE/hash identities
pass; MyNES 0044, owner INIs, firmware/media and snapshots are unchanged.
The qualification runner is terminal with exit zero. [Measured results](t2-s35-results.json)
and [artifact identities](t2-s35-artifacts.json) retain proof. Coordinator
actual committed-diff acceptance remains before closure. Complete external
integration, manual desktop/RDP/audio, physical timing and gameplay are not
inferred from these unit and selected receiving results.

## Complete Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation, MyRaidenII M1 T2 S35, next unused S after accepted S34. |
| Admission And Approval | Owner authorizes continued automatic governed single-person coordinator/executor MyRaidenII implementation. Coordinator admits original V04/V15 ordinary NEC data nominal clocks on 2026-10-18 under existing V30 approval, targets Shared CPU/tests, NXVM eight affected receiving releases/status and MyRaidenII evidence. No Lib/Emulator/MyNES source or new public contract. |
| Objective | Consume all sixteen TEST1/CLR1/SET1/NOT1 CL/immediate byte/word suffixes, ROL4/ROR4, three packed-BCD string suffixes and four bit-field suffixes as one complete ordinary NEC data source batch: allocate documented nominal rows/formulas, keep source-underdetermined members zero/unattributed with direct proof. |
| Non-goals | No instruction semantics, decoder/executor edit, new model/helper/framework/API/enum/layout/persistent state, extra operand/target read, Intel fallback, BRKEM/software/async entry, prefix/8080 or complete physical timing/Machine/board/game admission. Unknown ranges and rounding remain original V04/V15 work, not completed CPU claims. |
| Reference Baseline | Clean pushed 016fa8f84. Pinned NEC October 1986 manual SHA FABBFCD19D4A2FCDAAC9F0BC06B00BF0A6A4BEEDAFBD3FF16ACCD075A374EA4C: printed 12-21..24,42..46,81..98; PDF 66..69,87..91,126..143 rendered and visually inspected. S4/S5/S6 result/failure proof, S13 wrapped fetch/observation owner and S30..34 nominal publication. |
| Candidate Proposal | proposals/m1-v30.md and original V04/V14/V15 ledger. Existing selector owns suffix dispatch and publication; opaque form keys preserve existing primary keys and put NEC suffix in the high word. TEST/CLR/SET/NOT CL register clocks 3/5/4/4, memory byte/even word 12/14/13/18, immediate adds one, odd word adds four for TEST and eight otherwise. ROL4 register/memory 25/28, ROR4 29/33. Even CL BCD 7+19*(CL/2); odd CL rounding and all four field ranges remain zero/unattributed. |
| Files And ABI Surface | src/x86-core/cpu/cpu_timing.c and manifest; existing cpu_v30_bit_nibble, cpu_v30_packed_bcd, cpu_v30_bit_field, cpu_v30_memory_transfer, cpu_v30_alu and cpu_v30_nominal_transfer smoke C files plus test manifest. Estimate production +35..65/-5..12, tests +105..165/-20..45. No registration/public declaration. NXVM artifacts/status and MyRaidenII records in separate target Ps. |
| Applicable Rules | MyRaidenII guide/goal/proposal/ledger/design/roadmap/source policy, shared Execution/Architecture/Coding/Documentation and architecture/coding governance skills, read-only PDF review. Receiving NXVM guide/current/architecture/coding/source policy and X86 Core README. Lib Types, single decode/data/time/publication owner, frozen qualification and actual committed-diff review. |
| Verification | Retain every original bit/nibble/BCD/field semantic, prefix, invalid-form and bus-failure case; add even memory parity, source clocks/opaque keys/formula inputs, no extra observations, range/rounding non-eligibility and no failed completion. Preserve all 917460 ALU and 24160 transfer cases, replacing newly allocated bit placeholders with legal range-unknown bit-field forms; validate wrapped timing against existing real execution references. Nine transient affected/contrast units, then both original cached Ninja all-target builds -j8, full repository units -j4, specialized gates/498 strict rows, fourteen PC receiving integrations plus restored-default identity per width, ten PE/hashes and unchanged owner inputs. |
| Expected Markers | Existing V30:NEC-BIT-NIBBLE:OK, V30:PACKED-BCD:OK, V30:BIT-FIELD:OK and V30:MEMORY-TRANSFER:OK remain. Add V30:NOMINAL-NEC-DATA:OK and V30:NOMINAL-BCD:OK; original ALU/transfer counts remain. Both full suites zero failures/skips; V30 total-time unallocated, maximum zero and Machine rejection retained. |
| Asset Needs | Synthetic ROM-free inputs and pinned manual only. Eight current PC 0564 EXEs rebuilt/deployed, MyNES 0044 unchanged; ten identities checked. No new ROM/media/INI/snapshot or MyRaidenII EXE; original graphs/cache retained. |
| Reporting Requirements | Report source decisions and estimated per-production/test delta before code; actual failures/progress; final added/deleted/net, retained paths, source-unknown and similar-issue dispositions, verification scope, artifacts/commits and remaining complete CPU/game boundary. |
| Stop Conditions | New public/source-model requirement, changed CPU/Intel semantics, owner-input mutation or failed qualification stops acceptance. No inferred range midpoint/rounding, replay of accepted effects, weakened assertion/budget or nominal-to-elapsed promotion. |
| Exit Criteria | Every admitted ordinary NEC data member has its documented allocation or explicit original unknown disposition, directly proved at the sole real path including parity/prefix/wrap/failure. Both widths complete builds/units/gates/selected receiving/artifact/docs pass; complete target-separated Ps pushed and actual committed-diff coordinator acceptance. Only S35 may close. |
| Original Owner Request | Governed single-person/two-role implementation of complete playable MyRaidenII, including full NEC V30 and required facilities. Partial CPU source clocks or units never replace board and game outcome. |
| Similar-Issue Sweep | Inspect all ordinary NEC suffix/form/width/count and memory-parity classes, decoded opcode recording across wrap, no extra reads, all timing-only exclusions and failure publication, opaque key collisions, preview/max/Machine consumers and existing Intel owners. Retain distinct entry/mode/prefix/electrical and source-range/rounding obligations at original ledger receivers. |
