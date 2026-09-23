# T535 S4 Artifact And Integration Verification

This record verifies the final convergence of the committed SoftPC shared
corpus import. The copied roots are compared to the frozen upstream revision
`1c5a47146dd4fd87b09423b7a7b960becb50cd67`; no dirty upstream media is read.

## Source And Gates

- Current target/version: `vm-0-5-0535` / `0.5.0535`.
- The NXVM aggregate now builds every CTest-registered shared target. This
  adds the three canonical audio targets that the imported Lib test CMake file
  registers: `audio_stream`, `audio_native`, and `audio_win32_platform`.
- Repository-only unit: 336/336 pass at `-j8`.
- External integration: default 20/20, IBM 5170 3/3, XT 1/1 and Model 40
  3/3 pass.
- Each artifact is optimized Release, is stripped of `.debug` sections and
  is deployed only under its profile's ignored `assets/binary/` directory.

## Exact Imported Corpus

Relative path and SHA-256 comparisons against SoftPC are exact:

| Root | Files |
| --- | ---: |
| `src/lib` | 97 |
| `src/common` | 23 |
| `src/x86` | 15 |
| `test/lib` | 49 |
| `test/common` | 20 |
| `test/x86` | 10 |

## Integration Matrix

| Product profile | Result |
| --- | --- |
| `default-pc-at-80386-1440k-hdd` | 20/20 |
| `ibm-5170-model-339-1200k` | 3/3 |
| `ibm-5160-model-268-360k` | 1/1 |
| `compaq-deskpro-386-model-40-1200k` | 3/3 |

## Sole Deployment Matrix

| Profile | Architecture | Artifact | PE | SHA-256 |
| --- | --- | --- | --- | --- |
| `compaq-deskpro-386-model-40-1200k` | x64 | `nxvm_model40_0_5_0535_x64.exe` | `pei-x86-64` | `577F23F83837B00BD71711A4179798703BA09BF7C5D5DC14575E2D5DFFA7BCC8` |
| `compaq-deskpro-386-model-40-1200k` | x86 | `nxvm_model40_0_5_0535_x86.exe` | `pei-i386` | `E91D09C7E6E1B0C83B7716F2D0B56B44E411C6A551DDEB8B130E59ADFFC885C4` |
| `default-pc-at-80386-1440k-hdd` | x64 | `nxvm_default_0_5_0535_x64.exe` | `pei-x86-64` | `DA886B830DA666111239053E2A1394219B2744FCB01524085BD4F9AEDBC5AF72` |
| `default-pc-at-80386-1440k-hdd` | x86 | `nxvm_default_0_5_0535_x86.exe` | `pei-i386` | `D7B92C3E6B9582CC745C06721EE792B4FB6A96DD917764CD50D096A31A31EFF5` |
| `ibm-5160-model-268-360k` | x64 | `nxvm_xt_0_5_0535_x64.exe` | `pei-x86-64` | `31CA055FB437BED27816A1EF3C014F9C4BB81838E6CF1BF051523A7FDF3C4910` |
| `ibm-5160-model-268-360k` | x86 | `nxvm_xt_0_5_0535_x86.exe` | `pei-i386` | `448D95D8D20221568A208603BFFD17FE578CE8E05786D3879B86F5D3967DDBE3` |
| `ibm-5170-model-339-1200k` | x64 | `nxvm_at_0_5_0535_x64.exe` | `pei-x86-64` | `72663DC3184D1E61F605AE691E91893AA88FA4CA564295923CFC63F2BB074CAE` |
| `ibm-5170-model-339-1200k` | x86 | `nxvm_at_0_5_0535_x86.exe` | `pei-i386` | `BEF9ABBDFEAAF6B18C9504370F9EE04D2AEA05B9D7FB599E1E63F1F700CEAC19` |

Every listed file has zero `.debug` sections. Each profile directory also
contains its generated adjacent `NXVM.ini`; no artifact is deployed elsewhere.
