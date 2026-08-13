# M5 T339: 80286 Descriptor-Table And Protected-Transfer Closure

## Task Record

T339 is the Queue-ordered 80286 protected-state package following the closed
8086/80186 baseline. It owns the 80286-specific descriptor/table, selector,
gate, 16-bit stack/frame, transfer, task, validation, commit, rollback, and
delivery rows. It preserves genuine 16-bit layouts and transfers 80386-only
state explicitly rather than treating later implementation as proof.

The active packet in [CURRENT](../states/CURRENT.md) defines each admitted S.
The active [proposal](../proposals/m5-80286-descriptor-transfer-closure.md)
defines the task boundary until task closure retains it in this history
directory.

## S1 Allocation

The [S1 allocation ledger](../etc/evidence/t339-s1-80286-descriptor-transfer-allocation.md)
divides implementation by mechanism owner: descriptor/table control;
selector/cache materialization; protected 16-bit gate entry; protected 16-bit
return; and TSS16 task transition. It retains true 16-bit layouts and records
all 80386DX transfers explicitly. S7 is the T339 audit-only closure step; it
cannot implement a missing S2--S6 mechanism.

The Queue and linked program proposals use the same dependency chain:
T339 descriptor/transfer closure, 80286 `LOCK`/profile closure, 80386DX form
closure, 80386DX state closure, 80386DX audit, then final four-profile audit.

## S2--S7 Closure

S2--S6 closed table/system-word, selector/cache, protected entry, protected
return, and TSS16 task-transition mechanisms. S7 reconciles their source
owners, focused proof, publication/fault boundaries, and exact transfers in
the [80286 closure audit](../etc/evidence/t339-s7-80286-descriptor-transfer-audit.md).
The remaining `LOCK`, 80386DX width, and 80386DX state rows remain queued
work, not implicit T339 completion.
