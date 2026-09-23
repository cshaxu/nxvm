# M2 T8 S3 Reset And Interrupt Closure

S3 keeps reset, IRQ and NMI on the production CPU/Bus path. Reset records its
seven reads: two discarded PC reads, three read-only stack cycles, and reset
vector low/high. IRQ/NMI entry records two discarded PC reads, PCH/PCL/status
writes and the selected vector reads; it retires no instruction.

The optional Core cycle hook is a hardware-facing input seam: it runs before a
CPU-visible transfer and updates only the established IRQ/NMI pins. It lets
fixtures assert/release lines at an admitted bus-cycle position without adding
a second CPU engine. Fixtures prove CLI/SEI/PLP pre-change I polling, RTI
post-restore polling, taken-branch boundary behavior, NMI edge priority,
pre-vector IRQ takeover, post-gate delayed NMI and an inhibited-window pulse
loss. They also prove cycle-budget exhaustion after a completed 7-cycle entry
with zero retired instructions.

Both x64 and x86 trees were rebuilt on 2026-09-21 and every executable under
`test-*` passed. S4 owns final step/run equivalence, artifact evidence and the
T8 closure audit.
