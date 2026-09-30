# T539 S46 Debug-state migration map

S46 consumes exactly the two mixed sources named by its active packet.  The
receivers are selected by what can be observed without a PC board, not by the
source file where a case happened to be written.

| Original family | Intended sole receiver | Boundary |
| --- | --- | --- |
| MOV DR0/1/2/3/6/7 transfer, legal register forms and 66/67 attributes | `cpu-debug-state-smoke` | CPU-local |
| MOV DR reserved and memory encodings, CPL0/CPL3/VM86 and LOCK rollback | `cpu-debug-state-smoke` | CPU-local |
| Pre-80386 MOV DR #UD delivery and real interrupt frame | board receiver | Public Core / IVT |
| MOV DR followed by real PIC delivery without an interrupt shadow | board receiver | Public Core / PIC |
| Hardware execute/read/write breakpoint state, DR6 cause bits and RF suppression | `cpu-debug-state-smoke` | CPU-local |
| TF delivery, protected-IDT delivery and TF-over-IRQ | board receiver | Public Core / IVT, IDT and PIC |
| TF 66/67 and legacy/LOCK #UD delivery | board receiver | Public Core / IVT |

The CPU receiver was calibrated against the actual fixture: MOV DR semantics
are independently observable there, while TF is only meaningful when the
machine can accept and deliver the resulting exception.  It therefore remains
a board observation rather than becoming a fabricated CPU-only result.

`core_machine_debug_mov_s59_smoke.c` and
`core_machine_tf_db_s60_smoke.c` are retired by this packet.  Historical
evidence retains their names as provenance; active CMake inventories name only
the two receivers above.

## Verification

The two focused receivers pass on x86 and x64, each emitting its sole success
marker:

```
M5:T539:S46:DEBUG-STATE:OK
M5:T539:S46:DEBUG-STATE-BOARD:OK
```

Complete repository-only `unit` runs pass **416/416** on both x86 and x64.
The x64 result was taken only after rebuilding the complete test executable
set; a stale prior `LastTestsFailed.log` from an incomplete build is not part
of this run.  The specialized-gate aggregate passes per width, including
T317, T332, T337, T344, T345, CPU/PIC authority, direct-matrix, manifest and
documentation governance gates.  `git diff --check` passes.

The T332 fixture gate initially found that the new CPU receiver was not
classified alongside the existing `cpu_instruction_fixture` users.  The gate
inventory now names that receiver explicitly; no test or production execution
path changed.  A current-source sweep finds neither retired source in active
CMake or test paths, and the remaining MOV-DR/TF cases are all assigned to
the two receivers above.

The two retired sources contained 1,006 lines. Their two receivers contain 714
lines, for a net reduction of 292 test lines; the focused CMake/gate edits add
no execution path or public surface.
