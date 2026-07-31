# M2 Machine And DOS Architecture Requirements

## Purpose

M2 is a design milestone. It defines the minimum whole-machine structure needed
by the owned DOS runtime before M3 changes the imported NXVM baseline.

## Required Decisions

1. Freeze the minimum virtual-device profile needed by M5 and identify every
   retained, deferred, and removed NXVM device or bridge.
2. Specify reset, boot, halt, fault, execution-budget, timer, interrupt, and
   stop-reason ownership.
3. Define versioned C contracts for CPU execution control, memory access,
   interrupt/vector control, I/O dispatch, device lifecycle, trace events, and
   Machine-to-DOS calls. Define thread ownership for every mutable state.
4. Define DOS ownership of COM loading and `INT 20h`/`INT 21h` dispatch without
   permitting `machine -> dos` dependency.
5. Define the abstract host-service trust boundary: visible-root capabilities,
   operation/error categories, and the invariant that an operation cannot escape
   an exposed root. M2 does not define DOS path grammar, reparse traversal,
   race handling, Win32 handles, or CLI syntax; M6 owns those concrete rules.
6. Map every M1 baseline observation to its M3 owning module and focused test.

## Required Deliverables

- a module/dependency diagram and lifecycle state diagram;
- C header or pseudocode interface contracts with ownership and failure rules;
- a minimum-device decision table and removal list;
- a migration sequence with no behavior-changing step lacking an M1 regression;
- M3 test commands, checkpoints, and progress budgets; and
- an explicit list of decisions deferred to M4 and M6.

## Stop Rule

An uncertain device, DOS, or host behavior is recorded as an evidence question.
Do not resolve it by expanding the M3 implementation or by adding a DOS service.
