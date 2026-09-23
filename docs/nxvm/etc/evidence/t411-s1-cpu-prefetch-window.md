# T411 S1 CPU Prefetch Window

`M5:T411:S1:CPU-PREFETCH-WINDOW:OK`

T411 turns Core's per-round opcode snapshot into a persistent 15-byte
instruction-prefetch window. A window hit supplies instruction bytes without a
new physical instruction-fetch transaction; a refill remains classified as
`INSTRUCTION_PREFETCH` through the existing T409 provenance and T410 external
cycle lifecycle.

The state is Core-private. It is reset during context initialization, CPU state
initialization, and CPU state reset. Sequential decode records the next expected
linear address. The next execution round invalidates the window when its
CS:EIP-derived linear address differs, covering near/far control transfer,
interrupt/exception delivery, task changes and any other non-sequential path
without adding a VM interface or per-opcode transfer hook.

The focused transaction smoke exercises a short jump, verifies the initial and
control-transfer refill cycles, verifies that consumed bytes no longer publish
an `INSTRUCTION_FETCH` transaction, then resets the machine, overwrites reset
code, and proves that the next execution uses a fresh refill. The reset-vector
window crosses the 32-bit wrap in that fixture, so its two logical refills yield
three observable physical cycles; this is a range split, not a third window.

Verification used an isolated GCC build:

- `core-machine-transaction-s2-smoke`, T359 S4 timing and the 80286 instruction
  timing ledger passed together;
- the full serial `current-gate` replay was run after the complete gate targets
  were built; two known BYOB/media work-directory-sensitive cases were also
  replayed independently and passed;
- `vm-0-5-0411` is 3,203,866 bytes with SHA-256
  `E4A0786E8C80CFCCFBB37EE7BB14B502DDBA5A02C825041B8FA89A3074DFC366`.

This is a Core generic/reference-derived prefetch mechanism and not a claim
that it duplicates a DeskPro 386 microarchitecture. T408's original D4
2 KiB page, row-miss/page-hit and two-wait facts remain unbound: actual overlap,
row arbitration, writes, DMA/refresh and BWAIT are still required before any
DeskPro Model-L3 timing publication.