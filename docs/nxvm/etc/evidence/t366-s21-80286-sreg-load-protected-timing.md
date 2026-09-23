# T366 S21: 80286 Protected-Mode Segment-Register Load Timing

The [AMD 80286 instruction summary](https://www.bitsavers.org/components/amd/x86/_dataSheets/1985_80286.pdf)
lists protected-mode `8E MOV register/memory to segment register` at 17 clocks
for a register operand and `19*` for memory. The [Intel 80286 clock-count rule](https://tv.manualsonline.com/manuals/mfg/intel/80286.html?p=216)
adds one clock for base-plus-index-plus-displacement and two for an odd 16-bit
memory operand. S21 therefore publishes `17` and `19 + EA + odd-word` only for
successful unprefixed legal ES/SS/DS loads.

The focused ledger smoke boots one valid 80286 descriptor context, halts, then
resets the measurement boundary before every single-instruction run. It proves
direct DS (17), direct even-memory ES (19), direct odd-memory ES (21), and
odd base-plus-index-plus-displacement ES (22), including loaded selectors.
Real mode, invalid/386-only encodings, `MOV CS`, descriptor faults, prefix,
delivery, interrupt shadow, bus/device service and physical time remain
transferred.

The focused target passed and emitted `M5:T357:S6:80286-INSTRUCTION-TIMING-LEDGER:OK`.
The configured `run-current-smokes` gate, documentation governance and `git
diff --check` passed before P1.
Coordinator review of P1 `e71d2262` confirms the legal-target and protected-mode
guards retain transferred paths while the fixture isolates every admitted
direct/indexed parity construction.
