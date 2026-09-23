# M5 T409: CPU Memory-Access Provenance

## Task Record

T409 introduces a single Core-owned label for the source of each CPU memory
transaction: instruction prefetch, execution-time instruction fetch, ordinary
data, page-table read, or page-table accessed/dirty writeback. It preserves the
existing transaction path and Core/VM dependency boundary.

## S1 Acceptance And Closure

P1 (`c6c5b462`) adds the classifications and focused proof. The transaction
smoke covers prefetch, execution fetch and data; the existing 80386 paging
smoke covers actual page-walk reads and accessed/dirty writes. The isolated
x64 full current gate passes 287/287, and `vm-0-5-0409` is recorded in the
T409 evidence with its SHA-256.

This is an original-source prerequisite only, not an original timing
implementation: the D4 rule requires a new external CPU-cycle/prefetch-overlap
boundary before its 2 KiB page identity can distinguish an overlapping
sequential request. No D4 wait scalar or Model-L3 conclusion is accepted.