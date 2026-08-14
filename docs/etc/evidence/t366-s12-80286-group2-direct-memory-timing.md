# T366 S12: 80286 Group-2 Direct-Memory Shift/Rotate Timing

## Source And Scope

The [AMD 80286 instruction summary](https://www.bitsavers.org/components/amd/x86/_dataSheets/1985_80286.pdf)
lists implicit-count-one Shift/Rotate register/memory as `2,7*`. S11 closes
the register value; S12 applies the memory base value only to `mod=00,r/m=110`
direct-address forms, which the current 80286 effective-address helper assigns
no additional term. Valid `D0` and `D1` Group-2 extensions therefore publish
seven clocks through the existing successful-retirement owner.

## Sweep And Transfers

The sweep covers `D0`--`D3`, `C0`/`C1`, the Group-2 executor, effective-address
helper, classifier and timing smoke. Register forms retain S11's two clocks.
All other memory addressing forms retain the `7*` effective-address receiver;
`D2`/`D3` and `C0`/`C1` retain count-dependent CPU-ledger receivers; `/6`
remains undefined. No device, bus or physical-clock claim is made.
