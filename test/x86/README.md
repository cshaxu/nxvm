# Shared x86 tests

This package selects C11 without extensions in standalone and embedded builds.
GNU/Clang builds enable -Wall -Wextra -Wpedantic -Werror in this package only.

Keep the six directories `src/lib`, `src/common`, `src/x86`, `test/lib`,
`test/common`, `test/x86` and shared `test/register.cmake` together.
Each test package has its own CMake entry
and manifest; no importing-product sources, configuration, firmware or images
are required. Lib and Common can instead use their four-directory neutral set.

```text
cmake -S test/x86 -B build/x86-tests -DCMAKE_BUILD_TYPE=Release
cmake --build build/x86-tests
ctest --test-dir build/x86-tests --output-on-failure
```

Coverage: DOS/X command transcripts, full register values, real/linear memory
boundaries and failures, assembly/disassembly bytes, source manifest/DAG and
negative probes. On Windows, `debug_machine` also exercises the real Common
executor/paused lease through the neutral `test/common/machine_fixture` and an
x86-owned fake protocol. It preserves the original register and CLI assertions
without duplicating the fixture's thread loop or neutral lifecycle scenarios.

`core/neutral_link.c` exercises the actual production Core without a board or
App library: construction rejection, memory aliases, routes, firmware mapping,
CPU execution, time, debug observation and reset/stop. The standalone test entry
builds and runs it together with chip contracts. Shared board contract tests
live under ibmpc-common/ibmpc-at/ibmpc-xt and build without an App. Actual
machine-profile, firmware and external-media scenarios remain product-owned.

`chips/pit825x` covers 8253 read-back exclusion, 8254 register read-back,
all six counter modes and aliases, GATE transitions, binary/BCD zero loads,
rewrite boundaries, deadlines, instance isolation and output release at
reset/destroy. Tests use only the opaque public chip API. Shared board port,
clock-ratio and PIC/refresh/speaker contracts have ibmpc-* receivers; specific
machine wiring and firmware composition remain receiving-product tests.
The same entry with `-DX86_BUILD_TOOLS=OFF` builds the chip suite independently
of Common, Debug and all product sources.

`chips/rtc146818` retains calendar/SET/12-hour/BCD, phase/reset, divider,
SQW and alarm-deadline cases. Contract tests exercise IRQ release, invalid
selectors, no-event results and all 256 month inputs in binary and BCD through
both deadline preview and actual midnight advancement. Invalid dates are a
memory-safety check, not a claim of undocumented hardware behavior.

All x86 tests are headless. Native-thread coverage is Windows-only; the other
tests remain portable. Assertions are enabled in Release builds. This suite
does not imply emulator, firmware, or desktop-interaction qualification.
