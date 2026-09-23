# T359 S5: 80386 Secondary-Integer Source Ledger

## Authority and boundary

This is the S5 source ledger for the non-privileged `0F` integer forms assigned
by the active T359 S5 packet.  Numeric rows are from Intel's *80386
Programmer's Reference Manual*, section 17.2.2.3 and the individual
instruction pages.  They are core clocks under the manual's prefetched,
no-wait, no-HOLD assumptions; bus service, cache, prefetch, exception
delivery, privileged `0F` forms, and undefined-FLAGS timing remain transfers.

The sole publisher is `core_machine_instruction_cost()` after successful
instruction refresh.  `core_machine_80386_secondary_source_instruction_cost()`
is an encoding/outcome classifier; it does not add a handler-local clock.

## Allocated successful rows

| Forms | Intel 80386 PRM row | S5 selection |
| --- | --- | --- |
| `0F 80`--`8F` near `Jcc` | true `7+m`, false `3` | Retains the existing source-backed lexeme component term for `m`; both 16- and 32-bit displacement encodings use their actual operand-size outcome. |
| `0F 90`--`9F` `SETcc` | `4/5` register/memory | Retained S2 primary-shape owner; S5 verifies this is the identical successful-retirement mechanism. |
| `0F A3` `BT r/m,r` | `3/12` register/memory | Allocated. |
| `0F BA /4 ib` `BT r/m,imm8` | `3/6` register/memory | Allocated. |
| `0F AB/B3/BB` `BTS/BTR/BTC r/m,r` | `6/13` register/memory | Allocated. |
| `0F BA /5`--`/7 ib` `BTS/BTR/BTC r/m,imm8` | `6/8` register/memory | Allocated. |
| `0F A4/A5/AC/AD` `SHLD/SHRD` | `3/7` register/memory | Allocated for immediate and `CL` count forms. |
| `0F AF` two-operand `IMUL` | `9`--`22` / `12`--`25` (word); `9`--`38` / `12`--`41` (dword) | Reuses the S2 PRM early-out formula and decoder-captured r/m optimizing multiplier; memory adds three clocks. |
| `0F BC/BD` `BSF/BSR` | `10+3n` | Allocated from the captured source operand. `n` is the number of scanned zero bits; all-zero word/dword inputs select 16/32 respectively, yielding 58/106. |
| `0F B6/B7/BE/BF` `MOVZX/MOVSX` | `3/6` register/memory | Allocated. |

The owner accepts legal 80386 operand-size (`66`), address-size (`67`), and
segment lexical variants because the cited rows distinguish only the actual
register/memory outcome.  `F0` is selected only when the existing shared LOCK
owner accepts a memory modifying bit form (`BTS/BTR/BTC`); rejected forms do
not reach successful retirement and receive zero instruction clocks.

## Source and mode dispositions

- 8086's historical `0F` POP-CS byte route and 80186/80286 `0F` rejection are
  not secondary-integer successful forms.  They remain decoder dispositions,
  not fabricated timing rows.
- Real, protected, and ordinary VM86 executions of the admitted non-privileged
  forms share the same PRM core row only after successful refresh.  Segment,
  page, permission, and synchronous fault paths publish no successful
  instruction timing.
- `0F A0/A1/A8/A9`, `0F B2/B4/B5`, `0F 00`--`0F 26`, and all table/control,
  debug, task, selector, and delivery forms remain S6.  Physical service,
  waits, prefetch/cache, HOLD/DMA, and source contradictions remain T360 or
  later physical receivers.

## Mechanism and regression record

The S5 smoke exercises near branch truth outcomes, all fixed-row families,
register/memory distinctions, immediate bit extensions, standalone `66` and
`67`, an FS memory-source override, legal LOCK, dynamic IMUL, scan extremes,
preflight maximum, and rejected-LOCK zero publication. Existing semantic owner
smokes retain exhaustive condition, prefix, segment, fault, and mode behavior;
S5 proves their timing row has one shared successful-retirement publisher.

The maximum instruction preflight is raised from 46 to 106 clocks because the
new source-backed 32-bit all-zero `BSR` row is the maximum selected successful
instruction.  This changes no instruction semantics or physical timing model.
The retained generic, S2, and protected-I/O timing smokes now use the global
105/106 preflight boundary while retaining their own 3-, 27-, and 46-clock
instruction-result assertions.  That distinction prevents a stale former
maximum from masquerading as a per-instruction timing claim.
