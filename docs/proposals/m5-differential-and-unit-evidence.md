# CPU Differential Tools And Specification Tests

## Goal

Retain the useful Bochs comparison and specification-driven test work without
making it a dependency of the emulator or resurrecting withdrawn product lines.

## Batches

1. Freeze copied instruction/state comparison contracts, supported CPU models,
   first-divergence masks, trace/time/byte limits and external-tool provenance.
   Reuse existing Core debug/retirement observation; no second executor.
2. Implement the bounded external bridge only through owned public contracts.
   Bochs behavior locates discrepancies; original manuals decide semantics.
3. Audit repository-only tests by CPU, device, board and App/Common adapter
   ownership. Reuse table-driven fixtures for equal semantics and preserve
   retained CPU coverage independently from the fixed-product matrix.
4. Add explicitly admitted missing semantic tests from the finite ledgers,
   with no external INI/YAML/ROM/media input in unit tests.

## Exit

Each admitted batch has owned regressions, full unit proof and no external
runtime dependency. This is not permission to copy Bochs source or reduce
integration coverage. Broader new test implementation is separately bounded
from structural test relocation.
