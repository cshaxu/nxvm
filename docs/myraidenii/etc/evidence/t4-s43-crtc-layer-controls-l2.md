# T4 S43 L2 CRTC Layer Controls

## Reference And Scope

The owner-approved implementation-reference policy applies. The inspected,
untracked MAME master is revision `ab2d1ca215c6e14dbe83fd2d0a70a4862fd16253`;
the relevant external path is `src/mame/seibu/seibu_crtc.cpp`, correlated with
the Raiden II video caller in `src/mame/seibu/raiden2_v.cpp`. No source text,
data table, ROM byte, palette payload or decryption material from that reference
enters this repository.

This is an L2 CRTC model, not an original-board accuracy claim.

## Independently Implemented Facts

- CRTC word `1ch` has active-low enable bits 0--3 for background, middle,
  foreground and text respectively;
- CRTC word pairs `20h/22h`, `24h/26h`, and `28h/2ah` provide the horizontal
  and vertical source scroll values for background, middle and foreground;
- the three 16 by 16 tile maps are 512 by 512 source pixels, so their source
  coordinates wrap modulo 512;
- enabled opaque tiles remain in the accepted S42 composition order, and text
  receives no scroll behavior in this bounded slice.

Video remains the only composition owner. Board retains the existing CRTC byte
route and raw register-file contract; no new route, renderer, queue, state
mirror or public API is introduced.

## Regression

The ROM-free `graphics_smoke` selects each 16 by 16 layer alone, proves its
initial source, horizontal shift and vertical shift through three different
palette outputs, then proves text-only and all-disabled output. The existing
`board_bus_smoke` continues to prove that the same CRTC byte file is reachable
through the public Board bus and resets correctly. Together they reject the
former inert CRTC composition behavior without exposing Board-private Video
state or creating a test-only frame path.

## Deliberate Limits

Sprites, their encryption/decryption, sprite enable, brightness/blending,
priority, base-scroll fields, text scroll, rotation, physical timing and
Machine publication remain unmodeled. No real-game frame, ROM/media payload,
desktop or RDP result is claimed by this evidence.
