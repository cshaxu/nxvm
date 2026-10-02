# M5 T540 S33 Board Construction And Plan Application

## Actual ownership diff

The same `core_machine_create_internal` validates neutral and board inputs
before allocation, calls the sole neutral create phase, then calls one private
board create phase. Board inputs are checked by one owner-local predicate.
Board creation retains the original sequence: port registration checkpoint,
92h, VADP, exclusive XT PPI/keyboard or AT KBC, DMA, PIC, IRQ0 source, system
and optional auxiliary PIT, PIT0 output binding, keyboard/speaker/A20 binding,
PIT1 unbound state, then registration-status check. Every original early
failure still destroys the one candidate; the registration-status failure
also rolls back to the original port checkpoint. The public Core result is
published only after the board phase succeeds.

`core_machine_create_from_plan` moved from the mixed constructor source to
the existing board-plan implementation beside `core_machine_plan_validate`
and `core_machine_plan_apply_topology`. It validates before allocation, calls
the same public `core_machine_create` route, applies topology in the original
memory-alias, absent-memory, parity, D4, display, DMA, RTC, FDC, HDC order,
destroys and nulls the candidate on first failure, and copies the frozen plan
only after success. The copied retirement qualification pointer is nulled;
Core already owns the key copy. There is no second plan parser, constructor,
device register state, rollback or machine-name branch.

The production diff changes two NXVM files: **66 lines added, 52 removed,
net +14**. This is a physical move of the existing board plan route plus
one private board-construction boundary, not a new abstraction layer. Reset
and destructor ownership remain S34 and S35 work, respectively.

## Verification

- Final-source full repository-only units: x64 **469/469**, x86 **469/469**.
- Current specialized gate target: **102/102** build/gates passed.
- Existing plan-invalid, board-port, profile composition, partial-failure and
  rollback tests are included in both complete unit runs.
- External boot checkpoint passed once for default, XT, 5170 and Model 40
  per width: **8/8**.
- Eight optimized 0540 products rebuilt. `objdump -f` verifies four
  `pei-x86-64` and four `pei-i386`; `objdump -h` finds no `.debug` section.
  SHA-256 by profile, x64 then x86:
  - Model 40: `DF163C5D75C14DB87FF504D10EF71B43D8B9C89B8AC227618063B93D23FEB737`, `A23DB674847C5F1B62FE685EA6AD8F5F76CB85E4D17328E85C8B176A65013B23`.
  - Default: `8D5801B6F6701F75CE95A5C9C566D16B31E6AC7ED41B528269A01D724B237B30`, `C2FD0031EE7562B628C1FEA54091389A330B824C591618E649B32E5D64D3B7AD`.
  - XT: `42388E6E76EAA1F91D779AE540360F71838D2CE15426BD9480F4A13F08AB245D`, `811D80F8558348AA42BDC220E66175047FCB331376B15278BC50416D5CF20F6A`.
  - 5170: `2D90F538D5D451C4D318EBAC16CF3570F47765342689DB62E0F998FF873E03B0`, `6AD86644F6C8AFF129041AE7025C6E97D2B3EFF45A953CC69DA7AC3166D8AFC2`.

No Shared/MyNES, INI, firmware input, protected asset or timing grade was
modified. T540 remains open for reset, teardown and physical extraction.
