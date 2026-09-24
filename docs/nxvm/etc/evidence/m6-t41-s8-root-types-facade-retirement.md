# M6 T41 S8 Root Types Facade Retirement

## Result

NXVM no longer has `src/type.h`, `src/type.c`, or the `type-facade` CMake target. Production and tests use Lib fixed-width/status/atomic APIs, direct C `void`/text ABI spellings, and NXVM-owned device/decoder helpers. The EGA allocation-failure regression now injects an explicit VADP callback; production remains on `lib_allocate_zero`.

## Proof

- Static source/test/CMake sweep for root includes, aliases, runtime forwarding, and build edges: zero matches.
- `verify-global-fixed-width-vocabulary`, dependency DAG, and all affected static construction gates pass.
- Complete repository test suites: x64 336/336, x86 336/336; each includes 279 NXVM unit cases.
- Current optimized x64/x86 NXVM product binaries rebuilt and deployed beside the profile INI.

## Boundary

Owner-local INI files were preserved and are not part of this delivery.

## Four-profile delivery correction

| Artifact | PE machine | SHA-256 |
| --- | --- | --- |
| nxvm_model40_0_5_0535_x64.exe | 0x8664 | C30F2A5DD56918D9B92D14BCF4DA2E4BE73A149A715D890096A6ED37ECF5396C |
| nxvm_model40_0_5_0535_x86.exe | 0x014C | 6A60AA6EF2AA87DE114E2CF682DAA79C64587F5437B800E79AF7010A29BECB2E |
| nxvm_default_0_5_0535_x64.exe | 0x8664 | 05C340E16EB509284098DE6A0B6345F2ECAD1C1B69238E2A1F5E1B0EE8D75FCA |
| nxvm_default_0_5_0535_x86.exe | 0x014C | 72588B765B0464F9FC75FBBF24D91A80CD125779C16BFAF4EB27622828E2BE1B |
| nxvm_xt_0_5_0535_x64.exe | 0x8664 | ABCE5D90D70482F5DD3E5A7EB20BA365DE2353E4151C004EEC12ECE120FFDC81 |
| nxvm_xt_0_5_0535_x86.exe | 0x014C | D0DD1256065C16226537612E626FF1B087352210BA64E091A640D11320AA9503 |
| nxvm_at_0_5_0535_x64.exe | 0x8664 | 4C42265E8AE7D34203AD7D5DA000DF747ECC8DE596F789FD60B8A1189E6EAAEE |
| nxvm_at_0_5_0535_x86.exe | 0x014C | 186D0148B132E82F747368FC97400B3C240F41E6C11155438E920800CA40DCB6 |

