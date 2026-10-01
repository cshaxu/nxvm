# M5 T539 S91 CPU fault/event receiver

S91 moves `cpu_interrupt_prepare()`, `cpu_ud_cache_preservation()` and
`cpu_interrupt_pending_and_rollback()` from the NXVM execution-context smoke
to the sole Shared `cpu_execution_fault_event` receiver. They use only the
CPU-local bus fixture and an artificial IDT; public Core paging, PIC and board
paths remain outside this receiver. NXVM retains its separate debug API
snapshot/patch test.

Focused x64/x86 successors and residuals pass. T332, CPU/PIC authority,
Shared manifest/corpus, documentation governance and diff checks pass.
Detached full units pass x64 **444/444** (exit 0, 246.98 s) and x86
**444/444** (exit 0, 74.21 s). This is test/CMake/documentation-only work; no
executable input changed.
