# M1 T4 S30 — Sprite-protection destination output (L2)

## Basis and limit

S29's selected-set probes first stopped at the read pair `0x762--0x763`.
Permitted L2 map research establishes that pair as a Sprite-protection
destination-output endpoint. This record does not import reference code,
algorithms, tables, ROM content, decryption, object transfer, or rendering
behavior.

## Board contract

The Board already has one reset-zero private destination latch: the write-only
pair at `0x6c6--0x6c7`. The new exact read-only pair at `0x762--0x763` is an L2
mirror of that existing latch. It introduces no second state owner:

- both pairs preserve the existing Core little-endian byte splitting;
- ingress remains only at `0x6c6--0x6c7`;
- output remains only at `0x762--0x763`;
- a write to the output pair stays rejected by the existing Board control
  failure path; and
- reset clears the sole underlying latch.

This endpoint is not a claim that all protection operations or Sprite behavior
are implemented.

## Verification

- Existing Board smoke proves destination write, output word and high-byte
  read/inspect, output-write rejection, and reset-zero state on x64 and x86.
- Existing selected-set probes pass the former `0x762` read on both widths and
  reach their existing two-million-instruction containment budget without a
  remaining control-window failure (`reason=1`, `detail=0`).
- Complete repository CTest qualification: 668 regular tests pass on x64 and
  x86; the separately executed junction-based `x86-nxvm.dependency-dag-negative`
  also passes on each width.

No desktop, RDP, ROM payload, media, frame, sound, or playable-game
qualification is claimed.
