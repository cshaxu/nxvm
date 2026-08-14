# T361 S3 synchronous-fault retirement audit

Status: implementation and evidence in progress; this record is not yet an
acceptance record.

## Dynamic-arithmetic reference-model audit

The owner-approved model-derived branch uses no imported source or binary.
The consulted public revisions are 86Box `375e09f049f6eda75638cec0c19933b2111e42b4`,
MAME `21afc3b69c7aea273e4e16e6a52fa8e4fd207aa3`, and PCjs
`c7f21b4fa2bdedac3d5c73094a6402fdc8b24c70`. They were read only on
2026-08-14. Intel's 1979 and 1985 tables remain the profile authority; an
external value is a labelled model choice, never a physical measurement.

| Profile/forms | Candidate result | S3 disposition |
| --- | --- | --- |
| 8086 Group-3 `MUL`/`IMUL` | 86Box `src/cpu/808x.c` has a machine-coupled bus/prefetch cycle model, not a standalone form table under NXVM's no-bus/prefetch scope. PCjs `machines/pcx86/modules/v3/cpux86.js` exposes fixed entries but explicitly calibrates several word values to Norton SI. MAME `src/devices/cpu/i86/i86.cpp` exposes a complete fixed i8086 table with register/memory form values and separate EA timing. | MAME is the selected `model-derived` source; NXVM adds only its already-owned 8086 EA, odd-word, and segment-override contracts. The complete range comparison is below. |
| 80186 Group-3 and `69`/`6B` immediate `IMUL` | MAME `src/devices/cpu/i86/i186.cpp` exposes i80186 Group-3 and immediate-IMUL register/memory entries and states that effective-address time is already included. PCjs has no independent 80186 table; 86Box's detailed machine/bus model is outside this S's allocation boundary. | MAME supplies range-valid Group-3 rows and only the `6B` register row. `69` and immediate memory-source rows are explicitly reference-exhausted because their candidate/source domains do not meet the Intel comparison rule. |
| QEMU, Bochs, PC110-EMU | QEMU's documented `icount` is instruction counting, explicitly not cycle-accurate. Bochs and PC110-EMU provide functional/event references but no matching 8086/80186 form timing table. | No timing allocation from these references. |

The focused `core-machine-legacy-dynamic-arithmetic-timing-s3-smoke` proves
every selected register/memory width family, every explicitly unallocated
immediate form, and divide-by-zero's zero-publication boundary.

## Intel-range disposition ledger

This ledger completes that comparison.  The primary record is Intel
`210912-001`, Table 1-16 (PDF pages 1-27 through 1-34 for `DIV` through
`MUL`), read on 2026-08-14; its `n(m)` notation makes `m` the 80186 count.
The same table says 8086 memory rows add EA, and Table 1-15 says a segment
override adds two clocks.  The model constants below are a transcription into
NXVM's own timing owner, not copied source code.

| Profile/form | Intel domain | Candidate/revision/path | Published S3 value | Disposition |
| --- | --- | --- | --- | --- |
| 8086 `MUL r8`, `r16` | 70--77; 118--133 | MAME `21afc3b`, `i86.cpp`: 70; 118 | 70; 118 | model-derived, in range |
| 8086 `MUL m8`, `m16` | 76--83 + EA; 124--139 + EA | MAME: 76; 128, plus NXVM's pre-existing EA/odd-word/override contracts | 76 + EA; 128 + EA | model-derived, in range before/additive to the same documented modifiers |
| 8086 `IMUL r8`, `r16` | 80--98; 128--154 | MAME: 80; 128 | 80; 128 | model-derived, in range |
| 8086 `IMUL m8`, `m16` | 86--104 + EA; 134--160 + EA | MAME: 86; 138, plus the same NXVM modifiers | 86 + EA; 138 + EA | model-derived, in range before/additive modifiers |
| 80186 `MUL r8/r16/m8/m16` | 26--28; 35--37; 32--34; 41--43 | MAME `21afc3b`, `i186.cpp`: 26/35/32/41; its table states EA is included | 26/35/32/41 | model-derived, each in range |
| 80186 `IMUL r8/r16/m8/m16` | 25--28; 34--37; 31--34; 40--43 | MAME: 25/34/31/40; EA included | 25/34/31/40 | model-derived, each in range |
| 80186 `DIV r8/r16/m8/m16` | 29; 38; 35; 44 | MAME: 29/38/35/44; EA included | 29/38/35/44 | model-derived, exact table cells |
| 80186 `IDIV r8/r16/m8/m16` | 44--52; 53--61; 50--58; 58--67 | MAME: 44/53/50/59; EA included | 44/53/50/59 | model-derived, each in range |
| 80186 `6B /r ib`, register source | 22--24 | MAME: 22 | 22 | model-derived, in range |
| 80186 immediate `69 /r iw`, and either immediate form with a memory source | `69` register row is 29--32; Table 1-16 gives no independent memory-source domain for these EA-included MAME rows | MAME has 25 for `69` register (out of range), and 29/32 EA-included memory constants | no allocation; existing explicit one-tick unallocated marker | reference-exhausted: no range-valid same-form model may be selected without inventing an EA interpretation |

The last row is intentionally a non-allocation, not a fallback to an Intel
endpoint.  This preserves the packet's exact-Intel/range-constrained/no-Intel/
reference-exhausted ladder and avoids claiming that a timing-model constant is
a physical measurement.

## Mechanism finding

`ExecFinal()` can successfully commit an architected synchronous exception
frame and transfer control to an IVT/IDT handler while clearing the current
instruction exception.  The outer machine loop previously had no distinct
outcome for that condition and consequently treated the faulting instruction
as retired when it assigned instruction ticks.

The correction carries a private execution-round outcome from `ExecFinal()`
through `core_machine_cpu_execution_refresh()` to `core_machine_run()`.  It is
set only while `ExecIns()` is active and only after successful delivery.  The
machine ends that round at the handler entry with `STOP_BUDGET`, zero executed
instructions, zero round ticks, and unchanged elapsed machine time.  A later
`core_machine_run()` call is the first round permitted to retire the handler.

This intentionally excludes terminal delivery failures (which remain faults),
external PIC/NMI delivery (which occurs outside the in-progress instruction),
and TF single-step delivery after a successful instruction (which also occurs
outside the in-progress instruction).

## Audited class matrix

| Boundary | Existing focused owners selected for migration/recheck |
| --- | --- |
| #DE, #UD, #GP | real-mode corpus, legacy arithmetic/LOCK, exception-delivery, real/protected #UD delivery |
| #BR | BOUND S54 |
| #NM, #MF | FPU interface S65 and FPU escape |
| #NP, #SS, #TS | 80286 protected mode, protected privilege/return, gate entry, task switch, TSS I/O map |
| #PF | 80386 paging |
| #DB | debug MOV S59 and TF DB S60 |
| VM86 exception delivery | VM86 delivery and VM86 LGDT/LIDT S5 |

The first selected-current-gate audit found 14 retained corpus owners that
observed handler-visible state in the same run as delivery.  Those targets now
opt into `test_core_machine_fixture_run_after_delivery()`: it observes a
delivered synchronous exception stop, then invokes the distinct handler round.
The S3-specific smoke and the BOUND/FPU/real-mode focused owners retain direct
first-round assertions.  This keeps compatibility assertions separate from
the production one-publisher boundary rather than restoring same-round
retirement.

## P1 verification index

- `core-machine-legacy-dynamic-arithmetic-timing-s3-smoke` passed after the
  range-exhausted immediate-IMUL correction; it proves model allocation,
  explicit unallocated fallback, and `#DE` delivery/nonpublication followed
  by handler retirement on the next run.
- Direct focused owners passed: `core-machine-real-mode-corpus-smoke` (`#UD`),
  `core-machine-bound-s54-smoke` (`#BR`, `#GP`),
  `core-machine-fpu-interface-s65-smoke` (`#NM`, `#MF`), and
  `core-machine-task-switch-smoke` (`#NP/#SS/#TS/#PF` plus post-commit task
  debug trap).  Their source assertions retain the original-fault round as
  zero retirement where that owner observes it directly.
- `core-machine-debug-mov-s59-smoke`, `core-machine-tf-db-s60-smoke`,
  `core-machine-vm86-delivery-smoke`, and `core-machine-interrupt-entry-smoke`
  remain current-gate regressions for execution-before `#DB`, post-success TF
  `#DB`, VM86 delivery, and external interrupt ordering.
- `ninja -C build\\mingw-gcc-x64 verify-documentation-governance` and
  `git diff --check` passed. The full `current-gate` CTest invocation
  completed after the final source change with no active CTest process; its
  temporary failure list is stale from an earlier invocation, while the
  formerly listed `core-machine-t359-s4-timing-smoke` was independently
  rerun and passed.
