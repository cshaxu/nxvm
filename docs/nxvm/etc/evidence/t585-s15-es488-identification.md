# T585 S15 — PC110 ES488 identification

The PC110 board includes an ES488 digital-audio device. The authorized
Open-Source-PC110 `Discovery/ES488/readme.md` records a configured real unit
answering reset at 226h with AAh at 22Ah, command E1h with version 02h/01h,
and E7h with 48h/82h. The live observations are L2 reference evidence;
they do not establish the board's power-on configuration. This implementation
uses 220h as an explicit L1 default, not as a sourced universal reset value.

MyPC110 now connects this small identification protocol to the existing Core
port router and the existing board reset. Data-available status follows the
pending response, a reset assertion discards stale data, and a board reset
clears the device. Unknown commands are ignored by this incomplete model.
No audio, DMA, IRQ, programmable address, mixer or YM3812 synthesis is
implemented by this batch. Recognition must not be represented as working
playback; the remaining audio work belongs to the subsequent batches.

The existing machine-construction test exercises the actual driver/debug and
Core-port route, both identification replies, response status and ordering,
unasserted reset, unknown command, stale-data clearing and board reset. No Lib
or Emulator source changed. Cached Release builds succeeded on both widths.
The full repository unit suites passed 712/712 on each width, and the PC110
firmware integrations passed 9/9 on each width on an isolated desktop. These
tests do not establish RDP interaction or audible output.

The deployed executables match their respective build outputs; both passed
the build's PE-width check. SHA-256: x64
`89F7BA14863D854E84B3A7C057FCF9B6DBAF138EB58ABDFE061D096FD924C31D`,
x86 `494EB1EC85935665E4D470B60B87D5EE223D0309A5C4A4552D4790D28A3BE211`.
Documentation governance and `git diff --check` passed. User configuration,
firmware inputs and media were not modified.
