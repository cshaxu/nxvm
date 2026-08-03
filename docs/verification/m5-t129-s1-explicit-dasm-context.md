# M5 T129 S1: Explicit Disassembler Context

`dasm32` now creates one local context per public invocation and passes that
context through every private helper and opcode dispatch entry. The former
thread-local `dasmContext` has been removed.

Windows GCC builds and the core debug-target and presentation-debugger smoke
tests passed. T129 remains active until assembler and debugger context state
are also explicit.
