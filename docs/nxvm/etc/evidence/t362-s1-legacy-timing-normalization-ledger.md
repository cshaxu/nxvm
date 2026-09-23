# T362 S1: Legacy timing normalization ledger

## Source and accounting boundary

Intel's *iAPX 86/88, 186/188 User's Manual*, order 210912-001 (1985),
[Table 1-16](https://www.bitsavers.org/components/intel/8086/210912-001_iAPX_86_88_186_188_Users_Manual_1985.pdf), is the primary timing-domain
authority. Its p. 1-23 explanation identifies parenthesized values as 80186
clocks and separately describes the table's EA, segment-override, odd-word,
prefetch, and bus assumptions. The manual's 80186 description at p. 2-5 says
the non-immediate input to immediate `IMUL` can be a register or memory.

The selected reference is MAME revision
`21afc3b69c7aea273e4e16e6a52fa8e4fd207aa3`,
`src/devices/cpu/i86/i186.cpp` and `i86inline.h`, read only on 2026-08-14.
`i186.cpp` supplies separate `CLKM(IMUL_RRI*, IMUL_RMI*)` constants and
initializes its EA timing table to zero with an explicit comment that 80186
instruction timings already include effective-address calculation. `CLKM`
selects the register or memory scalar from ModR/M; no MAME code is imported.

Thus every selected MAME scalar is a same-profile CPU-model total. T362 never
adds NXVM's 8086 EA/odd-word contract to an 80186 scalar, and only applies the
existing documented 80186 segment-prefix treatment. Bus waits, prefetch,
device service, and physical cycles remain outside this owner.

## Form ledger

`direct` means the raw model scalar lies within the closed Intel interval.
`constrained` means the scalar is deterministically clamped with
`min(max(raw, lo), hi)`; it is model-derived, not Intel-exact or measured.

| Profile/form | Intel Table 1-16 domain | MAME raw scalar | Accounting and source match | S1 disposition |
| --- | --- | --- | --- | --- |
| 8086 `F6/F7 MUL/IMUL`, register and memory byte/word | Existing T361 rows: 70--77, 118--133, 76--83 + EA, 124--139 + EA; 80--98, 128--154, 86--104 + EA, 134--160 + EA | 70/118/76/128 and 80/128/86/138 | T361 S3 already proves exact form and NXVM-owned 8086 EA/odd-word/segment additions. | Direct; already allocated, not reopened. |
| 80186 `F6/F7 MUL/IMUL/DIV/IDIV`, register and memory byte/word | Existing T361 rows: 26--28 through 58--67 | 26/35/32/41; 25/34/31/40; 29/38/35/44; 44/53/50/59 | MAME i186 constants include EA; all are in their matched primary domains. | Direct; already allocated, not reopened. |
| 80186 `6B /r ib`, register source | 22--24 | 22 | `IMUL_RRI8`; same i80186 model and table immediate form. | Direct 22; already allocated. |
| 80186 `6B /r ib`, memory source | 22--24 | 29 | `IMUL_RMI8`; MAME includes EA while Table 1-16 provides one immediate-form domain without an EA suffix or separate memory row. The source-form relationship is exact; normalize total CPU-model scalar only. | Constrained: `min(max(29,22),24) = 24`. |
| 80186 `69 /r iw`, register source | 29--32 | 25 | `IMUL_RRI16`; exact i80186 immediate form. | Constrained: `min(max(25,29),32) = 29`. |
| 80186 `69 /r iw`, memory source | 29--32 | 32 | `IMUL_RMI16`; same form/accounting boundary as the preceding memory row. | Direct 32. |

No scoped row remains reference-exhausted: every former one-tick immediate
route has an exact profile/form MAME scalar and a primary Intel closed domain.
The constrained rows deliberately use visible endpoints only through the
specified clamp operation; they do not infer an operand formula.

## Fallback and consumer sweep

The S1 static sweep used:

```text
rg -n -i "legacy_dynamic_arithmetic|IMUL|CORE_MACHINE_SOURCE_UNALLOCATED_TICKS" src/core/machine tests/machine CMakeLists.txt cmake docs
```

| Hit class | Disposition |
| --- | --- |
| `core_machine_legacy_dynamic_arithmetic_model_cost` | Sole eligible 8086/80186 dynamic owner. Its `IMUL_IMMEDIATE` branch is the only scoped one-tick route and is the S2 change surface. |
| `core_machine_instruction_cost` | Calls the dynamic owner before generic classifiers; retain the one successful-retirement publisher. |
| T361 S3 smoke and CMake target | Existing smoke covers all four immediate forms, but expects fallback one tick for three. S2 updates it and adds direct lower/upper clamp evidence. |
| Other `CORE_MACHINE_SOURCE_UNALLOCATED_TICKS` uses | Other profiles, prefixes, unrelated forms, and general transfer fallbacks. Outside T362 scope; retain their named receivers. |

## S2 implementation boundary

S2 may change only the 80186 immediate-IMUL branch and its dedicated smoke to
centralize the four raw/domain/selected rows. It must expose provenance in the
private model representation, preserve prefix and successful-retirement
behavior, prove no double EA/odd-word/segment charge, and create the T362
developer artifact plus the full current gate. No broader classifier or
machine-profile refactor is needed.
