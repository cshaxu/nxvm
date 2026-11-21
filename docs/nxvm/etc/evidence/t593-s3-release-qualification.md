# T593 S3 release qualification

The [isolated evidence](t593-s3-clock-microbench.md) records the retained
Core deadline and timeline simplifications and exact RTC and Turbo changes.
Production is +49/-42 lines, the existing scheduler regression is
+22/-6, and the three manifest updates replace six hash entries. No second
scheduler, memory routing change or public ABI change was retained.

## Functional qualification and release identity

Cached GCC Release builds and publication passed for sixteen EXEs across
eight Apps. x64 passed 717/717 units and 48/48 integrations. x86 passed the
same 717/717 and 48/48. The registered integration suites include the PC110
firmware checkpoint and desktop and audio fixtures. Both actual published
MyOB300 EXEs showed the Monitor and Ctrl+Alt+O help in a private desktop,
with clean exit. These startup probes do not exercise native RDP or
power switching. Each deployed PE width and SHA-256 was checked.
The default NXVM pair matches the post-integration build used by CTest.

## Original DOS once per width

Each run used the production Release archives, the established diagnostic
observer, the same original System 1.0 ABA Card and derived read-only DOS
fixture as S2. It reached the real DOS prompt and printed CONFIG.SYS after
DIR before returning to the prompt. Each disposable guest medium was
removed after the process exited. The preserved owner disk and System Card
were not modified.

| Width | S2 initial DOS | S3 initial DOS | S2 DIR | S3 DIR | Cold target | DIR target |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| x64 | 77.637 s | 90.016 s | 5.938 s | 7.432 s | 60 s | 1 s |
| x86 | 92.398 s | 87.457 s | 7.965 s | 6.828 s | 90 s | 1 s |

The x64 run was slower and the x86 run was faster than the earlier separate
S2 samples. The replay is one sample per width, not an interleaved S2/S3
performance comparison. The timer counters establish reduced work on
this workload, not a whole-machine speedup. During DIR, x64 invoked its
advance callbacks 3,433,071 times with 3,388,935 deadline observations.
x86 invoked its advance callbacks 3,435,539 times with 3,391,711 deadline
observations. The deadline-query reduction in this workload is around
1.3 percent of these advances, even though the isolated one-tick path is
substantially cheaper. A major recorder or timing rewrite cannot be
justified by extrapolating that microbenchmark.

Functional publication is qualified. Both widths still miss the DIR
target and x64 misses the cold target even at the initial prompt.
The x86 initial prompt satisfies 90 seconds. The full cold-through-VER
gate and sustained native input/redraw were not remeasured in these two
runs. The current criterion is unchanged. Keep T593 open for the remaining
CPU execution/time-selection investigation and usability qualification.

## Final artifacts

| Deployment under assets | PE machine | SHA-256 |
| --- | --- | --- |
| nxvm/nxvm_0_5_0593_x64.exe | 8664 | `cd0a25a29c08c769203fa1e8025678466bfe0c3e12bf427c342647ebb8c404da` |
| nxvm/nxvm_0_5_0593_x86.exe | 014C | `b87f2ebcb7005c5c56da41f42f8be79f87e9ebf247b4111926e3428e13381ada` |
| my5160/my5160_0_5_0593_x64.exe | 8664 | `3da4e3e66d5c0f0fa5536a0dba292ef8d2fd0a4c8025fcd3bf5c87a0a44393d7` |
| my5160/my5160_0_5_0593_x86.exe | 014C | `cac9868cb862f4d7942ab0467e4736e2385e7614149b3a5752fbdb7acca8f81f` |
| my5170/my5170_0_5_0593_x64.exe | 8664 | `d606e1c9f3de49105106072d993b17f11b87a2483c733daa324a223e8f5dd6a5` |
| my5170/my5170_0_5_0593_x86.exe | 014C | `03a63858d271f290feae077dab9d4fef015bfee953ecc0985979107c4e7c92a1` |
| mydeskpro386/mydeskpro386_0_5_0593_x64.exe | 8664 | `91407910d88782c7f74226a17802fd84a880a63031e0cbdd49556b1aba2c78f8` |
| mydeskpro386/mydeskpro386_0_5_0593_x86.exe | 014C | `ad6cd130779a9f90e7e28b59b1a3216f64816e460c3578348db8bf8f4c3634ea` |
| mypc110/mypc110_x64.exe | 8664 | `c2b895ecdc1c9c82e5371af7615a6b36cfbe0f5af9804d4f90437630cd195780` |
| mypc110/mypc110_x86.exe | 014C | `ecbcc99d2efd271214f69c2f58b6eee94d680160df84164a629ff3fbaf3a1d5c` |
| myob300/myob300_0_5_0593_x64.exe | 8664 | `fc60f43f0902fa4090c98b9f67c360cd89d2b9fd1e94acbb70cefe68eabc7003` |
| myob300/myob300_0_5_0593_x86.exe | 014C | `b1a2a09a3976218ca8fe260c257f0c51c786fb69618833f68ecc2271bf23cbbe` |
| mynes/mynes_0_0_0044_x64.exe | 8664 | `7e42a1d63a9aab50c28a056673af59540fe4f16aa690fb02b7ef15ea68c059d3` |
| mynes/mynes_0_0_0044_x86.exe | 014C | `f8781d95bf9ad47c28b555374dfd626a0f85fc285769b4e01786fea516657ab1` |
| myraidenii/myraidenii_0_0_0007_x64.exe | 8664 | `8b52393e004fade1e82a8206768d4ed2241026a52390da897c27657ff051c743` |
| myraidenii/myraidenii_0_0_0007_x86.exe | 014C | `10b8f55b7ec33fa525c62195e0362fb9f92b4a638bb7e73e59c4a32be2883a3b` |

## Transcript identities

| Run | SHA-256 |
| --- | --- |
| x64 unit 717 290.28 | `34c5ebac3b0663637d39cbad86370db4badc0f5106462390cca1d0322e91c111` |
| x64 integration 48 257.93 | `2436db1dfe1044d7f72b39a00ab81da30078dc63b0d2fe310236b061b55a40b7` |
| x64 F1 42489 DOS 90016 DIR 7432 | `c35c1772364b3ae05d8fb128236f4243f1af4dbab0c8c88401118e37ddd00696` |
| x64 native startup pass | `88141733f33150d644dadac7dd696ade99d55438bc0ae04ac2ef4b4d4381e672` |
| x86 unit 717 112.96 | `d91535c99ad88534b18c957dfdcc447f4fc521806972e4ab5508210c5b561cfb` |
| x86 integration 48 275.58 | `d8549e3a662e2436b37fa0d2fbc2eb4982352f27b0dcfec05a5017af0e120fd1` |
| x86 F1 43213 DOS 87457 DIR 6828 | `d4fbdc8b93464197e69b8250b9bd17244701c0dac9535a0bfff18516c6122224` |
| x86 native startup pass | `88141733f33150d644dadac7dd696ade99d55438bc0ae04ac2ef4b4d4381e672` |
