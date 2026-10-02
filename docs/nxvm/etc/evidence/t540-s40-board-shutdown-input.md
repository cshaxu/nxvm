# M5 T540 S40 Board Shutdown And Native Input Boundary

## Actual source diff

The Core run loop no longer reads the D4 configuration field. Its existing
processor-shutdown priority and CPU-only reset remain unchanged; one
board-owned read-only predicate supplies whether shutdown is a reset pulse.
It receives `const core_machine *`, not the replaceable `board_owner` used by
scheduler probes and the other board callbacks. The first attempt incorrectly
used that owner and the scheduler smoke crashed; isolated reproduction found
the owner mismatch, then the corrected contract passed both full suites.

The five existing native keyboard/scan-set/XT fault/mouse entry functions
moved verbatim from `machine.c` to `machine_board.c`. The public input ABI,
validation, XT-PPI versus 8042 dispatch and event order did not change.
Board construction still lives in the mixed source pending S42; no second
input queue, state mirror or chip implementation was introduced.

Three NXVM source files changed: **83 lines added, 71 removed, net +12**.
Shared/MyNES source and tests, owner INIs, firmware inputs and timing
contracts did not change. S41-S43 remain open.

## Verification

- The isolated x64 scheduler regression passes after the callback correction.
- Final-source repository-only units: x64 **469/469**, x86 **469/469**. The
  first x86 run had one load-induced timeout while dual builds and suites
  competed; that test passed in 0.09 seconds alone, then the entire x86
  suite passed at bounded concurrency.
- NXVM `verify-current-specialized-gates` passes, including source-owner,
  direct-compilation and test-boundary checks.
- External DOS boot checkpoint passed once per fixed profile and width:
  Default, XT, IBM 5170 and Model 40, **8/8**.
- Eight optimized 0540 products rebuilt. `objdump -f` confirms four
  `pei-x86-64` and four `pei-i386`; `objdump -h` finds no `.debug` section.
  SHA-256 by profile, x64 then x86:
  - Model 40: `16C53235772B4DBF717A7B5FA96200677ACB6A0E50410A2C79F2C51EF787BD94`, `B5E0E5944D8C5D74F0925D1E11E0D19D38FA4236F29C399813CD1B2FBF8D669F`.
  - Default: `EEF4154654952F3569CEE9062D2FC5C9D01BE79728C79DAE9F2DA539DC65F34E`, `5388371EE7978C7E82B106CADC575F680EFBC69884B307C5F6FE4B62B2F2F770`.
  - XT: `44A97C9E267407932CEB5C627AB3ED43316FCF80EC49DB46752753685DBEB33E`, `6050900C17B9183A9E17224FEC8094CD4BE6785B9C25941F44AFD5707F377444`.
  - IBM 5170: `3523A3F510B3283CCC6CADEDEE034D4FC5FBFA55EDF29EAA2B159BA7FC625128`, `0261ACB0E92DC83FA1011DC7E460E772399D002EFDC47E23487839AABE288325`.

T540 remains open for S41-S43 and later IBM-PC board extraction.
