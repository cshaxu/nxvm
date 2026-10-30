# T582 S14 — local memory-route selection

Shared commit `e92409c90` collects address-matching provider indices in a local bounded array after the existing raw-overlay and selected-alias steps. Replacement, ordinary and fallback queries retain their previous priority and registration order. The provider registry limit is 64; the local index fits in `lib_u8`. Query is observational and the runtime owns the frozen route registry. No additional persistent state, public API, guest event or execution path is introduced.

The actual Shared diff is `memory.c` +17/-18, existing memory registration test +68/-1, and the two corresponding manifest hash updates. The test now exercises 64 overlapping matching providers, first/last precedence and mixed raw/replacement/ordinary/fallback winners after successive declines. Existing tests cover registration, failure reporting, inspection, aliases, CPU-only overlays and ROM routes. Six targeted tests passed after the final test edit on both architectures; the shared source and test manifests and Core dependency gate passed. This is not proof of every possible device callback behavior beyond the existing observational contract.

The existing cached x64 and x86 Release trees built all receivers. Each architecture passed 712/712 unit and 48/48 default integration tests; additional profile integration results per width were My5160 3/3, My5170 5/5 and MyDeskPro386 6/6. The full suite ran before the final test-only addition; that addition was subsequently rebuilt and its six affected tests passed on both widths. The MyRaidenII selected-set startup and product Console probes also passed both widths. Desktop/native tests registered in those suites are included in their results; interactive RDP use and real MIDI hardware were not separately verified.

Measured fixed-work and PC110 product-path comparisons are in [S13 route evidence](t582-s13-scheduler-observation.md): around 3.1% more guest ticks in one x64 252-second product comparison, with identical final sampled frame. This is a bounded gain, not evidence of realtime or interactive usability. No user INI, CMOS, external ROM, media or snapshot was changed.

## Receiver artifacts

The existing cached build/deployment path produced or reused the following artifacts; PE machine types and SHA-256 were checked. MyNES did not depend on the changed component and its artifacts remained byte-identical to the committed versions.

| File | Architecture | SHA-256 |
| --- | --- | --- |
| `assets/nxvm/nxvm_0_5_0582_x64.exe` | x64 | `B8CC247A431F0F2A55D5A569682F9C6B4106CC6137ED5418F1AAAA8AF133E79F` |
| `assets/my5160/my5160_0_5_0582_x64.exe` | x64 | `5B10A7C3D8113F16DE7F60DED3F82B0269A9E4D3FE4F88472A4E39F58D100CCA` |
| `assets/my5170/my5170_0_5_0582_x64.exe` | x64 | `50BA1607EB28A5F0848890B98D4614FC639A5911026F9F2CB41B16DEE09DEF20` |
| `assets/mydeskpro386/mydeskpro386_0_5_0582_x64.exe` | x64 | `B855C20C16522BCB2DEC95CEE8B111273281F9A5C6742DC21C7430CD65C3F164` |
| `assets/mypc110/mypc110_x64.exe` | x64 | `46BB4C915EE5933F7B7E812A4869CABCD3CADAC189E4449F866D2B694F144087` |
| `assets/myraidenii/myraidenii_0_0_0007_x64.exe` | x64 | `A80D82AD5EB4D95EA58AC1AC92B738E3D35C39AF8DC4517184023B5E09C4FDFA` |
| `assets/mynes/mynes_0_0_0044_x64.exe` | x64 | `B499CD2583ECDDD1AE01EED882E21D49D20187558E2138858E5A27117CF74FC8` |
| `assets/nxvm/nxvm_0_5_0582_x86.exe` | x86 | `A11F869A0FCDF80F6117C481FB6F723DE543541E4F635C63D298C7E85194A1EB` |
| `assets/my5160/my5160_0_5_0582_x86.exe` | x86 | `F1974A039E248C2D8F5F3B68A45F34FD65D23A8177C07AB79382BC39C39A9E3C` |
| `assets/my5170/my5170_0_5_0582_x86.exe` | x86 | `10DF7737A9E052AF4365BC0F646FC7106D9610C1371F8157F32C1A0505E25AA8` |
| `assets/mydeskpro386/mydeskpro386_0_5_0582_x86.exe` | x86 | `8A50C9568331ADB6FA960BC537CF31E920FAD57CB39671DE00BD4C8AB43A3588` |
| `assets/mypc110/mypc110_x86.exe` | x86 | `2D187D0789B2A84E0808EF21227E3666C902AEE45C7A422101406B720CB60C93` |
| `assets/myraidenii/myraidenii_0_0_0007_x86.exe` | x86 | `293646D92D83EE031CA1677A6DF867910685D54049F3C0FD914C3A38E6823FD0` |
| `assets/mynes/mynes_0_0_0044_x86.exe` | x86 | `718617AF2FBF6D49C36AE3BB10043DC3B2383E35440E7A371E503B8BD4B4EA16` |

Evidence SHA-256: `s14-qualification.log` `4463F6F92ED965709F910D8586715509BC986F0634515B5FFE4ED344F63CBC1C`, `s14-targeted-final.log` `007A9556BFC7E3327397FF649B112AC2DE51D57A4B14A8B9722CD68F77534D89`, `s14-external.log` `D829B6B9159B4FC435B77EA9664536C7FBF0141BD92E96C5ABA8C6E63833C585`, `s14-artifacts.json` `DC5D03318F57982BCA70E5B9EF23636D5E0F14600C1B8A7FCD4B2DDF4CA64A11`.
