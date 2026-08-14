# T366 S14: 80286 Group-2 Count-Dependent Timing

## Source And Scope

The [AMD 80286 instruction summary](https://www.bitsavers.org/components/amd/x86/_dataSheets/1985_80286.pdf)
gives Group-2 by-CL and by-immediate-count forms as `5+n` for registers and
`8+n*` for memory. 80286 masks CL and immediate count to five bits; the count
therefore ranges from zero through 31. S14 captures pre-execution CL for
`D2`/`D3` and the decoder's `cimm` immediate capture for `C0`/`C1`, then applies the existing
Appendix-B effective-address addition to valid memory forms.

## Sweep And Transfers

The sweep covers `C0`--`C1`, `D0`--`D3`, all valid Group-2 extensions, the
executor's count masking, EA helper and fallback classifier. `D0`/`D1` retain
S11--S13 values. `/6` remains undefined. No device, bus or physical-clock
claim is made.

## Verification

The focused 80286 instruction-timing-ledger smoke, configured
`run-current-smokes` gate, documentation-governance check, and `git diff
--check` passed. The implementation changes no public ABI or device behavior.
