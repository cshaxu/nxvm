# Shared x86 tests

This package selects C11 without extensions in standalone and embedded builds.
GNU/Clang builds enable -Wall -Wextra -Wpedantic -Werror in this package only.

This package requires only `src/lib`, `src/common`, `src/x86`, `test/x86`
and the shared CMake tools directly in `test/`. It does not need test/lib,
test/common, test/ibmpc or any App. Each test package has its own CMake entry
and manifest; no importing-product sources, configuration, firmware or images
are required. Lib and Common can instead use their four-directory neutral set.

```text
cmake -S test/x86 -B build/x86-tests -DCMAKE_BUILD_TYPE=Release
cmake --build build/x86-tests
ctest --test-dir build/x86-tests --output-on-failure
```

Coverage: DOS/X command transcripts, full register values, real/linear memory
boundaries and failures, assembly/disassembly bytes, source manifest/DAG and
negative probes; neutral Core time publication, route rollback/ownership,
timeline order/capacity/cancellation, entry preload transactions, CPU decoder
inventory and controller configuration/serial delivery. On Windows, `debug_machine` also exercises the real Common
executor/paused lease through a minimal x86-owned injected driver and protocol.
No fixture or CMake target comes from test/common or test/ibmpc.

Independent chip and Core tests are imported from NXVM with existing Types
stdio aliases in place of direct CRT names; assertions and expected output are
unchanged. PC-composition tests and their transitive fixtures are deliberately
excluded, not rewritten with copied board logic. The frozen upstream path
dispositions are recorded in the T85 S6 proposal. This package alone configures
and runs with the three production roots and shared test tools, without other
test packages or IBM PC. Lib/Common likewise need only their inward sources.

All x86 tests are headless. Native-thread coverage is Windows-only; the other
tests remain portable. Assertions are enabled in Release builds. This suite
does not imply emulator, firmware, or desktop-interaction qualification.
