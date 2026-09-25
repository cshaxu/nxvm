# Shared Common tests

This package selects C11 without extensions in standalone and embedded builds.
GNU/Clang builds enable -Wall -Wextra -Wpedantic -Werror in this package only.

This neutral suite needs only sibling `src/common` and `src/lib` corpora.
The four-directory set `src/lib`, `src/common`, `test/lib`, `test/common`
plus `test/register.cmake` builds and tests without any x86 or importing-product
files. The shared helper only registers tests; each package selects its own suite.

```text
cmake -S test/common -B build/common-tests -DCMAKE_BUILD_TYPE=Release
cmake --build build/common-tests
ctest --test-dir build/common-tests --output-on-failure
```

Coverage: session FIFO/wake, derived presentation/state matrix, input admission
and retirement, source-local physical-key identity, machine input FIFO,
machine/executor/debug byte-transport lease contracts with a fake driver,
and independent source manifest/DAG negative probes. Common sync
tests use Base; the existing native-thread machine fake is Windows-only and
does not imply Linux execution coverage. Production Common stays platform-free.

`machine_fixture.h` and `machine_fixture.c` own that neutral fake driver and its
native-thread test resources. Common tests inject a token protocol; the optional
`test/x86` suite reuses the same fixture with an x86 protocol. No architecture
commands or registers belong to this fixture. The scripted `machine_wait` test
separately covers failure interleavings without scheduling or desktop input.

CLI bindings, original machine execution, cooked/raw product presentation,
firmware and media integration remain in the importing product's test suite.
They are not copied or weakened to turn them into Common unit tests.
