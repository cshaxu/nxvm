# X86 Debug

Standalone CMake projects are `shared-x86-debug` (source) and `shared-test-x86-debug` (tests).

Owns the x86 debugger in `product/` and assembler/disassembler in `xasm32/`.
Uses only Lib and Emulator; no board, firmware or machine-product configuration.
The DOS DEBUG-style grammar and injected CPU protocol retain their behavior.

`N` accepts a bare filename or one quoted string, decoded by the existing DEBUG
string parser. Quoted names preserve case, UTF-8 and spaces for `L` and `W`;
invalid names leave the previously selected filename unchanged.
