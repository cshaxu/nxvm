# Receiving Shared Core write-width update

NXVM M5 T580 S47 explicitly admits receiving MyRaidenII artifacts in its
cross-target packet. Shared P1 `d603e81e7` adds access width only to the existing
Core port write callback. MyRaidenII has no custom callback requiring a source
change, but links Core and therefore receives rebuilt artifacts. This is not
new MyRaidenII gameplay implementation or a new product qualification.

The original reusable x64/x86 Release Ninja graphs build the existing
`myraidenii-0-0-0007` target. Existing release stripping removes debug sections;
deployment preserves version 0.0.0007. PE headers identify x64 and x86 correctly.
All 13 registered MyRaidenII unit cases pass on each width as part of the
repository's current unit runs. No ROM-backed gameplay, new desktop/RDP or
real-time performance qualification is claimed. Product source, test,
configuration, ROM and snapshot inputs remain unchanged.

| Artifact | SHA-256 |
| --- | --- |
| `assets/myraidenii/myraidenii_0_0_0007_x64.exe` | `BCE9C72F4309DF835CDA4165ECEEDE243C9DAF74C68E18F8B3F3FD2091BF3CB6` |
| `assets/myraidenii/myraidenii_0_0_0007_x86.exe` | `04D58F7ACD03D3970921BFB9DB74594DC343F3C1010C1C0621C5CCB002A939E2` |

The NXVM evidence retains complete cross-target build and regression details.
This receiving record and artifact pair form a separate MyRaidenII-target P
commit; the PC family artifacts and App adapters are not mixed into it.
