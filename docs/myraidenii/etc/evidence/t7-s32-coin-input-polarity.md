# M1 T7 S32 — Coin Input Polarity Reconciliation

## Question

S28 and S31 delivered Coin1 then Start1 through the local Machine-driver and
Board route, but their bounded result cannot establish whether the local Coin
electrical polarity agrees with the selected Raiden II sound hardware route.
This record resolves only that finite route fact.

## External L2 reference

The public MAME source was inspected read-only at commit
`cd680acf012140761f9dee37dadd4c11a1207fbd`:

- `src/mame/seibu/raiden2.cpp` selects `SEIBU_COIN_INPUTS_INVERT` for Raiden
  II and reads the resulting Coin port through the Seibu Sound CPU at `0x4013`.
- `src/mame/shared/seibusound.h`, blob
  `437e02cb6ffe2c9f01118c4e06734fa7389ec892`, SHA-256
  `9187332DB23D13D100A7250D2920CBE0524CA7DC4F51126BACCDC73384CD7F98`,
  defines the selected macro with Coin1 and Coin2 as `IP_ACTIVE_LOW`.

This is L2 behavior/layout evidence. It does not import, translate, link, or
redistribute MAME source, data, tables, ROM content, or a runtime dependency.

## Local owner comparison

The local Board owns the complete Coin state used by the existing Sound
transport callback:

| State | Reference Coin1/Coin2 | Local `coin_inputs` |
| --- | --- | --- |
| Reset / released | high | reset to `0xff`; release sets the selected bit |
| Pressed | low | press clears the selected bit |
| Sound read | read through the Seibu Sound path | existing `coin_get` callback reads the Board byte |

Both Coin1 and Coin2 use the same one-bit active-low rule. The local state is
therefore already consistent with this bounded reference fact.

## Disposition

No source change is justified. Reversing the Board logic would be a regression,
not a repair. S31's no-audio result remains a bounded observation: it excludes
neither later title/gameplay state nor other unmodelled hardware, but Coin
polarity is no longer an open host-input explanation for that result.

## Verification

- Reviewed the local Board reset/press/release owner and the existing Board
  input tests.
- No external asset, staging root, protected media, executable or test output
  was created.
- Documentation governance and whitespace checks are the only required gates
  because this S has no source/build/test change.
