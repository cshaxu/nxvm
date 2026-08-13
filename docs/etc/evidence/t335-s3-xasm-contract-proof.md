# T335 S3 xasm Capacity And Failure-Atomicity Proof

`current.core-product-xasm-contract-smoke` compiles only the smoke-local xasm
source set with `STD_MALLOC=test_xasm_malloc`. Production libraries continue to
use the normal allocator; the redirect is neither a public API nor a product
runtime path.

The smoke proves all failed calls retain both output bytes and the caller's
result-length sentinel:

- zero output capacity for single assembly;
- a 256-byte statement rejected before the 255-byte engine representation;
- paragraph output one byte short of two `nop` results;
- one-byte disassembly text destination;
- malformed single assembly input;
- failure of the facade paragraph-input allocation; and
- failure of the xasm paragraph instruction-table allocation after the facade
  input copy succeeds.

It also proves an exactly 255-byte padded `nop` succeeds, retaining the
single/paragraph/disassembly success smoke separately. The marker is
`M5:T335:S3:XASM-CONTRACT:OK`.

Verification passed:

- exact contract smoke: 1/1 in 0.13 s;
- static sweep: no deprecated `core_product_utils_aasm32`,
  `core_product_utils_aasm32x`, or `core_product_utils_dasm32` product symbol
  or caller remains;
- full parallel `current-gate`: 216/216 passed in 11.21 s;
- current artifact: `build/output/nxvm_0_5_0335.exe`, SHA-256
  `3BB4A1B2FFAED8979B3D15A2B363C322B38A318F3570408E87E554FFA42F9013`.
