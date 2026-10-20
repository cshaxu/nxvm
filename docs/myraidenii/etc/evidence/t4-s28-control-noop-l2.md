# M1 T4 S28 — Selected `0x6ca` Control No-Op (L2)

## Basis and scope

S27's bounded startup probes reached a one-byte Board write at `0x6ca` on both
host widths. Permitted external-emulator research distinguishes that byte as a
no-op while assigning the adjacent `0x6cb` byte to a separate Raiden II program
ROM-bank receiver. This is L2 address ownership only; no reference code,
algorithm, payload or bank behavior is imported.

## Local contract

Board adds one exact replacement route for a one-byte write at `0x6ca`, reusing
the existing selected-control no-op owner. It retains no value and provides no
readback. Reads and inspection refuse. A two-byte access beginning at `0x6ca`
uses Core's existing byte splitting: its first lane is this no-op and its
second reaches the already-existing, separate `0x6cb` bank owner. No generic
control window or state is added.

## Verification

The existing Board smoke proves the exact one-byte write succeeds, read and
inspection fail, and the cross-byte write remains Core's existing split route.
Its existing unsupported-DMA row now uses still-unimplemented mode `0x0016`:
S27 legitimately made former failure mode `0x0014` a tilemap receiver, so that
old row no longer represented a failure contract.
Both selected-set startup probes then reach the next independent one-byte Board
write at `0x6d8`:

```text
fault-code=1752 control-valid=1 control-physical=1752
control-bytes=1 control-access=1 executed=728235 ticks=12103376
```

This S does not receive `0x6d8`, the `0x6cb` bank receiver, sprite semantics,
or rendering. It makes no playable-game or hardware-exact claim.

The focused Board smoke and selected-set probe pass on x64 and x86. The regular
641-test unit selection plus the separately-run dependency-DAG negative probe
pass on each width, yielding the complete 642-case qualification per width.
The negative probe is separate only because it deliberately creates an isolated
temporary dependency tree; its assertion still requires a nonzero checker
result and both source/destination tokens for each forbidden edge.
