# T419 S5 External-Cycle Prefetch Overlap

`M5:T419:S5:EXTERNAL-CYCLE-OVERLAP:OK`

## Evidence Tier

`original` for the selected D4 result: a 2048-byte CPU-memory page has two
waits for the initial/page-miss read and zero waits for a page hit. The source
does not define an executable asynchronous CPU-prefetch pipeline, row/bank PAL
logic, BCLK duration, or an overlap identity protocol. Those missing parts are
not inferred from adjacent completed accesses.

## Implemented Contract

Core owns one `external_cycle_timing` profile contract. A profile can select a
2 KiB page, miss/hit tick values, and the explicit-sequential-overlap policy.
The CPU external-cycle callback records BEGIN, explicit overlap declaration,
COMMIT, and CANCEL. A hit requires all of: a matching pending request, the
selected policy, an in-flight instruction-prefetch predecessor, exact next
physical address, and the same page. DMA HOLD acknowledgement, D4 refresh,
reset, cancellation, instruction-round boundary, and successful stopped-state
physical memory writes invalidate the private prefetch/cycle state.

Model-40 composition selects `{ 2048, 2, 0, explicit-sequential }`. IBM 5170
Model 339 selects the same Core owner with the disabled all-zero policy; no VM
transaction route or CPU/DMA owner is duplicated.

## Verification

`core-machine-prefetch-locality-smoke` proves an explicit 0-to-1 prefetch
overlap charges miss plus hit, while completed adjacency stays two misses. It
also proves cancellation, DMA HOLD, D4 refresh, reset, and instruction-boundary
invalidation. `vm-model40-private-composition-s7-smoke` proves Model-40
selection; `vm-ibm-5170-model-339-composition-smoke` proves the disabled 5170
selection. The stale-prefetch similar-issue sweep found and repaired
stopped-state physical writes; `core-machine-fpu-8087-smoke` and
`core-machine-exception-delivery-s2-smoke` now pass.

The rebuilt developer artifact is `nxvm_0_5_0419.exe`, SHA-256
`5820F4B1A7D7B92548C0AA144C717276D8E3F8A14E29ABBF75BF5FAC92BB5BDB`.
The registered current-gate CTest replay passed all 289 tests with four workers and
the unchanged 30-second per-test limit. The two media smokes now pass in 4.87
seconds (video) and 9.94 seconds (FDC): their persistent display checkpoints
are sampled every 256 executed instructions, while the video smoke retains its
per-instruction opcode/INT 10h accounting and the FDC smoke retains both
one-instruction and 128-instruction execution cases.

## Remaining Transfer

No synchronous execution path currently declares a physical asynchronous
prefetch overlap, so it cannot manufacture a hit. An original- or
reference-backed producer, D4 row/bank/PAL sequencing, BCLK calibration,
CPU BWAIT, external bus-master behavior, and physical memory-transfer phases
remain separately admitted physical timing work.