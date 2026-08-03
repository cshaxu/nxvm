# M5 T129 S2: Explicit Assembler and Debugger Context

`aasm32` now creates and passes a local context through all internal parsing
helpers. `core_product_debug_main` owns its debugger context lexically and
passes it to every command helper; the previous thread-local debug selection is
gone. Debugger initialization preserves the caller-provided wait scope.

T129's completion gate passed: Windows GCC build; the focused xasm,
debug-target, and presentation-debugger smoke tests; the retained NXVM Console
lifecycle smoke test; and the FDD DOS-prompt smoke test. The xasm smoke covers
a local assembler call, a local disassembler call, and the paragraph assembler
path that reuses the same explicit context across its internal calls.

The task artifact is `build/output/nxvm_0_5_0129.exe`
(`4E5818EC475175661F18A6A108A810E72DE9EFC6AF773075CAC878A1019E5B15`).
