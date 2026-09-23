# T357 S4: 8086 Instruction-Timing Ledger

## Decision

S4 gives the selected 8086 corpus an immutable form-cost ledger at the one
existing CPU-retirement publisher.  It deliberately does not claim an 8088
model, prefetch-queue state, bus waits, HOLD/DMA ownership, device latency, or
exact interrupt/fault delivery cycles.

The primary source is the *Intel 8086 Family User's Manual*, Tables 2-20 and
2-21 (manual pages 2-50--2-51; retained source URL in the T357 S1 contract).
The table values are used under its documented instruction-time assumptions;
later bus-timed and cycle-exact packages own the omitted physical conditions.

## Ledger and owner mapping

| Source row | 8086 clocks | Owner form/proof |
| --- | ---: | --- |
| `NOP`; `CLC` | 3; 2 | `90`; `F8` focused vectors |
| `MOV reg,imm`; `MOV reg,reg` | 4; 2 | `B8`; `8B C1` |
| `MOV r/m,reg`; `MOV reg,r/m` | 9+EA; 8+EA | direct and BP+DI/displacement vectors |
| accumulator/memory `MOV` | 10 | odd-address `A1`/`A3` vectors |
| `MOVSB`; `REP MOVSB` | 18; 9+17/repetition | single and three-iteration restart-owned vectors |
| short conditional branch | 16 taken; 4 not taken | `JZ`; `JNZ` vectors |
| `IN` immediate/DX; `OUT` immediate/DX | 10/8; 10/8 | installed-provider vectors |
| direct EA; BP+DI+disp EA | 6; 12 | direct and indexed vectors |
| segment override; odd word transfer | +2; +4/word | `ES`-override and odd word vectors |

`core_machine_8086_source_instruction_cost` owns selection only after the
existing successful instruction refresh.  The sole elapsed-tick publisher is
unchanged.  Unsupported 8086 forms return the visible one-tick source fallback
instead of consulting `instruction_timing`; the test configuration deliberately
sets that legacy structure to distinct non-source values.

## Boundary proof

`core_machine_8086_instruction_timing_ledger_smoke` proves the listed costs,
port callbacks, fallback, delivered `#UD` zero publication, explicit requested
stop zero publication, reset/replay, insufficient-budget preflight, overflow,
and provider/scheduler publication.  Its 27-clock maximum form is an
ES-overridden word `MOV r/m,reg` using BP+DI+disp and an odd physical address;
26 ticks refuses it without publication and 27 retires it.

## Similar-issue sweep and transfer

The S4 sweep covered CPU-profile timing dispatch, source-ledger lookup,
`maximum_instruction_ticks`, compatibility timing configuration, elapsed-tick
writers/readers, and timing smokes.  The existing generic timing and real-mode
tick smokes now select 80186 explicitly: they remain compatibility-policy regressions and
must not constrain source-backed 8086 rows.  The only source-ledger profiles
are now 8086 (S4) and the selected 80386 corpus (S3); 80186 and 80286 retain
their explicit compatibility policy until separate primary-source ledgers are
admitted.  All unallocated 8086 forms retain the one-tick transfer policy.

No external source material is imported into the repository.
