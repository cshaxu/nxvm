# T419 S5 External-Cycle Overlap Closure Audit

## Scope Review

T419 S5 closes one original-source-backed receiver boundary: the Model-40 can
select D4's documented 2 KiB CPU-memory page miss/hit result only through the
single Core-owned external-cycle lifecycle. It does not claim that NXVM now
models the physical asynchronous prefetch producer, PAL row/bank sequence,
BCLK duration, CPU BWAIT, or external bus-master cycles.

## Requirement Mapping

| Required fact or invariant | Implemented owner and proof |
| --- | --- |
| D4 selects a 2 KiB page with two initial/page-miss waits and zero page-hit waits. | Model-40 composition selects `{ 2048, 2, 0, explicit-sequential }`; the focused prefetch-locality smoke proves the configured result. |
| A completed logical adjacency must not create a physical page hit. | Core requires BEGIN, an explicit in-flight sequential prefetch declaration, and the matching COMMIT; the focused smoke proves completed adjacency is two misses. |
| Lifecycle interruption removes hit eligibility. | The Core invalidates the private state on cancellation, reset, instruction boundary, D4 refresh, and acknowledged DMA HOLD; the focused smoke covers each boundary. |
| Other profiles retain one owner, not an alternate timing path. | IBM 5170 Model 339 selects the all-zero disabled policy and its composition smoke proves that selection. |
| A stopped-state host write cannot execute stale queued instruction bytes. | The shared Core physical-write path invalidates queued prefetch; FPU-8087 and exception-delivery regressions prove the repaired mechanism. |

## Similar-Issue Sweep

The review covered CPU prefetch, execution fetch, data and page-walk cycle
provenance, cancellation, reset, instruction boundary, DMA HOLD, D4 refresh,
Model-40 selection, and IBM 5170 composition. The only shared stale-prefetch
writer exposed by the sweep was the stopped-state public physical-memory write
path; it is repaired in the Core owner. No VM transaction executor, duplicate
CPU/DMA route, or 5170-specific timing route was introduced.

## Acceptance

Focused external-cycle, Model-40, IBM 5170, FPU-8087, and exception-delivery
smokes pass. The registered current gate passes all 289 tests with the existing
30-second limit; the repaired media smokes preserve their original instruction
and execution-quantum assertions. Implementation commits `f14f245b` and
`93f0c15b` are pushed to `master`. The verified T419 developer artifact remains
`nxvm_0_5_0419.exe`, SHA-256
`5820F4B1A7D7B92548C0AA144C717276D8E3F8A14E29ABBF75BF5FAC92BB5BDB`.

The remaining physical details transfer through the DeskPro physical-cycle
proposal and later CPU/L3 work.