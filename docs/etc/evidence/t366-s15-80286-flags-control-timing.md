# T366 S15: 80286 FLAGS-Control Timing

## Source And Scope

The [AMD 80286 instruction summary](https://www.bitsavers.org/components/amd/x86/_dataSheets/1985_80286.pdf)
lists `CMC`, `STC`, `CLD`, and `STD` as two clocks, `CLI` as three, and `STI`
as two in both real and protected virtual-address modes. S15 assigns those
fixed costs only after their existing execution handlers succeed; it changes
neither FLAGS semantics nor interrupt delivery.

## Sweep And Transfers

The sweep covers `F5`, `F8`--`FD`, `F4`, `F0`, `9B`, prefix handling and
delivery paths. `CLC` and `HLT` retain their existing timing owners. WAIT,
LOCK, x87, prefix, IRQ/NMI delivery and interrupt-shadow timing remain separate
CPU or physical-time receivers. No device, bus or physical-clock claim is made.

## Verification

The focused 80286 instruction-timing-ledger smoke, configured
`run-current-smokes` gate, documentation-governance check, and `git diff
--check` passed. The first full-gate build attempt hit a transient unlocked
host file-open failure while relinking a smoke executable; the immediate retry
completed successfully. The implementation changes no public ABI or device
behavior.
