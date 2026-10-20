# T3 S29: Remaining COP Command Boundary

## Scope

S29 reconciles only the three remaining uploaded-command refusals `1905`,
`9100` and `9900`. It changes no production or test source, public contract,
ROM/media, executable or asset.

## Local Dispatcher

`myraidenii_core_cop_classify()` is the sole command admission owner. It
recognizes only commands whose operation contract is already complete. Its
numeric path covers `8100` and `8900`; `1905`, `9100` and `9900` remain
`MYRAIDENII_CORE_COP_UNSUPPORTED`, so upload/trigger admission fails before a
pending command, result write or guest-memory effect is created. This preserves
the existing explicit failure rather than creating a second execution path.

## Evidence Review

The pinned MAME research record is LGPL reference material, not an
implementation source. Its available facts do not provide a complete contract:

| Form | Available fact | Missing contract facts | Disposition |
| --- | --- | --- | --- |
| `1905` | An upload/comment reference exists, but no executed handler. | Operand layout, output, result/status, reset, failure prefix, timeline and selected-set use. | Retain explicit refusal. |
| `9100` | A comment suggests an alternate trigonometric operation. | Input encoding, signedness, scale, output offset/width, result/status, reset, failure prefix, timing and selected-set use. | Retain explicit refusal; do not alias `8100`. |
| `9900` | A comment suggests another trigonometric operation with a different output location. | Input encoding, signedness, scale, output offset/width, result/status, reset, failure prefix, timing and selected-set use. | Retain explicit refusal; do not alias `8900`. |

Public searches found no original Seibu hardware/service documentation or
independent board observation that fills those gaps. A selected-set run cannot
pass the earlier Sprite/Video flags receiver yet, so it does not establish
whether any of these forms occur later in startup. No protected program data,
register state, instruction bytes or trace is retained.

## Required Future Evidence

An implementation receiver needs either primary hardware documentation or a
finite reproducible observation that independently fixes each form's complete
input, output, reset, failure-prefix and publication contract. It must then
extend the existing single COP classifier/advance path, retain failed accepted
prefixes exactly, and prove the form through the real Board route on both
widths. Reference comments alone, an alias to an existing numeric command, or
a successful no-op do not qualify.

## Result

S29 discovers no lawful implementation candidate. The three refusals are live
and necessary, not dead code or an incomplete test fixture. They remain T3
obligations and cannot be transferred to Video or Sound. The selected-set
startup blocker is unchanged: Video/Sprite must first own `0x6ce--0x6cf`.
