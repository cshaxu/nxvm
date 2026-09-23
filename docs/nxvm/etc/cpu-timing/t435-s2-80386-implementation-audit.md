# T435 S2 - 80386DX Timing Implementation Audit

The [80386DX manifest](t435-s2-80386-timing-manifest.json) maps the accepted
[Intel ledger](t435-s1-80386-ledger.md) into 451 source-classified base keys
(449 Manual-L3 and two External-L2: `WAIT` and `ESC`). The current
route is `string -> dynamic multiply -> secondary -> privileged -> primary ->
control-stack -> 80386 fallback -> unallocated -> retirement observation` in
`src/core/machine/machine.c`.

Static inspection confirms dedicated secondary selectors for Jcc, bit,
shift-double, MOVSX/MOVZX and BSF/BSR; a dedicated privileged selector for
system/CR/DR/TR rows; and an owned early-out multiply implementation. Those
routes do not constitute per-key proof. The strict initial audit is zero
conforming: 257 keys lack a focused result, 192 dynamic/path/privilege keys
lack one or more normalized inputs, and `I386-XLAT` is unallocated because the
legacy selector admits it only for 8086. No L2 or L1 is permitted by S1.

The manifest generates 960 legal size, repeat-phase, multiplier, segment and memory-RMW
`LOCK` contexts under the [shared legality contract](t435-s2-context-legality.md).
Operand width is already part of string and multiply base keys, so no redundant
cross-product is generated. B0
replaces parallel successful selectors with one publisher; B1 maps every
Chapter-17 constant/formula; B3 publishes r/m, size, mode, path, next-component,
repeat, multiplier and legal LOCK inputs; B4 requires a result per key and removes any
successful unallocated route. `WAIT`'s minimum and external BUSY# completion,
and `ESC`'s delegated coprocessor duration, remain explicit External-L2
records rather than fabricated L3 scalars. READY/HOLD, bus lock arbitration, device waits
and event delivery remain outside this retirement program.

Markers: `M5:T435:S2:80386-IMPLEMENTATION-AUDIT:OK`.
