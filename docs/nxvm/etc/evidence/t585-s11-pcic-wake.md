# T585 S11 — PC Card wake reconciliation

Ricoh RF5C296/RF5C396L/RB5C396 manual EA-028-9804, June 1995,
SHA-256 `7eb037b495d7ed3f4334a1cdc6c6582c8fa9ceadf0fd680ebadec2575b60c7ec`,
printed pages 34–35 and the interrupt chapter describe RI_OUT# for an
enabled card-detect change and I/O-card ring input. Previously selection of
card-detect resume removed that status change from IRQ routing without exposing
the alternative output. The Ricoh model now computes one logical output from
the two sockets and the existing latches, enable bits and I/O signal. No
second interrupt latch, scheduler, thread or clock was introduced.
The reviewed Shared source and test change is `d9b93a28d`.

PC110 feeds a rising card-wake indication into its existing chipset wake
path. This board connection is an L1 functional model: the available PC110
references establish a chipset ring-resume capability but do not prove the
physical RI_OUT# wire or electrical timing. F12 remains an independent input;
a held card signal cannot hide a later F12 press, and a card event is not
hidden by a held F12. The existing chipset continues to own the processor
reset and SMI continuation, rather than treating an Emulator pause as guest
suspend. No claim is made that a physical card or guest APM is qualified.

The existing chip and PC110 fixtures exercise both sockets, card-detect and
I/O-ring selection, acknowledgement, the actual Core/PIC/PC110 receiver,
and the separate F12/card wake edges. The source and test manifests are
updated. Lib and Emulator are unchanged. Actual card hotplug while a guest is
running, real I/O-card transport, physical wake wiring, battery/APM and
interactive RDP remain separate qualification items.

Both cached full unit suites passed 712/712; isolated-desktop registered
integrations passed 48/48 on each architecture. Additional XT, AT and DeskPro
integration rows passed 3/3, 5/5 and 6/6 respectively on each width. MyRaidenII
also completed its bounded two-million-instruction selected-set probe and
deployed Console lifecycle probe on each width; these do not establish gameplay
or audio correctness. Documentation governance and actual-diff checks passed.

Both cached product graphs built all seven products; the following deployed
artifacts were checked for PE machine width and hashed. MyNES build outputs
are byte-identical to the deployed files. MyRaidenII build outputs, after
stripping, match their deployed files apart from PE timestamp and checksum;
neither product links the changed X86-NXVM component.

| Artifact | PE | SHA-256 |
| --- | --- | --- |
| assets/my5160/my5160_0_5_0582_x64.exe | x64 | `0213579468489C9B2E95A15BD316903555C7047BA967F21B7B0E8296D3BD3B12` |
| assets/my5160/my5160_0_5_0582_x86.exe | x86 | `D18E5142CA50B4C65B485C32A21FD8032719AC7481F74B9EA7D56C3ABA5FBE4F` |
| assets/my5170/my5170_0_5_0582_x64.exe | x64 | `6883DF9B83BADE033150EEF81E8EFFD6E419AB41FD6409AEC18A5D57EEF53C79` |
| assets/my5170/my5170_0_5_0582_x86.exe | x86 | `E48F0EC4A368A46044743A7510C364A143E4C56CDBAD6F4B39408FEA352DAC13` |
| assets/mydeskpro386/mydeskpro386_0_5_0582_x64.exe | x64 | `685A53AA8B45787FDBCD3DCF1273A0F78BBB47D23D25E07596EC4E50CBB08BA6` |
| assets/mydeskpro386/mydeskpro386_0_5_0582_x86.exe | x86 | `50A7D61548FC76395D557AAA388597E77C81C179489A6A3A72FD201CB0399EA5` |
| assets/mynes/mynes_0_0_0044_x64.exe | x64 | `B499CD2583ECDDD1AE01EED882E21D49D20187558E2138858E5A27117CF74FC8` |
| assets/mynes/mynes_0_0_0044_x86.exe | x86 | `718617AF2FBF6D49C36AE3BB10043DC3B2383E35440E7A371E503B8BD4B4EA16` |
| assets/mypc110/mypc110_x64.exe | x64 | `85D892A493302B1C6FCC5FEADBBD7951B3665845D98BEE0BF70FFCC4FF8BD6E0` |
| assets/mypc110/mypc110_x86.exe | x86 | `FAEC1D969179B4DCDDCDB4819B393F45F6906D9566C2FC9D88DF091C5A019967` |
| assets/myraidenii/myraidenii_0_0_0007_x64.exe | x64 | `3F8C61E1630D85D23E06D44EA4FDF34C8E107DEEC53563AAB92DB82BE668033E` |
| assets/myraidenii/myraidenii_0_0_0007_x86.exe | x86 | `056607CC84588A475301FDB9DE98E162B700D46B64254E7BCB2A2B76049EFF92` |
| assets/nxvm/nxvm_0_5_0582_x64.exe | x64 | `A6A6CC475C830C048AD5D8524F566C7C36A2FBE4E2EF2BFF91157E870E55B2B8` |
| assets/nxvm/nxvm_0_5_0582_x86.exe | x86 | `DB4B2A3DD6F54359905F71453035842F5D7F28349E43BFE12CC4BD2DB1C325C8` |
