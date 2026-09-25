# M6 T43 S10 NXVM Receiving Delivery

MyNES-hosted S10 explicitly admits Shared, NXVM and MyNES receivers. Shared
source is 75099c178. This NXVM delivery rebuilds existing 0535 fixed products
against byte-sized lib_bool; it does not open or close an NXVM task.

Two composition-test substitutes now declare lib_bool, matching their public
Common API instead of the formerly equivalent lib_i32 alias. No product runtime
or INI content changes. Source/test delta: +2/-2. All 335 repository-only units
pass on x64 (18.96 seconds) and x86 (27.15 seconds), including Shared units.
The final runs isolate desktop use from other suites. An earlier concurrent
native-modal failure is recorded in the hosting S10 evidence, not suppressed.

All four profiles were configured and linked on each width using the current
vm-0-5-0535 Release target. PE machine values and absence of .debug/.zdebug
sections were checked for every deployed EXE. These are current developer
artifacts, not new external-ROM integration qualification. INIs and runtime
debugger remain intact. Both receiving build trees return to the default profile.

| Artifact | Bytes | PE | SHA-256 |
| --- | ---: | --- | --- |
| `assets/nxvm/compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0535_x64.exe` | 1244499 | 8664 | `240F6EDD5DD3DDF11FBF61EE4D2A24AE6EEF90D697520E5480201362607C7041` |
| `assets/nxvm/compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0535_x86.exe` | 1413051 | 014C | `F1366919B871073BF26E2FC6569673C98C657E2397B0F344DFD25919880EAE39` |
| `assets/nxvm/default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0535_x64.exe` | 1243987 | 8664 | `1EACB4631BD35078235250BB173ADC895A7710851F882B662D4C7647D390B65D` |
| `assets/nxvm/default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0535_x86.exe` | 1412539 | 014C | `A888DE2590394BD087010F5810404B1A72C7ED78C7479F37CA1DCEDC6760645C` |
| `assets/nxvm/ibm-5160-model-268-360k/nxvm_xt_0_5_0535_x64.exe` | 1243987 | 8664 | `B6C66A7372ED989B94E31DD575CD9AC1705E845966CF1C3131A3C5C36A2CB557` |
| `assets/nxvm/ibm-5160-model-268-360k/nxvm_xt_0_5_0535_x86.exe` | 1412539 | 014C | `FD6BAB96C93F08ACFA2AC9BA06BBE4327537F1A1B9DA0AF0DC879960CECC73F7` |
| `assets/nxvm/ibm-5170-model-339-1200k/nxvm_at_0_5_0535_x64.exe` | 1244499 | 8664 | `8FAAEB7BFD66CD9A4104BDC0A06F3B6199D4CEEC468721ECD7E84F7A6E2B3B80` |
| `assets/nxvm/ibm-5170-model-339-1200k/nxvm_at_0_5_0535_x86.exe` | 1413051 | 014C | `F12C2CBFFAFE876C6C4013E1B1F017EF1C67A6956A9FF6506B641EE438962CD5` |
