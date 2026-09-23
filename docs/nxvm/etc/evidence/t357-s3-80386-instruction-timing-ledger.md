# T357 S3: 80386 Instruction Timing Ledger

## Decision

T357 S3 replaces the 80386 branch of the former configuration-driven
prefix/memory/I-O surcharge mechanism with one immutable, machine-private
ledger at successful CPU retirement.  The change measures selected 80386 core
clock rows only.  It does not claim a PC/AT oscillator, device service time,
wait state, arbitration, prefetch, cache, fault-delivery, or host-time cost.

The retained `core_machine_instruction_timing` configuration remains the
compatibility cadence for pre-80386 profiles and the explicit one-tick
unallocated-80386 disposition.  It is not consulted for a ledger-selected
80386 form.  Thus it cannot be a second selected-form timing truth.

## Source Ledger

The authority is the Intel 80386 Programmer's Reference Manual, chapter 17.
All values below use its documented prefetched/decoded, no-wait, no-HOLD,
aligned-memory assumptions from the T357 S1 contract.

| Selected form | 80386 clocks | Source row | Owner classification |
| --- | ---: | --- | --- |
| `90` `NOP` | 3 | [NOP](https://www.ardent-tool.com/CPU/docs/Intel/386/manuals/prref386/NOP.htm), row `90` | `CORE_MACHINE_SOURCE_TIMING_NOP` |
| `F8` `CLC` | 2 | [CLC](https://www.ardent-tool.com/CPU/docs/Intel/386/manuals/prref386/CLC.htm), row `F8` | `CORE_MACHINE_SOURCE_TIMING_CLC` |
| `B0`--`BF` immediate-register `MOV` | 2 | [MOV](https://www.ardent-tool.com/CPU/docs/Intel/386/manuals/prref386/MOV.htm), `B0+rb` / `B8+rw/rd` rows | `MOV_IMMEDIATE` |
| `88` / `89` `MOV r/m,r` | 2 register or memory | MOV `88` / `89` rows `2/2` | `MOV_RM_REGISTER` or `MOV_REGISTER_REGISTER` |
| `8A` / `8B` `MOV r,r/m` | 2 register, 4 memory | MOV `8A` / `8B` rows `2/4` | `MOV_REGISTER_REGISTER` or `MOV_REGISTER_RM` |
| `A0` / `A1` moffs read | 4 | MOV `A0` / `A1` rows | `MOV_MOFFS_READ` |
| `A2` / `A3` moffs write | 2 | MOV `A2` / `A3` rows | `MOV_MOFFS_WRITE` |
| unprefixed `A4` `MOVSB` | 7 | [MOVS](https://www.ardent-tool.com/CPU/docs/Intel/386/manuals/prref386/MOVS.htm), `A4` row | `MOVSB` |
| `F3 A4` `REP MOVSB` | `5 + 4 * (E)CX` | [REP](https://www.ardent-tool.com/CPU/docs/Intel/386/manuals/prref386/REP.htm), `REP MOVS` row | first primitive `9`, later primitive `4`, zero count `5` |
| short `70`--`7F` `Jcc`, false | 3 | [Jcc](https://www.ardent-tool.com/CPU/docs/Intel/386/manuals/prref386/Jcc.htm), short rows `7+m,3` | `JCC_NOT_TAKEN` |
| short `70`--`7F` `Jcc`, true | `7 + m` | Jcc short rows; `m` is S2 lexical next-instruction components | `JCC_TAKEN` plus S2 preview |
| `E4` / `E5` real-mode `IN imm8` | 12 | [IN](https://www.ardent-tool.com/CPU/docs/Intel/386/manuals/prref386/IN.htm), immediate rows | `IN_IMMEDIATE` |
| `EC` / `ED` real-mode `IN DX` | 13 | IN DX rows | `IN_DX` |
| `E6` / `E7` real-mode `OUT imm8` | 10 | [OUT](https://www.ardent-tool.com/CPU/docs/Intel/386/manuals/prref386/OUT.htm), immediate rows | `OUT_IMMEDIATE` |
| `EE` / `EF` real-mode `OUT DX` | 11 | OUT DX rows | `OUT_DX` |

`IN`/`OUT` protected and VM86 variants remain unallocated in S3: the same
manual has distinct `CPL <= IOPL` and I/O-permission timing rows, so their
successful timing is transferred rather than guessed.  Near `Jcc`, `JCXZ`,
other string forms, non-`F3` repeat forms, and every other CPU form are also
unallocated.

## Mechanism And Publication Boundary

`core_machine_instruction_cost()` is called after
`core_machine_cpu_execution_refresh()` and immediately before the existing
`executed`, `ticks`, `elapsed_ticks`, retire trace, and scheduler publication.
When a synchronous exception has been delivered, no executable instruction
lexeme remains (`oplen == 0`); the source owner explicitly yields zero ticks,
clears its private repeat state, and publishes neither elapsed time nor a
scheduler advance for that delivery.  This preserves the existing separate
exception-delivery model rather than assigning a guessed instruction row.
For `CORE_MACHINE_CPU_PROFILE_80386` it calls only
`core_machine_source_instruction_cost()`; the old additive compatibility
classifier is selected only for earlier profiles.  The source owner has a
bounded private repeat continuation key `(CS, EIP)`, reset with the machine,
so the `REP MOVSB` table formula is distributed across this executor's
one-primitive refreshes without adding a second scheduler.

The pre-run tick-budget guard uses the source owner maximum of 22 clocks: the
largest admitted short taken `Jcc` with a 15-component next instruction.  A
fault, stop, unavailable lexical preview, or overflow publishes no selected
form time.  Unallocated 80386 forms deterministically publish exactly one
compatibility tick; this is a visible transfer policy, never an Intel clock
claim.  The focused smoke proves a nonzero legacy surcharge configuration
cannot alter that fallback or any selected source row.

## Caller, Writer, And Variant Sweep

| Search / owner | Result |
| --- | --- |
| `rg -n "ticks_per_instruction|instruction_timing|maximum_instruction_ticks|elapsed_ticks" src tests CMakeLists.txt` | `machine.c` owns the sole elapsed-tick writer.  VM profiles and tests only configure/observe it; clock domains consume published ticks. |
| `core_machine_run()` fault, delivered-exception, reset, stop, budget, trace, scheduler paths | The source classifier is post-refresh and pre-publication.  Reset clears private repeat state; an empty post-delivery lexeme yields zero ticks, and all fault/stop/overflow paths retain zero selected-form publication. |
| S2 `core_machine_cpu_execution_preview_lexeme()` | Used only for taken selected short `Jcc` `m`; its nonpublishing fetch/translation contract is retained. |
| `REP MOVSB` refresh/restart route | One private continuation key covers first, later, and zero count.  Exact fault/interrupt cycle timing remains a cycle-exact transfer. |
| device clocks / T354 ordering | CPU retire trace precedes the unchanged deterministic scheduler.  The source ledger supplies only elapsed core ticks. |
| T344 direct-fixture inventory | The static inventory is reconciled from 53 to 56 exact direct-constructor files: the previously unlisted T357 S2 preview, this S3 ledger owner, and the retained RTC CMOS S3 owner.  They remain explicitly classified rather than bypassing the fixture-shape gate. |
| ATA PIO feature boundary | Its static check incorrectly demanded that a primary-master-only composition materialize profile-only `slave_present` and `secondary_channel_present`.  The repaired check verifies all three declarations in the profile, `lba28_supported` at the supported composition boundary, and the descriptor policy that explicitly rejects the two unsupported topology features.  No ATA runtime behavior changed. |

## Focused Proof

`core-machine-instruction-timing-ledger-smoke` proves the exact selected
values for baseline, register/memory/moffs `MOV`, real I/O, taken/not-taken
short `Jcc` with an S2-previewed NOP target (`m=1`), and three-primitive
`REP MOVSB` (`9+4+4`).  It also proves the one-tick unallocated policy,
invalid-`LOCK` fault nonpublication, budget preflight, overflow
nonpublication, reset/replay, and one post-retirement execution-provider time
advance per published cost.  `core-machine-instruction-timing-smoke` continues
to prove the retained pre-80386 compatibility classifier, and
`core-machine-cpu-timing-preview-smoke` continues to prove the lexical `m`
operation.  Existing cadence-oriented device smokes now explicitly select the
80286 compatibility profile; the VM86 delivery smoke remains 80386, isolates
unrelated clock domains, and proves a delivered paging exception does not
produce the synthetic `TIME` fault before its handler executes.

The S3 developer artifact is `build/output/nxvm_0_5_0357.exe`, SHA-256
`12921803AAC84B617326A832654BB5E27D4C2DF6E016B9D6FCF7278C651999CC`.

S4 must allocate exact source rows for 8086, 80186, and 80286 rather than
reusing these 80386 rows.  A later T357 S must allocate protected/VM86 I/O,
near/`JCXZ` branch, remaining string, and other CPU timing forms before T357
can close.  Bus/hold/wait/device timing remains with the queued bus-timed and
cycle-exact candidates.
