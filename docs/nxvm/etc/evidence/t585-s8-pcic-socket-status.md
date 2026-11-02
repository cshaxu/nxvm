# M5 T585 S8 — PC Card ready and battery status

Ricoh EA-028-9804 printed pages 28–29 specify the battery-pin combinations,
ready low-to-high latch, and status-change bits. Pages 33–35 define the
interrupt masks and acknowledgement. Both sockets now latch ready transitions
and memory-card battery warning/dead transitions through the existing status
register and PIC source. A virtual memory medium supplies asserted BVD pins
rather than falsely reporting a dead battery. I/O-card mode masks the
memory-only ready/warning status bits; I/O-card STSCHG/RI and wake signals are
not implemented by this change.

Battery change detection on entering a condition while the same card remains
present is an L2 operational model of the documented states, not a measured
physical power model. No claim is made for a battery-backed physical card.
Tests exercise both sockets, status masking and acknowledgements in the chip,
and a PC110 Core port → PIC ready interrupt. The owner retains the original
firmware, INI, disks and external media unchanged.

Mode-switch review also found that an unacknowledged memory battery-dead bit
must not become an invented I/O-card STSCHG interrupt. While I/O-card STSCHG
is unmodeled, memory-only status-change bits are masked in that mode. The
interface-status pin still lacks a distinct I/O STSCHG input and is not claimed
as a complete I/O-card signal model. The regression switches modes with the
battery latch pending and checks both status and selected IRQ.

The initial PIC test allocated a second machine solely for the new assertion.
It was simplified to reuse the existing PC110 fixture and exercise the same
Core-port/PIC route. The focused test passed on both architectures; final unit
suites are being repeated after this fixture-only cleanup. The completed
integration runs use the unchanged production code and do not depend on that
unit fixture.

## Verification and release

Shared `0c96c0b30` is the reviewed status-model change. Final unit suites,
including the simplified existing PC110 fixture, passed 712/712 on x64 and
x86. The registered integration suites passed 48/48 on both architectures;
they ran before the unit-fixture-only cleanup and use the same production
code. Additional XT, AT and DeskPro profile integrations passed 3/3, 5/5
and 6/6 on each architecture. No physical card or interactive RDP test is
claimed.

All 14 executables were rebuilt and checked for PE width. PC-family outputs
are delivered with the NXVM record. MyNES bytes match the build unchanged;
MyRaidenII matches its stripped build except PE timestamp/checksum and is not
changed because it does not link this component. Default NXVM and PC110
release bytes matched the corresponding build products. User INI, ROM and
media were preserved.

| Artifact | PE | SHA-256 |
| --- | --- | --- |
| assets/nxvm/nxvm_0_5_0582_x64.exe | x64 | `D43F0DDCE6F58C5A20B8766C730DCDF1CC7A0FAFB4CEFFF0F7CF081FB95E8513` |
| assets/nxvm/nxvm_0_5_0582_x86.exe | x86 | `3C41093213F80B8942C5788E4CB0E7056434067EBD02B0F19EF0DC620FF8E411` |
| assets/my5160/my5160_0_5_0582_x64.exe | x64 | `7120308D87B0F1B24E96273841A3A3D3EB9D0B834383A31A8BC5CEE2DA2FA3B4` |
| assets/my5160/my5160_0_5_0582_x86.exe | x86 | `DB6FE7DC437123FC6C94AE45E6872230A560DF91224D5704A13B296430155935` |
| assets/my5170/my5170_0_5_0582_x64.exe | x64 | `674BC5637D59AD11081C725476BEA36446C5FF06EA6A37F34234843B24CE8E52` |
| assets/my5170/my5170_0_5_0582_x86.exe | x86 | `C741684EED72743F92BB2BABD0F4702EFC4625EE6C0306217539D390C58DB69A` |
| assets/mydeskpro386/mydeskpro386_0_5_0582_x64.exe | x64 | `49E707D0F7A259AEE8C0A0583EBC0B0F10559C79F04D6773A9695D66B2AA0D70` |
| assets/mydeskpro386/mydeskpro386_0_5_0582_x86.exe | x86 | `46D7DE61F64FE129D27BCB10840520666076B030CEC5CE35983624034C2928CB` |
| assets/mypc110/mypc110_x64.exe | x64 | `1E8F475B7E80DBBBA41ED657163F2BD6630E89AB49AB8120674506FE4B2CD874` |
| assets/mypc110/mypc110_x86.exe | x86 | `30E7CA06DED1C773F5A5044ADF76EA90966E451E63D638E051AB482483145D79` |
| assets/mynes/mynes_0_0_0044_x64.exe | x64 | `B499CD2583ECDDD1AE01EED882E21D49D20187558E2138858E5A27117CF74FC8` |
| assets/mynes/mynes_0_0_0044_x86.exe | x86 | `718617AF2FBF6D49C36AE3BB10043DC3B2383E35440E7A371E503B8BD4B4EA16` |
| assets/myraidenii/myraidenii_0_0_0007_x64.exe | x64 | `3F8C61E1630D85D23E06D44EA4FDF34C8E107DEEC53563AAB92DB82BE668033E` |
| assets/myraidenii/myraidenii_0_0_0007_x86.exe | x86 | `056607CC84588A475301FDB9DE98E162B700D46B64254E7BCB2A2B76049EFF92` |
