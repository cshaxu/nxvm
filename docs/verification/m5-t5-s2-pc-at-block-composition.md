# M5 T5 S2 PC/AT Block Composition Verification

GCC built and ran `nxvm-product-pc-at-smoke` with the owner-supplied local FDD
and HDD paths. It emitted `M5:T5:S2:NXVM-PC-AT:OK`, configuring both frozen
providers and observing `F000:FFF0` for FDD and HDD boot selection before clean
teardown. The inputs revalidated as 1,474,560 bytes with SHA-256
`fadeb3a27c6a0e1cf582dde0b9aecb7e5d30678f2f967f2f4562f167cc0cb1d5` and
51,609,600 bytes with SHA-256
`f4d1e81bc410bb9a7558667b7c3741a9664e84077a3774e73104cd24b631d688`.

The retained bootable NXVM Console task artifact is
`build/output/nxvm-m5_t5.exe`, SHA-256
`2c75c244f6ecca927e8f17bf0be9b7597c9e2f433e741cf70669fde3547ab942`.
It emitted banner `0.4.015d.m5t5`; piped `exit` returned zero.
