# M6 T39 S4 MMC3 Reload Contract

## Repair

The Mapper-004 `$C001` write now clears `mmc3_irq_counter` and sets the
existing reload-pending flag.  The next qualified A12 rising edge therefore
loads the latch instead of decrementing a stale partial count.  Cartridge
remains the sole owner of mapper register, counter and IRQ state; PPU remains
the A12 publisher.

## Owned Regression

`mynes.core.mapper4-smoke` establishes a nonzero count, writes `$C001`, and
asserts all of the following:

- the live counter becomes zero immediately;
- reload remains pending until one qualified rising edge;
- that edge loads the latch rather than decrements the old count; and
- `$E000` clears the asserted Cartridge IRQ, disables it and refreshes the
  aggregate machine IRQ line.

## Verification And Disposition

| Check | Result |
| --- | --- |
| x64 focused Mapper-004 linkage | Passed. |
| x64 full MyNES CTest | 53/53 passed. |
| x86 focused Mapper-004 linkage | Passed. |
| x86 full MyNES CTest | 53/53 passed. |
| Ignored synchronized visual probe | After the finite warm-up and Start injection, the no-input and Start paths still publish identical black frames. |

The finite standard Mapper-004 reload defect is repaired, but the owner-local
visual transition is still absent.  S4 therefore does not claim gameplay
success.  The next receiver is the PPU fetch schedule's represented A12 level
sequence during rendering, which must be measured against the Cartridge's
qualified-edge contract before a repair is selected.

Reference: [NESdev MMC3 IRQ specifics](https://www.nesdev.org/wiki/MMC3).
