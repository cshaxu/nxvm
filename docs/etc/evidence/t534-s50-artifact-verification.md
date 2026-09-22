# T534 S50 Artifact Verification

This supporting record proves the local developer artifacts for the current
T534 source revision. It does not define product architecture or release
policy; those remain in the principal authorities.

## Source And Gates

- Source commit: `e4c32a4910ed3321243a52deea7571f931f100b2`.
- Current target/version: `vm-0-5-0534` / `0.5.0534`.
- Repository-only unit: 333/333 at `-j8`.
- External integration: default 20/20, IBM 5170 3/3, XT 1/1 and Model 40 3/3.
- PE inspection: every `x64` file is `pei-x86-64`, every `x86` file is
  `pei-i386`; no artifact has a `.debug` section.

## Sole Deployment Matrix

| Profile | Architecture | Artifact | SHA-256 |
| --- | --- | --- | --- |
| `compaq-deskpro-386-model-40-1200k` | x64 | `nxvm_model40_0_5_0534_x64.exe` | `A3133121AAB1994014CE9147F0DE9EFCC6D72C72A9215252320BCBF7F0E5E7C8` |
| `compaq-deskpro-386-model-40-1200k` | x86 | `nxvm_model40_0_5_0534_x86.exe` | `0B989832FEAD4F72FFAE11DE8243899EAFB46C735CCA213C376E656C8EAB0F1F` |
| `default-pc-at-80386-1440k-hdd` | x64 | `nxvm_default_0_5_0534_x64.exe` | `283F67D5CA95FE1AE4CB4A779706DF728AE1030A9EFA65E3AE4D19AFB1A6E89C` |
| `default-pc-at-80386-1440k-hdd` | x86 | `nxvm_default_0_5_0534_x86.exe` | `3B4955FAD28A9B0FEE09E06178E3D3C91E04D62C046E6F8DBDBC4B6976D16B61` |
| `ibm-5160-model-268-360k` | x64 | `nxvm_xt_0_5_0534_x64.exe` | `D17937AC32894243BAB3979F2C0C72D789F813F18F4E7B8F278588866B49E3C9` |
| `ibm-5160-model-268-360k` | x86 | `nxvm_xt_0_5_0534_x86.exe` | `86FBFD447C603798A3953AA51BD7DBCC4D315AC73722FE13106550E28450C8B5` |
| `ibm-5170-model-339-1200k` | x64 | `nxvm_at_0_5_0534_x64.exe` | `B22F09F0A6888606CF0E03140CFB891783CAEEC5EE93279036D01F50808FB8D5` |
| `ibm-5170-model-339-1200k` | x86 | `nxvm_at_0_5_0534_x86.exe` | `9967969DFBEF73EA6BC1B421C4327104F93F59F5ED09F2548558379D84D965CB` |

Every table entry is located only beneath its corresponding ignored
`assets/binary/<profile>/` directory, with the generated adjacent `NXVM.ini`.
