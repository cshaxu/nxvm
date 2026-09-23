# M2 T6: Complete CPU And Bus Execution

## Admission And S Plan

T6 is automatically admitted after T5 as queue item 2. S1 creates one checked
151-row opcode ledger and expands the existing decoder by addressing-family
slices, using production-machine fixtures. S2 completes arithmetic, stack and
control, BRK/RTI and functional IRQ/NMI, then audits every official form and
all 105 excluded-byte traps.

## S1 Progress

The first implementation slice extends the existing CPU with LDA zero-page X,
absolute, absolute X/Y, indexed-indirect and indirect-indexed forms. Address
calculation remains private to the one CPU step path and carries functional
page-cross information into its documented extra-cycle total. No opcode is
marked complete by this source addition alone; S1 next adds production fixtures
for every admitted addressing form before extending the next family.

The same address helpers now serve LDY zero-page/X/absolute/absolute-X and LDX
zero-page/Y/absolute/absolute-Y, retaining load-specific index selection and
page-cross totals. These rows remain pending fixture evidence in S1.

## S1 Completion And S2 Admission

S1 expanded the single production decoder through all addressing, arithmetic,
logic, RMW, branch, stack and control families. A mechanical comparison of
`cpu-opcodes.csv` against `case 0xNN` labels in `core/cpu.c` reports 151 ledger
rows, 151 implemented official opcodes and zero missing rows. This proves only
decoder coverage, not row-level functional correctness.

S2 is admitted to build the ledger-backed execution proof: per-form length,
result, flags and cycle tests; exhaustive ADC/SBC combinations; all 105 illegal
bytes; and original control-flow fixtures. It may correct S1 behavior, but does
not close T6 without those runtime proofs.

## S2 Convergence Plan

S2 keeps one package-level completion standard and delivers it through bounded
commits.  These are evidence batches, not preallocated successor S tasks.

| Delivery | Outcome | Closure evidence |
| --- | --- | --- |
| P1 | The production engine distinguishes every legal byte from every excluded byte. | A generated-in-test mapper-0 program executes each byte once: all 151 ledger bytes continue and all other 105 bytes report a trap. |
| P2 | Every official form has a production-path assertion for length, result, N/Z/C/V/P effects and declared base/conditional cycle total. | The 151-row ledger is consumed by focused Core fixtures, including zero-page wrapping and indexed page-cross cases. |
| P3 | Binary RP2A03 addition and subtraction are complete for both D-flag values. | Exhaustive A/M/carry input combinations exercise ADC and SBC through the public machine path and compare result and C/Z/N/V. |
| P4 | Stack, subroutine, branch, BRK/RTI, functional IRQ/NMI and mapper-0 bus boundaries reach named observable checkpoints. | Original ROM fixtures assert PC, registers, RAM, stack/vector state and expected unsupported-device traps. |
| P5 | T6 is fit to close or has a concrete remaining gap. | Full admitted suite, x64/x86 task artifacts and closure audit; any unmet condition remains active or transfers only to its named receiving package. |

P1 is complete in the current worktree.  Its test uses no private CPU setter or
external ROM: it constructs a strict mapper-0 image, resets through the public
machine interface and executes exactly one instruction for each byte.  The
remaining deliveries stay open; P1 is deliberately not a claim that the 151
operations have correct semantics.

P2 now has its first all-row fixture.  It executes every official form from
the same reset state and validates the ledger's default-path cycle total and
the next PC (including the deliberate control-flow destinations).  Remaining
P2 work still proves data results, status flags, indexed-cross and branch
conditional totals; this baseline is not sufficient to close P2 or T6.

The branch fixture now covers taken and untaken BPL/BMI/BVC/BVS/BCC/BCS/BNE/BEQ
with real flag-producing prefix instructions, and a taken cross-page BPL.  It
asserts destination PC and 2/3/4 cycle totals.  This discharges the branch
conditional part of P2; data-result/status coverage remains open.

P3 is complete.  A separate public-path fixture covers ADC and SBC for every
accumulator, operand and carry input under both clear and set D flag states.
It verifies A and C/Z/N/V/D/I/U.  This fixes RP2A03's binary arithmetic rule:
set D is preserved but does not select BCD arithmetic.

P4 now supplies functional external interrupt input.  The machine accepts IRQ
level and NMI level facts; a rising NMI latches once, IRQ respects I and NMI
has priority.  Entry pushes PC and B-clear status, sets I, reads the appropriate
vector and consumes seven cycles.  The fixture verifies those facts through the
public interface.  Per-cycle polling position and bus-order fidelity transfer
to T8 as planned.

The public reset contract now distinguishes POWER from WARM.  POWER restores
the reproducible volatile initial state; WARM retains cartridge, RAM and A/X/Y,
performs the documented three-stack-slot reduction, sets I and reloads reset
PC.  The Common driver reset selects POWER.  The separate fixture verifies both
paths and therefore leaves no ambiguous reset meaning in the Core API.

One production ROM now proves the remaining functional stack/control cases:
PHP/PLP, PHA/PLA, JSR/RTS and BRK/RTI.  It inspects saved bytes through public
peek, confirms BRK's padding-byte return PC and B-set saved P, and confirms RTI
restores the pre-interrupt N state.  Bus-specific checkpoints remain open.

The bus fixture now proves 2 KiB RAM mirroring, the $4020--$7FFF open-bus
range, and unsupported PPU/APU access traps.  It caught and fixed a production
metadata defect: a pre-established unsupported-device trap omitted its opcode
PC and byte.  CPU step now attaches those fields to every failed bus operation
without replacing the earlier address/reason.  The full Core selection passes
after this similar-issue sweep.

P2 now distinguishes the four memory shift/rotate operations with inputs that
exercise shifted-out carry, carry injection, zero and negative results.  It
checks resulting RAM and C/N/Z through the production machine path.  The
accumulator variants and the remaining operation families still require their
own direct result/flag evidence.

The logical/comparison fixture now proves ORA/AND/EOR result N/Z behavior,
CMP/CPX/CPY C/N/Z behavior, and BIT's independent N/V-from-memory plus
Z-from-A-and-memory rule.  The test includes a bit-six set case so a missing
BIT V assignment cannot be hidden by a zero-valued operand.

The register fixture proves TAX/TAY/TXA/TYA/TSX, INX/INY/DEX/DEY and TXS with
negative and zero cases.  It asserts the special TXS rule: S changes while
the already-set Z flag remains intact.  Load/store and memory increment/decrement
still need direct address-result fixtures.

The store fixture proves every indexed/indirect store family through resulting
RAM: STA zpx/absx/absy/(zp,X)/(zp),Y, STY zpx and STX zpy.  It complements the
earlier direct-store checks and removes store-address computation from the
remaining P2 data/flag gap.

Memory INC/DEC now has direct zero-page and absolute-X evidence.  The fixture
uses `$FF -> $00 -> $FF`, verifies N/Z on both outcomes and confirms C survives
each RMW operation.  Load result/flag cases are the remaining family-level P2
gap.

The same shift fixture now independently exercises accumulator ASL/LSR/ROL/ROR
with the carry/zero/negative cases used for memory.  The accumulator path is
therefore no longer inferred from RMW behavior; load result/flag cases are the
remaining named P2 family gap.

Load result/flag cases now cover LDA/LDX/LDY immediate, indexed zero page,
cross-page absolute indexed reads, plus LDA `(zp,X)` and `(zp),Y`.  Values zero
and `$80` exercise both Z and N and the cross-page cases independently check
the fifth cycle.  Family coverage is complete; the remaining P2 work is the
per-form convergence audit rather than an untested instruction family.

The implied status fixture completes family coverage of CLC/SEC, CLI/SEI, CLV
and CLD/SED.  It verifies each named bit and confirms CLV clears V without
clearing N.  P2 now needs row-level consolidation, not another missing opcode
family.

The indirect-wrap fixture verifies both `(zp,X)` and `(zp),Y` pointer high-byte
fetches wrap from zero-page `$FF` to `$00`, then reach `$0200`.  It checks the
loaded `$80` and N state through the public machine.  This closes the remaining
known addressing boundary before row-level consolidation.

The all-byte decode fixture now checks more than classification: each official
byte retires once, while every excluded byte consumes one opcode-fetch cycle,
retires none and records its exact `$8000` fault PC and opcode.  This closes the
complete excluded-byte behavior row of the T6 ledger.

The all-form cycle ledger now also asserts post-instruction P from the common
power-reset state for each of the 151 official forms, in addition to cycles and
PC.  It caught a test-model omission for TXA/TYA's resulting Z flag; after the
independent expectation was corrected, all rows pass.  Differentiated operand
and flag-input cases remain supplied by the family fixtures and exhaustive ALU
test.

The closure audit built the current product target in both architecture trees.
The x64 executable reports `pei-x86-64`; the i686 executable reports
`pei-i386`. Both complete CTest suites pass after the full test targets are
built. Ignored task artifacts are retained under `build/output`: x64
`mynes_0_1_0006_x64.exe` SHA-256
`36BC7E82BD8439C77F974F2FC29E4E819F284F87DC06E9D8207722CF8E6767D9`, and x86 `mynes_0_1_0006_x86.exe` SHA-256
`C00E12661C4C45FB9B14C3A2DD92AA4942B5EC65F91A467D0F073A6E55B97059`.
This completes the product artifact condition and the T6 closure audit.

T6 closes only when each ledger row proves decode, length, result, flags and
base/conditional cycles through the one production engine; wrap/cross and bus
boundaries are covered; constructed programs reach named checkpoints; and T5
tests remain green. Ordered transfer timing and interrupt sampling transfer to
T8. No second CPU or test-only state setter is allowed.
