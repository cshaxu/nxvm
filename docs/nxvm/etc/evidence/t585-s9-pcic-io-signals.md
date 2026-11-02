# M5 T585 S9 — PC Card I/O interrupt signals

Ricoh EA-028-9804 printed pages 27–29, 34–35 and 63 describe I/O-card
IREQ, STSCHG and their independent selectable destinations. The chip receives
logical active-low-pin assertions, distinguishes I/O signal status from the
memory-card battery/ready meanings, and exposes the selected functional IRQ
separately from the status-change IRQ. MyPC110 uses two existing PIC sources
per socket so acknowledging a status change cannot deassert an independent
card IRQ, including when both share an ISA line. RI_OUT, actual I/O port
transport and a physical or emulated I/O card remain separate work.

Entering or reselecting a status input that is already asserted is modeled as
a status event (L2); no electrical waveform timing is claimed. Type changes
clear the unrelated latched low status bits rather than reinterpret a memory
battery error as an I/O status signal or vice versa. No Lib or Emulator source
is changed and no additional scheduler or host thread is created.

The initial actual-PIC test expected IRQ5 before the slave's IRQ10. The test
failed because the cascaded slave is on the higher-priority master IRQ2; it
was corrected to check the real PIC priority and acknowledgements. Chip and
actual Core/PIC focused tests then passed on x64 and x86, including shared and
separate IRQ destinations. Full receiving verification follows.

The slot interface accepts logical IREQ assertions; it does not yet interpret
the programmable electrical-polarity bit. Edge/level electrical timing and
RI_OUT wiring are not established. They remain follow-on device work rather
than being inferred from the successful PIC tests.

## Verification and release

Shared `c7c7d345c` is the reviewed chip/test change. Both cached full unit
suites passed 712/712 and both isolated-desktop registered integration suites
passed 48/48. The additional XT, AT and DeskPro profiles passed 3/3, 5/5 and
6/6 on each architecture. Native MIDI tests passed in this environment.
Physical I/O cards and interactive RDP were not tested.

All 14 executables were built and PE-width checked. The PC-family products
are published with the NXVM receiver. MyNES was byte-identical to its build;
MyRaidenII was equivalent to the stripped build except for PE timestamp and
checksum and did not link the changed component. Default NXVM and PC110
release bytes matched their corresponding builds. User configuration, ROM and
media were preserved.

| Artifact | PE | SHA-256 |
| --- | --- | --- |
| assets/nxvm/nxvm_0_5_0582_x64.exe | x64 | `F5E9AAD888648BED5C4689FE476D140193980187ED9977BCAD7DA2D526DBDC27` |
| assets/nxvm/nxvm_0_5_0582_x86.exe | x86 | `4FD7EBDE744BA542A74D41BBC8CF01B05FE87A36C18EC27DB1B0A50219ECC653` |
| assets/my5160/my5160_0_5_0582_x64.exe | x64 | `7F8EF077ACBF88E4EAAE6E40219D617D285937F2C7C15D48D9F5C68044DCEBB6` |
| assets/my5160/my5160_0_5_0582_x86.exe | x86 | `9C82660FB9FADC6EA7048D91E9A79E2F4A769095C5C33F2B65734C79A0B0B6A5` |
| assets/my5170/my5170_0_5_0582_x64.exe | x64 | `188E6ED01473A1A5BB0F34A6DA71287DF8B951A18DD97DAB10AD5A744F31D8C9` |
| assets/my5170/my5170_0_5_0582_x86.exe | x86 | `F6CD32EE4314720AB8BDBD90AF68C28C202CCD98B8FE88524F3797D0A46FA888` |
| assets/mydeskpro386/mydeskpro386_0_5_0582_x64.exe | x64 | `3EFC4E1457C2270C297B467F3E5D9F31E4E738B03A111D9AFE38BC09A89088C8` |
| assets/mydeskpro386/mydeskpro386_0_5_0582_x86.exe | x86 | `D89BC2325A1F56D7A6938B571086544B354C826F73EFE901EB883ACAD02EF09D` |
| assets/mypc110/mypc110_x64.exe | x64 | `9F4D59D36DF5F67EA96F01D3699EDA68782DD5FCE9EEFB9373883CFFB467E5D6` |
| assets/mypc110/mypc110_x86.exe | x86 | `B1836F0B4F08E441C9E6428568AF095C62C0A3FDC56CCF4847A4B152FB92E170` |
| assets/mynes/mynes_0_0_0044_x64.exe | x64 | `B499CD2583ECDDD1AE01EED882E21D49D20187558E2138858E5A27117CF74FC8` |
| assets/mynes/mynes_0_0_0044_x86.exe | x86 | `718617AF2FBF6D49C36AE3BB10043DC3B2383E35440E7A371E503B8BD4B4EA16` |
| assets/myraidenii/myraidenii_0_0_0007_x64.exe | x64 | `3F8C61E1630D85D23E06D44EA4FDF34C8E107DEEC53563AAB92DB82BE668033E` |
| assets/myraidenii/myraidenii_0_0_0007_x86.exe | x86 | `056607CC84588A475301FDB9DE98E162B700D46B64254E7BCB2A2B76049EFF92` |
