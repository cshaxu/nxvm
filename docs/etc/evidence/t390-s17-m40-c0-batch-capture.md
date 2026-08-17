# T390 S17: M40-C0 Semantic Batch Capture

The existing Model-40 BYOB retirement-capture tool now defines its successful checkpoint as **M40-C0 protected return**: after cold reset, it observes at least one successful protected-mode retirement and then stops at the first subsequent successful real-address retirement. This consumes the existing lifecycle-owned copied observer only; no Core observer, profile, public interface or execution path changes.

Each aggregate row now retains a non-raw instruction-form key: decoded primary opcode, optional `0F` escape opcode, and Group-1/2/3/4/5 selector where the primary opcode defines one. It retains the prior CPU/mode/CPL/width/LOCK/repeat, source-tick and classifier-disposition context. It does not retain a PC, immediate, displacement, full instruction bytes, ROM/media data, asset path, hash or provenance. This removes the earlier `other`-only ambiguity for later batch classification without treating the local key as an Intel timing source.

A project-owned synthetic observation smoke proves that an initial real-mode record does not terminate C0, a protected-mode record arms the checkpoint, and the first following real-mode record reaches it with a complete three-row classified matrix. It prints `M5:T390:S17:M40-C0-CAPTURE:OK` and is registered in the current gate. The current-gate catalog and its paired T345 ownership inventories are updated together for the new owned test target.

A fresh owner-managed, deleted normalized capture reaches `protected-return-c0` at 18,255 successful retirements, with 75 aggregate form/context rows, zero source-unallocated successes and no fault/status terminal. This proves C0 is a finite observable corpus; it does not map those rows to Intel sources, enable physical retirement, prove C1 boot transfer, diagnose later FDC/media progress, establish board timing or make an L3 claim.

## Transfer

The next T390 continuation must turn the retained C0 aggregate into one complete finite batch matrix: identify each semantic class, map every class to an exact Intel-primary row with project-owned regression or transfer missing/underdetermined classes, then rerun C0 with the complete disposition. It must not reopen the former one-terminal-at-a-time strategy.
