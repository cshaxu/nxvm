# T3 S26: Selected Control Write Hole

## Boundary

S26 changes only the MyRaidenII Board and its existing Board bus unit. It does
not modify a Shared component, public interface, CPU decoder, COP, Video, ROM
archive, or published executable.

## Evidence And Model

The selected program lanes reached physical `0x43e` on both host widths. A
disposable Debug-only observation from S25 classified that access as the
existing V30 16-bit `MOV moffs, AX` write path. The fixed L2 reference map
does not assign a receiver to `0x43e--0x43f`; it therefore cannot justify
expanding the neighbouring COP register family.

The Board now owns precisely that two-byte word as a write-only L2 hole:

- writes of one or two bytes wholly inside `0x43e--0x43f` complete without
  retained state;
- reads and inspection retain the Board's existing `INVALID_STATE` refusal;
- every other unknown address in the explicit `0x400--0x7ff` control window
  remains refused.

This is not RAM, a general fallback, or a claimed physical-register model. It
is the narrowest behavior consistent with the observed write and fixed map.

## Verification

The existing Board bus executable proves the original rejected control word,
the accepted V30 word write, byte/word admission, no readback, and unchanged
unknown-hole refusal on x64 and x86.

The opt-in selected-set startup probe then advances beyond the former `0x43e`
terminal point on both widths:

| Width | Stop | Fault code | Executed | Ticks |
| --- | --- | ---: | ---: | ---: |
| x64 | internal CPU case fault | `0x6c8` | 65,589 | 1,049,763 |
| x86 | internal CPU case fault | `0x6c8` | 65,589 | 1,049,763 |

The new `0x6c8` stop is deliberately not consumed here. The fixed map assigns
nearby `0x6c0--0x6c7`, `0x6cb`, and `0x6cc` receivers but not `0x6c8`; its
Board/Video owner and direction must be established independently. No boot,
rendering, sound, desktop, or gameplay qualification is claimed.
