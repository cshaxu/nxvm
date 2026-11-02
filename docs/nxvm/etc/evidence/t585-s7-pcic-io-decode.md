# M5 T585 S7 — Ricoh PC Card I/O-window decoding

Ricoh RB5C396 EA-028-9804 printed pages 18, 32, 36–37 and 42 specify two
per-socket I/O windows, inclusive start/stop ports, enable bits, control bits,
and a 16-bit address offset whose low bit is fixed zero. This change exposes
those register facts from the existing chip and computes card port and a span
that ends at the window or translated 16-bit wrap. The two sockets and both
windows are independently covered; zero/one wait flags and bus-width/source
flags are decoded but do not claim cycle-accurate bus timing.

The chip decoder is not a second port dispatcher. PC110 currently has no
populated I/O-card device or dynamic Core port-window receiver. Its existing
machine card-memory receiver has no user-facing attach command. Those are
explicit follow-on work; neither guest I/O card operation nor physical slot
compatibility is claimed by this chip-only step. Onboard CF remains ATA and
is not treated as a PC Card. Neither Lib nor Emulator is changed.

The initial focused build caught an accidental misplaced register-write branch;
the branch was corrected and the existing chip contract test then passed on
both widths. Full release and repository qualification follows below.

## Verification and release

Shared `5c54e8714` is the reviewed chip/test change. Complete cached unit
suites passed 712/712 on each host architecture; isolated-desktop registered
integrations passed 48/48 on each. Additional selected XT, AT and DeskPro
integrations passed 3/3, 5/5 and 6/6 respectively on each architecture.
No native RDP or physical card was exercised.

All 14 product executables were built and PE-width checked. The PC-family
artifacts changed and are included with the NXVM record. MyNES deployed bytes
match the build and remain unchanged; MyRaidenII matches its stripped build
apart from PE timestamp/checksum and remains unchanged because it does not
link the changed component. PC110 and default NXVM deployed bytes match their
final builds. The other PC profiles were deployed by their tested builds.

| Artifact | PE | SHA-256 |
| --- | --- | --- |
| assets/nxvm/nxvm_0_5_0582_x64.exe | x64 | `C498E36DC71A2811A7B9FA43DA27ABE54233B253040A3A8FCF4D8179C9E015C0` |
| assets/nxvm/nxvm_0_5_0582_x86.exe | x86 | `B7C9443BAEDB071C510796D70BFDBEA32B93A960DE26A6EB8DAC4F373B67924F` |
| assets/my5160/my5160_0_5_0582_x64.exe | x64 | `204C8D596D9E0DF2ED6095FD4DEF6EC0CDFB64F661F416036210AFEF6307B857` |
| assets/my5160/my5160_0_5_0582_x86.exe | x86 | `DE89D2977991F8030EB43F72418E72DC465E3A8657686A8DEAEEF0810AD5C332` |
| assets/my5170/my5170_0_5_0582_x64.exe | x64 | `83F6A23C58A77ABE5473BFC329B1B29ED6712D8FC9F5D92BFF222240E499DB92` |
| assets/my5170/my5170_0_5_0582_x86.exe | x86 | `7AB85CB1D37C0C47BD0767C3908CEA598CD7BE33DA3B2EF2B56030EE4D1C1527` |
| assets/mydeskpro386/mydeskpro386_0_5_0582_x64.exe | x64 | `0C8909CE121298886107881DB2566C3A6550E0C0210370C85ECCEE3E2D97C787` |
| assets/mydeskpro386/mydeskpro386_0_5_0582_x86.exe | x86 | `8B5A1D1F9D5D2ECE491F097B3881846C55A75013A1C6F4426AFC16B77005BEFD` |
| assets/mypc110/mypc110_x64.exe | x64 | `090C0AF27EA87C42320D0C04D72D82DEB81C11A69C9F8AFA6A60905C42C5D72A` |
| assets/mypc110/mypc110_x86.exe | x86 | `73B7B9DAE03FD4E975175B33FE2439A2BFDE643FE2C0595A8AE2340A104A2E4E` |
| assets/mynes/mynes_0_0_0044_x64.exe | x64 | `B499CD2583ECDDD1AE01EED882E21D49D20187558E2138858E5A27117CF74FC8` |
| assets/mynes/mynes_0_0_0044_x86.exe | x86 | `718617AF2FBF6D49C36AE3BB10043DC3B2383E35440E7A371E503B8BD4B4EA16` |
| assets/myraidenii/myraidenii_0_0_0007_x64.exe | x64 | `3F8C61E1630D85D23E06D44EA4FDF34C8E107DEEC53563AAB92DB82BE668033E` |
| assets/myraidenii/myraidenii_0_0_0007_x86.exe | x86 | `056607CC84588A475301FDB9DE98E162B700D46B64254E7BCB2A2B76049EFF92` |
