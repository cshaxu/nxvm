# T361 S2: Legacy Dynamic-Arithmetic Cross-Reference Research

## Scope and source boundary

The owner authorized this bounded research on 2026-08-14 after T361 S1:
retain the primary-source finding that the 8086/80186 tables provide ranges,
then inspect local Bochs 2.6, PCjs, PC110-EMU, and public material.  The
authorization permits observation and comparison only.  It does not make an
external implementation authoritative, importable, distributable, or a
runtime dependency, and no external code or asset was copied.

The target forms are exactly S1's 8086 `F6`/`F7` `MUL` and `IMUL`, and 80186
`F6`/`F7` `MUL`/`IMUL`/`DIV`/`IDIV` plus `69`/`6B` immediate `IMUL`.  The
external checkouts are identified below by upstream project and source path,
not by a machine-local path.

## Primary baseline

Intel's 1979 8086 manual supplies the 8086 ranges, including register byte
`MUL` 70--77 and register word `MUL` 118--133.  The 1985 iAPX 86/88/186/188
manual repeats those 8086 ranges and supplies parenthesized 80186 ranges.  It
also distinguishes CPU clocks from memory transfers and 8086 odd-word bus
penalties.  Neither table specifies a function of operands, quotient,
multiplier, sign, or memory form that selects a particular value in its
range.  This confirms rather than revises S1.

Sources: [Intel 1979 8086 Family User's Manual](https://www.bleeve.me/fZ80/Docs/9800722-03_The_8086_Family_Users_Manual_Oct79.pdf),
[Intel 1985 iAPX 86/88/186/188 User's Manual](https://www.bitsavers.org/components/intel/8086/210912-001_iAPX_86_88_186_188_Users_Manual_1985.pdf).

## External implementation findings

| Source | Observed model | Timing usefulness | Disposition |
| --- | --- | --- | --- |
| PCjs, upstream `jeffpar/pcjs`, observed revision `c7f21b4fa2bdedac3d5c73094a6402fdc8b24c70`, `machines/pcx86/modules/v3/cpux86.js` | Has 8086-style `nOpCycles*` constants and a separate 80286 table, and links machine timers to its CPU cycle counter.  Its multiply/divide entries are scalar constants by register/memory class, not functions of operands.  Several word entries explicitly say they were lowered to match a Norton SI index; its 80386 table explicitly says it is based on 80286 counts pending a real 80386 table. | Useful evidence that PCjs supports distinct CPU/machine configuration and that its values are intentionally an emulation model.  It supplies no 80186 dynamic formula and cannot independently validate one. | Observation only; rejected as formula authority and as a second convergent source. |
| Bochs 2.6 source snapshot, `cpu/cpu.cc`, `cpu/cpu.h`, `iodev/virt_timer.*` | CPU progress uses an instruction counter (`icount`) and virtual-timer/event synchronization.  The inspected tree models 386-and-later CPU levels; it contains functional multiply/divide handlers but no 8086/80186 operand-to-clock timing table or timing formula. | Useful for functional/cross-state behavior and virtual-timer architecture, not for pre-286 cycle selection. | No candidate timing formula. |
| PC110-EMU, upstream `ahmadexp/PC110-EMU`, observed revision `81235b5fdf7d17ae180fc5cb6148e44bcbb87b4c`, `Sources/PC110Core/pc110_core.c` | This experimental ROM-driven machine increments an instruction counter in its execution loop.  Its RTC/progress facilities derive selected events from instruction-count ratios (for example, an instruction count divided by 262), and its portable frontend exposes instruction-step/IPS controls. | Useful as a machine-specific functional and controller reference only.  It does not expose a four-profile CPU-cycle model or an operand-dependent legacy arithmetic formula.  Its ROM/media assets are outside this task. | No candidate timing formula; no asset used. |

PCjs therefore answers the machine-profile question constructively: its CPU
model, base cycle rate, chipset timers, and machine configuration are separate
layers.  It does **not** make any particular CPU-cycle entry physical truth.
Bochs and PC110-EMU reinforce that functional instruction stepping and device
time are useful but distinct from a profile-local microarchitectural clock
formula.

## Public cross-reference result

[Ken Shirriff's die-based 8086 multiplication analysis](https://www.righto.com/2023/03/8086-multiplication-microcode.html)
independently explains why the 8086 unsigned ranges vary: an 8- or 16-step
shift/add loop conditionally skips or executes an add according to multiplier
bits; signed forms add sign-preparation and result-negation work.  It repeats
the 8086 ranges and says the variable time comes from conditional jumps.

That is a valuable mechanism observation, but it is not a published complete
clock-selection function.  In particular it does not calibrate every
micro-operation against the architectural table, separate instruction-fetch
and memory-transfer contributions, or provide 80186 `DIV`/`IDIV` and
immediate-`IMUL` rules.  Deriving `base + popcount` from the loop would be a
new project hypothesis, not an independently validated formula.  It therefore
cannot meet this S's two-independent-observation convergence rule.

The public Am186/Am188 material found during the search repeats range-shaped
tables but supplies no operand function.  It is not treated as a substitute
for the named Intel 80186 authority.

## Form disposition

| Profile/forms | Cross-reference result | Runtime allocation |
| --- | --- | --- |
| 8086 Group-3 `MUL`/`IMUL`, register or memory, byte or word | One public microcode analysis explains the source of variability; PCjs uses fixed/calibrated values; Bochs and PC110-EMU supply no formula. | None.  Continue `CORE_MACHINE_SOURCE_UNALLOCATED_TICKS`. |
| 80186 Group-3 `MUL`/`IMUL`/`DIV`/`IDIV`, register or memory, byte or word | No inspected external source supplies an 80186 operand-dependent formula. | None.  Continue `CORE_MACHINE_SOURCE_UNALLOCATED_TICKS`. |
| 80186 `69`/`6B` immediate `IMUL` | No inspected external source supplies an immediate-value-dependent formula. | None.  Continue `CORE_MACHINE_SOURCE_UNALLOCATED_TICKS`. |

## Consequence and next admission

This research makes no timing allocation.  A successful instance of any listed
form still receives the explicit one-tick unallocated marker in the current
source-timing model; that marker is a transfer sentinel, **not** a hardware
cycle claim.  Consequently these forms remain a known barrier to a
cycle-exact/L3 timing conclusion.

The existing `TODO(Medium)` remains the receiver.  Its next admissible path is
a hardware-observation contract naming exact chips/steppings, clock and bus
configuration, calibrated capture apparatus, input corpus, uncertainty and
repetition rules, raw-result provenance, and a profile-local
publication/rollback implementation.  Any resulting formula must be tested
against all named forms and all documented Intel ranges.  A future hardware
trace can use Bochs/PCjs only as behavioral scaffolding or differential tooling,
never as the result source.

## Verification record

- Re-read the S1 source-form and consumer audit; all forms remain at their
  explicit profile-local unallocated route in `src/core/machine/machine.c`.
- Inspected every named local external project and the cited public sources.
- No runtime, ABI, CMake, external asset, or third-party source change was
  made by this S.
