# M5 T218: Real-Mode CPU Trust And Tick Attribution

## S1: Corpus And Contract

**Status:** complete.

T218 turns T217's generic per-completed-instruction coarse tick into a trusted
real-mode attribution boundary. Prefix bytes are part of one architectural
instruction and do not separately advance time. A faulting instruction records
its existing machine fault outcome but does not claim a completed-instruction
tick. `HLT`, `INT`, I/O, and string forms must be classified from actual
execution behavior rather than inferred from a DOS boot alone.

### Required Evidence

1. Map the ROM/DOS/device paths to instruction families and prefixes used in
   retained system-image and focused probes.
2. Add deterministic instruction/tick probes for every admitted family or
   document its deferred status and affected corpus.
3. Reproduce and repair only concrete real-mode defects found by that corpus.
4. Verify equal input/reset/budget sequences produce equal CPU state, stop
   reason, executed count, and elapsed ticks.

### S1 Inventory

The current retained corpus already proves real-mode CPU delivery for MOV and
arithmetic, relative control transfer, segment prefixes, INT/IVT delivery,
port I/O, `HLT`, undefined-opcode fault handoff, DOS boot/prompt, FDD/HDD ROM
paths, KBC IRQ1, PIT IRQ0, FDC IRQ6, HDC IRQ14, and Console/debugger command
boundaries. `cpu_int_ivt_smoke` covers 8086 and 80386 INT dispatch; the
retained FDD `MEM` sample remains a system-level regression. Prefix decoding
is internal to one `ExecIns()` execution call, so no prefix byte is independently
counted as a T217 completed-instruction boundary.

S2 adds a core-only real-mode tick probe for 8086 MOV, port output, INT, segment
prefix plus NOP, HLT, and rejected 386 operand-size prefix. It requires a
successful instruction to advance one configured coarse tick quantum, and a
fault to advance none.

The probe initially exposed its own incorrect reset-vector mapping rather than
a CPU defect; it now writes through the documented `FFFFFFF0h -> 000FFFF0h`
mapping and actually executes every asserted opcode. No new real-mode CPU
defect was found in this admitted corpus.

### S3 Evidence

`core-machine-real-mode-tick-smoke` reports
`M5:T218:S2:REAL-MODE-TICKS:OK`. The full current GCC/CTest matrix passes
54/54 tests, including retained DOS prompt, keyboard, video, FDD/HDD, Console,
and debugger regressions.

Developer artifact: `build/output/nxvm_0_5_0218.exe`.

SHA-256: `15C09CB261FF44FECB8584AD26B782C377E5BED786C36111486482198DA4DB83`.

Source commit: `8930907` (`M5 T218 S3 P1`).

### Boundaries

The default profile remains `80386 + no FPU`, but T218's compatibility claim is
only the evidence-backed 8086/80186 real-mode subset. Bochs comparison, when
useful, is bounded by the device-verification trace budgets and cannot replace
owned instruction probes. No instruction-cost timing model, host-clock path,
or second executor is permitted.
