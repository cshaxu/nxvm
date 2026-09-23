# T534 S52 Artifact Verification

This supporting record proves the local developer artifacts for the current
T534 source revision. It does not define product architecture or release
policy; those remain in the principal authorities.

## Source And Gates

- Source commit: `f52eb5ef`.
- Current target/version: `vm-0-5-0534` / `0.5.0534`.
- Repository-only unit: 333/333 at `-j8`.
- External integration: default 20/20 serial, IBM 5170 3/3, XT 1/1 and Model
  40 3/3. One preceding default `-j4` run had a timer-firmware test failure;
  it passes isolated and in the serial complete route, but remains an admitted
  parallel-stability finding rather than being represented as green at `-j4`.
- PE inspection: every `x64` file is `pei-x86-64`, every `x86` file is
  `pei-i386`; no artifact has a `.debug` section.

## Sole Deployment Matrix

| Profile | Architecture | Artifact | SHA-256 |
| --- | --- | --- | --- |
| `compaq-deskpro-386-model-40-1200k` | x64 | `nxvm_model40_0_5_0534_x64.exe` | `471216DC99F9D893B9D078FA7F400FF51F3A8FA14C7A85BE2950BF33E5EA082B` |
| `compaq-deskpro-386-model-40-1200k` | x86 | `nxvm_model40_0_5_0534_x86.exe` | `985B3A2792A21134B81F08A348C53AA72EA3AB73E2BB33392242A69693F7DB16` |
| `default-pc-at-80386-1440k-hdd` | x64 | `nxvm_default_0_5_0534_x64.exe` | `84DD93E93AEFC0EE5A0C89218892B4847FA3412347568D4F5E7E5C92794C078D` |
| `default-pc-at-80386-1440k-hdd` | x86 | `nxvm_default_0_5_0534_x86.exe` | `8824D5DAF58025204AF4A718E6FDD5FD11613B548BA1373CD9961AE26446FC10` |
| `ibm-5160-model-268-360k` | x64 | `nxvm_xt_0_5_0534_x64.exe` | `0CC439D3B727596D0ACD000A841FA8188002C76AA7D978D671BF89F6D8F82E8A` |
| `ibm-5160-model-268-360k` | x86 | `nxvm_xt_0_5_0534_x86.exe` | `4F9E3A184DCE92594F20AA1C669EA6E72AB4B267534144D42DFC9A2369F853C2` |
| `ibm-5170-model-339-1200k` | x64 | `nxvm_at_0_5_0534_x64.exe` | `4A0E6043D2621CAC3B3B925744EC2276782B0C5CE9C61AD1AF063D74EACBD1E2` |
| `ibm-5170-model-339-1200k` | x86 | `nxvm_at_0_5_0534_x86.exe` | `20A309A4E2B213D535641C6F1977D1F4D0EE1D11837540259F2A5168704A955F` |

Every table entry is located only beneath its corresponding ignored
`assets/binary/<profile>/` directory, with the generated adjacent `NXVM.ini`.
