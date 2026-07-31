# M3 Developer Artifacts

The owner requested a local executable after every completed M3 subtask. The
following ignored `build/output/` files were copied from GCC outputs built at
M3 closure commit `da1c7d6c90f34e918b0cd95605e8ae81f300fec5`. They are
task-specific smoke replay artifacts, not release or product binaries and not
byte-identical historical builds of each earlier commit.

| Subtask | Local artifact | SHA-256 | Invocation |
| --- | --- | --- | --- |
| T1 S1 | `ntvdm64-m3_t1_s1.exe` | `3faba0e91ebf9ac59a4f2d1548d65d0f60883c8528aa363619edb2e2c5486372` | no arguments |
| T1 S2 | `ntvdm64-m3_t1_s2.exe` | `27493779d6fc034d4a191e2d7fbb6a0408a82d415e341bfd668b6516e2851a34` | no arguments |
| T2 S1 | `ntvdm64-m3_t2_s1.exe` | `104a4dbfd44ca9f0e235c196ecc7d1fb112349626d2868a1c47ff2a30722e2c5` | no arguments |
| T2 S2 | `ntvdm64-m3_t2_s2.exe` | `d16e7c578f47c6a7999732f63d9f9c21aee06445ef6bb04d2730eb48e8d95775` | no arguments |
| T3 S1 | `ntvdm64-m3_t3_s1.exe` | `25f06311fe1d015ba7d51a57601b46fd021dbe9cfbe73d42bcb5858dc09dd68a` | `<fdd.img> <hdd.img>` |
| T3 S2 | `ntvdm64-m3_t3_s2.exe` | `2b8ec7f21f0c8913d78410d64a310d8e93f96f9d7ccd5891c67623a2717e2f00` | no arguments |
| T4 S1 | `ntvdm64-m3_t4_s1.exe` | `6f48f25700dea8254087567776b86cd86c227c4debb12de574ce6ebd1deb6063` | no arguments |
| T4 S2 | `ntvdm64-m3_t4_s2.exe` | `bf2c8c57b601916f04bd9630594bf4dfb835f76194785356673fd6c210830fb9` | no arguments |
| T5 S1 | `ntvdm64-m3_t5_s1.exe` | `24b701535392e3c1306b7fe5ffe424f3fdce37bed5bb6dfbad009f76aa9218a3` | `<fdd.img> <hdd.img>` |

All artifacts follow the pre-cutover rule: they are developer verification
tools and do not claim the future Virtual DOS Machine product identity.
