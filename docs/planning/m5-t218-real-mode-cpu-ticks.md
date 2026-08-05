# M5 T218: Real-Mode CPU Trust And Tick Attribution

## S1: Corpus And Contract

**Status:** active. This subtask inventories the exact real-mode instruction,
prefix, I/O, fault, and interrupt forms exercised by the retained ROM, DOS
boot, FDD/HDD, KBC, VADP, PIT, and debugger paths. It does not admit protected
mode, paging, task switching, or a present FPU.

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

### Boundaries

The default profile remains `80386 + no FPU`, but T218's compatibility claim is
only the evidence-backed 8086/80186 real-mode subset. Bochs comparison, when
useful, is bounded by the device-verification trace budgets and cannot replace
owned instruction probes. No instruction-cost timing model, host-clock path,
or second executor is permitted.
