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
