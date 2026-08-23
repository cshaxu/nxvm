# M5 T437: 80386DX Instruction Timing Closure

## Task Record

T437 owns the complete 80386DX successful-retirement instruction function and
timing closure transferred from the owner-approved requeue of the former
four-profile program. Its fixed universe is the 450 all-L3 base forms and 961
finite legal context keys in the retained 80386DX manifest: 1,411 canonical
keys. It must implement every key through one Core-private timing-selection and
retirement-publication route, with the Chapter-17 value/formula, actual
normalized inputs, origin and `source_timing_unallocated=false`.

The task preserves accepted instruction semantics unless focused execution
proves a concrete 80386DX semantic defect required for a successful manifest
key. It does not take ownership of external waits/arbitration, prefetch,
physical board cycles, device response, exception/event delivery, public ABI
work, source import or a new profile.

## Subtask Decomposition

| S | owned closure boundary |
| --- | --- |
| S1 | freeze an executable, disjoint 1,411-key partition and proof plan |
| S2 | result-contract producer and sole 80386DX timing publisher |
| S3 | ordinary arithmetic/data, r/m, size, segment, LOCK, multiplier and XLAT forms |
| S4 | direct I/O, strings, port strings, repeat phases and overrides |
| S5 | ordinary stack, conditional control and real-mode transfer paths |
| S6 | protected/VM86 privilege, gate, task and interrupt transfer paths |
| S7 | protected segment, descriptor, special-register and system forms |
| S8 | full generated-key closure, regression, cleanup and task audit |

The retained [candidate proposal](../proposals/m5-80386dx-instruction-timing-closure.md)
defines the exact exit standard for each S. S8 cannot absorb unimplemented
families: each remains assigned to its originating implementation S.

## Initial Subtask

S1 derives the partition from the retained manual ledger, manifest, decoder
inventory, implementation tracker and result contract before runtime changes.
It must prove exactly one S2--S7 owner for each canonical key, preserve the
strict nonconforming baseline and record the focused evidence that later S
units must produce. It creates no timing result or implementation claim.

## Accepted Progress

### S1: Canonical-Key Partition

S1 added an executable partition verifier and indexed evidence. The verifier
assigns the complete 1,411-key manifest exactly once: S3 owns 217 base and 808
canonical keys, S4 60/162, S5 80/232, S6 40/94 and S7 53/115. It keeps the
existing strict baseline visible: zero conforming keys, 192 missing inputs, 257
missing tests and one unallocated XLAT form. No runtime source, CMake product,
developer artifact, public ABI or timing result changed.

## Coordinator Acceptance

The single-session coordinator reviewed P1 `467e8299` against the S1 packet.
The partition verifier, manifest verifier, decoder-ledger verifier, existing
80386 decoder-inventory runner and documentation governance gate passed. The
current GCC subprocess fails before source diagnostics in `audit-current-head`;
the retained current-source inventory runner replayed successfully and produced
no inventory diff. The code and evidence changes are confined to partition
proof and task governance; the review found no duplicate, orphaned or silently
reassigned canonical key.
