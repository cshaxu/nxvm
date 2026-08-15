# T366 S19: 80286 Segment-Register Memory-Store Timing

## Source And Scope

The [AMD 80286 instruction summary](https://www.bitsavers.org/components/amd/x86/_dataSheets/1985_80286.pdf)
gives `MOV Segment register to register/memory` (`8C`) a three-clock memory
base in either 80286 mode. The [Intel 80286 clock-count rule](https://tv.manualsonline.com/manuals/mfg/intel/80286.html?p=216)
adds one clock for base-plus-index-plus-displacement and two clocks for each
odd-addressed 16-bit memory operand. S19 assigns exactly `3 + EA + odd-word`
to successful unprefixed legal `8C` memory stores.

## Construction And Sweep

The existing 80286 classifier remains the sole retirement publisher and uses
its existing Appendix-B EA and odd-word helpers. The focused smoke proves an
even direct store (3), odd direct store (5), and odd base-plus-index-plus-
displacement store (6), with each written selector observed in memory.

`8C` register forms retain S18, `8E` selector loads, invalid/386-only
encodings, prefixes, fault/delivery timing, other profiles, bus/device service
and physical time remain their named receivers. No public ABI or device
behavior changes.

## Verification

The focused `core-machine-80286-instruction-timing-ledger-smoke` passed and
emitted `M5:T357:S6:80286-INSTRUCTION-TIMING-LEDGER:OK`; the configured
`run-current-smokes` gate passed. Documentation governance and `git diff
--check` are recorded with delivery and review.
