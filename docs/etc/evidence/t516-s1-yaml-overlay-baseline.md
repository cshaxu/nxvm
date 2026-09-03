# T516 S1 YAML Overlay Baseline

## Frozen Inputs

All paths below are resolved relative to their session document in
`assets/sessions/`.  The files remain outside this repository, apart from the
shared checked-in font.  `size: SHA-256` identifies each distinct input.

| Input | Identity |
| --- | --- |
| `default-pc-at-1440k.rom` | `65536: 3AA176FCEEA6AD25EB7B34FDCFDEC3FC790DEF9350CBF45F58A458D3A6CBF199` |
| `ibm-5160-16aug82-logical-f0000-fffff.rom` | `65536: BECFF9B23527E20CFB48C7362F315FD099723E9D1E371D2A782A1640A54E1A0A` |
| `ibm-5170 U27 / U47 ROM` | `32768: 3B921A451D78EFCD91232A01CD6936D6BA12DAD4374466655AFF8EF3A5DC764E` / `32768: AEC1759A8245D50BE93C9B081AB1AC7DE58DD8141F603C7745AE66D2D4C29E4D` |
| `DeskPro even / odd ROM` | `16384: 15FA21FE2B57970F4223DD15996B28983E726F42DC5B508226327F44099F8C41` / `16384: 00E5C4F74C7BAABC283B996AF2CF9D8538CAB52B16C8758CB96B136935B42762` |
| `DeskPro EGA ROM` | `16384: E3CE21B0B6C23E519D11568710EA59D669AB7167F75ECE0DD68157E4508BB0D7` |
| default PC/AT CMOS | `64: 8AD020B69B6590970E1F8669AD2B27EF0E297F308F1A3AA6970A00A0971F3082` |
| IBM 5170 CMOS | `64: B1BD8F855A7DCE0018775992AA4C577D6D177B96C84B89E74DFBF4A67DA0B582` |
| DeskPro CMOS | `64: A5120BAF675687727E7E9036BA487834F1CD720119FF264D5DE5A8EEC42D1011` |
| CP437 font | `4096: C9631BDB9FB00E02907CE4BCEAACD5E33C3D70D431CDD92309635218A69C6337` |
| 360 KB DOS media | `368640: DE271368874209C07A2FC25C81C17529D4BDD7718B2731B86C49A2DA923A256E` |
| 1.2 MB DOS media | `1228800: 0F51D92B482253FC468A2B470FFAB82DB43898D1C8B44E504808B7A3EF3D4BDE` |
| 1.44 MB DOS media | `1474560: FADEB3A27C6A0E1CF582DDE0B9AECB7E5D30678F2F967F2F4562F167CC0CB1D5` |
| 50 MB Windows media | `51609600: 61E5CDC0B76151CC65B73EB44094738B9DE86052B1B07F20FC03205984CD77E1` |
| DeskPro 40 MB media | `40256000: 2BBC68E612A72290A5181E070494A7580CBECE1F528F92F009A269DC05819E73` |

## Boot Ledger

| YAML row | Result and shared observation |
| --- | --- |
| `default-pc-at-80386.yaml` | Passes its declared floppy terminal. |
| `ibm-5160-model-268.yaml` | Passes its declared floppy terminal. |
| `ibm-5170-model-339-360k.yaml` | Times out halted at `F000:05C5`; no PIC pending request, PIT1 active and Core reports a deadline. |
| `ibm-5170-model-339.yaml` | Same `F000:05C5` halt/disposition with 1.2 MB media. |
| `compaq-deskpro-386-model-40.yaml` | Times out executing at `F000:F618`; FDC has command `04h`, KBC output `D1h`, and Core reports a deadline. |

The 5170 pair therefore has one media-independent early-POST divergence.  No
profile-specific repair is selected in S1.

## One Overlay Route

`integration_yaml_session_open_with_overlay_transform()` now opens the exact
catalog request first.  VM reads the YAML-declared external media into its
sole in-memory device backing; integration setup reads and writes that backing
through the existing Core media registry.  Restart repeats the same overlay
setup against a fresh session.  Neither the request paths nor any source asset
are rewritten.

Focused proof ran the five registered injected-media integrations successfully:
`vm-cga-graphics-dos-smoke`, `vm-ega-planar-dos-smoke`,
`vm-mouse-driver-dos-smoke`, `vm-fdc-read-track-dos-smoke`, and
`vm-ata-pio-dos-smoke`.  A before/after SHA-256 check around the CGA test left
the 1.44 MB source hash unchanged.  The strengthened YAML boundary verifier
rejects `CopyFileA`, the former `session-*.img` names, and the old request
transform entry point anywhere below `test/integration/`.

The complete repository-only unit run started 302 tests and recorded 302 pass
markers.  The retained `LastTestsFailed.log` names only the three known external
boot rows above, not a unit test.
