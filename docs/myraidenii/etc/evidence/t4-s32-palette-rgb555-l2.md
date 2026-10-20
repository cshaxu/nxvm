# T4 S32 — Palette RGB555 L2 Interpretation

## Scope

Video already owns one 2048-word raw palette image. This change adds its one
checked colour lookup for a future native-frame owner. It neither decodes a
graphics image nor publishes a frame.

## L2 Contract

The low fifteen bits of each stored word are interpreted as RGB555:

- bits 0--4: red;
- bits 5--9: green;
- bits 10--14: blue;
- each five-bit component expands with `(value << 3) | (value >> 2)`;
- the returned value is `0x00RRGGBB`.

This is an owner-approved L2 transport interpretation. It does not establish
the selected board's physical palette bit layout, analogue response,
brightness, alpha, priority, or final colour accuracy.

`myraidenii_core_video_palette_color()` rejects a null Video, null output or
out-of-range palette index and leaves the caller's output untouched. It adds no
cache, alternate colour path or persistent state.

## Verification

- `myraidenii.core/board-bus-smoke` passed in both `gcc-x64-release` and
  `gcc-x86-release`. Synthetic zero, primary extremes and the mixed word
  `0x4210` prove the bit mapping and expansion; invalid input proves output
  preservation.
- The owner-supplied selected-set probe passed on both widths after transient
  program-lane staging and cleanup, with its existing bounded result:
  `reason=1`, `executed=2000000`, `ticks=33301143`.
- No protected ROM bytes, extracted images, screenshots or traces were
  retained. `git diff --check` and the MyRaidenII documentation governance
  check remain required before closure.

This is not renderer, desktop, RDP, sound, or playable-game qualification.
