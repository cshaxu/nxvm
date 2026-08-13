# M5 T335 xasm Capacity And Failure Contract

## Outcome

T335 closed xasm's public capacity and failure contract. `core_product_utils`
now owns the only public assembly/disassembly operations; each has explicit
input/output spans, result length, `type_status`, and commit-on-success
publication. The raw pointer/length-return facade symbols are removed.

Debugger assembly/disassembly, profile firmware paragraph assembly, the VM
debug disassembler provider, and both xasm smoke paths migrated without a
compatibility bypass. The legacy engine remains an implementation detail; its
paragraph allocation, line storage, label-materialization formatting, and final
output publication are bounded by the facade contract.

## Delivery And Evidence

- S1 contract inventory: `038e74b6`; [inventory](../etc/evidence/t335-s1-xasm-contract-inventory.md).
- S2 migration: bounded facade, all five caller classes, and current artifact;
  [migration evidence](../etc/evidence/t335-s2-xasm-migration.md).
- S3 proof: the source-local allocator smoke retains output and result-length
  sentinels on all admission/failure paths; [proof](../etc/evidence/t335-s3-xasm-contract-proof.md).
- The new `current.core-product-xasm-contract-smoke` marker is
  `M5:T335:S3:XASM-CONTRACT:OK`; the retained success smoke remains
  `current.core-product-xasm-smoke`.
- Full parallel `current-gate` passed 216/216 in 11.21 s. Documentation
  governance, artifact-target verification, and diff checks passed at closure.
- T335 developer artifact: `build/output/nxvm_0_5_0335.exe`, SHA-256
  `3BB4A1B2FFAED8979B3D15A2B363C322B38A318F3570408E87E554FFA42F9013`.

## Boundary

This does not expand xasm opcode/syntax coverage, replace its legacy parser,
or create a general allocator abstraction. The allocator override exists only
in the contract smoke's separately compiled source set. Any future xasm syntax
or engine modernization must retain this public bounded contract and its
failure-atomicity proof.
