# M1 T7 S26 — Sound-Path Observation

## Scope and evidence grade

This packet adds a bounded, resettable aggregate observation at the existing
Board boundary.  It answers only whether the selected guest reaches existing
Sound-device ownership points; it does not retain ROM data, commands,
register values, samples, screenshots, or a trace.

The external comparison is L2.  The reference is the local MAME 0.289 Raiden
II driver revision `c1425a1af00b88aa07477b1b23a30ed5e5f856ff`, whose inspected
driver SHA-256 is
`FDDE57F6E9024657F2A9830864523305139562229750B87C967E597324CDB26A`.
No source, ROM data, decoder table or timing claim was imported.

## One owner path

Sound owns the counters while it already owns YM/OKI writes, OKI voice starts,
and generated PCM.  Board maps that immutable snapshot to its existing public
boundary.  The selected-set probe reads Board once after its normal PCM copy.
No additional execution route, audio queue, thread, state machine or host
delivery path exists.

The existing Sound and Board units exercise a synthetic program through the
real Z80/device path.  They prove YM address/data writes, OKI sample start,
nonzero generated PCM, Board read-only forwarding, and reset clearing.  The
Board handshake fixture additionally proves a real Board-to-Sound YM write
crossing and reset clearing without reaching into Sound private state.

## Selected-set result

The unchanged Coin/Start pulse was run once at a bounded 10,000,000
instructions on each host width.  The two aggregate outputs are identical:

| Fact | x64 and x86 |
| --- | --- |
| Executed instructions / guest ticks | 10,000,000 / 160,621,785 |
| Terminal result | Containment budget; no CPU, COP or Board-control fault |
| Frame | 320 x 240, 74,865 nonblack pixels, digest `e6d49832ee1363b8` |
| YM address/data writes | 5,494 / 5,494 |
| OKI 1 / OKI 2 writes | 1,810 / 1,810 |
| OKI 1 / OKI 2 voice starts | 0 / 0 |
| Generated / copied nonzero PCM | 0 / 0 |

The finite current-MAME comparison used its documented Lua write taps for
600 frames.  A temporary external-only archive combined the owner-supplied
set with the separately supplied PAL inputs because current MAME requires
them.  It observed 3,944 YM writes and 654 writes to each OKI address.  It
does not claim an audible output count or a timing equivalence to NXVM.

## Disposition

The Sound CPU reaches both real device address ranges on both host widths, so
the previous Board command handoff and local PCM-copy route are not the first
silent stage.  Neither local nor external observation establishes that the
current bounded guest state should start an OKI voice.  Consequently there is
no demonstrated local YM/OKI/PCM implementation defect to repair in this
packet.  Altering the device model merely to force nonzero audio would be an
unsupported behavior change.

The remaining receiver is title progression/qualification beyond this bounded
startup condition, including any protection, service/input or game-state fact
needed before sound content is selected.  This packet makes no gameplay,
audibility, fidelity, release or external-media compatibility claim.
