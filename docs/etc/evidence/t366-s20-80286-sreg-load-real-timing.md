# T366 S20: 80286 Real-Mode Segment-Register Load Timing

## Source And Scope

The [AMD 80286 instruction summary](https://www.bitsavers.org/components/amd/x86/_dataSheets/1985_80286.pdf)
lists `MOV register/memory to segment register` (`8E`) at two clocks for a
real-address register operand and five clocks plus the documented
effective-address term for a real-address memory operand. The same row gives
the distinct protected-mode values `17,19*`; S20 does not allocate those
descriptor-load paths. The [Intel 80286 clock-count rule](https://tv.manualsonline.com/manuals/mfg/intel/80286.html?p=216)
adds one clock for base-plus-index-plus-displacement and two clocks for each
odd-addressed 16-bit memory operand.

S20 therefore assigns exactly `2` to successful unprefixed real-address
register `8E` loads, and `5 + EA + odd-word` to their memory forms. Only ES,
SS and DS encodings are admitted; `MOV CS`, invalid/386-only encodings and
every protected-mode path retain their existing receivers.

## Construction And Sweep

`core_machine_80286_source_instruction_cost()` remains the sole successful
retirement publisher. Its `8E` branch uses the existing protected-mode
predicate and existing Appendix-B EA/odd-word helpers, so the new values do
not create a second timing owner or modify segment-load semantics.

The focused ledger smoke proves all legal real-mode segment targets (ES, SS,
DS) with their selector/base result and two-tick publication. It also proves
DS loads from even direct memory (5), odd direct memory (7), and odd
base-plus-index-plus-displacement memory (8). The established segment-MOV
smoke retains `MOV CS`, protected descriptor, fault, attribute and
interrupt-shadow coverage.

## Verification

The focused `core-machine-80286-instruction-timing-ledger-smoke` passed and
emitted `M5:T357:S6:80286-INSTRUCTION-TIMING-LEDGER:OK`. The configured
`run-current-smokes` gate, documentation governance and `git diff --check`
passed before P1. Coordinator review of P1 `25d3a773` confirms that the
protected-mode predicate and legal-target guard exclude every transferred
selector-load path, while focused proof covers every admitted real-mode target
and memory parity construction.
