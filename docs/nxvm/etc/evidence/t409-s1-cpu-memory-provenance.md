# T409 S1 CPU Memory Access Provenance

`M5:T409:S1:CPU-MEMORY-PROVENANCE:OK`

Core retains one CPU memory-transaction path and records speculative instruction
prefetch, execution-time instruction fetch, normal data, and page-table
read/write provenance. The focused transaction smoke proves the three ordinary
CPU sources; the existing 80386 paging smoke proves both page-table read and
accessed/dirty writeback classification on the real paging path.

The focused pair and the isolated 287/287 `current-gate` passed. The runnable
T409 developer artifact is `vm-0-5-0409` / `build/output/nxvm_0_5_0409.exe`,
version `0.5.0409`, 1,398,460 bytes, SHA-256
`04309056182200FA6F64C9804EB8CB2DD6525041F6A8A4358E867C326C6C6ACB`.

This is a prerequisite only: the original D4 sequential-request condition still
requires a CPU prefetch/external-cycle pipeline. No D4 page-hit or Model-L3
claim is made.