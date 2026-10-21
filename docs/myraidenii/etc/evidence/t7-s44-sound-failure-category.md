# T7 S44 Sound Failure Category

## Scope

S44 adds one resettable finite category to the existing Sound observation.  It
does not retain guest bytes, addresses, instructions, program counters, ROM
content, media, traces, or mutable guest state.  There is still one Sound
executor and one Board-to-Probe observation route.

## Contract

The only terminal `LIB_STATUS_UNSUPPORTED` owners relevant to the S43 transfer
now set one of these categories before returning their pre-existing status:

| Category | Sole owner |
| --- | --- |
| `DECODER` | An unsupported base, extended, indexed, or indexed-bit instruction form. |
| `MAP` | An unowned access in the existing unified Sound read/write map. |

The interrupt helper's existing `LIB_STATUS_UNSUPPORTED` sentinel remains
unchanged: it is internally consumed as "no interrupt accepted" and is not a
terminal category.  Reset clears the category to `NONE`.

Board copies the existing Sound observation field unchanged, and the existing
selected-set probe prints only the finite category.  No map route, instruction
semantics, timing, input route, renderer, queue, thread, or Shared contract
changed.

## Verification

- Sound smoke induces one synthetic decoder failure and one synthetic unified
  map failure, observes their distinct categories, and confirms reset returns
  `NONE`.
- Board smoke reaches both categories through the real Board advance path,
  confirms the existing terminal fault route, and confirms a machine reset
  clears the copied category.
- The complete MyRaidenII unit label passes on x64 and x86.
- One bounded, persistent-background selected-set probe per width reached the
  same finite `DECODER` category.  Both probes remained on the existing later
  unsupported terminal; this is diagnostic evidence only, not a title-start or
  gameplay claim.
- The external staging root was empty after each wrapper invocation.  No ROM or
  probe output was retained in the repository.

## Receiving Work

The result selects the Sound decoder, not a Board-map widening.  The next
receiver must inventory and implement one source-authorized, bounded decoder
family as a complete batch.  It must not infer an individual guest instruction
or repair behavior from this category alone.
