# M6 T43 S7 Receiving Artifacts

MyNES-hosted Shared S7 repairs Common synchronous request failure handling,
Audio prefix accounting/cancellation and assembler label resolution. This
receiving-only NXVM delivery changes no product source or configuration.
The source baseline is S7 Shared P1 `38ed0f26f` over `f1ce6763e`; the receiving commit
follows it without source changes.

All four admitted profiles were configured and built in each Release width
using the existing `vm-0-5-0535` target, then the complete repository-only
unit target passed 335/335 in x64 (61.40 seconds) and x86 (61.15 seconds).
Each deployed developer artifact retains identity 0.5.0535; PE machine width
and absence of debug sections were checked. No new external-ROM integration
or manual boot acceptance is claimed.

| Artifact | Bytes | PE | SHA-256 |
| --- | ---: | --- | --- |
| `assets/nxvm/compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0535_x64.exe` | 1245516 | 8664 | `996259385CC7465CD32C22307BE5B6F788A526723D1958D46E5F8E8CFD74F880` |
| `assets/nxvm/compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0535_x86.exe` | 1414580 | 014C | `449E977CAEF71F38AFC4F23EF3FE11346D62F1A6F6B693FC4BD9F0693C8F9804` |
| `assets/nxvm/default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0535_x64.exe` | 1245004 | 8664 | `01D4DD3C02595EDC8F0942CD2D88EC0D6D76ACAEF61F6227F8BB2EF52E4BC19A` |
| `assets/nxvm/default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0535_x86.exe` | 1414068 | 014C | `C2E769FDB95AFD37AE837DD6F7EF3E2593B872E998DE75084DCA06F15ABE949E` |
| `assets/nxvm/ibm-5160-model-268-360k/nxvm_xt_0_5_0535_x64.exe` | 1245004 | 8664 | `D4075B9CC9579FC330BB9E92BEB0DF5AA3BB984BCFD9C2B817938D5A4D27ACDA` |
| `assets/nxvm/ibm-5160-model-268-360k/nxvm_xt_0_5_0535_x86.exe` | 1414068 | 014C | `9ED1C89427F972F78AAF7E4E2F5094067C9D6B2A56CF29B6E7341330DD0E37BE` |
| `assets/nxvm/ibm-5170-model-339-1200k/nxvm_at_0_5_0535_x64.exe` | 1245516 | 8664 | `C0428EF7A8C32CD63AAA98AE1A0849D110BA068F5B50B5829576A63128E78E71` |
| `assets/nxvm/ibm-5170-model-339-1200k/nxvm_at_0_5_0535_x86.exe` | 1414580 | 014C | `DF14C7274926B0661DB06C3538DAF35FC41C5D2E002BF65A49E3699CF7E4C66E` |

Build trees `build/t41-s8-nxvm-x64` and `build/t41-s8-nxvm-x86` remain for
open T43. Profile configuration was restored to default after the matrix.
The pre-existing default NXVM.ini edit remains excluded and untouched.
[Shared repair evidence](../../../mynes/etc/evidence/m6-t43-s7-shared-quality-repairs.md)
owns the finite repair batch, tests and recovery boundaries.
