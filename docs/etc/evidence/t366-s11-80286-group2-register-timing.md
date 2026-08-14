# T366 S11: 80286 Group-2 Register Shift/Rotate Timing

## Source And Scope

The [AMD 80286 instruction summary](https://www.bitsavers.org/components/amd/x86/_dataSheets/1985_80286.pdf)
lists Shift/Rotate `Register/Memory by 1` as `2,7*`: two clocks for a register
operand and seven-plus-effective-address clocks for memory. `D0` and `D1` are
the byte and word encodings for that implicit-count-one form.

S11 therefore assigns two clocks only when the captured ModR/M route is
register-direct and the Group-2 extension is a documented operation (`/0`--`/5`
or `/7`). It changes no instruction semantics and keeps the sole post-refresh
retirement publisher.

## Sweep And Transfers

The sweep covers `D0`--`D3`, `C0`/`C1`, the Group-2 executor and the 80286
fallback classifier. `D2`/`D3` and `C0`/`C1` have count-dependent rows and
remain the next CPU-ledger receiver. Memory `D0`/`D1` forms retain the source
table's `7*` effective-address-dependent receiver. `/6` remains the existing
undefined-opcode path, proven by the focused smoke. No device, bus or physical
clock claim is made.
