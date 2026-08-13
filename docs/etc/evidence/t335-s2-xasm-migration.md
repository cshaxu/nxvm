# T335 S2 xasm Bounded Migration

## Delivered Boundary

`core_product_utils` now exposes only `core_product_utils_assemble`,
`core_product_utils_assemble_paragraph`, and
`core_product_utils_disassemble`. Each operation accepts byte spans, explicit
output capacity, a result-length output, and returns `type_status`.

The facade validates mode, nulls, zero capacities, embedded NULs, and the
255-byte legacy engine statement/line limit before invoking xasm. Single
assembly stages the 15-byte instruction result. Paragraph assembly copies its
input to an owned terminated span, validates allocation, validates every
instruction-line storage write, totals all instruction bytes against caller
capacity before copying any byte, and publishes the result length only after
the copy. Disassembly stages text in the engine-sized local buffer and copies
both terminator and result length only after the complete text fits.

The legacy `aasm32x` engine declaration is implementation-private and has an
explicit final output capacity/result boundary. Label materialization now uses
bounded formatting instead of unbounded concatenation. The retained single
assembler and disassembler entry declarations are implementation details; no
product caller includes them.

## Migrated Callers

- Debugger assembly prompts pass their exact input span and `acode[15]`; guest
  writes occur only after `TYPE_STATUS_OK`.
- Debugger listings pass `ucode[15]` and their exact text arrays.
- Profile firmware preserves its line-count allocation but passes its exact
  byte capacity and frees unpublished output on every failure.
- The VM debug disassembler provider now carries status, statement capacity,
  code span, and result length; the recorder emits `<ERROR>` on failure.
- `current.core-product-xasm-smoke` uses only the bounded facade for single,
  paragraph, and disassembly success paths.

## Verification

- A repository sweep finds no `core_product_utils_aasm32`,
  `core_product_utils_aasm32x`, or `core_product_utils_dasm32` symbol/caller.
- `cmake --build --preset current-gcc --target core-product-xasm-smoke` and
  `ctest --test-dir build/mingw-gcc-x64 --output-on-failure -R
  ^current\\.core-product-xasm-smoke$` passed; the exact test passed in 0.16 s.
- Fresh `cmake --preset mingw-gcc-x64` plus `cmake --build --preset current-gcc`
  passed and produced `build/output/nxvm_0_5_0335.exe` with SHA-256
  `3BB4A1B2FFAED8979B3D15A2B363C322B38A318F3570408E87E554FFA42F9013`.

S3 remains limited to deterministic insufficient-capacity, malformed-input,
overlong-input, and allocation-failure atomicity proof.
